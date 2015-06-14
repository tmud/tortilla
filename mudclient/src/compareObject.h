#pragma once

struct CompareRange
{
    CompareRange() : begin(-1), end(-1) {}
    int begin;
    int end;
};

class CompareObject
{
public:
    CompareObject();
    ~CompareObject();
    bool init(const tstring& key);
    bool compare(const tstring& str);
    void getRange(CompareRange *range) const;
    void getParameters(std::vector<tstring>* params) const;
private:
    void createCheckPcre(const tstring& key, tstring *prce_template);
    void checkVars(tstring *pcre_template);

private:
    Pcre16  m_pcre;
    tstring m_key;
    tstring m_str;
    std::vector<tstring> m_vars_pcre_parts;
};
