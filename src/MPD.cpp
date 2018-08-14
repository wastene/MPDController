#include "MPD.h"
#include <iostream>

MPD::MPD()
{
    mpd = new MPDConnection("127.0.0.1", 6600);
}

MPD::MPD(const char* hostname, const int port)
{
    mpd = new MPDConnection(hostname, port);
}

int MPD::sendCommand(const char* command)
{
    if(mpd->send(command) < 0)
    {
        cerr << "Failed to send Command." << endl;
        return -1;
    }
    return 0;
}

string MPD::receive()
{
    string response = "";
    string tmp;
    do {
        tmp = mpd->receive(64);
        response += tmp;
    }while(tmp.length() > 0);

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