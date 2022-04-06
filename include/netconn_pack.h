#ifndef __NETCONN_PACK_H__
#define __NETCONN_PACK_H__

#include "winsize.h"
#include "bits.h"


class STRU_PACK_HEAD;

class NetConnPack
{
public:
    NetConnPack();
    ~NetConnPack();
    uint32_t SetWinParam(WIN_SIZE_T begin, WIN_SIZE_T size, WIN_SIZE_T drop_num);

    int OnRecvPack(int sockfd, STRU_PACK_HEAD* pack);
private:
    WinSize<WIN_SIZE_T, 0xFFFF> _win_size;
    BitsFile _bitsfile;
};




#endif

