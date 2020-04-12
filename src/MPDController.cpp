#include "MPD.h"
#include "LCD.h"
#include "Button.h"
#include "RotaryEncoder.h"

#include <unistd.h>
#include <string>
#include <list>
#include <iostream>

#include <chrono>
// #include <pthread.h>

using namespace std;

#define SECONDWITHOUTTICKS 1000000

#define COUNTOFTICKSFORONESECOND 100    // 1 ms (before 10ms = 100)
#define TICK SECONDWITHOUTTICKS/COUNTOFTICKSFORONESECOND 
#define ONESECOND COUNTOFTICKSFORONESECOND
#define HUNDREDMS ONESECOND/10

#define DURATIONOFDISPLAYVOLUME 3*ONESECOND
#define ROTARYENCODERTICKSTOLASTCHANGE 5*HUNDREDMS  // 200ms - (If new Change within this time interval occur, 
                                                    //the last change of the rotary encoder should be taken)
#define BUTTONPOLLINGTIMEINTERVAL 2*HUNDREDMS // 200ms - Check new state after this time interval

#define MPDPARSINGINTERVAL HUNDREDMS

#define STATE_VOLUME 1
#define STATE_TIME 2

#define DEBUG

typedef struct MPDInfo_Struct {
    string title = "";
    int volume = 0;
    int time = 0;
    bool state = false;
} MPDInfo;

typedef struct Settings_Struct {
    int volumeRemainingTicks = 0;
    int oneSecondRemainingTicks = ONESECOND; // 100 Ticks = 1 Second
    int lastStateOfLineOne = 0; // 0 Nothing, 1 Volume, 2 Time
    int elapsedTicksSinceRotaryEncoderChange = 0;
    int lastChangeOfRotaryEncoder = 0; // 0 Nothing, -1 Down, 1 Up
} Settings;

string timeToString(int time);
void sendCommandWithIdle(MPD *mpd, const char* command);
void sendCommand(MPD *mpd, const char* command);
void updateState(MPDInfo*, LCD*, Settings*);
void parseMPDData(MPD* mpd, LCD* lcd, MPDInfo* mpdInfo, Settings* settings);
void checkingRotaryEncoder(RotaryEncoder* rotaryEncoder, MPD* mpd, LCD* lcd, MPDInfo* mpdInfo, Settings* settings);
void checkingButton(Button* button, int* lastChangeTicks, MPD* mpd, const char* command);

void* oneSecondFunction();

int main()
{
    // Initialization
    int ticks = 0; // Needed for knowing how many milliseconds are gone
    int oneSecondTicks = 0;
    int mpdParsingTicks = MPDPARSINGINTERVAL;

    LCD lcd = LCD(0x27);
    if(lcd.off() < 0){
        cerr << "Cannot speak to LCD Display." << endl;
        lcd.reinit();
    }

    Input inPrev = Input(26);
    Input inNext = Input(16);
    Input inPlay = Input(13);
    Input inPause = Input(19);

    // Need to call Buttons with Input-Objects, otherwise its not working, 
    // if you initialize these Buttons with the Pin-Constructor
    Button prev = Button(&inPrev);
    Button next = Button(&inNext);
    Button play = Button(&inPlay);
    Button pause = Button(&inPause);

    int prevLastChangeTicks = -1;
    int nextLastChangeTicks = -1;
    int playLastChangeTicks = -1;
    int pauseLastChangeTicks = -1;

    Button clk = Button(20);
    Button dt = Button(21);
    RotaryEncoder rotaryEncoder = RotaryEncoder(&clk, &dt);

    MPDInfo mpdInfo;
    mpdInfo.volume = 0;
    mpdInfo.title = "";
    mpdInfo.time = 0;
    mpdInfo.state = false;

    Settings settings;
    settings.volumeRemainingTicks = 0;
    settings.oneSecondRemainingTicks = ONESECOND;
    settings.lastStateOfLineOne = 0;
    settings.elapsedTicksSinceRotaryEncoderChange = 0;
    settings.lastChangeOfRotaryEncoder = 0;

    // MPD mpd = MPD();
    // sendCommand(&mpd, "password \"MPDPlayerIsCool\"\n");
    // sendCommand(&mpd, "status\ncurrentsong\nidle\n");
    MPD mpd = MPD("127.0.0.1", 6600, "MPDPlayerIsCool");

    long long lastTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

    
    // pthread_t *thread;
    // pthread_create(thread, NULL, oneSecondFunction, NULL);

    // int pthread_create( pthread_t *thread, 
    //                 const pthread_attr_t *attribute,
    //                 void *(*funktion)(void *),
    //                 void *argumente );

    // Do Stuff
    while(1)
    {
        if(oneSecondTicks >= ONESECOND)
        {
            oneSecondTicks = 0;
            long long ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            // cout << "Time: " << ms-lastTime << endl;
            lastTime = ms;
        }


        if(settings.volumeRemainingTicks > 0)
        {
            settings.volumeRemainingTicks--;
        }
        else if(settings.volumeRemainingTicks == 1) // TODO: Are these condition even called?
        {
            if(lcd.clear(1) < 0){
                cerr << "Cannot speak to LCD Display." << endl;
                lcd.reinit();
            }
            settings.lastStateOfLineOne = STATE_VOLUME;
        }

        if(settings.oneSecondRemainingTicks > 0)
        {
            settings.oneSecondRemainingTicks--;
        }
        else
        {
            settings.oneSecondRemainingTicks = ONESECOND;

            if(settings.volumeRemainingTicks <= 0)
            {
                if(settings.lastStateOfLineOne == 1)
                {
                    if(lcd.clear(1) < 0){
                        cerr << "Cannot speak to LCD Display." << endl;
                        lcd.reinit();
                    }
                }

                mpdInfo.time++;
                string timeString = timeToString(mpdInfo.time);
                if(lcd.centralWrite(timeString.c_str(), timeString.length(), 1) < 0){
                    cerr << "Cannot speak to LCD Display." << endl;
                    lcd.reinit();
                }

                settings.lastStateOfLineOne = STATE_TIME;
            }
        }
        /**
         * Receiving Data from MPD-Server
         * 
         **/
        if(mpdParsingTicks == 0)
        {
            parseMPDData(&mpd, &lcd, &mpdInfo, &settings);
            mpdParsingTicks = MPDPARSINGINTERVAL;
        }

        /**
         * Checking Rotary Encoder (for Volume)
         **/
        checkingRotaryEncoder(&rotaryEncoder, &mpd, &lcd, &mpdInfo, &settings);

        /**
         * Check Button States (Play, Pause, Prev, Next)
         * 
         **/
        checkingButton(&play, &playLastChangeTicks, &mpd, "play\n");
        checkingButton(&pause, &pauseLastChangeTicks, &mpd, "pause 1\n"); 
        checkingButton(&prev, &prevLastChangeTicks, &mpd, "previous\n");
        checkingButton(&next, &nextLastChangeTicks, &mpd, "next\n");


        if(settings.elapsedTicksSinceRotaryEncoderChange <= ROTARYENCODERTICKSTOLASTCHANGE)
        {
            settings.elapsedTicksSinceRotaryEncoderChange++;
        }

        ticks++;
        oneSecondTicks++;
        mpdParsingTicks--;
        usleep(TICK); // Sleep
    }

}

void updateState(MPDInfo *mpdInfo, LCD *lcd, Settings *settings)
{
    if(mpdInfo->state) // Play
    {
        // Title
        if(lcd->centralWrite(mpdInfo->title.c_str(), mpdInfo->title.length(), 0) < 0){
            cerr << "Cannot speak to LCD Display." << endl;
            lcd->reinit();
        }
        #ifdef DEBUG
        cout << "Write Title: " << mpdInfo->title << endl;
        #endif

        // Time
        if(settings->volumeRemainingTicks <= 0)
        {
            string timeString = timeToString(mpdInfo->time);
            if(lcd->clear(1) < 0){
                cerr << "Cannot speak to LCD Display." << endl;
                lcd->reinit();
            }
            if(lcd->centralWrite(timeString.c_str(), timeString.length(), 1) < 0){
                cerr << "Cannot speak to LCD Display." << endl;
                lcd->reinit();
            }
            //lcd->writeLine(mpdInfo->title.c_str(), 0);
            

            settings->lastStateOfLineOne = STATE_TIME;

            #ifdef DEBUG
            cout << "Write Time: " << timeString << endl;
            #endif                    
        }
        settings->oneSecondRemainingTicks = ONESECOND;
    }
    else // Pause / Stop
    {

    }
}

string timeToString(int time)
{
    int minutes = int(time/60);
    int seconds = time-(minutes*60);

    string str = to_string(minutes) + ":";
    if(seconds < 10)
    {
        str += "0" + to_string(seconds);
    }
    else 
    {
        str += to_string(seconds);
    }
    return str;
}

void sendCommand(MPD* mpd, const char* command)
{
    int sendedBytes = mpd->sendCommand(command);
    if(sendedBytes < 0){
        cout << "Failed to send Command. Try to reconnect to MPD. " << endl;
        mpd->connect();
        mpd->sendCommand("password \"MPDPlayerIsCool\"\nstatus\ncurrentsong\nidle\n");
    }
    //cout << "Sended " << sendedBytes << " Bytes" << " from " << string(command).length() << endl; 
}

void sendCommandWithIdle(MPD *mpd, const char* command)
{
    sendCommand(mpd, "noidle\n");
    sendCommand(mpd, command);
    sendCommand(mpd, "idle\n");
}

void parseMPDData(MPD* mpd, LCD* lcd, MPDInfo* mpdInfo, Settings *settings)
{
    list<KeyValue> response = mpd->receiveCommand();
    if(response.size() > 0)
    {
        for(list<KeyValue>::iterator iter = response.begin(); iter != response.end(); iter++)
        {
            cout << "Key: " << iter->key << " Value: " << iter->value << endl;
            if(iter->key == "Title")
            {
                // Write Title to LCD
                mpdInfo->title = iter->value;
                if(mpdInfo->state)
                {
                    if(lcd->writeLine(mpdInfo->title.c_str(), 0) < 0){
                        cerr << "Cannot speak to LCD Display." << endl;
                        lcd->reinit();
                    }
                }
                #ifdef DEBUG
                cout << "Received Title: " << mpdInfo->title << endl;
                #endif 
                // TODO: Need Scrolling?
            }
            else if(iter->key == "time")
            {
                // Format of time: s:ms
                int pos = iter->value.find(":");
                string seconds = iter->value.substr(0, pos);
                mpdInfo->time = stoi(seconds);

                if(mpdInfo->state && settings->volumeRemainingTicks <= 0)
                {
                    string timeString = timeToString(mpdInfo->time);
                    
                    if(lcd->clear(1) < 0){
                        cerr << "Cannot speak to LCD Display." << endl;
                        lcd->reinit();
                    }
                    if(lcd->centralWrite(timeString.c_str(), timeString.length(), 1) < 0){
                        cerr << "Cannot speak to LCD Display." << endl;
                        lcd->reinit();
                    }
                    settings->lastStateOfLineOne = STATE_TIME;
                    
                    #ifdef DEBUG
                    cout << "Time: " << timeString << endl;
                    #endif                    
                }
                settings->oneSecondRemainingTicks = ONESECOND;
            }
            else if(iter->key == "volume")
            {
                // Write Volume to LCD and rewrite Volume after 2 Seconds again with Time
                int recvVolume = stoi(iter->value);
                if(mpdInfo->volume != recvVolume)
                {
                    mpdInfo->volume = recvVolume;

                    if(mpdInfo->state){
                        string volumeString = "Volume: "+ iter->value + "%";

                        settings->volumeRemainingTicks = DURATIONOFDISPLAYVOLUME;
                        if(lcd->centralWrite(volumeString.c_str(), volumeString.length(), 1) < 0){
                            cerr << "Cannot speak to LCD Display." << endl;
                            lcd->reinit();
                        }

                        #ifdef DEBUG
                        cout << "Volume: " << iter->value << endl;
                        #endif

                        settings->lastStateOfLineOne = STATE_VOLUME;
                    }
                }
            }
            else if(iter->key == "state")
            {
                // Turn LCD on or off
                if(iter->value == "play")
                {
                    if(mpdInfo->state == false)
                    {
                        mpdInfo->state = true;
                        lcd->reinit();
                        if(lcd->on() < 0){
                            cerr << "Cannot speak to LCD Display." << endl;
                            lcd->reinit();
                        }

                        updateState(mpdInfo, lcd, settings);
                    }          
                }
                else 
                {
                    if(mpdInfo->state)
                    {
                        mpdInfo->state = false;
                        if(lcd->off() < 0){
                            cerr << "Cannot speak to LCD Display." << endl;
                            lcd->reinit();
                        }

                        updateState(mpdInfo, lcd, settings);
                    }
                }

                #ifdef DEBUG
                cout << "State: " << iter->value << endl;
                #endif                
            }
            else if(iter->key == "changed")
            {
                if(iter->value == "mixer")
                {
                    // Volume (check for status)
                    sendCommandWithIdle(mpd, "status\n");
                }
                else if(iter->value == "player")
                {
                    // State or Time changed (check for status and currentsong)
                    sendCommandWithIdle(mpd, "status\ncurrentsong\n");
                }

                #ifdef DEBUG                    
                cout << "Changed: " << iter->value << endl;
                #endif
            }
        }
    }
}

void checkingRotaryEncoder(RotaryEncoder *rotaryEncoder, MPD* mpd, LCD* lcd, MPDInfo* mpdInfo, Settings* settings)
{
    int rotaryEncoderStatus = rotaryEncoder->check();
    if(rotaryEncoderStatus != 0)
    {    
        cout << "ElapsedTicksSinceLastRotaryEncoder Change: " << settings->elapsedTicksSinceRotaryEncoderChange 
            << " Last Change: " << settings->lastChangeOfRotaryEncoder << endl;

        if(settings->elapsedTicksSinceRotaryEncoderChange > ROTARYENCODERTICKSTOLASTCHANGE)
        { // 200ms
            settings->lastChangeOfRotaryEncoder = 0;
        }
        settings->elapsedTicksSinceRotaryEncoderChange = 0;

        if(settings->lastChangeOfRotaryEncoder != 0)
        {
            if(settings->lastChangeOfRotaryEncoder == -1)
            {
                if(mpdInfo->volume > 0)
                {
                    mpdInfo->volume--;
                    cout << "Reduced Volume (with Last Change)" << endl;
                }
            }else if(settings->lastChangeOfRotaryEncoder == 1)
            {
                if(mpdInfo->volume < 100)
                {
                    mpdInfo->volume++;
                    cout << "Increased Volume (with Last Change)" << endl;
                }
            }
        }
        else 
        {
            if(rotaryEncoderStatus == -1 && mpdInfo->volume > 0)
            {
                mpdInfo->volume--;
                settings->lastChangeOfRotaryEncoder = -1;
                cout << "Reduced Volume" << endl;
            }
            else if(rotaryEncoderStatus == 1 && mpdInfo->volume < 100)
            {
                mpdInfo->volume++;
                settings->lastChangeOfRotaryEncoder = 1;
                cout << "Increased Volume " << endl;
            }
        }

        string cmd = "setvol "+to_string(mpdInfo->volume)+"\n";
        sendCommandWithIdle(mpd, cmd.c_str());

        settings->volumeRemainingTicks = DURATIONOFDISPLAYVOLUME;

        string volumeString = "Volume: "+ to_string(mpdInfo->volume) + "%";

        if(lcd->centralWrite(volumeString.c_str(), volumeString.length(), 1) < 0){
            cerr << "Cannot speak to LCD Display." << endl;
            lcd->reinit();
        }

        settings->lastStateOfLineOne = STATE_VOLUME;
    }
}

void checkingButton(Button* button, int* lastChangeTicks, MPD* mpd, const char* command)
{
    int prevEventState = button->getEvent();
    if(prevEventState == EVENT_UP)
    {
        (*lastChangeTicks) = 0;
    }
    else if(prevEventState == EVENT_DOWN && (*lastChangeTicks) >= BUTTONPOLLINGTIMEINTERVAL)
    {
        #ifdef DEBUG
        cout << "Prev" << endl;
        #endif

        sendCommandWithIdle(mpd, command);
        (*lastChangeTicks) = -1;
    }
    else if(prevEventState != EVENT_DOWN && (*lastChangeTicks) >= 0){
        (*lastChangeTicks)++;
    }
}

void* oneSecondFunction()
{

}