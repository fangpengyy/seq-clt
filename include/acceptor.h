#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <sys/epoll.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <atomic>
#include <map>

#include "packet.h"


class AcceptClt;

//Acceptor------
class Acceptor
{
public:
    Acceptor();
    ~Acceptor();

    void Init();
    void UnInit();

    int Listen(const char* ip, int port);
    void Wait();
private:
    static void* OnProc(void* param);
    void DoWork();
    int AddToEpoll(int epollFd, int sockFd);
    void Cleanup();
private:
    int _epollFd;
    struct epoll_event* _pEvents;
    bool _start;
    int _listenFd{-1};
    int _stopEventFd{-1};

    pthread_t _th;
    std::map<int, AcceptClt*> _map_client;

};

#endif

