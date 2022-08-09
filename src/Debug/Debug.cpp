#include "Debug.hpp"

void Debug::Log(const char* Message) {
    cout << "\033[32m[Log] \033[0m" << Message << endl;
}
void Debug::Log(const string& Message) {
    Log(Message.c_str());
}

void Debug::Alert(const char* Message) {

    cout << "\033[33m[Alert] \033[0m" << Message << endl;
}

void Debug::Alert(const string& Message) {
    Alert(Message.c_str());
}

void Debug::Error(const char* Message) {

    cout << "\033[31m[Error] \033[0m" << Message << endl;
    throw runtime_error("\n\033[31m[Error] \033[0m" + (string)Message);
}

void Debug::Error(const string& Message) {
    Error(Message.c_str());
}