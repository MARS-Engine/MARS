#ifndef __MATERIAL__
#define __MATERIAL__

#include <string>

using namespace std;

/// Generic class with values that can be used by renderers and is used for sorting
struct Material {
public:
    string name;
    bool enableTransparency;

    Material(const string& name);
};

#endif