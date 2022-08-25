#ifndef __PIPELINE__MANAGER__
#define __PIPELINE__MANAGER__

#include <map>
#include <string>
#include "Graphics/Pipeline.hpp"

using namespace std;

class PipelineManager {
public:
    static map<string, Pipeline*> pipelines;

    static Pipeline* GetPipeline(const string& pipelineName);
    static void AddPipeline(const string& pipelineName, Pipeline* pipeline);
};

#endif