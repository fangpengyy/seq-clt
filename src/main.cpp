#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdint.h>

#if 1

#include "winque.h"

#include "acceptor.h"
#include "netconn_pack.h"
#include "objpool.h"
#endif


#include "packet.h"
#include "netconn.h"
#include "netclient.h"


int main(int argc, char** argv)
{ 
    uint16_t j = 65535;
    uint16_t l = j +1;
    uint16_t k = j + 2;

    uint16_t m = 65530 -j;
    uint8_t n = 2 -30;



    printf("l %d ,k %d m=%d n=%d\n", l, k, m, n);

    if (argc < 2) {
        printf("param error\n");
    }
    
    if (strcmp(argv[1], "-s") == 0) {
        Acceptor acceptor;
        acceptor.Init();
   
   	int ret = acceptor.Listen("127.0.0.1", 21000);
        if (ret != 0) {
            printf("listen error\n");
	    return 0;
        }
        printf("server start succ\n");

        acceptor.Wait();
	printf("exit\n");
    } 
    else if (strcmp(argv[1], "-c") == 0) {

        STRU_WIN_SEQ<int> seq;
       
        printf("instance  ...\n");
        ObjPool<STRU_PACK_HEAD>& obj_pool = ObjPool<STRU_PACK_HEAD>::Instance();
	obj_pool.Init(1, 10);

        NetConn netConn;
        NetConnPack netconn_pack;
        
	StruParam param {
	    .app_path = "./",
	    .fileId = 1 
	};

        StruSocket struSocket{
            .host = "127.0.0.1",
            .port = 21000,
            .sndTimeout = 10,
            .sys_recv_buf_size = 1024*128,
      	    .sys_send_buf_size = 1024*64,
            .nagle = true
        };

        uint32_t begin = netconn_pack.SetWinParam(10, 100, 2);
	netConn.Init(1, std::bind(&NetConnPack::OnRecvPack, &netconn_pack, 
				std::placeholders::_1, std::placeholders::_2), param);	
       
        bool ret = netConn.ConnServer(struSocket);	
	if (!ret) {
            printf("conn server error\n");
	    return 0;
	}

        NetClient netClt;
	int r = netClt.Init(param);
	if (r != 0) {
            printf("NetClient Init error\n");
            return 0;
	}

	netClt.Attach(&netConn);

	char ch;
	uint32_t k = 0;
        
	STRU_PACK_HEAD* pack = obj_pool.GetObj();
        pack->ver = 1;
        pack->head_size = sizeof(STRU_PACK_HEAD);
        pack->cmd_type = enum_ack_begin;
        pack->ack = begin;
        pack->pack_size = sizeof(STRU_PACK_HEAD);

        netConn.PutPack(pack);

	while (1) {
	    sleep(1);

	    STRU_PACK_HEAD* pack = obj_pool.GetObj();
            if (pack) {
		Link* plink = (Link*)((char*)pack - sizeof(Link));

	//	printf("seq obj_type =%d\n", plink->obj_type);
#if 1
	        pack->ver = 1;
                pack->head_size = sizeof(STRU_PACK_HEAD);
                pack->cmd_type = 1;
                //pack->seq = k++;
                pack->ack ;
                pack->pack_size = sizeof(STRU_PACK_HEAD);

                netConn.PutPack(pack);
#endif
	    }
	}
    }

    return 0;	
}
