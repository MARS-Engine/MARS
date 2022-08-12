#include "PipelineManager.hpp"
#include "Graphics/Pipeline.hpp"

map<string, Pipeline*> PipelineManager::pipelines;

Pipeline *PipelineManager::GetPipeline(const string& pipelineName) {
    if (pipelines.count(pipelineName))
        return pipelines[pipelineName];

    return nullptr;
}

void PipelineManager::AddPipeline(const std::string& pipelineName, Pipeline *pipeline) {
    pipelines.insert(pair<string, Pipeline*>(pipelineName, pipeline));
}