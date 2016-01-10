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
        if (msgBox(m_hWnd, IDS_ERROR_LASTLOAD_FAILED, MB_YESNO|MB_ICONSTOP) != IDYES)
            return false;

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

MudViewHandler* MudGameView::getHandler(int view)
{
    if (view >= 0 && view <= OUTPUT_WINDOWS)
        return m_handlers[view];
    return NULL;
}

void MudGameView::findText()
{
    tstring text;
    m_find_view.getTextToSearch(&text);
    if (text.empty())
        return;
    int view = m_find_view.getSelectedWindow();
    bool shift = (GetKeyState(VK_SHIFT) < 0);
    int find_direction = (shift) ? -1 : 1;
    MudView *v = (view == 0) ? &m_history : m_views[view - 1];
    int current_find = v->getCurrentFindString();
    int new_find = v->findAndSelectText(current_find, find_direction, text);
    if (new_find == -1)
    {
       // not found
       if (current_find == -1)
          return;
    }
    // found / not found with last found

    // clear find in last find window (if it another window)
    if (m_last_find_view != view && m_last_find_view != -1)
    {
        MudView *lf = (m_last_find_view == 0) ? &m_history : m_views[m_last_find_view - 1];
        lf->clearFind();
        m_last_find_view = -1;
        if (new_find == -1)
            return;
    }
   
    /*int count = m_history.getStringsCount();
    int delta = m_history.getStringsOnDisplay() / 2;  // center on the screen
    int center_vs = new_vs + delta;                   // пробуем поставить по центру
    if (center_vs < count)
        new_vs = center_vs;
    */

    if (view == 0 && !m_history.IsWindowVisible())
    {
       showHistory(new_find, 0);
    }
}

void MudGameView::showFindText(int view, int string)
{
    //todo del
}

