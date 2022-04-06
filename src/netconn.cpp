#include "netconn.h"
#include "socketfun.h"
#include "objpool.h"

#if 0

#include <signal.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <ctype.h>
#include <netdb.h>

#endif


NetConn::NetConn()
{
    _status = enum_disconnected;
    _op_pack.sockfd = -1;
}

NetConn::~NetConn()
{
    if (_recv_buf) {
        free(_recv_buf);
        _recv_buf = nullptr;
    }
}

void NetConn::Init(int id, TOnRecvPack onRecvPack, StruParam& param)
{
    _id = id;
    char name[32];
    snprintf(name, sizeof(name), "netconn-%d-", id);     
    _synfile.Open(param.app_path.c_str(), name, 1024*1024*200, 5);

    _recv_buf = (char*)malloc(DEF_RECV_BUF_MAX);    
    
    _OnRecvPack = onRecvPack;    
    _op_pack.recv_buf = _recv_buf;
    _op_pack.recv_buf_max = DEF_RECV_BUF_MAX;
    _op_pack.recved_data_size = 0;
    _op_pack._CallOnRecvPack = std::bind(&NetConn::OnRecvPack, this, std::placeholders::_1, std::placeholders::_2);
}

bool NetConn::ReConnect()
{
    return ConnServer(_struSocket);
}

bool NetConn::ConnServer(StruSocket& struSocket)
{
    _struSocket = struSocket;	
    Close();	
    _status = enum_connecting;	

    _op_pack.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_op_pack.sockfd == -1)
    {
	_status = enum_disconnected;    
	return false;
    }

    //second time
    Socket::SetSendTimeout(_op_pack.sockfd, 5);

    struct sockaddr_in svr_addr;
    if (Socket::Name2Addr(struSocket.host.c_str(), &svr_addr.sin_addr.s_addr) == -1)
    {
	_status = enum_disconnected;    
        return false;
    }
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(struSocket.port);

    int liRet = connect(_op_pack.sockfd, (struct sockaddr*)&svr_addr, sizeof(svr_addr));
    if (liRet == -1)
    {
        _status = enum_disconnected;    
        close(_op_pack.sockfd);
        _op_pack.sockfd = -1;
        return false;
    }
    if (struSocket.nagle)
        Socket::SetNoDelay(_op_pack.sockfd);

    Socket::SetRecvBufSize(_op_pack.sockfd, struSocket.sys_recv_buf_size);
    Socket::SetSendBufSize(_op_pack.sockfd, struSocket.sys_send_buf_size);

    _status = enum_connected;
    return true;
}

void NetConn::Close()
{
    if (_op_pack.sockfd > 0)
    {
        close(_op_pack.sockfd);
	SYNLOG(_synfile, "netconn.cpp succ, sockFd=%d", _op_pack.sockfd);
        _op_pack.sockfd = -1;
    }
    _status = enum_disconnected;
}

int NetConn::Read()
{
    return ReadRawPack(_op_pack);
}

int NetConn::Send()
{
    const int num = 5;	
    void* arr[num];
   
    ObjPool<STRU_PACK_HEAD>& obj_pool = ObjPool<STRU_PACK_HEAD>::Instance();

    int n = _slink.PopArr(arr, num);
    for (int i = 0; i < n; ++i) {    
        char* p = (char*)arr[i];
        if (p) {
 	    p += sizeof(Link);
            STRU_PACK_HEAD* pack = (STRU_PACK_HEAD*)p;

	    int ret = SendPack(pack);
#if 0
	    printf("send ver=%d seq=%d head_size=%d pack_size=%d ret=%d ----\n",
			  pack->ver, pack->seq, pack->head_size, pack->pack_size, ret);
#endif
            if (ret != 0) {
                for (int j = i; j< n; ++j)
		    _slink.Put(arr[j]);
	        return ret;	    
	    }
	    //recover pack mem
           // printf(" free seq=%d\n", pack->seq);
	    obj_pool.FreeObj(pack);            
	     
        }
    }
    return 0;
}

void NetConn::PutPack(STRU_PACK_HEAD* pack)
{
    char* p = (char*)pack;
    p -= sizeof(Link);	
    _slink.Put(p);
}

int NetConn::SendPack(STRU_PACK_HEAD* pack)
{
    if (_status != enum_connected) {
        return -1;
    }

    pack->seq = _seq_id++;
    int ret = SendRawData(_op_pack.sockfd, (char*)pack, pack->pack_size, _status);
    if (ret == -1) {
	return -1;    
    }
    return 0;
}

int NetConn::SendHeart()
{
    STRU_PACK_HEAD head;
    head.seq = _seq_id++;
    head.pack_size = sizeof(head);
    head.cmd_type = CMD_TYPE::enum_heart;

    return SendPack(&head);
}

int NetConn::OnRecvPack(int sockfd, STRU_PACK_HEAD* pack)
{
    if (_OnRecvPack)
	return _OnRecvPack(sockfd, pack);
    return -1;    
}


