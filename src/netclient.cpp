#include "netclient.h"
#include "socketfun.h"
#include "netconn.h"

#include <sys/eventfd.h>
#include <sys/epoll.h> 
#include <unistd.h>
#include <errno.h>
#include <list>


//10 分钟
#define  DEF_TIMEOUT (1000*60*10)



NetClient::NetClient():
    _stopEventFd(-1),
    _epollFd(-1)
{

}

NetClient::~NetClient()
{

}

int NetClient::Init(StruParam& param)
{
    _start = true;
    _param = param; 

    _epollFd = epoll_create(64);
    if (_epollFd == -1)
    {
        return -1;
    }

    _pRthread = new (std::nothrow) std::thread(&NetClient::OnClientRead, this);
    if (_pRthread == nullptr) {
        return -1;
    }

    return 0;
}

void NetClient::Stop()
{
    _start = false;
    if (_pRthread) {
	_pRthread->join();
	delete _pRthread;
        _pRthread = nullptr;
    }

    if (_epollFd > 0) {
        close(_epollFd);
        _epollFd = -1;
    }
    
    Clean();
}

void NetClient::Clean()
{
    if (_stopEventFd > -1) {
        close(_stopEventFd);
        _stopEventFd = -1;
    }
}

int NetClient::Attach(NetConn* pNetConn)
{
    epoll_event event;
    bzero(&event, sizeof(event));
    event.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
    event.data.ptr = pNetConn;

    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, pNetConn->GetSockFd(), &event) == -1) {
        return -1;
    }

    return 0;
}

int NetClient::Detach(NetConn* pNetConn)
{
    epoll_event event;
    bzero(&event, sizeof(event));
    event.events = EPOLLIN | EPOLLOUT;
    event.data.ptr = pNetConn;

    epoll_ctl(_epollFd, EPOLL_CTL_DEL, pNetConn->GetSockFd(), &event);
    pNetConn->Close();

    return 0;
}

void NetClient::OnClientRead()
{
    Socket::MaskPipe();
    char name[16];
    sprintf(name, "cl-read");
    pthread_setname_np(pthread_self(), name);

    epoll_event loEvents[64];
    NetConn* lpNetConn = nullptr;
    
    char filename[32];
    snprintf(filename, sizeof(filename), "clt-read-f%d-", _param.fileId);

    SynFile synfile;
    synfile.Open(_param.app_path.c_str(), filename, 1024*1024*200, 5);

    std::list<NetConn*> lst_conn;

    while (_start)
    {  
	for (auto it = lst_conn.begin(); it != lst_conn.end(); ++it) {
            if ( (*it)->ReConnect()) {
                Attach(lpNetConn);
                it = lst_conn.erase(it);
		continue;
	    }
	}	

        int liRet = epoll_wait(_epollFd, &loEvents[0], 10, 1000);
        if (liRet == 0) {
            continue;
	}
        else if (liRet > 0) {
            for (int i = 0; i < liRet; ++i) {
	        lpNetConn = (NetConn*)loEvents[i].data.ptr;
                if (loEvents[i].events & EPOLLIN) {
                    if (lpNetConn->Read() != 0) {
                        SYNLOG(synfile, "netclient.cpp NetConn->Close read error=%s, connId=%d sockFd=%d",
				    strerror(errno), lpNetConn->GetId(), lpNetConn->GetSockFd());
                        
		        Detach(lpNetConn);
                        lst_conn.push_back(lpNetConn); 
                    }
                }
                else if (loEvents[i].events & (EPOLLERR | EPOLLHUP)) {
                     SYNLOG(synfile, "netclient.cpp NetConn->Close events error, connId=%d sockFd=%d",
			       	lpNetConn->GetId(), lpNetConn->GetSockFd());
                     Detach(lpNetConn);
                }
                
		if (loEvents[i].events & EPOLLOUT) {
                    lpNetConn->Send();               
                }
	    }
        } 
        else if (liRet == -1) {
	    SYNLOG(synfile, "netclient.cpp epoll_wait failed, err=%s", strerror(errno));	
            
	    if (errno == EINTR)
                continue;
       
	    if (EBADF == errno || EINVAL == errno) {
	       _epollFd = epoll_create(64);
               if (_epollFd == -1) {
		   SYNLOG(synfile, "netclient.cpp epoll_create failed err=%s", strerror(errno));    
		   break;       
               }
	       continue;
	    }
	    break;
        }
    }

    SYNLOG(synfile, "netclient.cpp OnRead exit");
    synfile.Close();
}

