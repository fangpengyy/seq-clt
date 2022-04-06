#include "bits.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>


//BitsFile ----

BitsFile::BitsFile()
{

}

BitsFile::~BitsFile()
{

}

int BitsFile::Open(const char* file, uint32_t limit_max_num, uint32_t win_size, uint32_t& begin)
{
    uint32_t bytes = ((limit_max_num + 7) >> 3);
    _file_size = sizeof(STRU_FILE_NODE) + bytes; 

    bool exists_file = false;

    if (access(file, 0) == 0) {
        _fd = open(file, O_RDWR, 0664);
	if (_fd < 0) {
            printf("open file error, %s error=%s", file, strerror(errno));
            return -1;
        }
        exists_file = true;
    }

    if (!exists_file) {
    	_fd = open(file, O_CREAT|O_RDWR, 0664);
        if (_fd < 0) {
            printf("create file error, %s error=%s", file, strerror(errno));
            return -2;
        }

        if (ftruncate(_fd, _file_size) < 0) {
            printf("ftruncate file error, %s error=%s", file, strerror(errno));
            return -3;
        }

    }

    _pBuf = (uint8_t*)mmap(nullptr, _file_size, PROT_READ|PROT_WRITE, MAP_SHARED, _fd, 0);
    if (_pBuf == MAP_FAILED) {
        printf("mmap file error, %s error=%s", file, strerror(errno));
        return -4;
    }
    
    if (!exists_file) {
        STRU_FILE_NODE* pNode = (STRU_FILE_NODE*)_pBuf;
	pNode->version = 1;
        pNode->file_size = _file_size;
	pNode->begin = begin;
        pNode->end = begin + win_size - 1;
	pNode->win_size = win_size;
        pNode->limit_max_num = limit_max_num;
	pNode->buf_len = bytes;
        msync(_pBuf, sizeof(STRU_FILE_NODE), MS_SYNC); 
    }
    else {
        STRU_FILE_NODE* pNode = (STRU_FILE_NODE*)_pBuf;
	if (pNode->version != 1)
	    return -5; 	
        if (pNode->limit_max_num != limit_max_num || pNode->win_size != win_size)
            return -6;
        if (pNode->buf_len + sizeof(STRU_FILE_NODE) != _file_size)
            return -7; 		
        
	begin = pNode->begin;
	//exists file
	return 1;
    }

    return 0;
}

void BitsFile::Close()
{
    if (_pBuf) {	
	msync(_pBuf, _file_size, MS_SYNC);    
        munmap(_pBuf, _file_size);
        close(_fd);
        _pBuf = nullptr;
        _fd = -1;
    }
}



//Bits----------------
Bits::Bits()
{

}

Bits::~Bits()
{
    Close();
}

int Bits::Open(const char* filename, uint32_t limit_max_num, uint32_t win_size, uint32_t& begin)
{
    if (limit_max_num <= win_size)
        return -100;

    _nbyte = (limit_max_num + sizeof(uint8_t) - 1) >> 3;	
    int ret = _bitsfile.Open(filename, limit_max_num, win_size, begin);
    if (ret < 0)
	return ret;

    _arr = _bitsfile.GetBuf();
    _pflag = _arr + sizeof(STRU_FILE_NODE);
    return ret;
}

void Bits::Close()
{
    _bitsfile.Close();
}

uint8_t Bits::IsFlag(uint32_t pos)
{
    if (!_pflag)
        return 0;

    uint32_t i = pos >> 3;
    uint8_t j = pos & 7;
    if (i <= _nbyte) {
        uint8_t* p = _pflag + i;
        uint8_t v = 1 << j;
        return *p & v;
    }
    return 0;
}

void Bits::SetFlag(uint32_t pos, uint8_t flag)
{
    if (!_pflag)
        return;
	
    uint32_t i = pos >> 3;
    uint8_t j = pos & 7;
    if (i <= _nbyte) {
        uint8_t* p = _pflag + i;
        uint8_t v = 1 << j;;
        if (flag)
            *p |= v;
        else
            *p &= ~v;
	return;
    }
}

void Bits::SetRange(uint32_t begin, uint32_t end)
{
    if (!_arr)
        return;
	
    STRU_FILE_NODE* node = (STRU_FILE_NODE*)_arr;
    node->begin = begin;
    node->end = end;
}

