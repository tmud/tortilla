#pragma once
#include "PropertiesData.h"

class PropertiesManager
{
public:
    PropertiesManager();
    ~PropertiesManager();

    bool init();
    bool loadProfile(const tstring& force_profile, tstring *error);
    bool saveProfile();
    PropertiesData* getConfig() { return &m_propData; }
    const tstring& getProfileGroup() const { return m_profile.group; }
    const tstring& getProfileName() const { return m_profile.name; }
    const Profile& getProfile() const { return m_profile; }
    bool isFirstStartup() const { return m_first_startup; }
    bool createEmptyProfile(const Profile& profile);
    bool copyProfile(const Profile& src, const Profile& dst, tstring* error);
    bool loadProfile(const Profile& profile, tstring* error);
    bool checkProfile(const Profile& profile);

private:
    bool loadSettings(tstring *error);
    bool loadHistory(tstring* error);
    bool loadProfileData(tstring *error);
    bool saveProfileData();
    bool saveHistory();
    bool saveSettings();
    bool loadWindow(xml::node parent, OutputWindow* w);
    void saveWindow(xml::node parent, const OutputWindow& w);
    enum ValueReq { VALUE_ABSENT, VALUE_EXIST, VALUE_EXIST_CAN_EMPTY };
    enum GroupReq { GROUP_ABSENT, GROUP_EXIST };
    void loadArray(xml::node parent, const tstring& name, ValueReq values_req, GroupReq groups_req, PropertiesValues* values);
    void saveArray(xml::node parent, const tstring& name, ValueReq values_req, const PropertiesValues& values);
    void loadList(xml::node parent, const tstring& name, PropertiesList* values);
    void saveList(xml::node parent, const tstring& name, const PropertiesList& values);
    bool loadValue(xml::node parent, const tstring& name, int min, int max, int *value);
    void saveValue(xml::node parent, const tstring& name, int value);
    bool loadString(xml::node parent, const tstring& name, tstring* value);
    void saveString(xml::node parent, const tstring& name, const tstring& value);
    bool loadRgbColor(xml::node n, tstring* name, COLORREF* color);
    void saveRgbColor(xml::node parent, const tstring& name, COLORREF color);
    bool loadRECT(xml::node n, RECT *rc);
    void saveRECT(xml::node n, const RECT &rc);
    bool loadFromFile(xml::node& node, const tstring& file, tstring* error);
    bool saveToFile(xml::node node, const tstring& file);
    bool loadMapperData();
    bool saveMapperData();

private:
    Profile m_profile;
    PropertiesData m_propData;
    bool m_first_startup;
};
