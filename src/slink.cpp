#include "slink.h"


void SLink::PutUnlock(void * p)
{
    Link* link = (Link*)p;
    link->next = nullptr;

    tail->next = link;
    tail = link;

    ++count;
}


void SLink::Put(void * p)
{
    if (p == nullptr)
        return;

    Link* link = (Link*)p;
    link->next = nullptr;

    def_lock(spin);
    tail->next = link;
    tail = link;

    ++count;
    def_unlock(spin);
}

void* SLink::Pop()
{
    Link* link = nullptr;
    
    def_lock(spin);
    if (count == 0) {
        def_unlock(spin);	    
        return nullptr;
    }

    link = (Link*)head.next;
    if (link) {
        head.next = link->next;
        if (head.next == nullptr)
            tail = &head;
        --count;
    }
    def_unlock(spin);
    return link;
}

int SLink::PopArr(void** arr, int num)
{
    Link* link = nullptr;

    def_lock(spin);
    if (count == 0) {
        def_unlock(spin);
        return 0;
    }

    int i = 0;
    while (i < num && (link = (Link*)head.next) ) {
        head.next = link->next;
        if (head.next == nullptr)
            tail = &head;
	arr[i] = link;
	++i;
    }
    count -= i;
    def_unlock(spin);
    return i;
}
