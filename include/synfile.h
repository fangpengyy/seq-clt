#ifndef __SYNC_FILE_H__
#define __SYNC_FILE_H__

#include <thread>
#include <string>
#include <stdio.h>


class SynFile
{
public:
    SynFile();
    ~SynFile();
    int Open(const char* szPath, const char* szFilePreFix, uint32_t aiFileSize, uint32_t fileMaxNum);
    void Close();

    void WLogTime(uint64_t secTime, int aiLine, const char* asFuncName, const char* asFormat, ...);
    void WLog(int aiLine, const char* asFuncName, const char* asFormat, ...);
private:   
    int Open();
    int CheckFile();
    uint64_t GetMsTime();
    uint64_t GetUsTime();
    void Createdir(const char* szPath);

private:
    std::string _sPath;
    std::string _sFilePreFix;
    uint32_t _iFileSize;

    uint32_t _fileMaxNum;
    int32_t _index;
    int _thid;
    uint32_t _iCurSize;

    FILE* _file;
    char _pBuf[8192];
};

#define SYNLOG_TIME(synfile, usTime, strFormat, ...) \
       synfile.WLogTime(usTime, __LINE__, __FUNCTION__, strFormat, ##__VA_ARGS__);

#define SYNLOG(synfile, strFormat, ...) \
       synfile.WLog( __LINE__, __FUNCTION__, strFormat, ##__VA_ARGS__);

#endif
