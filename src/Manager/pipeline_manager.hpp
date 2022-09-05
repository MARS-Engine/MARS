#ifndef __PIPELINE__MANAGER__
#define __PIPELINE__MANAGER__

#include <map>
#include <string>
#include "Graphics/pipeline.hpp"

using namespace std;

class pipeline_manager {
public:
    static map<string, pipeline*> pipelines;

    static pipeline* get_pipeline(const string& name);
    static void add_pipeline(const string& name, pipeline* _pipeline);
};

#endif