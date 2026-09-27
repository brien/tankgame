//
//  Logger.h
//  tankgame
//
//

#pragma once

#include <iostream>
#include <fstream>

// Keep verbose gameplay diagnostics on native builds and browser Debug builds.
// NDEBUG is supplied by CMake's optimized configurations. A macro also avoids
// evaluating diagnostic arguments when browser release logging is disabled.
#if defined(__EMSCRIPTEN__) && defined(NDEBUG)
#define TANKGAME_LOG_DEBUG(...) do {} while (false)
#else
#define TANKGAME_LOG_DEBUG(...) Logger::Get().Write(__VA_ARGS__)
#endif

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
