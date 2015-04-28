#pragma once

#include "propertiesPages/propertiesData.h"
#include "logicHelper.h"

class InputCommand
{
public:
    InputCommand(const tstring& cmd);
    tstring full_command;                   // full command (with parameters)
    tstring command;                        // only command
    tstring parameters;                     // only parameters (without command) as single line (without trimming)
    std::vector<tstring> parameters_list;   // list of parameters separately
    bool empty;
};

typedef std::vector<InputCommand*> InputCommandsList;
class InputProcessor
{   
public:
    InputProcessor();
    ~InputProcessor();
    void updateProps(PropertiesData *pdata);
    void process(const tstring& cmd, LogicHelper* helper, std::vector<tstring>* loop_cmds);
    InputCommandsList commands;

private:   
    void processSeparators(const tstring& sep_cmd, InputCommandsList* result);
    void processParameters(const tstring& cmd, InputCommand* params, tstring* result);
    void clear();
    tchar m_separator, m_prefix;
};
