#include "pipeline_manager.hpp"
#include "Graphics/pipeline.hpp"

map<string, pipeline*> pipeline_manager::pipelines;

pipeline *pipeline_manager::get_pipeline(const string& name) {
    if (pipelines.count(name))
        return pipelines[name];

    return nullptr;
}

void pipeline_manager::add_pipeline(const std::string& name, pipeline* _pipeline) {
    pipelines.insert(pair<string, pipeline*>(name, _pipeline));
}