#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginsTriggerData.h"

triggerParseData::triggerParseData(triggerKeyData *t) : tr(t), m_current_compare_pos(0)
{
    int count = t->getLen();
    m_strings.resize(count, NULL);
    for (int i=0;i<count;++i) { m_strings[i] = new triggerParseDataString;  }
    resetindex();
}

triggerParseData::~triggerParseData()
{
    m_parseData.detach();
    std::for_each(m_strings.begin(), m_strings.end(), [](triggerParseDataString* tpd) { delete tpd;} );
}

void triggerParseData::reset()
{
    m_parseData.detach();
    std::for_each(m_strings.begin(), m_strings.end(), [](triggerParseDataString* tpd) { tpd->clear(); });
    resetindex();
    m_current_compare_pos = 0;
}

void triggerParseData::resetindex()
{
    int count = m_strings.size();
    m_indexes.resize(count);
    for (int i=0;i<count;++i) m_indexes[i] = i;
}

bool triggerParseData::correctindex(int string_index) const 
{
    int count = m_indexes.size();
    return (string_index >= 0 && string_index < count) ? true : false;
}

void triggerParseData::pushString(const CompareData& cd, const CompareObject &co, bool incompl_flag)
{
    m_parseData.strings.push_back(cd.string);
    m_parseData.last_finished = !incompl_flag;
    triggerParseDataString* tpd = m_strings[m_current_compare_pos];
    co.getParameters(&tpd->params);
    co.getRange(&tpd->range);
    cd.string->getMd5(&tpd->crc);
}

void triggerParseData::markDeleted(int string_index) 
{
    if (!correctindex(string_index))
       return;
    m_indexes.erase(m_indexes.begin()+string_index);
}

void triggerParseData::markDeletedAll()
{
    m_indexes.clear();
}

int triggerParseData::getParameters(int string_index) const
{
    triggerParseDataString *s = get(string_index);
    return (s) ? s->params.size() : 0;
}

bool triggerParseData::getParameter(int string_index, int parameter, tstring* p) const
{
    triggerParseDataString *s = get(string_index);
    if (s) 
    {
        int count = s->params.size();
        if (parameter >= 0 && parameter < count)
            { p->assign(s->params[parameter]); return true; }
    }
    return false;
}

bool triggerParseData::getCRC(int string_index, tstring* crc) const
{
    triggerParseDataString *s = get(string_index);
    if (!s) return false;
    crc->assign(s->crc);
    return true;
}

triggerParseDataString* triggerParseData::get(int string_index) const
{
    if (correctindex(string_index))
    {
       int s = m_indexes[string_index];
       return m_strings[s];
    }
    return NULL;
}

bool triggerParseData::getKey(int string_index, tstring* key) const
{
    if (correctindex(string_index)) {
       int s = m_indexes[string_index];
       return tr->getKey(s, key);
    }
    return false;
}

bool triggerParseData::getCompareRange(int string_index, CompareRange* range) const
{
    if (correctindex(string_index)) {
        int s = m_indexes[string_index];
        *range = m_strings[s]->range;
        return true;
    }
    return false;

}