#pragma once

struct CompareRange
{
    CompareRange() : begin(-1), end(-1) {}
    int begin;
    int end;
};

class CompareObjectVarsHelper {
public:
    CompareObjectVarsHelper(const tstring& name, bool multivar);
    bool next(tstring *val);
private:
    tstring pred;
    bool multflag;
};

class CompareObject
{
public:
    CompareObject();
    ~CompareObject();
    bool init(const tstring& key, bool endline_mode);
    bool initOnlyVars(const tstring& key);
    bool compare(const tstring& str);
    void getRange(CompareRange *range) const;
    void getParameters(std::vector<tstring>* params) const;
    bool isFullstrReq() const;
    void getKey(tstring* key) const;
private:
    void createCheckPcre(const tstring& key, bool endline_mode, tstring *prce_template);
    void checkVars(tstring *pcre_template);
    void maskRegexpSpecialSymbols(tstring *pcre_template, bool use_first_arrow);
    void reset();

    struct cutdata {
        int index;
        tstring pred;
        std::vector<tstring> vars;                
    };


    bool checkCuts();
    bool oneCutTest();
private:
    Pcre16  m_pcre;
    tstring m_key;
    tstring m_str;
    std::vector<tstring> m_vars_pcre_parts;
    bool m_fullstr_req;
    bool m_std_regexp;
    ParamsHelper *ph;
};
