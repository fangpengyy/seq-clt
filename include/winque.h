#ifndef __WIN_QUE_H__
#define __WIN_QUE_H__

#include <unistd.h>
#include <stdint.h>


template <typename T, int NUM=128>
struct STRU_WIN_SEQ {
    uint32_t win_size;
    uint32_t pos;     
    uint32_t arr_resp[NUM];
};


class WinQue
{
public:
    WinQue();
    ~WinQue();
    void Init(int limit);
    bool CheckExists(int val);

private:    
    inline void Push(int val);

private:
    int _max_limit{0};
    int _data_count{0};    

    int _pos{0};
    int _end{0};

    int* _arr_val;
};



#endif
