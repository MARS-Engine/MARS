#ifndef __PIPELINEMANAGER__
#define __PIPELINEMANAGER__

#include <map>
#include <string>

using namespace std;

class Pipeline;

class PipelineManager {
public:
    static map<string, Pipeline*> pipelines;

    static Pipeline* GetPipeline(const string& pipelineName);
    static void AddPipeline(const string& pipelineName, Pipeline* pipeline);
};

#endif