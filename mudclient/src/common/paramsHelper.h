#pragma once

class ParamsHelper
{
public:
    ParamsHelper(const tstring& param, bool include_anyid = false);
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
