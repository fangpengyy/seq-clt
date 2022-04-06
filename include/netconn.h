#ifndef __NET_CONN__H__
#define __NET_CONN__H__

#include "packet.h"
#include "synfile.h"
#include "slink.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <atomic>
#include <functional>
#include <arpa/inet.h>


class NetConn
{
public:
    NetConn();
    ~NetConn();

    void Init(int id, TOnRecvPack onRecvPack, StruParam& param);
    bool ConnServer(StruSocket& struSocket);
    void Close();

    bool ReConnect();

    void PutPack(STRU_PACK_HEAD* pack);
  
    //读取网络数据   
    int Read();
    int Send();

    int GetId(){return _id;} 
    int GetSockFd(){return _op_pack.sockfd;} 
    int GetConnStatus(){return _status;}

    void SetConnStatus(int status){_status = status;};
    int SendHeart(); 
private:
     int OnRecvPack(int sockfd, STRU_PACK_HEAD* pack);
     int SendPack(STRU_PACK_HEAD* pack);
private:
    char* _recv_buf;
    int _id{0};
    int _status{1};
    TOnRecvPack _OnRecvPack{nullptr};
    SLink _slink;
    StruSocket _struSocket;

    STRU_OP_PACK _op_pack;
    SynFile _synfile;
    std::atomic<uint32_t> _seq_id{0};
};

#endif

