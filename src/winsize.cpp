#include "winsize.h"
#include "bits.h"
#include <string.h>
#include <stdio.h>


template<typename T, WIN_SIZE_T MAX>
WinSize<T, MAX>::WinSize()
{

}

template<typename T, WIN_SIZE_T MAX>
WinSize<T, MAX>::~WinSize()
{

}     

template<typename T, WIN_SIZE_T MAX>
int WinSize<T, MAX>::SetWinParam(const char* filename, T begin, T size, T limit_drop_num)
{
    int ret = 0;
    uint32_t begin_num = begin;
    _limit_drop_num = limit_drop_num;

    ret = _bits.Open(filename, MAX, size, begin_num);
    if (ret == 1) {
        _begin = begin_num;	    
        _end = begin_num + size -1;
        return 0;
    }
    else if (ret == 0) {
        _begin = begin;
        _end = begin + size -1;
        return 0;
    }
    return ret;
}

template<typename T, WIN_SIZE_T MAX>
int WinSize<T, MAX>::CheckAck(T pos, WIN_SIZE_T& lose_ack)
{
    printf(" winsize recv ack=%d begin=%d end=%d\n", pos, _begin, _end);

      
    if (_begin <= pos && pos <= _end) {
        if (_bits.IsFlag(pos)) 
            return -1;
    }
    else if (_end < _begin) {
        if (_begin <= pos && pos <= MAX) {
            if (_bits.IsFlag(pos)) 
                return -2;
        }
        else if (0 <= pos && pos <= _end) {
            if (_bits.IsFlag(pos))  
                return -3;
        }
	else {
            return -4;
	}
    }
    else {
        return -5;   
    }

    _bits.SetFlag(pos, 1);

    T num = _begin;
    if (_bits.IsFlag(num) == 0) {
        //丢失计数	    
	_drop_count++;
	if (_drop_count >= _limit_drop_num) {
	    lose_ack = num;
	    return -6;
        }	    
    }
    else {
	_drop_count = 0;  
        num = _begin;

        while (_bits.IsFlag(num)) {
	    num++;	
            _begin++;
            _end++;
	    _bits.SetFlag(_end, 0);
        }
	_bits.SetRange(_begin, _end);
    }

    return 0;
}

