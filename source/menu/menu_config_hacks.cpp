#include "menu.hpp"

// Define the UI elements for this specific page
s16 m_ciosShiftLblTitle;
s16 m_ciosShiftBtnBack;
s16 m_ciosShiftLblUser[4];

TexData m_ciosShiftBg;

void CMenu::_hideCiosShift(bool instant)
{
	m_btnMgr.hide(m_ciosShiftLblTitle, instant);
	m_btnMgr.hide(m_ciosShiftBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_ciosShiftLblUser); ++i)
		if(m_ciosShiftLblUser[i] != -1)
			m_btnMgr.hide(m_ciosShiftLblUser[i], instant);

	_hideConfigButtons(instant);
}

void CMenu::_showCiosShift(void)
{
    _setBg(m_ciosShiftBg, m_ciosShiftBg);
    m_btnMgr.show(m_ciosShiftLblTitle);
    m_btnMgr.show(m_ciosShiftBtnBack);
    for(u8 i = 0; i < ARRAY_SIZE(m_ciosShiftLblUser); ++i)
        if(m_ciosShiftLblUser[i] != -1)
            m_btnMgr.show(m_ciosShiftLblUser[i]);

    _textCiosShift();

    // Setup Values
    m_btnMgr.setText(m_configBtn1, m_autoShiftIOS ? _t("on", L"On") : _t("off", L"Off"));
    m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", m_d2xIOSStartSlot));
    m_btnMgr.setText(m_configLbl3Val, wfmt(L"%i", m_otherIOSStartSlot));
    m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_IOSShiftCount));

    // Show Row 1 (Checkbox equivalent)
    m_btnMgr.show(m_configLbl1);
    m_btnMgr.show(m_configBtn1);

    // Show Row 2 (Source Integer Picker)
    m_btnMgr.show(m_configLbl2);
    m_btnMgr.show(m_configLbl2Val);
    m_btnMgr.show(m_configBtn2M); 
    m_btnMgr.show(m_configBtn2P); 

    // Show Row 3 (Target Integer Picker)
    m_btnMgr.show(m_configLbl3);
    m_btnMgr.show(m_configLbl3Val);
    m_btnMgr.show(m_configBtn3M); 
    m_btnMgr.show(m_configBtn3P); 

    // Show Row 4 (Number of cIOS slots to shift)
    m_btnMgr.show(m_configLbl4);
    m_btnMgr.show(m_configLbl4Val);
    m_btnMgr.show(m_configBtn4M);
    m_btnMgr.show(m_configBtn4P);
}

void CMenu::_CiosShift(void)
{
    SetupInput();
    _showCiosShift();

    // Store previous state to know if we need to write to config later
    bool prev_auto = m_autoShiftIOS;
    int prev_d2x_slot = m_d2xIOSStartSlot;
    int prev_other_slot = m_otherIOSStartSlot;
	int prev_cios_slots = m_IOSShiftCount;

    while(!m_exit)
    {
        _mainLoopCommon();
        
        // Exit page conditions
        if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_ciosShiftBtnBack)))
            break;
        else if(BTN_UP_PRESSED)
            m_btnMgr.up();
        else if(BTN_DOWN_PRESSED)
            m_btnMgr.down();
        else if(BTN_A_PRESSED)
        {
            // ROW 1: Toggle the boolean
            if(m_btnMgr.selected(m_configBtn1))
            {
                m_autoShiftIOS = !m_autoShiftIOS;
                m_btnMgr.setText(m_configBtn1, m_autoShiftIOS ? _t("on", L"On") : _t("off", L"Off"));
            }
            // ROW 2: Adjust the Source cIOS start slot
            else if(m_btnMgr.selected(m_configBtn2M) || m_btnMgr.selected(m_configBtn2P))
            {
                bool increase = m_btnMgr.selected(m_configBtn2P); // Fixed Typo
                if(increase)
                {
                    m_d2xIOSStartSlot++;
                    if(m_d2xIOSStartSlot > 253 - (m_IOSShiftCount - 1)) // Upper boundary
                        m_d2xIOSStartSlot = 200;
                }
                else
                {
                    m_d2xIOSStartSlot--;
                    if(m_d2xIOSStartSlot < 200) // Lower boundary
                        m_d2xIOSStartSlot = 253 - (m_IOSShiftCount - 1); 
                }
                m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", m_d2xIOSStartSlot));
            }
            // ROW 3: Adjust the Target cIOS start slot
            else if(m_btnMgr.selected(m_configBtn3M) || m_btnMgr.selected(m_configBtn3P))
            {
                bool increase = m_btnMgr.selected(m_configBtn3P);
                if(increase)
                {
                    m_otherIOSStartSlot++;
                    if(m_otherIOSStartSlot > 253 - (m_IOSShiftCount - 1)) // Upper boundary
                        m_otherIOSStartSlot = 200;
                }
                else
                {
                    m_otherIOSStartSlot--;
                    if(m_otherIOSStartSlot < 200) // Lower boundary
                        m_otherIOSStartSlot = 253 - (m_IOSShiftCount - 1);
                }
                m_btnMgr.setText(m_configLbl3Val, wfmt(L"%i", m_otherIOSStartSlot));
            }
            // ROW 4: Adjust the number of cIOS slots to shift
            else if(m_btnMgr.selected(m_configBtn4M) || m_btnMgr.selected(m_configBtn4P))
            {
                bool increase = m_btnMgr.selected(m_configBtn4P);
                if(increase)
                {
                    m_IOSShiftCount++;
					if(m_IOSShiftCount > 10)
						m_IOSShiftCount = 1;
                }
                else
                {
                    m_IOSShiftCount--;
                    if(m_IOSShiftCount < 1)
                        m_IOSShiftCount = 10;
                }

                if(m_d2xIOSStartSlot > 253 - (m_IOSShiftCount - 1))
                {
                    m_d2xIOSStartSlot = 253 - (m_IOSShiftCount - 1);
                    m_btnMgr.setText(m_configLbl2Val, wfmt(L"%i", m_d2xIOSStartSlot));
                }
                    
                if(m_otherIOSStartSlot > 253 - (m_IOSShiftCount - 1))
                {
                    m_otherIOSStartSlot = 253 - (m_IOSShiftCount - 1);
                    m_btnMgr.setText(m_configLbl3Val, wfmt(L"%i", m_otherIOSStartSlot));
                }
                
                m_btnMgr.setText(m_configLbl4Val, wfmt(L"%i", m_IOSShiftCount));
            }
        }
    }
    _hideCiosShift();

    // Save to memory if changes were made
    if(prev_auto != m_autoShiftIOS || prev_other_slot != m_otherIOSStartSlot || prev_d2x_slot != m_d2xIOSStartSlot || prev_cios_slots != m_IOSShiftCount)
    {
        m_cfg.setBool("GENERAL", "auto_shift_ios", m_autoShiftIOS);
        m_cfg.setInt("GENERAL", "d2x_cios_start_slot", m_d2xIOSStartSlot);
        m_cfg.setInt("GENERAL", "other_cios_start_slot", m_otherIOSStartSlot);
		m_cfg.setInt("GENERAL", "cios_slots", m_IOSShiftCount);
		m_cfg.save();
    }
}

void CMenu::_initCiosShift()
{
	// Load the background from the theme (uses the default config BG if no custom one exists)
	m_ciosShiftBg = _texture("CIOS_SHIFT/BG", "texture", theme.bg, false);
	
	_addUserLabels(m_ciosShiftLblUser, ARRAY_SIZE(m_ciosShiftLblUser), "CIOS_SHIFT");
	
	m_ciosShiftLblTitle = _addLabel("CIOS_SHIFT/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_ciosShiftBtnBack = _addButton("CIOS_SHIFT/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	_setHideAnim(m_ciosShiftLblTitle, "CIOS_SHIFT/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_ciosShiftBtnBack, "CIOS_SHIFT/BACK_BTN", 0, 0, 1.f, -1.f);

	_hideCiosShift(true);
	_textCiosShift();
}

void CMenu::_textCiosShift(void)
{
	m_btnMgr.setText(m_ciosShiftLblTitle, _t("cfgshft0", L"cIOS Auto-Shift (Secondary USB)"));
	m_btnMgr.setText(m_ciosShiftBtnBack, _t("cfg10", L"Back"));
	m_btnMgr.setText(m_configLbl1, _t("cfgshft1", L"Auto-Shift cIOS slots"));
	m_btnMgr.setText(m_configLbl2, _t("cfgshft2", L"Standard cIOS Start Slot (e.g. 244)"));
	m_btnMgr.setText(m_configLbl3, _t("cfgshft3", L"Alternate cIOS Start Slot (e.g. 204)"));
	m_btnMgr.setText(m_configLbl4, _t("cfgshft4", L"Number of cIOS slots (e.g. 10)"));
}