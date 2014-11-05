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
public:
    virtual void tmcLog(const tstring& msg) = 0;
    virtual void simpleLog(const tstring& msg) = 0;
    virtual void updateLog(const tstring& msg) = 0;
    virtual void skipCheckMode() = 0;
};

class ParamsBuffer
{
public:
    ParamsBuffer() : buffer_len(1024) {}
    tchar buffer[1024];
    size_t buffer_len;
};

class AddParams3 : public ParamsBuffer
{
public:
    int process(parser *p, PropertiesValues *values, PropertiesValues *groups,
        const tstring& label1,  const tstring& label2,  const tstring& label3, 
        MethodsHelper* helper, TestControl* control = NULL)
    {
        int n = p->size();
        if (n == 0)
        {
            helper->skipCheckMode();
            swprintf(buffer, buffer_len, L"%s:", label1.c_str());
            helper->tmcLog(buffer);
            for (int i=0,e=values->size(); i<e; ++i)
            {
                const property_value& v = values->get(i);                        
                swprintf(buffer, buffer_len, L"'%s' '%s' '%s'", v.key.c_str(), v.value.c_str(), v.group.c_str());
                helper->simpleLog(buffer);
            }
            if (values->size() == 0)
            {
                helper->tmcLog(L"Список пуст");
            }
            return 0;
        }
        if (n == 1)
        {
            helper->skipCheckMode();
            tstring pattern(p->at(0));
            swprintf(buffer, buffer_len, L"%s с '%s':", label2.c_str(), pattern.c_str());
            helper->tmcLog(buffer);
            Pcre16 pcre; 
            if (!pcre.setRegExp(pattern))
            {
                helper->tmcLog(L"Неверный параметр.");
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
                        swprintf(buffer, buffer_len, L"'%s' '%s' '%s'", v.key.c_str(), v.value.c_str(), v.group.c_str());
                        helper->simpleLog(buffer);
                        count++;
                    }
                }               
                if (!count)
                    helper->tmcLog(L"Варианты не найдены.");
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
                    swprintf(buffer, buffer_len, L"Недопустимое значение: %s", p ? text.c_str() : pattern.c_str());
                    helper->tmcLog(buffer);
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
            swprintf(buffer, buffer_len, L"%s: '%s' '%s' '%s'", label3.c_str(), pattern.c_str(), text.c_str(), group.c_str());
            helper->updateLog(buffer);
            return 1;
        }
        p->invalidargs();
        return 0;
    }
};

class DeleteParams3 : public ParamsBuffer
{
public:
    int process(parser *p, PropertiesValues *values,
        const tstring& label,
        MethodsHelper* helper, TestControl* control = NULL)
    {
        if (p->size() == 1)
        {
            tstring pattern(p->at(0));
            swprintf(buffer, buffer_len, L"%s '%s'",label.c_str(), pattern.c_str());
            helper->tmcLog(buffer);
            if (control && !control->checkPattern(&pattern))
            {
                swprintf(buffer, buffer_len, L"Недопустимое значение: %s", pattern.c_str());
                helper->tmcLog(buffer);
                return 0;
            }

            bool deleted = false;
            for (int i=0,e=values->size(); i<e; ++i)
            {
                const property_value& v = values->get(i);
                if (v.key == pattern)
                {
                    swprintf(buffer, buffer_len, L"Удалено '%s' '%s' '%s'", v.key.c_str(), v.value.c_str(), v.group.c_str());
                    helper->simpleLog(buffer);
                    values->del(i);
                    deleted = true;
                    break;
                }
            }

            if (!deleted)
            {
                helper->tmcLog(L"Варианты не найдены.");
                return 0;
            }
            return 1;
        }
        p->invalidargs();
        return 0;
    }
};

class AddParams2 : public ParamsBuffer
{
public:
    int process(parser *p, PropertiesValues *values, PropertiesValues *groups,
        const tstring& label1, const tstring& label2, MethodsHelper* helper)
    {
        int n = p->size();
        if (n == 0)
        {
            helper->skipCheckMode();
            swprintf(buffer, buffer_len, L"%s:", label1.c_str());
            helper->tmcLog(buffer);
            for (int i=0,e=values->size(); i<e; ++i)
            {
                const property_value& v = values->get(i);                        
                swprintf(buffer, buffer_len, L"'%s' '%s'", v.key.c_str(), v.group.c_str());
                helper->simpleLog(buffer);
            }
            if (values->size() == 0)
            {
                helper->tmcLog(L"Список пуст");
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
            swprintf(buffer, buffer_len, L"%s: '%s' '%s'", label2.c_str(), pattern.c_str(), group.c_str());
            helper->updateLog(buffer);
            return 1;  
        }
        p->invalidargs();
        return 0;
    }
};

class DeleteParams2 : public ParamsBuffer
{
public:
    int process(parser *p, PropertiesValues *values, const tstring& label, MethodsHelper* helper)
    {
        if (p->size() == 1)
        {
            tstring pattern(p->at(0));
            swprintf(buffer, buffer_len, L"%s '%s'", label.c_str(), pattern.c_str());
            helper->tmcLog(buffer);
          
            bool deleted = false;
            for (int i=0,e=values->size(); i<e; ++i)
            {
                const property_value& v = values->get(i);
                if (v.key == pattern)
                {
                    swprintf(buffer, buffer_len, L"Удалено '%s' '%s'", v.key.c_str(), v.group.c_str());
                    helper->simpleLog(buffer);
                    values->del(i);
                    deleted = true;
                    break;
                }
            }

            if (!deleted)
            {
                helper->tmcLog(L"Варианты не найдены.");
                return 0;
            }
            return 1;
        }
        p->invalidargs();
        return 0;
    }
};
