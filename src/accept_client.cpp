#include "accept_client.h"


// AcceptClt -----

AcceptClt::AcceptClt()
{

}

AcceptClt::~AcceptClt()
{

}

void AcceptClt::Init(int sockfd)
{
    _op_pack.sockfd = sockfd;
    _op_pack.recv_buf = _recv_buf;
    _op_pack.recv_buf_max = DEF_RECV_BUF_MAX;
    _op_pack.recved_data_size = 0;
    _op_pack._CallOnRecvPack = std::bind(&AcceptClt::OnRecvPack, this, std::placeholders::_1, std::placeholders::_2);
}

void AcceptClt::Close()
{
    if (_op_pack.sockfd > -1) {
        close(_op_pack.sockfd);
        _op_pack.sockfd = -1;
    }
}

int AcceptClt::OnRecvPack(int sockfd, STRU_PACK_HEAD* pack)
{
    switch (pack->cmd_type)
    {
        case enum_ack_begin:
	    _ack = pack->ack;

	    printf("enum_ack_begin %d\n", _ack);
            break;	    
    } 

    printf("recv seq %d, send ack=%d\n", pack->seq, _ack);


    int status = 0;

#if 0    
    static int n = 0;
    n++;
    
    if (n % 4 == 0) {
        pack->ack = (WIN_SIZE_T)(n - 10);
    }
    else if (n % 10 == 0) {
        pack->ack = (WIN_SIZE_T)(n * 2 + 10);
    }
    else
        pack->ack = _ack++;
#else
    pack->ack = _ack++;    
#endif

    if (0 != SendRawData(sockfd, (char*)pack, pack->pack_size, status)) {
         printf(" error send ack %d status=%d\n", pack->ack, status);	    
    }

    return 0;
}

