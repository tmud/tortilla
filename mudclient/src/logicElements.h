#pragma once

#include "compareObject.h"
#include "propertiesPages/propertiesData.h"
#include "MudViewString.h"
#include "inputProcessor.h"

class CompareData
{
public:
    CompareData(MudViewString *s);
    void reinit();
    void fullinit();
    void setBlock(int blockpos, MudViewStringBlock &b);
    void insertBlocks(int blockpos, int count);
    void delBlocks(int blockpos, int count);
    void del(CompareRange& range);
    int  fold(CompareRange& range);
    void appendto(CompareRange& range, std::vector<MudViewStringBlock>& b);
    bool cut(CompareRange& range); // distinguish in individual blocks
    bool findBlocks(CompareRange& range);
    int  findBlockBySymbol(int pos);

    MudViewString *string;
    tstring fullstr;
    int  start;
private:
    int  cutpos(int pos, int d);
    int  findblockpos(int &pos, int d);
    int  findblock(int pos, int d);
};

class Alias
{
public:
    Alias();
    void init(const property_value& v, const InputTemplateParameters& p);
    bool processing(const InputCommand cmd, InputCommands *newcmds);
private:
    CompareObject m_compare;
    InputTemplateCommands m_cmds;
};

class Hotkey
{
public:
    Hotkey();
    void init(const property_value& v, const InputTemplateParameters& p);
    bool processing(const tstring& key, InputCommands *newcmds);
private:
    tstring m_key;
    InputTemplateCommands m_cmds;
};

class Action
{
public:
    Action();
    void init(const property_value& v, const InputTemplateParameters& p);
    bool processing(CompareData& data, bool incompl_flag, InputCommands* newcmds);
    const tstring& getKey() const;
private:
    CompareObject m_compare;
    InputTemplateCommands m_cmds;
};

class Sub
{
public:
    Sub();
    ~Sub();
    void init(const property_value& v);
    bool processing(CompareData& data);
    const tstring& getKey() const;
private:
    CompareObject m_compare;
    tstring m_value;
    ParamsHelper* m_phelper;
};

class AntiSub
{
public:
    AntiSub(); 
    void init(const property_value& v);
    bool processing(CompareData& data);
    const tstring& getKey() const;
private:
    CompareObject m_compare;
};

class Gag
{
public:
    Gag();
    void init(const property_value& v);
    bool processing(CompareData& data);
    const tstring& getKey() const;
private:
    CompareObject m_compare;
};

class Highlight
{
public:
    Highlight();
    void init(const property_value& v);
    bool processing(CompareData& data);
    const tstring& getKey() const;
private:
    CompareObject m_compare;
    PropertiesHighlight m_hl;
};

class Timer
{
public:
    Timer();
    void init(const property_value& v, const InputTemplateParameters& p);
    void makeCommands(InputCommands *cmds);
    bool tick(int dt);
    void reset();
    int  left();
    tstring id;

private:
    int timer;
    int period;
    InputTemplateCommands m_cmds;
};
