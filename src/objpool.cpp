#include <stdlib.h>
#include "objpool.h"



template <typename T>
ObjPool<T>& ObjPool<T>::Instance()
{
    static ObjPool<T> _obj_pool;
    return _obj_pool;
}

template <typename T>
ObjPool<T>::ObjPool()
{

}

template <typename T>
ObjPool<T>::~ObjPool()
{

}

template <typename T>
int ObjPool<T>::Init(uint8_t obj_type, uint32_t obj_count)
{
    _obj_type = obj_type;

    int obj_size =  sizeof(T) + sizeof(Link);
    _pBuf = (char*)malloc(obj_size * obj_count);

    char* p = _pBuf;

    Link* link = (Link*)p;
    link->obj_type = obj_type;
    
    _slink.PutUnlock(p);

    for (int i = 1; i < obj_count; ++i) {
	p += obj_size;
	link = (Link*)p;
	link->obj_type = obj_type;

	_slink.PutUnlock(p);
    }
    return 0;
}

template <typename T>
void ObjPool<T>::UnInit()
{
    free((void*)_pBuf);
    _pBuf = nullptr;
}

template <typename T>
T* ObjPool<T>::GetObj()
{
    Link* link = (Link*)_slink.Pop();
    if (link) {
        return (T*)((char*)link + sizeof(Link));
    }
    return nullptr;    
}

template <typename T>
int ObjPool<T>::FreeObj(T* obj)
{
    void* head = (void*) ((char*)obj - sizeof(Link));
    _slink.Put(head);
    return 0;
}

