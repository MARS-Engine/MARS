#ifndef __DEBUG__
#define __DEBUG__

#include <string>



class debug {
public:
    static void log(const char* message);
    static void log(const std::string& message);
    static void alert(const char* message);
    static void alert(const std::string& message);
    static void error(const char* message);
    static void error(const std::string& message);
};

#endif