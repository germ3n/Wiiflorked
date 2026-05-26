#include "menu.hpp"

void CMenu::_initStatusMenu(void)
{
    m_statusVisible = m_cfg.getBool("GENERAL", "debug_status_text", true);

    for(int idx = 0; idx < 5; ++idx)
    {
        m_statusShadow[idx] = _addLabel(fmt("STATUS_SHADOW_%d", idx), theme.fontSet[0], L"", 
                                      22, 322 + (idx * 30), 600, 30, CColor(0, 0, 0, 255), 
                                      FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);

        m_statusLbl[idx] = _addLabel(fmt("STATUS_LBL_%d", idx), theme.fontSet[0], L"", 
                                   20, 320 + (idx * 30), 600, 30, theme.btnFontColor, 
                                   FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);

        _setHideAnim(m_statusShadow[idx], fmt("STATUS_SHADOW_%d", idx), 0, 0, 0.f, 0.f);
        _setHideAnim(m_statusLbl[idx], fmt("STATUS_LBL_%d", idx), 0, 0, 0.f, 0.f);
        
        m_btnMgr.hide(m_statusShadow[idx], true);
        m_btnMgr.hide(m_statusLbl[idx], true);
    }
}

void CMenu::_drawStatusMenu(void)
{
    if(!m_statusVisible) return;

    std::vector<std::string> logs = m_status.Get();
    
    for(int idx = 0; idx < 5; ++idx)
    {
        if(idx < (int)logs.size())
        {
            wstringEx text(logs[idx]);
            
            m_btnMgr.setText(m_statusShadow[idx], text);
            m_btnMgr.setText(m_statusLbl[idx], text);
            
            m_btnMgr.show(m_statusShadow[idx]);
            m_btnMgr.show(m_statusLbl[idx]); 
        }
        else
        {
            m_btnMgr.hide(m_statusShadow[idx]);
            m_btnMgr.hide(m_statusLbl[idx]); 
        }
    }
}

void CMenu::_hideStatusMenu(bool instant)
{
    for(int idx = 0; idx < 5; ++idx)
    {
        m_btnMgr.hide(m_statusShadow[idx], instant);
        m_btnMgr.hide(m_statusLbl[idx], instant);
    }
}

void CMenu::_showStatusMenu(void)
{
    for(int idx = 0; idx < 5; ++idx)
    {
        m_btnMgr.show(m_statusShadow[idx]);
        m_btnMgr.show(m_statusLbl[idx]);
    }
}

void CMenu::_toggleStatusMenu(void)
{
    m_statusVisible = !m_statusVisible;
    
    m_cfg.setBool("GENERAL", "debug_status_text", m_statusVisible);
    m_cfg.save();
    
    if(m_statusVisible)
        _showStatusMenu();
    else
        _hideStatusMenu(false);
}