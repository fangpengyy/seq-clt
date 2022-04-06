#ifndef __OBJ_POOL_H__
#define __OBJ_POOL_H__

#include "slink.h"


class BaseObjPool
{

};

template <typename T>
class ObjPool
{
public:
    static ObjPool& Instance();	
    ObjPool();
    ~ObjPool();
    int Init(uint8_t obj_type, uint32_t obj_count);
    void UnInit();

    T* GetObj();
    int FreeObj(T* obj);
private:
    char* _pBuf{nullptr};
    SLink _slink;
    uint8_t _obj_type;
};

#if 0

class ObjPoolMng
{
public:

 ObjPool<T>

};

#endif

#include "../src/objpool.cpp"

#endif
