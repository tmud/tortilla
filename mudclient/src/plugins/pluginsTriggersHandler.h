#pragma once

class PluginsTriggersHandler
{
public:
    virtual bool processTriggers(MudViewString*s, bool incompl) = 0;
};
