#ifndef __DEBUG__
#define __DEBUG__

#include <iostream>
#include <string>

using namespace std;

class Debug {
public:
    static void Log(const char* Message);
    static void Log(const string& Message);
    static void Alert(const char* Message);
    static void Alert(const string& Message);
    static void Error(const char* Message);
    static void Error(const string& Message);
};

#endif