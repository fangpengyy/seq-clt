#ifndef __SLINK_H__
#define __SLINK_H__

#include <stdint.h>


#define def_init(locker) {\
    locker = 0;\
}

#define def_lock(locker) {\
    while (__sync_lock_test_and_set(&(locker),1)) {}\
}

#define def_unlock(locker) {\
    __sync_lock_release(&(locker));\
}


struct Link
{
    Link* next;
    uint8_t obj_type;
};


class SLink
{
public:
    Link head;
    Link* tail;
    int spin;
    int count;

    SLink() {
        head.next = nullptr;
        tail = &head;
        spin = 0;
        count = 0;
    }

    void PutUnlock(void * p);

    void Put(void * p);
    void* Pop();
    int PopArr(void** arr, int num);

};




#endif
