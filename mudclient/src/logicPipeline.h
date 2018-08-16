#pragma once

#include "mudViewParser.h"
#include "inputProcessor.h"

class PluginsTrigger;
struct LogicPipelineElement 
{
   parseData lua_processed;
   parseData not_processed;
   InputCommands commands;                  // from actions
   std::vector<TriggerAction> triggers;     // from plugin's triggers
};

class LogicPipeline
{
    std::vector<LogicPipelineElement*> m_allocated;
    std::vector<LogicPipelineElement*> m_free;
public:
    LogicPipeline() {}
    ~LogicPipeline() {
        std::for_each(m_allocated.begin(), m_allocated.end(), 
            [](LogicPipelineElement *e){ delete e; } );
    }

    LogicPipelineElement* createElement()
    {
        if (!m_free.empty())
        {
            int last = m_free.size()-1;
            LogicPipelineElement* e = m_free[last];
            m_free.pop_back();
            return e;
        }
        LogicPipelineElement* e = new LogicPipelineElement();
        m_allocated.push_back(e);
        return e;
    }

    void freeElement(LogicPipelineElement* e)
    {
        e->triggers.clear();
        e->commands.clear();
        e->not_processed.clear();
        e->lua_processed.clear();
        m_free.push_back(e);
    }
};
