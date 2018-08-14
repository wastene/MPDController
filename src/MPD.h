#ifndef __MPD_h
#define __MPD_h

#include "MPDConnection.h"
#include <string>
#include <list>

using namespace std;

typedef struct KeyValue_T {
    string key;
    string value;
} KeyValue;

typedef struct KeyValues_T {
    KeyValue *values;
    int size = 0;
} KeyValues;

class MPD
{
private:
    MPDConnection *mpd;

public:
    MPD();
    MPD(const char* hostname, const int port);

    int sendCommand(const char* command);
    string receive();

    static list<KeyValue> convertKeyValues(string values);
};

#endif