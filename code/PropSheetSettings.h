/*----------------------------------------------------------------------------
| File:    PropSheetSettings.h
| Project: Playlist Creator
|
| Description:
|   Declaration of the CPropSheetSettings class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 468 $
| $Id: PropSheetSettings.h 468 2009-11-12 16:48:36Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#pragma once

#include "controls/TreePropSheetEx/TreePropSheetOffice2003.h"
#include "PropPageGeneralSettings.h"
#include "PropPageUserInterface.h"
#include "PropPagePlaylist.h"
#include "PropPageFileTypes.h"
#include "PropPageExtendedInfo.h"
#include "PropPageDonations.h"
#include "ITaskbarListProvider.h"

using namespace TreePropSheet;

// CPropSheetSettings

class CPropSheetSettings : public CTreePropSheetOffice2003
{
    ///// types /////

    typedef CTreePropSheetOffice2003 inherited;


    ///// members /////
private:
    CFont   m_font;
    bool    m_bRestartNeeded;

    ITaskbarListProvider*       m_pTaskbarListProvider;

    CPropPageGeneralSettings    m_PropPageGeneralSettings;
    CPropPageUserInterface      m_PropPageUserInterface;
    CPropPagePlaylist           m_PropPagePlaylist;
    CPropPageExtendedInfo       m_PropPageExtendedInfo;
    CPropPageFileTypes          m_PropPageFileTypes;
    CPropPageDonations          m_PropPageDonations;


    ///// methods /////
private:
    void __init__();

    DECLARE_DYNAMIC(CPropSheetSettings)

public:
    CPropSheetSettings(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CPropSheetSettings(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    virtual ~CPropSheetSettings();

    bool IsRestartNeeded() { return m_bRestartNeeded; }
    void SetRestartNeeded(bool bRestartNeeded) { m_bRestartNeeded = bRestartNeeded; }

    ITaskbarListProvider* GetTaskbarListProvider() { return m_pTaskbarListProvider; }
    void SetTaskbarListProvider(ITaskbarListProvider* pProvider) { m_pTaskbarListProvider = pProvider; }

protected:
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnDestroy();
};


