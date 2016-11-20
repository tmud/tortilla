#pragma once

class ParamsHelper
{
public:
    ParamsHelper(const tstring& param, bool block_doubles);
    ParamsHelper(const tstring& param, bool block_doubles, tstring *param_without_cuts);
    int getSize() const;
    int getFirst(int index) const;
    int getLast(int index) const;
    int getId(int index) const;
    int getMaxId() const;
    void cutParameter(int index, tstring* param);
    const tstring& getCutValue(int index);
private:
    void init(const tstring& param, bool block_doubles, tstring *nocuts);
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

#ifdef _DEBUG
class ParamsHelperUnitTests {
public:
    static void run();
private:
    static bool testCutValue(ParamsHelper& ph, int index, const tchar* value);
    static bool testCutParameter(ParamsHelper& ph, int index, const tchar* srcparam, const tchar* testparam);
};
#define RUN_PARAMSHELPER_TESTS ParamsHelperUnitTests::run();
#else
#define RUN_PARAMSHELPER_TESTS
#endif
