#include "acceptor.h"
#include "socketfun.h"
#include "accept_client.h"

#include <strings.h>
#include <errno.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <sys/eventfd.h>



#define MAX_EVENT_NUM       1024


//Acceptor------

Acceptor::Acceptor():
     _epollFd(-1),
     _pEvents(nullptr),
     _start(true),
     _listenFd(-1)
{

}

Acceptor::~Acceptor()
{
          
}

void Acceptor::Init()
{
}

void Acceptor::UnInit()
{
    _start =false;

    if (_stopEventFd > -1) {
        uint64_t liSend = 1;
        int ret = write(_stopEventFd, &liSend, sizeof(liSend));
    }
}

void Acceptor::Wait()
{
    pthread_join(_th, nullptr);
}

void Acceptor::Cleanup()
{
    if (_epollFd > 0) {
        close(_epollFd);
        _epollFd = -1;
    }

    if (_listenFd > 0) {
        close(_listenFd);
        _listenFd = -1;
    }

    if (_pEvents) {
        delete[] _pEvents;
        _pEvents = nullptr;
    }

    for (auto it = _map_client.begin(); it != _map_client.end(); it++) {
        it->second->Close(); 
    }
    _map_client.clear();
}

int Acceptor::Listen(const char* ip, int port)
{
    bool error = false; 	
    do {
        struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	if (ip && ip[0] != '\0')
	    addr.sin_addr.s_addr = inet_addr(ip);	
	else {	
	    addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	addr.sin_port = htons(port);

        _epollFd = epoll_create(MAX_EVENT_NUM);
        if (unlikely(_epollFd  < 0)) {
	    return -1;
        }
    
        _pEvents = new struct epoll_event[MAX_EVENT_NUM];
        if (unlikely(_pEvents == nullptr)) {
            error = true;
	    break;
        }

        _listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (unlikely(_listenFd < 0)) {
	    error = true;
	    break;    
        }

        int val = 1;
        if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &val, static_cast<socklen_t>(sizeof val)) == -1) {
	    error = true;
	    break;    
        }

        int ret = bind(_listenFd, (struct sockaddr*)&addr, static_cast<socklen_t>(sizeof addr));
        if (unlikely(ret < 0)) {
	   error = true;
	   break;
        }

 	if (unlikely(AddToEpoll(_epollFd, _listenFd) == -1)) {
            error = true;
	    break;
        }

        _stopEventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if(_stopEventFd == -1) {
            error = true;
            break;
        }
        
        AddToEpoll(_epollFd, _stopEventFd);
	
	ret = listen(_listenFd, 1024);
        if (unlikely(ret < 0)) {
            error = true;
            break;
        }
   } while (0);
   
   if (error) {
       Cleanup();
       return -1;  
   }

   int ret = pthread_create(&_th, nullptr, OnProc, this);
   if (ret != 0) {
       Cleanup();	   
       return -1;
   }
    
   return 0;
}

int Acceptor::AddToEpoll(int epollFd, int sockFd)
{
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = EPOLLIN;
    event.data.fd = sockFd;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, sockFd, &event) == -1) {
       return -1;
    }
    return 0;
}

void* Acceptor::OnProc(void* param)
{
    Acceptor* pAcceptor = (Acceptor*)param;
    pAcceptor->DoWork();
    return 0;
}

void Acceptor::DoWork()
{
    struct sockaddr_in cltAddr;
    socklen_t addrLen = static_cast<socklen_t>(sizeof cltAddr);

    int timePos = GetMsTime();
    int k = 0;
    int fd = 0;
    Socket::MaskPipe();

    while (_start)
    {
        int ret = epoll_wait(_epollFd, _pEvents, MAX_EVENT_NUM, 1000);
        if (ret == 0) {
	    continue;
	}

       	if (ret == -1)
       	{
            if (++k > 10 && GetMsTime() - timePos < 1000) {
                sleep(2);
                k = 0;
                timePos = GetMsTime();
            }
            else
                usleep(100);
            continue;
        }
        
	k = 0;
        for (int i = 0; i < ret; ++i)
	{	
	    fd = _pEvents[i].data.fd;	
	    if (fd == _stopEventFd) {
                break;
            }

	    if (fd == _listenFd) {
		int connFd = accept(_listenFd, (struct sockaddr*)&cltAddr, &addrLen);     
	        if (connFd > 0) {
		    Socket::SetNonBlock(connFd);
	            AddToEpoll(_epollFd, connFd);

		    AcceptClt* client = new AcceptClt();
		    client->Init(connFd);
		    _map_client.emplace(connFd, client);
		}
 	    }
            else {
		if (_pEvents[i].events & EPOLLIN) {
		    auto it = _map_client.find(fd);
		    if (it != _map_client.end()) {
                        if (0 != ReadRawPack(it->second->_op_pack)) {
                            it->second->Close();
			    _map_client.erase(it);
		        }
		    }
	     	}
	    }
        }
    }

    Cleanup();
}

