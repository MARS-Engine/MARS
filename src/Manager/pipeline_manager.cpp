#include "pipeline_manager.hpp"
#include "Graphics/pipeline.hpp"

std::map<std::string, pipeline*> pipeline_manager::pipelines;

pipeline *pipeline_manager::get_pipeline(const std::string& name) {
    if (pipelines.count(name))
        return pipelines[name];

    return nullptr;
}

void pipeline_manager::add_pipeline(const std::string& name, pipeline* _pipeline) {
    pipelines.insert(std::pair<std::string, pipeline*>(name, _pipeline));
}