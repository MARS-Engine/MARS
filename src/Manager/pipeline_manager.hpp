#ifndef __PIPELINE__MANAGER__
#define __PIPELINE__MANAGER__

#include <map>
#include <string>
#include "Graphics/pipeline.hpp"



class pipeline_manager {
public:
    static std::map<std::string, pipeline*> pipelines;

    static pipeline* get_pipeline(const std::string& name);
    static void add_pipeline(const std::string& name, pipeline* _pipeline);
};

#endif