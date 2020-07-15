//
//  Logger.h
//  tankgame
//
//

#ifndef Logger_h
#define Logger_h

#include <iostream>
#include <fstream>


class Logger
{
protected:
    Logger();
    
    std::ofstream appLog;
    
    bool LoadStrings();
    
public:
    static Logger &Get();
    
    bool Init();
    
    void Write(const char *msg, ...);
};

#endif /* Logger_h */
