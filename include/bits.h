#ifndef __BITS_H__
#define __BITS_H__

#include <stdint.h>
#include <stdio.h>


struct STRU_FILE_NODE
{
    uint32_t begin;
    uint32_t end;
    uint32_t version{1};
    uint32_t file_size;
    uint32_t win_size;
    uint32_t limit_max_num;
    uint32_t buf_len;
    char buf[0];
};



class BitsFile
{
public:
    BitsFile();
    ~BitsFile();
    int Open(const char* file, uint32_t limit_max_num, uint32_t win_size, uint32_t& begin);
    void Close();
    
    uint8_t* GetBuf(){return _pBuf;}
private:
    uint8_t* _pBuf{nullptr};
    int _fd;
    uint32_t _file_size;
};


class Bits
{
public:
    Bits();
    ~Bits();
    int Open(const char* filename, uint32_t limit_max_num, uint32_t win_size, uint32_t& begin);
    void Close();
    
    uint8_t IsFlag(uint32_t pos);
    void SetFlag(uint32_t pos, uint8_t flag);

    void SetRange(uint32_t begin, uint32_t end);
private:
    uint8_t* _pflag{nullptr};
    uint8_t* _arr{nullptr};
    uint32_t _nbyte;
    BitsFile _bitsfile;

    STRU_FILE_NODE* _cur_node{nullptr};
};





#endif
