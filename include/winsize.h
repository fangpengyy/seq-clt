#ifndef __WIN_SIZE_H__
#define __WIN_SIZE_H__

#include "bits.h"
#include <stdint.h>

#if 0
   #define DEF_WIN_SIZE  0xFF   
   typedef uint8_t  WIN_SIZE_T;   

#else
    #define DEF_WIN_SIZE  0xFFFF
    typedef uint16_t  WIN_SIZE_T;
#endif



class BitsFile;


template<typename T, WIN_SIZE_T MAX>
class WinSize
{
public:
    WinSize();
    ~WinSize();    

    int CheckAck(T pos, WIN_SIZE_T& lose_ack);
    int SetWinParam(const char* filename, T begin, T size, T limit_drop_num);

    T Begin(){return _begin;}
    T End(){return _end;}
private:	
    T _begin{0};
    T _end{0};
    T _drop_count{0};
    T _limit_drop_num{1};
    Bits _bits;
};

#include "../src/winsize.cpp"


#endif
