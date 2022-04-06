#include "packet.h"
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>


int ReadRawPack(STRU_OP_PACK& op)
{
    if (op.recved_data_size >= op.recv_buf_max) {
        op.status = enum_buf_full_error;
        return 0;
    }

    int recv_size = recv(op.sockfd, op.recv_buf + op.recved_data_size, op.recv_buf_max - op.recved_data_size, 0);
    if (recv_size == -1) {
        if (errno == EINTR || errno == EAGAIN) {
            return 0;
        }
        op.status = -errno;
        return -1;
    }
    else if (recv_size == 0) {
        op.status = enum_disconnected;
        return -1;
    }

    op.recved_data_size += recv_size;
    char* p = op.recv_buf;

    STRU_PACK_HEAD* pack = nullptr;
    uint32_t pack_size = 0;

    while (op.recved_data_size >= sizeof(STRU_PACK_HEAD))
    {
        pack = (STRU_PACK_HEAD*)p;
        pack_size = pack->pack_size;
       
	if (pack_size <= 0) {
            return -2;
	}

        if (pack_size > DEF_PACK_MAX_SIZE) {
            break;
        }

        if (pack_size > op.recved_data_size) {
            break;
        }

        if (op._CallOnRecvPack != nullptr)
            op._CallOnRecvPack(op.sockfd, pack);

        p += pack_size;
        op.recved_data_size -= pack_size;
    }

    if (op.recved_data_size > 0 && p) {
        memmove(op.recv_buf, p, op.recved_data_size);
	op.recved_data_size = 0;
    }
    
    return 0;
}

int SendRawData(int sockfd, char* buf, int data_len, int& status)
{
    int i = 0;
    while (i < data_len)
    {
        int size = send(sockfd, buf + i, data_len - i, 0);
        if (size > 0) {
            i += size;
        }
        else if (size == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100);
                continue;
		status = -errno;
            }
            return -1;
        }
        else if (size == 0) {
            status = enum_disconnected;		
            return -1;
        }
    }
    return 0;
}




uint64_t GetMsTime()
{
    struct timeval val;
    gettimeofday(&val, nullptr);
    return val.tv_sec * 1000 + val.tv_usec / 1000;
}

