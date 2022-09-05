#ifndef __DEBUG__
#define __DEBUG__

#include <string>

using namespace std;

class debug {
public:
    static void log(const char* message);
    static void log(const string& message);
    static void alert(const char* message);
    static void alert(const string& message);
    static void error(const char* message);
    static void error(const string& message);
};

#endif