/*----------------------------------------------------------------------------
| File:    AppSettings.cpp
| Project: Playlist Creator
|
| Description:
|   Implementation of the CAppSettings class.
|
|-----------------------------------------------------------------------------
| $Author: Michael $   $Revision: 490 $
| $Id: AppSettings.cpp 490 2010-01-14 17:41:35Z Michael $
|-----------------------------------------------------------------------------
| Copyright (c) oddgravity.  All rights reserved.
|----------------------------------------------------------------------------*/

#include "stdafx.h"
#include "AppSettings.h"


///// static members /////
CMapStringToString CAppSettings::m_mapStringTable;
LANGUAGE CAppSettings::m_currentLanguage = LANGUAGE_EN;


///// construction / destruction /////
CAppSettings::CAppSettings()
{
}


CAppSettings::~CAppSettings()
{
}


///// methods /////
CString CAppSettings::GetCompanyName()
{
    return _T("oddgravity"); // don't get it from resource file cause of regkey hacking!
}


CString CAppSettings::GetWebsiteUrl()
{
    return _T("http://www.oddgravity.de");
}


CString CAppSettings::GetAppName(NAMESTYLE style /*NAMESTYLE_FULL*/)
{
    CString strAppName;
    switch(style)
    {
        case NAMESTYLE_FULL:
        default:
            strAppName = LoadString(_T("IDS_APPNAME_FULL"));
            break;

        case NAMESTYLE_SHORT:
            strAppName = LoadString(_T("IDS_APPNAME_SHORT"));
            break;

        case NAMESTYLE_TINY:
            strAppName = LoadString(_T("IDS_APPNAME_TINY"));
            break;
    }
    return strAppName;
}


CString CAppSettings::GetRegistryKey(NAMESTYLE style /*NAMESTYLE_SHORT*/)
{
    CString strRegKey;
    switch(style)
    {
        case NAMESTYLE_FULL:
            strRegKey = _T("Software\\oddgravity\\PlaylistCreator3");
            break;

        case NAMESTYLE_SHORT:
        default:
            strRegKey = _T("PlaylistCreator3");
            break;
    }
        
    return strRegKey;
}


CString CAppSettings::GetVersionString(PRODUCT_VERSION version /*PRODUCT_VERSION_FIX*/)
{
    CString strVersion;

    if(version == PRODUCT_VERSION_MAJOR)
    {
        strVersion.Format(_T("%d"), m_VersionMajor);
    }
    else if(version == PRODUCT_VERSION_MINOR)
    {
        strVersion.Format(_T("%d.%d"), m_VersionMajor, m_VersionMinor);
    }
    else if(version == PRODUCT_VERSION_FIX)
    {
        strVersion.Format(_T("%d.%d.%d"), m_VersionMajor, m_VersionMinor, m_VersionFix);
    }
    else // version == PRODUCT_VERSION_BUILD
    {
        strVersion.Format(_T("%d.%d.%d.%d"), m_VersionMajor, m_VersionMinor, m_VersionFix, m_VersionBuild);
    }
    return strVersion;
}


CString CAppSettings::GetSmartVersionString()
{
    CString strSmartVersion;

    if(m_VersionBuild != 0)
    {
        strSmartVersion.Format(_T("%d.%d.%d.%d"), m_VersionMajor, m_VersionMinor, m_VersionFix, m_VersionBuild);
    }
    else if(m_VersionFix != 0)
    {
        strSmartVersion.Format(_T("%d.%d.%d"), m_VersionMajor, m_VersionMinor, m_VersionFix);
    }
    else if(m_VersionMinor != 0)
    {
        strSmartVersion.Format(_T("%d.%d"), m_VersionMajor, m_VersionMinor);
    }
    else
    {
        strSmartVersion.Format(_T("%d"), m_VersionMajor);
    }

    return strSmartVersion;
}


int CAppSettings::GetVersionInt(PRODUCT_VERSION version /*PRODUCT_VERSION_MAJOR*/)
{
    switch(version)
    {
        case PRODUCT_VERSION_MAJOR: return m_VersionMajor;
        case PRODUCT_VERSION_MINOR: return m_VersionMinor;
        case PRODUCT_VERSION_FIX:   return m_VersionFix;
        case PRODUCT_VERSION_BUILD: return m_VersionBuild;
        default: ASSERT(0); return -1;
    }
}


CString CAppSettings::GetUpdateCheckUrl()
{
    return _T("http://www.oddgravity.de/download/updateinfo-") + MakeLower(PRODUCT_ID) + _T(".xml");
}


int CAppSettings::GetIntProperty(PROPERTY eProperty, int iDefaultValue /*0*/)
{
    switch(eProperty)
    {
        case PROP_FIRSTTIMEINITCOMPLETE:
            return AfxGetApp()->GetProfileInt(_T(""), _T("FirstTimeInitComplete"), iDefaultValue);

        case PROP_SINGLEINSTANCE:
            return AfxGetApp()->GetProfileInt(_T("GeneralSettings"), _T("SingleInstance"), iDefaultValue);

        case PROP_PLAYSOUND_SUCCESS:
            {
                // check if legacy key is present
                // note: remove this sometime in the future (changed in version 3.6)
                CString strLegacyKey = GetRegistryKey(NAMESTYLE_FULL) + _T("\\GeneralSettings");
                if(RegistryIsDwordValuePresent(HKEY_CURRENT_USER, strLegacyKey, _T("PlaySound")))
                    return AfxGetApp()->GetProfileInt(_T("GeneralSettings"), _T("PlaySound"), iDefaultValue);

                return AfxGetApp()->GetProfileInt(_T("UI"), _T("PlaySound_Success"), iDefaultValue);
            }

        case PROP_TRANSPARENCY:
            return AfxGetApp()->GetProfileInt(_T("GeneralSettings"), _T("Transparency"), iDefaultValue);

        case PROP_WRITEEXTENDEDINFO:
            return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("WriteExtendedInfo"), iDefaultValue);

        case PROP_READTAGS:
            {
                // check if legacy key is present
                // note: remove this sometime in the future (changed in version 3.6)
                CString strLegacyKey = GetRegistryKey(NAMESTYLE_FULL) + _T("\\MP3");
                if(RegistryIsDwordValuePresent(HKEY_CURRENT_USER, strLegacyKey, _T("ReadID3Tags")))
                    return AfxGetApp()->GetProfileInt(_T("MP3"), _T("ReadID3Tags"), iDefaultValue);

                return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("ReadTags"), iDefaultValue);
            }

        case PROP_ADDNEWENTRIES:
            return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("AddNewEntries"), iDefaultValue);

        case PROP_SORTNEWITEMS:
            return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("SortNewItems"), iDefaultValue);

        case PROP_SAVEMODE:
            return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("SaveMode"), iDefaultValue);

        case PROP_SAVEMODE_USEFSLASHES:
            return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("SaveModeUseFSlashes"), iDefaultValue);

        case PROP_AUTOPLAYLISTNAME:
            return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("AutoPlaylistName"), iDefaultValue);

        case PROP_AUTOPLAYLISTFOLDER:
            return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("AutoPlaylistFolder"), iDefaultValue);

        case PROP_AUTOCLEAR:
            return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("AutoClear"), iDefaultValue);

        case PROP_TOTALPLAYLISTS:
            return AfxGetApp()->GetProfileInt(_T("Donations"), _T("TotalPlaylists"), iDefaultValue);

        case PROP_TOTALSONGS:
            return AfxGetApp()->GetProfileInt(_T("Donations"), _T("TotalSongs"), iDefaultValue);

        case PROP_AUTOCLOSECREATEDLG:
            return AfxGetApp()->GetProfileInt(_T("GeneralSettings"), _T("AutoCloseCreateDlg"), iDefaultValue);

        case PROP_MAINDLG_WNDPOSX:
            return AfxGetApp()->GetProfileInt(_T("Dialogs\\MainDlg"), _T("WndPosX"), iDefaultValue);

        case PROP_MAINDLG_WNDPOSY:
            return AfxGetApp()->GetProfileInt(_T("Dialogs\\MainDlg"), _T("WndPosY"), iDefaultValue);

        case PROP_MAINDLG_WNDSIZEX:
            return AfxGetApp()->GetProfileInt(_T("Dialogs\\MainDlg"), _T("WndSizeX"), iDefaultValue);

        case PROP_MAINDLG_WNDSIZEY:
            return AfxGetApp()->GetProfileInt(_T("Dialogs\\MainDlg"), _T("WndSizeY"), iDefaultValue);

        case PROP_MAINDLG_WNDSHOWSTATE:
            return AfxGetApp()->GetProfileInt(_T("Dialogs\\MainDlg"), _T("WndShowState"), iDefaultValue);

        case PROP_SHOWGRIDLINES:
            return AfxGetApp()->GetProfileInt(_T("UI"), _T("ShowGridlines"), iDefaultValue);

        case PROP_SHOWALTERNATEROWCOLOR:
            return AfxGetApp()->GetProfileInt(_T("UI"), _T("ShowAlternateRowColor"), iDefaultValue);

        case PROP_ALTERNATEROWCOLOR:
            return AfxGetApp()->GetProfileInt(_T("UI"), _T("AlternateRowColor"), iDefaultValue);

        case PROP_SNAPTOSCREENBORDER:
            return AfxGetApp()->GetProfileInt(_T("UI"), _T("SnapToScreenBorder"), iDefaultValue);

        case PROP_FOLDERDLG_ADDRECURSIVE:
            return AfxGetApp()->GetProfileInt(_T("GeneralSettings"), _T("AddSongFolderRecursive"), iDefaultValue);

        case PROP_SETTINGS_LASTACTIVEPAGE:
            return AfxGetApp()->GetProfileInt(_T("GeneralSettings"), _T("SettingsLastActivePage"), iDefaultValue);

        case PROP_HIGHLIGHTMISSINGITEMS:
            return AfxGetApp()->GetProfileInt(_T("UI"), _T("HighlightMissingItems"), iDefaultValue);

        case PROP_MISSINGITEMSCOLOR:
            return AfxGetApp()->GetProfileInt(_T("UI"), _T("MissingItemsColor"), iDefaultValue);

        case PROP_APPLYFILTERONOPENPLAYLISTS:
            return AfxGetApp()->GetProfileInt(_T("Playlist"), _T("ApplyFilterOnOpenPlaylists"), iDefaultValue);

        case PROP_AUTOUPDATECHECK:
            return AfxGetApp()->GetProfileInt(_T("GeneralSettings"), _T("AutoUpdateCheck"), iDefaultValue);

        default:
            ASSERT(0); // u suck!
            break;
    }

    return iDefaultValue;
}


BOOL CAppSettings::SetIntProperty(PROPERTY eProperty, int iValue)
{
    switch(eProperty)
    {
        case PROP_FIRSTTIMEINITCOMPLETE:
            return AfxGetApp()->WriteProfileInt(_T(""), _T("FirstTimeInitComplete"), iValue);

        case PROP_SINGLEINSTANCE:
            return AfxGetApp()->WriteProfileInt(_T("GeneralSettings"), _T("SingleInstance"), iValue);

        case PROP_PLAYSOUND_SUCCESS:
            {
                // remove legacy key if available
                // note: remove this sometime in the future (changed in version 3.6)
                CString strLegacyKey = GetRegistryKey(NAMESTYLE_FULL) + _T("\\GeneralSettings");
                if(RegistryIsDwordValuePresent(HKEY_CURRENT_USER, strLegacyKey, _T("PlaySound")))
                    RegistryDeleteValue(HKEY_CURRENT_USER, strLegacyKey, _T("PlaySound"));

                return AfxGetApp()->WriteProfileInt(_T("UI"), _T("PlaySound_Success"), iValue);
            }

        case PROP_TRANSPARENCY:
            return AfxGetApp()->WriteProfileInt(_T("GeneralSettings"), _T("Transparency"), iValue);

        case PROP_WRITEEXTENDEDINFO:
            return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("WriteExtendedInfo"), iValue);

        case PROP_READTAGS:
            {
                // remove legacy key if available
                // note: remove this sometime in the future (changed in version 3.6)
                CString strLegacyKey = GetRegistryKey(NAMESTYLE_FULL) + _T("\\MP3");
                if(RegistryIsDwordValuePresent(HKEY_CURRENT_USER, strLegacyKey, _T("ReadID3Tags")))
                    RegistryDeleteValue(HKEY_CURRENT_USER, strLegacyKey, _T("ReadID3Tags"));

                return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("ReadTags"), iValue);
            }

        case PROP_ADDNEWENTRIES:
            return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("AddNewEntries"), iValue);

        case PROP_SORTNEWITEMS:
            return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("SortNewItems"), iValue);

        case PROP_SAVEMODE:
            return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("SaveMode"), iValue);

        case PROP_SAVEMODE_USEFSLASHES:
            return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("SaveModeUseFSlashes"), iValue);

        case PROP_AUTOPLAYLISTNAME:
            return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("AutoPlaylistName"), iValue);

        case PROP_AUTOPLAYLISTFOLDER:
            return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("AutoPlaylistFolder"), iValue);

        case PROP_AUTOCLEAR:
            return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("AutoClear"), iValue);

        case PROP_TOTALPLAYLISTS:
            return AfxGetApp()->WriteProfileInt(_T("Donations"), _T("TotalPlaylists"), iValue);

        case PROP_TOTALSONGS:
            return AfxGetApp()->WriteProfileInt(_T("Donations"), _T("TotalSongs"), iValue);

        case PROP_AUTOCLOSECREATEDLG:
            return AfxGetApp()->WriteProfileInt(_T("GeneralSettings"), _T("AutoCloseCreateDlg"), iValue);

        case PROP_MAINDLG_WNDPOSX:
            return AfxGetApp()->WriteProfileInt(_T("Dialogs\\MainDlg"), _T("WndPosX"), iValue);

        case PROP_MAINDLG_WNDPOSY:
            return AfxGetApp()->WriteProfileInt(_T("Dialogs\\MainDlg"), _T("WndPosY"), iValue);

        case PROP_MAINDLG_WNDSIZEX:
            return AfxGetApp()->WriteProfileInt(_T("Dialogs\\MainDlg"), _T("WndSizeX"), iValue);

        case PROP_MAINDLG_WNDSIZEY:
            return AfxGetApp()->WriteProfileInt(_T("Dialogs\\MainDlg"), _T("WndSizeY"), iValue);

        case PROP_MAINDLG_WNDSHOWSTATE:
            return AfxGetApp()->WriteProfileInt(_T("Dialogs\\MainDlg"), _T("WndShowState"), iValue);

        case PROP_SHOWGRIDLINES:
            return AfxGetApp()->WriteProfileInt(_T("UI"), _T("ShowGridlines"), iValue);

        case PROP_SHOWALTERNATEROWCOLOR:
            return AfxGetApp()->WriteProfileInt(_T("UI"), _T("ShowAlternateRowColor"), iValue);

        case PROP_ALTERNATEROWCOLOR:
            return AfxGetApp()->WriteProfileInt(_T("UI"), _T("AlternateRowColor"), iValue);

        case PROP_SNAPTOSCREENBORDER:
            return AfxGetApp()->WriteProfileInt(_T("UI"), _T("SnapToScreenBorder"), iValue);

        case PROP_FOLDERDLG_ADDRECURSIVE:
            return AfxGetApp()->WriteProfileInt(_T("GeneralSettings"), _T("AddSongFolderRecursive"), iValue);

        case PROP_SETTINGS_LASTACTIVEPAGE:
            return AfxGetApp()->WriteProfileInt(_T("GeneralSettings"), _T("SettingsLastActivePage"), iValue);

        case PROP_HIGHLIGHTMISSINGITEMS:
            return AfxGetApp()->WriteProfileInt(_T("UI"), _T("HighlightMissingItems"), iValue);

        case PROP_MISSINGITEMSCOLOR:
            return AfxGetApp()->WriteProfileInt(_T("UI"), _T("MissingItemsColor"), iValue);

        case PROP_APPLYFILTERONOPENPLAYLISTS:
            return AfxGetApp()->WriteProfileInt(_T("Playlist"), _T("ApplyFilterOnOpenPlaylists"), iValue);

        case PROP_AUTOUPDATECHECK:
            return AfxGetApp()->WriteProfileInt(_T("GeneralSettings"), _T("AutoUpdateCheck"), iValue);

        default:
            ASSERT(0); // u suck!
            break;
    }

    return FALSE;
}


CString CAppSettings::GetStringProperty(PROPERTY eProperty, const CString& strDefaultValue /*_T("")*/)
{
    switch(eProperty)
    {
        case PROP_LANGUAGE:
            return AfxGetApp()->GetProfileString(_T("GeneralSettings"), _T("Language"), strDefaultValue);

        case PROP_LASTPLAYLISTFOLDER:
            return AfxGetApp()->GetProfileString(_T(""), _T("LastPlaylistFolder"), strDefaultValue);

        case PROP_LASTSONGFOLDER:
            return AfxGetApp()->GetProfileString(_T(""), _T("LastSongFolder"), strDefaultValue);

        case PROP_ADDFILESFOLDER:
            return AfxGetApp()->GetProfileString(_T(""), _T("LastSongs"), strDefaultValue);

        case PROP_ADDFILESFILTER:
            return AfxGetApp()->GetProfileString(_T(""), _T("LastFileFilter"), strDefaultValue);

        case PROP_ADDPLAYLISTFOLDER:
            return AfxGetApp()->GetProfileString(_T(""), _T("AddPlaylistFolder"), strDefaultValue);

        case PROP_ADDPLAYLISTFILTER:
            return AfxGetApp()->GetProfileString(_T(""), _T("AddPlaylistFilter"), strDefaultValue);

        case PROP_TITLEMASK:
            {
                // check if legacy key is present
                // note: remove this sometime in the future (changed in version 3.6)
                CString strLegacyKey = GetRegistryKey(NAMESTYLE_FULL) + _T("\\MP3");
                if(RegistryIsStringValuePresent(HKEY_CURRENT_USER, strLegacyKey, _T("TitleMask")))
                    return AfxGetApp()->GetProfileString(_T("MP3"), _T("TitleMask"), strDefaultValue);

                return AfxGetApp()->GetProfileString(_T("Playlist"), _T("TitleMask"), strDefaultValue);
            }

        case PROP_PLAYLISTTYPE:
            return AfxGetApp()->GetProfileString(_T("Playlist"), _T("PlaylistType"), strDefaultValue);

        case PROP_OPENPLAYLISTFOLDER:
            return AfxGetApp()->GetProfileString(_T(""), _T("OpenPlaylistFolder"), strDefaultValue);

        case PROP_OPENPLAYLISTFILTER:
            return AfxGetApp()->GetProfileString(_T(""), _T("OpenPlaylistFilter"), strDefaultValue);

        default:
            ASSERT(0); // u suck!
            break;
    }

    return strDefaultValue;
}


BOOL CAppSettings::SetStringProperty(PROPERTY eProperty, const CString& strValue)
{
    switch(eProperty)
    {
        case PROP_LANGUAGE:
            return AfxGetApp()->WriteProfileString(_T("GeneralSettings"), _T("Language"), strValue);

        case PROP_LASTPLAYLISTFOLDER:
            return AfxGetApp()->WriteProfileString(_T(""), _T("LastPlaylistFolder"), strValue);

        case PROP_LASTSONGFOLDER:
            return AfxGetApp()->WriteProfileString(_T(""), _T("LastSongFolder"), strValue);

        case PROP_ADDFILESFOLDER:
            return AfxGetApp()->WriteProfileString(_T(""), _T("LastSongs"), strValue);

        case PROP_ADDFILESFILTER:
            return AfxGetApp()->WriteProfileString(_T(""), _T("LastFileFilter"), strValue);

        case PROP_ADDPLAYLISTFOLDER:
            return AfxGetApp()->WriteProfileString(_T(""), _T("AddPlaylistFolder"), strValue);

        case PROP_ADDPLAYLISTFILTER:
            return AfxGetApp()->WriteProfileString(_T(""), _T("AddPlaylistFilter"), strValue);

        case PROP_TITLEMASK:
            {
                // remove legacy key if available
                // note: remove this sometime in the future (changed in version 3.6)
                CString strLegacyKey = GetRegistryKey(NAMESTYLE_FULL) + _T("\\MP3");
                if(RegistryIsStringValuePresent(HKEY_CURRENT_USER, strLegacyKey, _T("TitleMask")))
                    RegistryDeleteValue(HKEY_CURRENT_USER, strLegacyKey, _T("TitleMask"));

                return AfxGetApp()->WriteProfileString(_T("Playlist"), _T("TitleMask"), strValue);
            }

        case PROP_PLAYLISTTYPE:
            return AfxGetApp()->WriteProfileString(_T("Playlist"), _T("PlaylistType"), strValue);

        case PROP_OPENPLAYLISTFOLDER:
            return AfxGetApp()->WriteProfileString(_T(""), _T("OpenPlaylistFolder"), strValue);

        case PROP_OPENPLAYLISTFILTER:
            return AfxGetApp()->WriteProfileString(_T(""), _T("OpenPlaylistFilter"), strValue);

        default:
            ASSERT(0); // u suck!
            break;
    }

    return FALSE;
}


void CAppSettings::GetSupportedLanguages(CStringArray& arrayLanguages)
{
    arrayLanguages.RemoveAll();
    arrayLanguages.InsertAt(LANGUAGE_DE, _T("Deutsch"));
    arrayLanguages.InsertAt(LANGUAGE_EN, _T("English"));
}


CString CAppSettings::GetLanguageString()
{
    return GetStringProperty(PROP_LANGUAGE, _T("EN"));
}


BOOL CAppSettings::SetLanguageString(LPCTSTR strLanguageID /*_T("EN")*/)
{
    return SetStringProperty(PROP_LANGUAGE, strLanguageID);
}


int CAppSettings::GetLanguageInt()
{
    CString strLanguage = GetLanguageString();
    if(strLanguage == _T("DE"))
    {
        return (int)LANGUAGE_DE;
    }
    else if(strLanguage == _T("EN"))
    {
        return (int)LANGUAGE_EN;
    }
    else
    {
        return (int)LANGUAGE_EN; // use English as fallback language
    }
    return -1;
}


BOOL CAppSettings::SetLanguageInt(int iLanguage /*LANGUAGE_EN*/)
{
    CString strLanguage;
    if(iLanguage == (int)LANGUAGE_DE)
    {
        strLanguage = _T("DE");
    }
    else if(iLanguage == (int)LANGUAGE_EN)
    {
        strLanguage = _T("EN");
    }
    else
    {
        strLanguage = _T("EN");
    }
    return SetLanguageString(strLanguage);
}


CString CAppSettings::GetCurrentLanguageString()
{
    switch(m_currentLanguage)
    {
        case LANGUAGE_DE:
            return _T("DE");

        case LANGUAGE_EN:
            return _T("EN");

        default:
            ASSERT(0);
            break;
    }

    return _T("");
}


BOOL CAppSettings::SetCurrentLanguageString(LPCTSTR strLanguageID /*_T("EN")*/)
{
    if(strLanguageID == _T("DE"))
    {
        m_currentLanguage = LANGUAGE_DE;
        return TRUE;
    }
    else if(strLanguageID == _T("EN"))
    {
        m_currentLanguage = LANGUAGE_EN;
        return TRUE;
    }
    else
    {
        ASSERT(0); // u suck
    }
    return FALSE;
}


int CAppSettings::GetCurrentLanguageInt()
{
    return static_cast<int>(m_currentLanguage);
}


BOOL CAppSettings::SetCurrentLanguageInt(int iLanguage /*LANGUAGE_EN*/)
{
    if(iLanguage == (int)LANGUAGE_DE)
    {
        m_currentLanguage = LANGUAGE_DE;
        return TRUE;
    }
    else if(iLanguage == (int)LANGUAGE_EN)
    {
        m_currentLanguage = LANGUAGE_EN;
        return TRUE;
    }
    else
    {
        ASSERT(0); // u suck
    }
    return FALSE;
}


time_t CAppSettings::GetLastUpdateCheckTimestamp()
{
    time_t timeLastCheck = 0;

    // build registry key and value
    CString strRegistryKey = CAppSettings::GetRegistryKey(NAMESTYLE_FULL) + _T("\\GeneralSettings");
    CString strRegistryValue = _T("LastUpdateCheck");

    // try to get registry value
    if(RegistryIsDwordValuePresent(HKEY_CURRENT_USER, strRegistryKey, strRegistryValue))
    {
        timeLastCheck = (time_t)RegistryGetDwordValue(HKEY_CURRENT_USER, strRegistryKey, strRegistryValue);
    }

    return timeLastCheck;
}


BOOL CAppSettings::UpdateLastUpdateCheckTimestamp()
{
    // build registry key and value
    CString strRegistryKey = CAppSettings::GetRegistryKey(NAMESTYLE_FULL) + _T("\\GeneralSettings");
    CString strRegistryValue = _T("LastUpdateCheck");

    // get current time
    time_t timeNow;
    time(&timeNow);

    // create/update registry value
    return RegistryCreateDwordValue(HKEY_CURRENT_USER, strRegistryKey, strRegistryValue, (DWORD)timeNow);
}


CString CAppSettings::LoadString(LPCTSTR lpszID, BOOL bReload /*FALSE*/)
{
    // check for reloading the string table
    if(bReload)
    {
        m_mapStringTable.RemoveAll();
    }

    // create the string table if necessary
    if(m_mapStringTable.IsEmpty())
    {
        // find and load appropriate string table
        CString strResID = _T("LANGUAGE_") + GetLanguageString();
        HRSRC hRes = FindResource(AfxGetResourceHandle(), strResID, _T("TEXTSTRINGTABLE"));
        DWORD dwResSize = SizeofResource(AfxGetResourceHandle(), hRes);
        HGLOBAL hGlobal = LoadResource(AfxGetResourceHandle(), hRes);
        if(!hGlobal)
        {
            ASSERT(0);
            hRes = FindResource(AfxGetResourceHandle(), _T("LANGUAGE_EN"), _T("TEXTSTRINGTABLE"));
            dwResSize = SizeofResource(AfxGetResourceHandle(), hRes);
            hGlobal = LoadResource(AfxGetResourceHandle(), hRes);
        }

        // store which language the string table has
        m_currentLanguage = static_cast<LANGUAGE>(GetLanguageInt());

        // get the string table
        CString strTextStringTable;
        const char* pRawData = (const char*)LockResource(hGlobal);
        for(DWORD n = 0; n < dwResSize; n++)
        {
            strTextStringTable += pRawData[n];
        }

        // fill the map
        CString strKey, strValue;
        int iPosBreak;
        while(!strTextStringTable.IsEmpty())
        {
            // get key
            iPosBreak = strTextStringTable.Find(_T("\r\n"));
            strKey = strTextStringTable.Left(iPosBreak);
            strTextStringTable.Delete(0, iPosBreak + 2);

            // get value
            iPosBreak = strTextStringTable.Find(_T("\r\n"));
            strValue = strTextStringTable.Left(iPosBreak);
            strTextStringTable.Delete(0, iPosBreak + 2);

            // check whitespaces
            if(strValue.Find(_T("\\n")) != -1 || strValue.Find(_T("\\t")) != -1)
            {
                RepairWhitespaces(strValue);
            }

            // add map entry
            m_mapStringTable[strKey] = strValue;
        }
    }

    // extract string from map
    CString str = m_mapStringTable[CString(lpszID)];

    // return ID if no string was found
    if(str.IsEmpty())
    {
        str = lpszID;
    }

    return str;
}


void CAppSettings::ReloadStringTable()
{
    // just load a dummy string and reload the string table
    LoadString(_T("IDS_2342"), TRUE);
}


void CAppSettings::GetDefaultXMessageBoxParams(XMSGBOXPARAMS& params)
{
    // enable Vista style
    params.dwOptions |= XMSGBOXPARAMS::VistaStyle;

    // enable user defined button captions and load localized strings
    params.bUseUserDefinedButtonCaptions = TRUE;
    GetXMessageBoxUserDefinedButtonCaptions(params.UserDefinedButtonCaptions);
}


void CAppSettings::GetXMessageBoxUserDefinedButtonCaptions(XMSGBOXPARAMS::CUserDefinedButtonCaptions& captions)
{
    // load strings for XMessageBox from localized string table
    _tcsncpy_s(captions.szAbort,            _countof(captions.szAbort),             LoadString(_T("IDS_MB_ABORT")),             _TRUNCATE);
    _tcsncpy_s(captions.szCancel,           _countof(captions.szCancel),            LoadString(_T("IDS_MB_CANCEL")),            _TRUNCATE);
    _tcsncpy_s(captions.szContinue,         _countof(captions.szContinue),          LoadString(_T("IDS_MB_CONTINUE")),          _TRUNCATE);
    _tcsncpy_s(captions.szDoNotAskAgain,    _countof(captions.szDoNotAskAgain),     LoadString(_T("IDS_MB_DONOTASKAGAIN")),     _TRUNCATE);
    _tcsncpy_s(captions.szDoNotTellAgain,   _countof(captions.szDoNotTellAgain),    LoadString(_T("IDS_MB_DONOTTELLAGAIN")),    _TRUNCATE);
    _tcsncpy_s(captions.szDoNotShowAgain,   _countof(captions.szDoNotShowAgain),    LoadString(_T("IDS_MB_DONOTSHOWAGAIN")),    _TRUNCATE);
    _tcsncpy_s(captions.szHelp,             _countof(captions.szHelp),              LoadString(_T("IDS_MB_HELP")),              _TRUNCATE);
    _tcsncpy_s(captions.szIgnore,           _countof(captions.szIgnore),            LoadString(_T("IDS_MB_IGNORE")),            _TRUNCATE);
    _tcsncpy_s(captions.szIgnoreAll,        _countof(captions.szIgnoreAll),         LoadString(_T("IDS_MB_IGNOREALL")),         _TRUNCATE);
    _tcsncpy_s(captions.szNo,               _countof(captions.szNo),                LoadString(_T("IDS_MB_NO")),                _TRUNCATE);
    _tcsncpy_s(captions.szNoToAll,          _countof(captions.szNoToAll),           LoadString(_T("IDS_MB_NOTOALL")),           _TRUNCATE);
    _tcsncpy_s(captions.szOK,               _countof(captions.szOK),                LoadString(_T("IDS_MB_OK")),                _TRUNCATE);
    _tcsncpy_s(captions.szReport,           _countof(captions.szReport),            LoadString(_T("IDS_MB_REPORT")),            _TRUNCATE);
    _tcsncpy_s(captions.szRetry,            _countof(captions.szRetry),             LoadString(_T("IDS_MB_RETRY")),             _TRUNCATE);
    _tcsncpy_s(captions.szSkip,             _countof(captions.szSkip),              LoadString(_T("IDS_MB_SKIP")),              _TRUNCATE);
    _tcsncpy_s(captions.szSkipAll,          _countof(captions.szSkipAll),           LoadString(_T("IDS_MB_SKIPALL")),           _TRUNCATE);
    _tcsncpy_s(captions.szTryAgain,         _countof(captions.szTryAgain),          LoadString(_T("IDS_MB_TRYAGAIN")),          _TRUNCATE);
    _tcsncpy_s(captions.szYes,              _countof(captions.szYes),               LoadString(_T("IDS_MB_YES")),               _TRUNCATE);
    _tcsncpy_s(captions.szYesToAll,         _countof(captions.szYesToAll),          LoadString(_T("IDS_MB_YESTOALL")),          _TRUNCATE);
}


CString CAppSettings::GetCredits()
{
    CString strCredits = _T("");

    // find and load appropriate credits
    CString strResID = _T("CREDITS_") + GetLanguageString();
    HRSRC hRes = FindResource(AfxGetResourceHandle(), strResID, _T("TXT"));
    DWORD dwResSize = SizeofResource(AfxGetResourceHandle(), hRes);
    HGLOBAL hGlobal = LoadResource(AfxGetResourceHandle(), hRes);
    if(!hGlobal)
    {
        ASSERT(0); // warning: fallback to English!
        hRes = FindResource(AfxGetResourceHandle(), _T("CREDITS_EN"), _T("TXT"));
        dwResSize = SizeofResource(AfxGetResourceHandle(), hRes);
        hGlobal = LoadResource(AfxGetResourceHandle(), hRes);
    }

    // get the credits
    const char* pRawData = (const char*)LockResource(hGlobal);
    for(DWORD n = 0; n < dwResSize; n++)
    {
        strCredits += pRawData[n];
    }

    ASSERT(!strCredits.IsEmpty());
    return strCredits;
}


LOGFONT CAppSettings::GetDialogFont()
{
    // Tahoma, 8.5pt, normal

    CFont font;
    font.CreatePointFont(85, _T("Tahoma"));

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(LOGFONT));
    font.GetLogFont(&lf);

    return lf;
}


LOGFONT CAppSettings::GetDialogBoldFont()
{
    // Tahoma, 8.5pt, bold

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(LOGFONT));
    lf = GetDialogFont();
    lf.lfWeight = FW_BOLD;

    return lf;
}


LOGFONT CAppSettings::GetDialogLargeFont()
{
    // Tahoma, 9.5pt, normal

    CFont font;
    font.CreatePointFont(95, _T("Tahoma"));

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(LOGFONT));
    font.GetLogFont(&lf);

    return lf;
}


LOGFONT CAppSettings::GetDialogLargeBoldFont()
{
    // Tahoma, 9.5pt, bold

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(LOGFONT));
    lf = GetDialogLargeFont();
    lf.lfWeight = FW_BOLD;

    return lf;
}


int CAppSettings::GetToolBarIconSize()
{
    return 24; // 24x24px
}


int CAppSettings::GetContextMenuIconSize()
{
    return 16; // 16x16px
}


void CAppSettings::ResetRegistryEntries()
{
    DeleteRegistryEntries();
    InitRegistryEntries();
}


void CAppSettings::ResetFileTypes()
{
    RegistryDeleteSubKey(HKEY_CURRENT_USER, GetRegistryKey(NAMESTYLE_FULL), _T("FileTypes"));
    InitRegistryEntries_FileTypes();
}


void CAppSettings::ShowHiddenMessages()
{
    RegistryDeleteSubKey(HKEY_CURRENT_USER, GetRegistryKey(NAMESTYLE_FULL), _T("DontShowDialogs"));
}


void CAppSettings::DeleteRegistryEntries()
{
    CString strRegistryKey = GetRegistryKey(NAMESTYLE_FULL);
    RegistryDeleteSubKey(HKEY_CURRENT_USER, strRegistryKey, _T("Dialogs"));
    RegistryDeleteSubKey(HKEY_CURRENT_USER, strRegistryKey, _T("DontShowDialogs"));
    RegistryDeleteSubKey(HKEY_CURRENT_USER, strRegistryKey, _T("FileTypes"));
    RegistryDeleteSubKey(HKEY_CURRENT_USER, strRegistryKey, _T("GeneralSettings"));
    RegistryDeleteSubKey(HKEY_CURRENT_USER, strRegistryKey, _T("Playlist"));
    RegistryDeleteSubKey(HKEY_CURRENT_USER, strRegistryKey, _T("UI"));

    // legacy registry keys
    // note: can eventually be removed sometime in the future
    RegistryDeleteSubKey(HKEY_CURRENT_USER, strRegistryKey, _T("MP3")); // removed in version 3.6
}


void CAppSettings::InitRegistryEntries()
{
    InitRegistryEntries_Language();
    InitRegistryEntries_FileTypes();
}


void CAppSettings::InitRegistryEntries_Language()
{
    CString strDefaultLanguage;

    // try to get NSIS installer language ID
    // 1031: german
    // 1033: english
    CString strRegistryKey = GetRegistryKey(NAMESTYLE_FULL);
    CString strLanguageID = RegistryGetStringValue(HKEY_CURRENT_USER, strRegistryKey, _T("Installer Language"));
    if(strLanguageID == _T("1031"))
    {
        strDefaultLanguage = _T("DE");
    }
    else if(strLanguageID == _T("1033"))
    {
        strDefaultLanguage = _T("EN");
    }
    else
    {
        strDefaultLanguage = GetUserDefaultLanguage();
        if(strDefaultLanguage != _T("DE") &&
            strDefaultLanguage != _T("EN"))
        {
            strDefaultLanguage = _T("EN");
        }
    }

    SetLanguageString(strDefaultLanguage);
}


void CAppSettings::InitRegistryEntries_FileTypes()
{
    // assemble registry key
    CString strRegistryKey = GetRegistryKey(NAMESTYLE_FULL) + _T("\\FileTypes");

    // get active file types
    CStringArray listActiveFileTypes;
    GetDefaultActiveFileTypes(listActiveFileTypes);

    // create registry values for active file types
    for(int i = 0; i < listActiveFileTypes.GetCount(); i++)
    {
        RegistryCreateStringValue(HKEY_CURRENT_USER, strRegistryKey, (LPCTSTR)listActiveFileTypes[i], _T("1"));
    }

    // get inactive file types
    CStringArray listInactiveFileTypes;
    GetDefaultInactiveFileTypes(listInactiveFileTypes);

    // create registry values for inactive file types
    for(int i = 0; i < listInactiveFileTypes.GetCount(); i++)
    {
        RegistryCreateStringValue(HKEY_CURRENT_USER, strRegistryKey, (LPCTSTR)listInactiveFileTypes[i], _T("0"));
    }
}


void CAppSettings::GetDefaultActiveFileTypes(CStringArray& arrayActiveFileTypes)
{
    // audio
    arrayActiveFileTypes.Add(_T("cda"));
    arrayActiveFileTypes.Add(_T("flac"));
    arrayActiveFileTypes.Add(_T("mp3"));
    arrayActiveFileTypes.Add(_T("ogg"));
    arrayActiveFileTypes.Add(_T("wav"));
    arrayActiveFileTypes.Add(_T("wma"));

    // video
    arrayActiveFileTypes.Add(_T("avi"));
    arrayActiveFileTypes.Add(_T("mpeg"));
    arrayActiveFileTypes.Add(_T("mpg"));
    arrayActiveFileTypes.Add(_T("ogm"));
    arrayActiveFileTypes.Add(_T("wmv"));
}


void CAppSettings::GetDefaultInactiveFileTypes(CStringArray& arrayInactiveFileTypes)
{
    // audio
    arrayInactiveFileTypes.Add(_T("669"));
    arrayInactiveFileTypes.Add(_T("aac"));
    arrayInactiveFileTypes.Add(_T("ac3"));
    arrayInactiveFileTypes.Add(_T("aif"));
    arrayInactiveFileTypes.Add(_T("aiff"));
    arrayInactiveFileTypes.Add(_T("amf"));
    arrayInactiveFileTypes.Add(_T("ape"));
    arrayInactiveFileTypes.Add(_T("asf"));
    arrayInactiveFileTypes.Add(_T("au"));
    arrayInactiveFileTypes.Add(_T("dts"));
    arrayInactiveFileTypes.Add(_T("far"));
    arrayInactiveFileTypes.Add(_T("it"));
    arrayInactiveFileTypes.Add(_T("itz"));
    arrayInactiveFileTypes.Add(_T("kar"));
    arrayInactiveFileTypes.Add(_T("m4a"));
    arrayInactiveFileTypes.Add(_T("mid"));
    arrayInactiveFileTypes.Add(_T("midi"));
    arrayInactiveFileTypes.Add(_T("miz"));
    arrayInactiveFileTypes.Add(_T("mka"));
    arrayInactiveFileTypes.Add(_T("mod"));
    arrayInactiveFileTypes.Add(_T("mp1"));
    arrayInactiveFileTypes.Add(_T("mp2"));
    arrayInactiveFileTypes.Add(_T("mtm"));
    arrayInactiveFileTypes.Add(_T("nst"));
    arrayInactiveFileTypes.Add(_T("okt"));
    arrayInactiveFileTypes.Add(_T("rmi"));
    arrayInactiveFileTypes.Add(_T("s3m"));
    arrayInactiveFileTypes.Add(_T("s3z"));
    arrayInactiveFileTypes.Add(_T("snd"));
    arrayInactiveFileTypes.Add(_T("stz"));
    arrayInactiveFileTypes.Add(_T("ult"));
    arrayInactiveFileTypes.Add(_T("voc"));
    arrayInactiveFileTypes.Add(_T("xm"));
    arrayInactiveFileTypes.Add(_T("xmz"));

    // video
    arrayInactiveFileTypes.Add(_T("m2v"));
    arrayInactiveFileTypes.Add(_T("mkv"));
    arrayInactiveFileTypes.Add(_T("mov"));
    arrayInactiveFileTypes.Add(_T("mp4"));
    arrayInactiveFileTypes.Add(_T("nsv"));
    arrayInactiveFileTypes.Add(_T("qt"));
    arrayInactiveFileTypes.Add(_T("vob"));
}


CString CAppSettings::GetWindowTitle()
{
    return GetAppName(NAMESTYLE_FULL) + _T(" ") + GetVersionString(PRODUCT_VERSION_FIX);
}


BOOL CAppSettings::GetIsSupporter()
{
    // legacy: remove this sometime in the future
    CString strLegacyKey = _T("Software\\odd gravity\\PlaylistCreator3");
    if(RegistryIsDwordValuePresent(HKEY_LOCAL_MACHINE, strLegacyKey, _T("IsSupporter")))
    {
        return (BOOL)RegistryGetDwordValue(HKEY_LOCAL_MACHINE, strLegacyKey, _T("IsSupporter"));
    }

    return (BOOL)RegistryGetDwordValue(HKEY_LOCAL_MACHINE, GetRegistryKey(NAMESTYLE_FULL), _T("IsSupporter"));
}


void CAppSettings::SetIsSupporter(BOOL bSupporter)
{
    VERIFY(RegistryCreateDwordValue(HKEY_LOCAL_MACHINE, GetRegistryKey(NAMESTYLE_FULL), _T("IsSupporter"), (DWORD)bSupporter));
}


void CAppSettings::GetAddNewEntries(CStringArray& arrayNewEntries)
{
    arrayNewEntries.RemoveAll();
    arrayNewEntries.InsertAt(NEW_ENTRIES_TOP, LoadString(_T("IDS_SETTINGS_PLAYLIST_NEW_ENTRIES_TOP")));
    arrayNewEntries.InsertAt(NEW_ENTRIES_BEFORESELECTION, LoadString(_T("IDS_SETTINGS_PLAYLIST_NEW_ENTRIES_BEFORESEL")));
    arrayNewEntries.InsertAt(NEW_ENTRIES_AFTERSELECTION, LoadString(_T("IDS_SETTINGS_PLAYLIST_NEW_ENTRIES_AFTERSEL")));
    arrayNewEntries.InsertAt(NEW_ENTRIES_BOTTOM, LoadString(_T("IDS_SETTINGS_PLAYLIST_NEW_ENTRIES_BOTTOM")));
}


void CAppSettings::GetSaveModes(CStringArray& arraySaveModes)
{
    arraySaveModes.RemoveAll();
    arraySaveModes.InsertAt(SAVE_MODE_ABSOLUTE, LoadString(_T("IDS_SETTINGS_PLAYLIST_SAVE_MODE_ABSOLUTE")));
    arraySaveModes.InsertAt(SAVE_MODE_ABSOLUTE_NODRIVE, LoadString(_T("IDS_SETTINGS_PLAYLIST_SAVE_MODE_ABSOLUTE_NODRIVE")));
    arraySaveModes.InsertAt(SAVE_MODE_RELATIVE, LoadString(_T("IDS_SETTINGS_PLAYLIST_SAVE_MODE_RELATIVE")));
}


CString CAppSettings::GetDefaultTitleMask()
{
    // default: <Album> - <Track #> - <Title>
    return _T("%3 - %6 - %2");
}


BOOL CAppSettings::GetDontShowDialog(const CString& strID)
{
    return (BOOL)AfxGetApp()->GetProfileInt(_T("DontShowDialogs"), strID, FALSE);
}


void CAppSettings::SetDontShowDialog(const CString& strID)
{
    AfxGetApp()->WriteProfileInt(_T("DontShowDialogs"), strID, TRUE);
}


void CAppSettings::OnHelp()
{
    CString strExeFilePath = GetExeFilePath();
    if(strExeFilePath.IsEmpty())
    {
        ASSERT(0);
        strExeFilePath = PRODUCT_ID + CString(_T("_")) + CAppSettings::GetLanguageString().MakeUpper() + _T(".chm");
    }
    else
    {
        strExeFilePath.Replace(_T(".exe"), _T("_"));
        strExeFilePath += CAppSettings::GetLanguageString().MakeUpper() + _T(".chm");
    }

    // open html help
    ::HtmlHelp(::GetDesktopWindow(), strExeFilePath, HH_DISPLAY_TOPIC, 0);
}


CString CAppSettings::GetCompanyDonationsUrl()
{
    return CAppSettings::LoadString(_T("IDS_URL_DONATIONS_WEBSITE"));
}


CString CAppSettings::GetPayPalDonationsUrl()
{
    CString strAppName = CAppSettings::GetAppName(NAMESTYLE_FULL) + _T(" ") + CAppSettings::GetVersionString(PRODUCT_VERSION_FIX);
    strAppName.Replace(' ', '+'); // encode spaces

    CString strUrl = ReplaceTags(CAppSettings::LoadString(_T("IDS_URL_DONATIONS_PAYPAL")), _T("APPLICATION"), strAppName);

    return strUrl;
}
