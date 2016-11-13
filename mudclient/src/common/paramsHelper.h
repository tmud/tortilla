#pragma once

class ParamsHelper
{
public:
    ParamsHelper(const tstring& param, bool block_doubles);
    int getSize() const;
    int getFirst(int index) const;
    int getLast(int index) const;
    int getId(int index) const;
    int getMaxId() const;
    void cutParameter(int index, tstring* param);
    void getCutValue(int index, tstring* cutvalue);
private:
    static Pcre16 pcre;
    static Pcre16 cut;
    static bool m_static_init;
    struct param_values {
        int first;
        int last;
        int id;
        tstring cut;
    };
    std::vector<param_values> m_ids;
    int m_maxid;
};
