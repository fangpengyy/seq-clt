#include "netconn_pack.h"
#include "packet.h"
#include "winsize.h"

#include <stdio.h>



NetConnPack::NetConnPack()
{

}

NetConnPack::~NetConnPack()
{

}

uint32_t NetConnPack::SetWinParam(WIN_SIZE_T begin, WIN_SIZE_T size, WIN_SIZE_T drop_num)
{
    int ret = _win_size.SetWinParam("./bits.bin", begin, size, drop_num);
    printf("SetWinParam error=%d begin=%d end=%d\n", ret, _win_size.Begin(), _win_size.End());
  
    if (ret != 0)
       return ret;	    

    return _win_size.Begin();
}

int NetConnPack::OnRecvPack(int sockfd, STRU_PACK_HEAD* pack)
{
    WIN_SIZE_T lose_ack = 0;
    int ret = _win_size.CheckAck(pack->ack, lose_ack);

    if (ret == -6) {
        printf("recv: response ack=%d lose_ack=%d, error=%d\n",
		       	pack->ack, lose_ack, ret);
    }
    else
        printf("recv: response ack=%d ret=%d\n", pack->ack, ret);

    return 0;
}

