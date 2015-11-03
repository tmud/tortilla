#pragma once
#include "PropertiesData.h"

class PropertiesManager
{
public:
    PropertiesManager();
    ~PropertiesManager();

    bool init();
    bool loadProfile();
    bool saveProfile();
    PropertiesData* getConfig() { return &m_propData; }
    const tstring& getProfileGroup() const { return m_configName; }
    const tstring& getProfileName() const { return m_profileName; }
    bool isFirstStartup() const { return m_first_startup; }
    bool createNewProfile(const tstring& name);
    bool createCopyProfile(const tstring& from, const tstring& name);
    bool loadNewProfile(const tstring& group, const tstring& name);
    bool createNewProfile(const tstring& group, const tstring& name);
    bool renameProfile(const tstring& group, const tstring& name);

private:
    bool loadSettings();
    bool loadHistory();
    bool loadProfileData();
    bool saveProfileData();
    bool saveHistory();
    bool saveSettings();
    bool loadWindow(xml::node parent, OutputWindow* w);
    void saveWindow(xml::node parent, const OutputWindow& w);
    void loadArray(xml::node parent, const tstring& name, bool values_req, bool groups_req, PropertiesValues* values);
    void saveArray(xml::node parent, const tstring& name, const PropertiesValues& values);
    void loadList(xml::node parent, const tstring& name, PropertiesList* values);
    void saveList(xml::node parent, const tstring& name, const PropertiesList& values);
    bool loadValue(xml::node parent, const tstring& name, int min, int max, int *value);
    void saveValue(xml::node parent, const tstring& name, int value);
    bool loadString(xml::node parent, const tstring& name, tstring* value);
    void saveString(xml::node parent, const tstring& name, const tstring& value);
    bool loadRgbColor(xml::node n, tstring* name, COLORREF* color);
    void saveRgbColor(xml::node parent, const std::string& name, COLORREF color);
    bool loadRECT(xml::node n, RECT *rc);
    void saveRECT(xml::node n, const RECT &rc);
    bool loadFromFile(xml::node& node, const tstring& file);
    bool saveToFile(xml::node node, const tstring& file);
    bool loadMapperData();
    bool saveMapperData();

private:
    tstring m_configName;
    tstring m_profileName;
    PropertiesData m_propData;
    bool m_first_startup;
};
