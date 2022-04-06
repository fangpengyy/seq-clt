#ifndef  __NETCLIENT_H__
#define  __NETCLIENT_H__

#include "packet.h"

#include <thread>
#include <atomic>
#include <functional>
#include <map>


class NetConn;

class NetClient
{
public:
    NetClient();
    ~NetClient();

    int Init(StruParam& param);
    void Stop();

    int Attach(NetConn* pNetConn);
    int Detach(NetConn* pNetConn);
 
private:
    void OnClientRead();
    void Clean();
private:
    int _stopEventFd;
    int _epollFd;
    std::thread* _pRthread{nullptr};

    std::atomic<bool> _start;
    StruParam _param;
};




#endif
