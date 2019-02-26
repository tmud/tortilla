#pragma once

class TriggerActionHook {
public:
    virtual ~TriggerActionHook() {}
    virtual void run() = 0;
    virtual void triggeredOutput() = 0;
};
typedef std::shared_ptr<TriggerActionHook> TriggerAction;

class PluginsTriggersHandler
{
public:
    virtual bool processTriggers(parseData& parse_data, int start_string, std::vector<TriggerAction>& triggers) = 0;
};
