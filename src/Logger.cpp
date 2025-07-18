//
//  Logger.cpp
//  tankgame
//
//

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include <cstdarg>
#include <stdio.h>
#include <iostream>
#include "Logger.h"

Logger::Logger()
{
}

Logger &Logger::Get()
{
    static Logger log;
    return log;
}

bool Logger::Init()
{
    appLog.open(logFilename);

    return true;
}

void Logger::Write(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    char szBuf[1024];
    vsprintf(szBuf, msg, args);
    appLog << szBuf;
    std::cout << szBuf;
#ifdef DEBUG
    appLog.flush();
#endif
}
