#pragma once

struct triggerParseDataString
{
    std::vector<tstring> params;
    tstring crc;
    CompareRange range;
    void clear() {
        params.clear();
        crc.clear();
    }
};

class triggerKeyData {
public:
    virtual bool getKey(int index, tstring* key) = 0;
    virtual int getLen() const = 0;
};

class triggerParseData
{
    triggerKeyData *tr;
    parseData m_parseData;
    int m_current_compare_pos;
    std::vector<triggerParseDataString*> m_strings;
    std::vector<int> m_indexes;
public:
    triggerParseData(triggerKeyData *t);
    ~triggerParseData();
    parseData* getParseData() { return &m_parseData; }
    int  getComparePos() const { return m_current_compare_pos; }
    void incComparePos() { m_current_compare_pos++; }
    void pushString(const CompareData& cd, const CompareObject &co, bool incompl_flag);
    void reset();
    void markDeleted(int string_index);
    void markDeletedAll();
    int  getParameters(int string_index) const;
    bool getParameter(int string_index, int parameter, tstring* p) const;
    bool getCRC(int string_index, tstring* crc) const;
    triggerParseDataString* get(int string_index) const;
    bool getKey(int string_index, tstring* key) const;
    bool getCompareRange(int string_index, CompareRange* range) const;
private:
    void resetindex();
    bool correctindex(int string_index) const;
};

class TriggerParseDataParameters: public InputParameters
{
   const triggerParseData *data;
   int string_index;
public:
    TriggerParseDataParameters(const triggerParseData* tpd, int index) : data(tpd), string_index(index) { assert(data); }
    void getParameters(std::vector<tstring>* params) const 
    {
        int parametersCount = data->getParameters(string_index);
        params->resize(parametersCount);
        for (int i = 0; i < parametersCount; ++i) {
            tstring v;
            data->getParameter(string_index, i, &v);
            params->at(i).assign(v);
        }
    }
    void doNoValues(tstring* cmd) const {
    }
};
