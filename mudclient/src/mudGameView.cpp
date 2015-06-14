#include "stdafx.h"
#include "mudGameView.h"

bool MudGameView::initialize()
{
    if (!initPluginsSystem())
    {
        msgBox(m_hWnd, IDS_ERROR_SCRIPTENGINE_FAILED, MB_OK | MB_ICONSTOP);
        return false;
    }

    RUN_INPUTPROCESSOR_TESTS;
    if (!m_processor.init())
    {
        msgBox(m_hWnd, IDS_ERROR_SCRIPTENGINE_FAILED, MB_OK|MB_ICONSTOP);
        return false;
    }

    if (!m_manager.init())
    {
        msgBox(m_hWnd, IDS_ERROR_INITPROFILES_FAILED, MB_OK|MB_ICONSTOP);
        return false;
    }

    if (!m_manager.loadProfile())
    {
        if (!m_manager.isDefaultProfile())
        {
            if (msgBox(m_hWnd, IDS_ERROR_LASTLOAD_FAILED, MB_YESNO|MB_ICONSTOP) != IDYES)
                return false;
        }
        if (!m_manager.loadNewProfile(m_manager.getProfileGroup(), L"player"))
        {
            if (!m_manager.createNewProfile(L"player"))
            {
                msgBox(m_hWnd, IDS_ERROR_NEWPROFILE_FAILED, MB_OK|MB_ICONSTOP);
                return false;
            }
        }
    }
    return true;
}

void MudGameView::onStart()
{
    if (!loadModules())
        msgBox(m_hWnd, IDS_ERROR_MODULES, MB_OK | MB_ICONSTOP);
   updateProps();
   loadPlugins();
}

void MudGameView::onClose()
{
    m_manager.saveProfile();
}

void MudGameView::onNewProfile()
{
    NewProfileDlg dlg;
    dlg.loadProfiles(m_manager.getProfileGroup());
    if (dlg.DoModal() == IDOK)
    {
        tstring source, name;
        dlg.getProfiles(&source, &name);

        unloadPlugins();

        tstring cgroup = m_manager.getProfileGroup();
        tstring cname = m_manager.getProfileName();
        bool successed = true;
        if (source.empty())
        {
            if (!m_manager.createNewProfile(name)) {
                msgBox(m_hWnd, IDS_ERROR_NEWPROFILE_FAILED, MB_OK|MB_ICONSTOP); successed = false;
            }
        }
        else
        {
            if (!m_manager.createCopyProfile(source, name)) {
                msgBox(m_hWnd, IDS_ERROR_COPYPROFILE_FAILED, MB_OK|MB_ICONSTOP); successed = false;
            }
        }
        if (!successed)
            m_manager.loadNewProfile(cgroup, cname);

        updateProps();
        loadClientWindowPos();
        loadPlugins();
        m_bar.reset();
    }
}

void MudGameView::onLoadProfile()
{
    LoadProfileDlg dlg;
    if (dlg.DoModal() == IDOK)
    {
        tstring group, name;
        dlg.getProfiles(&group, &name);
        if (name.empty())
            return;

        if (m_manager.getProfileGroup() == group && m_manager.getProfileName() == name)
            return;

        saveClientWindowPos();
        savePluginWindowPos();
        unloadPlugins();
        if (!m_manager.saveProfile())
        {
            msgBox(m_hWnd, IDS_ERROR_CURRENTSAVEPROFILE_FAILED, MB_OK|MB_ICONSTOP);
            loadClientWindowPos();
            loadPlugins();
            return;
        }
        tstring cgroup = m_manager.getProfileGroup();
        tstring cname = m_manager.getProfileName();
        if (!m_manager.loadNewProfile(group, name))
        {
            msgBox(m_hWnd, IDS_ERROR_LOADPROFILE_FAILED, MB_OK|MB_ICONSTOP);
            m_manager.loadNewProfile(cgroup, cname);
        }
        updateProps();
        loadClientWindowPos();
        loadPlugins();
        m_bar.reset();
    }
}

void MudGameView::onNewWorld()
{
    NewWorldDlg dlg;
    if (dlg.DoModal() == IDOK)
    {
        NewWorldDlgData data;
        dlg.getData(&data);

        saveClientWindowPos();
        savePluginWindowPos();
        unloadPlugins();
        if (!m_manager.saveProfile())
        {
            msgBox(m_hWnd, IDS_ERROR_CURRENTSAVEPROFILE_FAILED, MB_OK | MB_ICONSTOP);
            loadPlugins();
            loadClientWindowPos();            
            return;
        }

        tstring cgroup = m_manager.getProfileGroup();
        tstring cname = m_manager.getProfileName();
        bool successed = true;
        if (!data.from_name.empty())
        {
            if (!m_manager.loadNewProfile(data.from_name, data.from_profile)) {
                msgBox(m_hWnd, IDS_ERROR_LOADPROFILE_FAILED, MB_OK | MB_ICONSTOP); successed = false;
            }
            else if (!m_manager.renameProfile(data.name, data.profile)) {
                msgBox(m_hWnd, IDS_ERROR_CURRENTSAVEPROFILE_FAILED, MB_OK | MB_ICONSTOP); successed = false; 
            }
        }
        else
        {
            if (!m_manager.createNewProfile(data.name, data.profile)) {
                msgBox(m_hWnd, IDS_ERROR_NEWPROFILE_FAILED, MB_OK | MB_ICONSTOP); successed = false;
            }
        }
        if (!successed)
            m_manager.loadNewProfile(cgroup, cname);

        updateProps();
        loadClientWindowPos();
        loadPlugins();
        m_bar.reset();
    }
}

void MudGameView::loadPlugins()
{
    m_plugins.loadPlugins(m_manager.getProfileGroup(), m_manager.getProfileName());
}

void MudGameView::unloadPlugins()
{
    m_plugins.unloadPlugins();
}

void MudGameView::preprocessCommand(InputCommand* cmd)
{
    m_plugins.processGameCmd(cmd);
}

void MudGameView::setOscColor(int index, COLORREF color)
{
    if (m_propData->disable_osc)
        return;
    m_propData->osc_colors[index] = color;
    m_propData->osc_flags[index] = 1;
    m_propElements.palette.setColor(index, color);
}

void MudGameView::resetOscColors()
{
    if (m_propData->disable_osc)
        return;
    m_propData->resetOSCColors();
    m_propElements.palette.updateProps(m_propData);
}
