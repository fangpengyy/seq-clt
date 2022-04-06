#ifndef __NET_PACK_H__
#define __NET_PACK_H__

#include <unistd.h>
#include <string>
#include <functional>


#define DEF_RECV_BUF_MAX   (1024*1024*10)

#define DEF_PACK_MAX_SIZE  (1024*16)


#define unlikely(x) __builtin_expect(!!(x), 0)
#define likely(x) __builtin_expect(!!(x), 1)


struct STRU_PACK_HEAD;
typedef std::function<int (int sockfd, STRU_PACK_HEAD*)> TOnRecvPack;


enum CMD_TYPE: uint8_t{
	enum_ack_begin,
	enum_heart,
        enum_data, 

}; 

enum EnumCallFunctionError {
        enum_notfound_class_error = -100,
        enum_strorderid_largelen_error = -101,
        enum_invalid_qty = -102,
        enum_invalid_price =-103,
        enum_invalid_symbolid = -104,
        enum_invalid_side = -105,
        enum_invalid_order_type = -106,
        enum_invalid_trigger_price =-107,
        enum_strorderid_encode_error =-108,

        enum_non_logined_error = -300,
        enum_buf_full_error = -400,
        enum_error=-1,
};


enum EnumConnStatus {
        enum_invalid_connid=-2,
        enum_notfound_connid=-1,
        enum_disconnected=0,
        enum_connecting=1,
        enum_connected=2,
        enum_logining=3,
        enum_logined=4, //成功登陆，收发数据
};


struct StruSocket
{
    std::string host;
    int port;
    int sndTimeout;
    int sys_recv_buf_size;
    int sys_send_buf_size;
    bool nagle;
};

struct STRU_OP_PACK
{
    int sockfd;
    char* recv_buf;
    uint32_t recv_buf_max;
    uint32_t recved_data_size;
   
    TOnRecvPack _CallOnRecvPack{nullptr};
    int status;
};

struct StruParam
{
    std::string app_path;
    int fileId;
};

#pragma pack(1)

struct STRU_PACK_HEAD
{
    uint8_t ver;
    uint8_t head_size;
    uint16_t cmd_type;
    uint32_t seq;
    uint32_t ack;
    uint32_t pack_size;
};

#pragma pack()


extern int ReadRawPack(STRU_OP_PACK& op);
extern int SendRawData(int sockfd, char* buf, int data_len, int& status);

extern uint64_t GetMsTime();



#endif
