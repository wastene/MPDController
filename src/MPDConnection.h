#ifndef __MPDConnection_h
#define __MPDConnection_h

#include <string>

using namespace std;

class MPDConnection
{
private:
    char hostname[64];
    unsigned int port;

    int socketFD = 0;

public:
    MPDConnection();
    MPDConnection(const char* hostname, unsigned int port);

    int connect();
    int disconnect();

    // Send a zero terminated Message
    int send(string message);

    // Receive a zero terminated Message
    string receive(int size);
};

#endif