#pragma once

class PropertiesGroupFilter 
{
public:
    PropertiesGroupFilter(PropertiesValues *propGroups) {
        for (int i=0,e=propGroups->size(); i<e; ++i)
        {
            const property_value& g = propGroups->get(i);
            const tstring &v = g.value;
            int val = 0;
            if (v == L"0")  {}
            else if (v == L"1")  { val = 1; }
            else { assert(false); }
            state[g.key] = val;
        }
    }

    bool isGroupActive(const tstring& group) {
        std::map<tstring, int>::iterator it = state.find(group);
        if (it == state.end()) 
            return false;
        return (it->second == 1) ? true : false;
    }
private:
    std::map<tstring, int> state;

};