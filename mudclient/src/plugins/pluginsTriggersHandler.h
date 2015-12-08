#pragma once

struct LogicPipelineElement;
class PluginsTriggersHandler
{
public:
    virtual bool processTriggers(parseData& parse_data, int start_string, LogicPipelineElement* pe) = 0;
};
