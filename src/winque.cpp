#include "winque.h"
#include <string.h>


WinQue::WinQue():_arr_val(0)
{

}

WinQue::~WinQue()
{
    if(_arr_val) {
       delete _arr_val;
       _arr_val = nullptr;
    }
}

void WinQue::Init(int limit)
{
    _max_limit = limit;
    _arr_val = new int[limit];
    memset(_arr_val, -1, limit * sizeof(int));
}

bool WinQue::CheckExists(int val)
{
    for (int i = _pos; i < _end; ++i) {
        if (_arr_val[i] == val) 
	   return true;	
    }	    

    Push(val);
    return false;
}

void WinQue::Push(int val)
{
    if (_data_count < _max_limit) {
        int i = _end % _max_limit;
        _arr_val[i] = val;

        _end++;
        _data_count++;
    }
    else {
        _pos = ++_pos % _max_limit;
        int i = _end % _max_limit;
        _arr_val[i] = val;
        _end++;
    }
}


