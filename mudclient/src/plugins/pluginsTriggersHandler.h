#pragma once

struct LogicPipelineElement;
class PluginsTriggersHandler
{
public:
    enum PTResult { FAIL = 0, OK, WAIT };
    virtual PTResult processTriggers(parseData& parse_data, int start_string, LogicPipelineElement* pe) = 0;
};
