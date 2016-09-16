#pragma once

class ParamsHelper
{
public:
    enum { DETECT_ANYID = 1, BLOCK_DOUBLEID = 2, EXTENDED = 4 };
    ParamsHelper(const tstring& param, unsigned int mode );
    int getSize() const;
    int getFirst(int index) const;
    int getLast(int index) const;
    int getId(int index) const;
    int getMaxId() const;
    void cutParameter(int index, tstring* param);
private:
    Pcre16 pcre;
    std::vector<int> m_ids;
    std::map<int,tstring> m_cuts;
    int m_maxid;
    static Pcre16 cut;
    static bool m_cutinitialized;
};
