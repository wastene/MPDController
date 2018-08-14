#include "MPD.h"
#include "LCD.h"
#include "Button.h"
#include "RotaryEncoder.h"

#include <unistd.h>
#include <string>
#include <list>
#include <iostream>

using namespace std;

#define TICK 10000 // 10 ms
#define DISPLAYVOLUMETICKS 300

#define DEBUG

string timeToString(int time);

int main()
{
    // Initialization
    int ticks = 0; // Needed for knowing how many milliseconds are gone

    LCD lcd = LCD(0x27);

    Input inPrev = Input(26);
    Input inNext = Input(16);
    Input inPlay = Input(13);
    Input inPause = Input(19);

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

    int volume = 0;
    string title = "";
    int time = 0;
    bool state = false; // False=Off / True=On

    int volumeRemainingTicks = 0;
    int oneSecondRemainingTicks = 100; // 100 Ticks = 1 Second
    int lastStateOfLineOne = 0; // 0 Nothing, 1 Volume, 2 Time

    int buttonCheckTicks = 0;

    MPD mpd = MPD();

    mpd.sendCommand("status\ncurrentsong\nidle\n");

    // Do Stuff
    while(1)
    {
        if(volumeRemainingTicks > 0)
        {
            volumeRemainingTicks--;
        }else if(volumeRemainingTicks == 1)
        {
            lcd.clear(1);
            lastStateOfLineOne = 1;
        }

        if(oneSecondRemainingTicks > 0)
        {
            oneSecondRemainingTicks--;
        }
        else
        {
            oneSecondRemainingTicks = 100;

            if(volumeRemainingTicks <= 0)
            {
                if(lastStateOfLineOne == 1)
                {
                    lcd.clear(1);
                }

                time++;
                string timeString = timeToString(time);
                lcd.centralWrite(timeString.c_str(), timeString.length(), 1);

                lastStateOfLineOne = 2;
            }
        }


        // Check if Data is available
        string data = mpd.receive();
        if(data.length() > 0)
        {
#ifdef DEBUG            
            cout << "Data: " << data << endl; 
#endif
            list<KeyValue> response = MPD::convertKeyValues(data);
            for(list<KeyValue>::iterator iter = response.begin(); iter != response.end(); iter++)
            {
                if(iter->key == "Title")
                {
                    // Write Title to LCD
                    title = iter->value;
                    lcd.writeLine(title.c_str(), 0);
#ifdef DEBUG                    
                    cout << "Title: " << title << endl;
#endif
                    // TODO: Need Scrolling?
                }
                else if(iter->key == "time")
                {
                    // Format of time: s:ms
                    int pos = iter->value.find(":");
                    string seconds = iter->value.substr(0, pos);
                    time = stoi(seconds);

                    if(volumeRemainingTicks <= 0)
                    {
                        string timeString = timeToString(time);
                        lcd.clear(1);
                        lcd.centralWrite(timeString.c_str(), timeString.length(), 1);

                        lastStateOfLineOne = 2;
#ifdef DEBUG
                        cout << "Time: " << timeString << endl;
#endif                    
                    }

                    oneSecondRemainingTicks = 100;
                }
                else if(iter->key == "volume")
                {
                    // Write Volume to LCD and rewrite Volume after 2 Seconds again with Time
                    int recvVolume = stoi(iter->value);
                    if(volume != recvVolume)
                    {
                        volume = recvVolume;

                        string volumeString = "Volume: "+ iter->value + "%";

                        volumeRemainingTicks = DISPLAYVOLUMETICKS; // 2 Seconds (1 TICK = 10MS)
                        lcd.centralWrite(volumeString.c_str(), volumeString.length(), 1);
#ifdef DEBUG
                        cout << "Volume: " << iter->value << endl;
#endif
                        lastStateOfLineOne = 1;
                    }
                }
                else if(iter->key == "state")
                {
                    // Turn LCD on or off
                    if(iter->value == "play")
                    {
                        if(state == false)
                        {
                            state = true;
                            lcd.reinit();
                            lcd.on();
                        }          
                    }
                    else 
                    {
                        if(state)
                        {
                            state = false;
                            lcd.off();
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
                        mpd.sendCommand("noidle\nstatus\nidle\n");
                    }
                    else if(iter->value == "player")
                    {
                        // State or Time changed (check for status and currentsong)
                        mpd.sendCommand("noidle\nstatus\ncurrentsong\nidle\n");
                    }
#ifdef DEBUG                    
                    cout << "Changed: " << iter->value << endl;
#endif
                }
            }
        }

        // Check Rotary Encoder
        int rotaryEncoderStatus = rotaryEncoder.check();
        if(rotaryEncoderStatus != 0)
        {    
            if(rotaryEncoderStatus == -1 && volume > 0)
            {
                volume--;
            }
            else if(rotaryEncoderStatus == 1 && volume < 100)
            {
                volume++;
            }

            string cmd = "noidle\nsetvol "+to_string(volume)+"\nidle\n";
            mpd.sendCommand(cmd.c_str());

            volumeRemainingTicks = DISPLAYVOLUMETICKS;

            string volumeString = "Volume: "+ to_string(volume) + "%";
            lcd.centralWrite(volumeString.c_str(), volumeString.length(), 1);

            lastStateOfLineOne = 1;
        }


        int prevEventState = prev.getEvent();
        if(prevEventState == EVENT_UP)
        {
            prevLastChangeTicks = 0;
        }
        else if(prevEventState == EVENT_DOWN && prevLastChangeTicks >= 10)
        {
#ifdef DEBUG
            cout << "Prev" << endl;
#endif
            mpd.sendCommand("noidle\nprevious\nidle\n");
            prevLastChangeTicks = -1;
        }
        else if(prevEventState != EVENT_DOWN && prevLastChangeTicks >= 0){
            prevLastChangeTicks++;
        }  

        int nextEventState = next.getEvent();
        if(nextEventState == EVENT_UP)
        {
            nextLastChangeTicks = 0;
        }
        else if(nextEventState == EVENT_DOWN && nextLastChangeTicks >= 10)
        {
#ifdef DEBUG
            cout << "Next" << endl;
#endif
            mpd.sendCommand("noidle\nnext\nidle\n");
            nextLastChangeTicks = -1;
        }
        else if(nextEventState != EVENT_DOWN && nextLastChangeTicks >= 0){
            nextLastChangeTicks++;
        }  

        int playEventState = play.getEvent();
        if(playEventState == EVENT_UP)
        {
            playLastChangeTicks = 0;
        }
        else if(playEventState == EVENT_DOWN && playLastChangeTicks >= 10)
        {
#ifdef DEBUG
            cout << "Play" << endl;
#endif
            mpd.sendCommand("noidle\nplay\nidle\n");
            playLastChangeTicks = -1;
        }
        else if(playEventState != EVENT_DOWN && playLastChangeTicks >= 0){
            playLastChangeTicks++;
        }  

        int pauseEventState = pause.getEvent();
        if(pauseEventState == EVENT_UP)
        {
            pauseLastChangeTicks = 0;
        }
        else if(pauseEventState == EVENT_DOWN && pauseLastChangeTicks >= 10)
        {
#ifdef DEBUG
            cout << "Pause" << endl;
#endif
            mpd.sendCommand("noidle\npause 1\nidle\n");
            pauseLastChangeTicks = -1;
        }
        else if(pauseEventState != EVENT_DOWN && pauseLastChangeTicks >= 0){
            pauseLastChangeTicks++;
        }  

        usleep(TICK); // Sleep 10ms
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