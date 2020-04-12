#include "MPD.h"
#include <iostream>
#include <cstring>

list<string> getLines(string text);
KeyValue getPair(string line);
void printList(list<string>);

MPD::MPD()
{
    hostname = "localhost";
    port = 6600;
    password = "";

    mpd = new MPDConnection(hostname, port);
    this->connect();
    this->initialize();
}

MPD::MPD(const char* hostname, const int port)
{
    strcpy(this->hostname, hostname);
    this->port = port;
    password = "";

    mpd = new MPDConnection(hostname, port);
    this->connect();
    this->initialize();
}

MPD::MPD(const char* hostname, const int port, const char* password)
{
    this->hostname = new char[64];
    strncpy(this->hostname, hostname, 64);
    this->port = port;
    this->password = new char[64];
    strncpy(this->password, password, 64);

    mpd = new MPDConnection(hostname, port);

    this->connect();
    this->initialize();
}

int MPD::connect()
{
    mpd->connect();
}

int MPD::disconnect()
{
    mpd->disconnect();
}

void MPD::initialize()
{
    char* command = new char[128];
    strncpy(command, "password \"", 128);
    strncat(command, this->password, 128);
    strncat(command, "\"\nstatus\ncurrentsong\nidle\n", 128);
    sendCommand(command);
}

int MPD::sendCommand(const char* command)
{
    int bytes = 0;
    if(bytes = mpd->send(command) < 0)
    {
        cerr << "Failed to send Command. Try to reconnect." << endl;
        this->disconnect();
        this->connect();
        this->initialize();
        // Doesn't call this function (MPD::sendCommand). 
        // Can produce endless loop
        mpd->send(command);
        return -1;
    }
    return bytes;
}

string MPD::receive()
{
    string response = "";
    string tmp;
    do {
        tmp = mpd->receive(64);
        response += tmp;
    }while(tmp.length() > 0);

    #ifdef DEBUG
    // Print all received data
    if(response.length() > 0){
        cout << "Received: " << endl;
        cout << response << endl;
    }
    #endif

    return response;
}

list<KeyValue> MPD::receiveCommand()
{
    list<KeyValue> response;

    string received = MPD::receive();
    list<string> receivedLines = getLines(received);

    list<string> lines = buffer;
    buffer.clear();
    lines.insert(lines.end(), receivedLines.begin(), receivedLines.end());

    list<string>::iterator iter = lines.begin();

    while(iter != lines.end())
    {
        string line = *iter;

        if(line.find("OK MPD") == 0)
        {
            // Found Version String
	        iter++;
            continue;
        }
        else if(line.find("OK") == 0)
        {
            // Found OK: Response ends and put rest into buffer
            iter++; // Increment to get not the current OK into buffer
            while(iter != lines.end())
            {
                buffer.push_back((*iter));
                iter++;
            }
            break;
        }
        response.push_back(getPair(line));

        iter++;
    }
    return response;
}

list<KeyValue> MPD::convertKeyValues(string values)
{
    int pos = 0;
    string line;

    list<KeyValue> list;    
    do {
        int occurrence = values.find('\n', pos);
        if(occurrence >= values.length()){
            break;
        }
        line = values.substr(pos, occurrence-pos);

        pos = occurrence+1;
        
        occurrence = line.find(": ");
        if(occurrence >= line.length()){
            continue;
        }
        KeyValue val;
        val.key = line.substr(0, occurrence);
        val.value = line.substr(occurrence+2);

        list.push_back(val);

    }while(pos < values.length());

    return list;
}

list<string> getLines(string text)
{
    list<string> result;
    string str = text;
    size_t pos = str.find("\n");
    string token;
    while(pos != str.npos) {
        token = str.substr(0, pos);
        result.push_back(token);

        str = str.substr(pos+1);
        pos = str.find("\n");
    }
    return result;
}

KeyValue getPair(string line)
{
    KeyValue pair;
    int occurrence = line.find(": ");
    if(occurrence >= line.length()){
        pair.key = "";
        pair.value = "";
        return pair;
    }
    pair.key = line.substr(0, occurrence);
    pair.value = line.substr(occurrence+2);

    return pair;
}

void printList(list<string> lst){
    list<string>::iterator iter = lst.begin();
    while(iter != lst.end()){
        cout << (*iter) << endl;
	iter++;
    }
}
