#ifndef __ACCEPT_CLIENT_H__
#define __ACCEPT_CLIENT_H__

#include "packet.h"
#include "winsize.h"


class AcceptClt
{
public:
    AcceptClt();
    ~AcceptClt();
    void Init(int sockfd);
    void Close();
    
private:
    int OnRecvPack(int sockfd, STRU_PACK_HEAD* pack);
private:
    friend class Acceptor;
    STRU_OP_PACK _op_pack;
    char _recv_buf[DEF_RECV_BUF_MAX];
    WIN_SIZE_T _ack;
};



#endif
