#include "MPDConnection.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

using namespace std;

MPDConnection::MPDConnection()
{
}

MPDConnection::MPDConnection(const char* hostname, unsigned int port)
{
    strncpy(this->hostname, hostname, 64);
    this->port = port;

    MPDConnection::connect();
}

int MPDConnection::connect()
{
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD < 0)
    {
        cerr << "Failed to create Socket." << endl;
        return socketFD;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(hostname);
    addr.sin_port = htons(port);

    if(::connect(socketFD, (struct sockaddr* )&addr, sizeof(addr)) < 0)
    {
        cerr << "Connect to " << hostname << ":" << port << " failed." << endl;
        return -1;
    }

    if(fcntl(socketFD, F_SETFL, fcntl(socketFD, F_GETFL) | O_NONBLOCK) < 0)
    {
        cerr << "Failed to put Socket in Non-Blocking Mode" << endl;
        return -1;
    }
    return 0;
}

int MPDConnection::disconnect()
{
    return ::close(socketFD);
}


int MPDConnection::send(string message)
{
    return ::send(socketFD, message.c_str(), strlen(message.c_str()), 0);
}

string MPDConnection::receive(int size)
{
    char buffer[size];
    int length = 0;
    if((length = ::recv(socketFD, &buffer, sizeof(buffer), 0)) < 0)
    {
        //cerr << "Failed to receive a Message." << endl;
        return string("",0);
    }
    return string(buffer,length);
}