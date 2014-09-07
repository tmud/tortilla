#pragma once

#include "hotkeyTable.h"
#include "highlightHelper.h"

class TestControl
{    
public:
    virtual bool checkPattern(tstring* param) = 0;
    virtual bool checkText(tstring* param) = 0;
};

class HotkeyTestControl : public TestControl
{
public:
    bool checkPattern(tstring* param) 
    {
        HotkeyTable table;
        tstring key;
        if (table.isKey(*param, &key))
        {
            param->assign(key);
            return true;
        }
        return false;
    }
    bool checkText(tstring* param) { return true; }
};

class HighlightTestControl : public TestControl
{
    HighlightHelper m_hh;
public:
    bool checkPattern(tstring* param) { return true; }
    bool checkText(tstring* param) { return m_hh.checkText(param); }   
};

class MethodsHelper
{
    LogicProcessorMethods *methods;
protected:
    void tmcLog(const tstring& msg) { methods->tmcLog(msg); }
    void simpleLog(const tstring& msg) { methods->simpleLog(msg); }
    void updateLog(const tstring& msg) { methods->updateLog(msg); }
    static tchar buffer[1024];
public:
    MethodsHelper(LogicProcessorMethods *callback) : methods(callback) {}
};

class AddParams3 : public MethodsHelper
{
public:
    AddParams3(LogicProcessorMethods *callback) : MethodsHelper(callback) {}
    int process(parser *p, PropertiesValues *values, PropertiesValues *groups,
        const tstring& label1,  const tstring& label2,  const tstring& label3,
        TestControl* control = NULL)
    {
        int n = p->size();
        if (n == 0)
        {
            swprintf(buffer, L"%s:", label1.c_str());
            tmcLog(buffer);
            for (int i=0,e=values->size(); i<e; ++i)
            {
                const property_value& v = values->get(i);                        
                swprintf(buffer, L"'%s' '%s' '%s'", v.key.c_str(), v.value.c_str(), v.group.c_str());
                simpleLog(buffer);
            }
            if (values->size() == 0)
            {
                tmcLog(L"Список пуст");
            }
            return 0;
        }
        if (n == 1)
        {
            tstring pattern(p->at(0));
            swprintf(buffer, L"%s с '%s':", label2.c_str(), pattern.c_str());
            tmcLog(buffer);
            Pcre16 pcre; 
            if (!pcre.setRegExp(pattern))
            {
                tmcLog(L"Неверный параметр.");
            }
            else
            {
                int count = 0;
                for (int i=0,e=values->size(); i<e; ++i)
                {
                    const property_value& v = values->get(i);
                    pcre.find(v.key);
                    if (pcre.getSize())
                    {
                        swprintf(buffer, L"'%s' '%s' '%s'", v.key.c_str(), v.value.c_str(), v.group.c_str());
                        simpleLog(buffer);
                        count++;
                    }
                }               
                if (!count)
                    tmcLog(L"Варианты не найдены.");
            }
            return 0;
        }
        if (n == 2 || n == 3)
        {
            tstring pattern(p->at(0));
            tstring text(p->at(1));
            if (control != NULL)
            {
                bool p = control->checkPattern(&pattern);
                bool t = control->checkText(&text);
                if (!p || !t)
                {
                    swprintf(buffer, L"Недопустимое значение: %s", p ? text.c_str() : pattern.c_str());
                    tmcLog(buffer);
                    return 0;
                }
            }

            tstring group;
            if (n == 3)
            {
                group.assign(p->at(2));
                if (!groups->exist(group))
                    groups->add(-1, group, L"1", L"");
            }
            else
                group.assign(groups->get(0).key);

            int index = values->find(pattern);
            values->add(index, pattern, text, group);
            swprintf(buffer, L"%s: '%s' '%s' '%s'", label3.c_str(), pattern.c_str(), text.c_str(), group.c_str() );
            updateLog(buffer);
            return 1;  
        }
        p->invalidargs();
        return 0;
    }
};

class DeleteParams3 : public MethodsHelper
{
public:
    DeleteParams3(LogicProcessorMethods *callback) : MethodsHelper(callback) {}
    int process(parser *p, PropertiesValues *values,
        const tstring& label, TestControl* control = NULL)
    {
        if (p->size() == 1)
        {
            tstring pattern(p->at(0));
            swprintf(buffer, L"%s '%s'",label.c_str(), pattern.c_str());
            tmcLog(buffer);
            if (control && !control->checkPattern(&pattern))
            {
                swprintf(buffer, L"Недопустимое значение: %s", pattern.c_str());
                tmcLog(buffer);
                return 0;
            }

            bool deleted = false;
            for (int i=0,e=values->size(); i<e; ++i)
            {
                const property_value& v = values->get(i);
                if (v.key == pattern)
                {
                    swprintf(buffer, L"Удалено '%s' '%s' '%s'", v.key.c_str(), v.value.c_str(), v.group.c_str());
                    simpleLog(buffer);
                    values->del(i);
                    deleted = true;
                    break;
                }
            }

            if (!deleted)
            {
                tmcLog(L"Варианты не найдены.");
                return 0;
            }
            return 1;
        }
        p->invalidargs();
        return 0;
    }
};

class AddParams2 : public MethodsHelper
{
public:
    AddParams2(LogicProcessorMethods *callback) : MethodsHelper(callback) {}
    int process(parser *p, PropertiesValues *values, PropertiesValues *groups,
        const tstring& label1,  const tstring& label2)
    {
        int n = p->size();
        if (n == 0)
        {
            swprintf(buffer, L"%s:", label1.c_str());
            tmcLog(buffer);
            for (int i=0,e=values->size(); i<e; ++i)
            {
                const property_value& v = values->get(i);                        
                swprintf(buffer, L"'%s' '%s'", v.key.c_str(), v.group.c_str());
                simpleLog(buffer);
            }
            if (values->size() == 0)
            {
                tmcLog(L"Список пуст");
            }
            return 0;
        }
        if (n == 1 || n == 2)
        {
            tstring pattern(p->at(0));
            tstring group;
            if (n == 2)
            {
                group.assign(p->at(1));
                if (!groups->exist(group))
                    groups->add(-1, group, L"1", L"");
            }
            else
                group.assign(groups->get(0).key);

            int index = values->find(pattern);
            values->add(index, pattern, L"", group);
            swprintf(buffer, L"%s: '%s' '%s'",label2.c_str(), pattern.c_str(), group.c_str() );
            updateLog(buffer);
            return 1;  
        }
        p->invalidargs();
        return 0;
    }
};

class DeleteParams2 : public MethodsHelper
{
public:
    DeleteParams2(LogicProcessorMethods *callback) : MethodsHelper(callback) {}
    int process(parser *p, PropertiesValues *values, const tstring& label)
    {
        if (p->size() == 1)
        {
            tstring pattern(p->at(0));
            swprintf(buffer, L"%s '%s'",label.c_str(), pattern.c_str());
            tmcLog(buffer);
          
            bool deleted = false;
            for (int i=0,e=values->size(); i<e; ++i)
            {
                const property_value& v = values->get(i);
                if (v.key == pattern)
                {
                    swprintf(buffer, L"Удалено '%s' '%s'", v.key.c_str(), v.group.c_str());
                    simpleLog(buffer);
                    values->del(i);
                    deleted = true;
                    break;
                }
            }

            if (!deleted)
            {
                tmcLog(L"Варианты не найдены.");
                return 0;
            }
            return 1;
        }
        p->invalidargs();
        return 0;
    }
};
