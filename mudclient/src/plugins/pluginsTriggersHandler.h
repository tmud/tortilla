#pragma once

class PluginsTrigger;
class PluginsTriggersHandler
{
public:
    virtual bool processTriggers(parseData& parse_data, int start_string, std::vector<PluginsTrigger*>& triggers) = 0;
};
