//
//  Logger.h
//  tankgame
//
//

#pragma once

#include <iostream>
#include <fstream>

class Logger
{

private:
    const char *logFilename = "applog.txt";

protected:
    Logger();

    std::ofstream appLog;

    bool LoadStrings();

public:
    static Logger &Get();

    bool Init();

    void Write(const char *msg, ...);
};
