#pragma once

class ParamsHelper
{
public:
    enum { DEFAULT = 0, DETECT_ANYID = 1, BLOCK_DOUBLEID = 2 };
    ParamsHelper(const tstring& param, unsigned int mode );
    int getSize() const;
    int getFirst(int index) const;
    int getLast(int index) const;
    int getId(int index) const;
    int getMaxId() const;
private:
    Pcre16 pcre;
    std::vector<int> m_ids;
    int m_maxid;
};
