#include "synfile.h"
#include <time.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>



#define gettid()  syscall(__NR_gettid)

//---SYNFILE------

SynFile::SynFile()
{
    _file = NULL;
    _iCurSize = 0;
}

SynFile::~SynFile()
{
    Close();
}

int SynFile::Open(const char* szPath, const char* szFilePreFix, uint32_t aiFileSize, uint32_t fileMaxNum)
{
    if (szPath == NULL) {
        std::cout<<__func__<<" szPath empty error\n";
	return -1;
    }

    _sPath = szPath;
    if(szPath[_sPath.size() - 1] != '/')
       _sPath += "/";
    
    Createdir(_sPath.c_str());

    _sFilePreFix = szFilePreFix;
    _iFileSize = aiFileSize;
    _fileMaxNum = fileMaxNum;
    _index = -1;
    _thid = gettid();

    return Open();
}

int SynFile::Open()
{
    Close();

    if (++ _index >= _fileMaxNum) {
        _index = 0;
    }
    char filename[256];
    sprintf(filename, "%s%s-t%d-%d.log", _sPath.c_str(), _sFilePreFix.c_str(), _thid, _index);
	
    _file = fopen(filename, "wb+");
    if (_file == NULL)
    {
        std::cout<<__func__<<" file="<<filename<<" error="<<strerror(errno)<<"\n"; 
        return -1;
    }
  
    _iCurSize = 0;
    return 0;
}

void SynFile::Close()
{
    if(_file)
    { 
        fflush(_file);
        fclose(_file);
        _file = NULL;
    }
}

int SynFile::CheckFile()
{
    if(_iCurSize >= _iFileSize) {
        _iCurSize = 0;
        return Open();
    }
    return 0;
}

void SynFile::WLogTime(uint64_t usTime, int line, const char* asFuncName, const char* asFormat, ...)
{
    if (CheckFile() != 0) {
        return;
    }
	
    va_list alist;
    va_start(alist, asFormat);
    char* lpBuf = _pBuf;

    //time + func name  + line
    int liSize = sprintf(lpBuf, "ustime:%ld %s line:%d ", usTime, asFuncName, line);
    liSize += vsnprintf(lpBuf + liSize, sizeof(_pBuf) - liSize,  asFormat, alist);
    va_end(alist);

    *(lpBuf + liSize) = '\n';
    _iCurSize += liSize + 1;

    fwrite(lpBuf, sizeof(char), liSize + 1,  _file);
}

void SynFile::WLog(int line, const char* asFuncName, const char* asFormat, ...)
{
    if (CheckFile() != 0) {
        return;
    }
	
    va_list alist;
    va_start(alist, asFormat);

    char* lpBuf = _pBuf;
    //time + func name  + line
   
    uint64_t timestamp = GetUsTime();
    int liSize = sprintf(lpBuf, "ustime:%ld %s line:%d ", timestamp, asFuncName, line);
    liSize += vsnprintf(lpBuf + liSize, sizeof(_pBuf) - liSize,  asFormat, alist);
    va_end(alist);
    
    *(lpBuf + liSize) = '\n';
    _iCurSize += liSize + 1;

    fwrite(lpBuf, sizeof(char), liSize + 1,  _file);
}

uint64_t SynFile::GetMsTime()
{
    struct timeval val;
    gettimeofday(&val, nullptr);
    return val.tv_sec * 1000 + val.tv_usec / 1000;
}

uint64_t SynFile::GetUsTime()
{
    struct timeval val;
    gettimeofday(&val, nullptr);
    return val.tv_sec * 1000000 + val.tv_usec;
}

void SynFile::Createdir(const char* szPath)
{
    if (access(szPath, 0) != 0) {
        if (mkdir(szPath, 664) != 0) 
            printf("%s create path=%s error=%s\n", __func__, szPath, strerror(errno));
    }
}

