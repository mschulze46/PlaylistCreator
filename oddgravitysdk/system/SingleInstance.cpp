#include "stdafx.h"
#include "SingleInstance.h"
#include <malloc.h>	// We use the _alloca() function

////////////////////////////////////////////////////////////////////////////////
// void CreateUniqueName( pszGUID, [out] sUniqueName, nMode )
//
// Creates a "unique" name, where the meaning of "unique" depends on the nMode
// flag values.
//
// pszGUID:		Copied to the beginning of sUniqueName, should be a GUID.
// sUniqueName:	Buffer for unique name.
// nMode:		Information, that should be used to create the unique name.
//				Can be one of the following values:
//
//				SI_SESSION_UNIQUE						- Allow one instance per login session
//				SI_DESKTOP_UNIQUE						- Allow one instance per desktop
//				SI_TRUSTEE_UNIQUE						- Allow one instance per user account
//				SI_SESSION_UNIQUE | SI_DESKTOP_UNIQUE	- Allow one instance per login session,
//														  instances in different login sessions
//														  must also reside on a different desktop
//				SI_TRUSTEE_UNIQUE | SI_DESKTOP_UNIQUE	- Allow one instance per user account,
//														  instances in login sessions running a
//														  different user account must also reside
//														  on different desktops.
//				SI_SYSTEM_UNIQUE						- Allow only one instance on the whole system
//
void CreateUniqueName(
		LPCTSTR pszGUID,
		CString& sUniqueName,
		int nMode /*SI_DESKTOP_UNIQUE*/
		)
{
	// reset passed name
	sUniqueName = _T("");

	// create buffer
	TCHAR pszBuffer[MAX_PATH]; // [MSDN] CreateMutex: The name of the mutex object is limited to MAX_PATH characters.

	// First copy GUID to destination buffer
	if( pszGUID )
	{
#if _MSC_VER >= 1400  // VS2005
		_tcsncpy_s( pszBuffer, _countof(pszBuffer), pszGUID, _TRUNCATE );
#else
		_tcscpy( pszBuffer, pszGUID );
#endif
	}
	else
	{
		*pszBuffer = 0;
	}

	if( nMode & SI_DESKTOP_UNIQUE )
	{
		// Name should be desktop unique, so add current desktop name

#if _MSC_VER >= 1400  // VS2005
		_tcscat_s( pszBuffer, _countof(pszBuffer), _T("-" ) );
#else
		_tcscat( pszBuffer, _T("-" ) );
#endif
		HDESK hDesk		= GetThreadDesktop( GetCurrentThreadId() );
		ULONG cchDesk	= ULONG( MAX_PATH - _tcslen( pszBuffer ) - 1 );

		if( !GetUserObjectInformation( hDesk, UOI_NAME, pszBuffer + _tcslen( pszBuffer ), cchDesk, &cchDesk ) )
		{
			// Call will fail on Win9x
#if _MSC_VER >= 1400  // VS2005
			_tcsncat_s( pszBuffer, _countof(pszBuffer), _T("Win9x"), cchDesk );
#else
			_tcsncat( pszBuffer, _T("Win9x"), cchDesk );
#endif
		}
	}
	if( nMode & SI_SESSION_UNIQUE )
	{
		// Name should be session unique, so add current session ID
		
		HANDLE hToken = NULL;
		// Try to open the token (fails on Win9x) and check necessary buffer size
		if( OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ) 
			&& ( MAX_PATH - _tcslen( pszBuffer ) > 9 ) ) 
		{
			DWORD cbBytes = 0;

			if( !GetTokenInformation( hToken, TokenStatistics, NULL, cbBytes, &cbBytes ) 
				&& GetLastError() == ERROR_INSUFFICIENT_BUFFER )
			{
				PTOKEN_STATISTICS pTS = (PTOKEN_STATISTICS) _alloca( cbBytes );
				if( GetTokenInformation( hToken, TokenStatistics, (LPVOID) pTS, cbBytes, &cbBytes ) )
				{
					wsprintf( 
						pszBuffer + _tcslen( pszBuffer ), 
						_T("-%08x%08x"), 
						pTS->AuthenticationId.HighPart, 
						pTS->AuthenticationId.LowPart 
						);
				}
			}
		}
	}
	if( nMode & SI_TRUSTEE_UNIQUE )
	{
		// Name should be unique to the current user

		TCHAR szUser[ 64 ] = {0};
		TCHAR szDomain[ 64 ] = {0};
		DWORD cchUser	= 64;
		DWORD cchDomain	= 64;

		if( GetUserName( szUser, &cchUser ) )
		{
			// Since NetApi() calls are quite time consuming
			// we retrieve the domain name from an environment variable
			cchDomain = GetEnvironmentVariable( _T("USERDOMAIN"), szDomain, cchDomain );

			UINT cchUsed = ULONG( _tcslen( pszBuffer ) );
			if( MAX_PATH - cchUsed > cchUser + cchDomain + 3 )
			{
				wsprintf(
					pszBuffer + cchUsed,
					_T("-%s-%s"),
					szDomain,
					szUser
					);
			}
		}
	}

	// copy result
	sUniqueName = pszBuffer;                                                ASSERT(!sUniqueName.IsEmpty());
}

////////////////////////////////////////////////////////////////////////////////
// BOOL IsInstancePresent( pszGUID, nMode )
//
// Returns TRUE, if there exists, according to the meaning of "unique" passed
// in nMode, another instance of this process.
//

BOOL IsInstancePresent(
		LPCTSTR pszGUID,
		int nMode /*SI_DESKTOP_UNIQUE*/
		)
{
	static HANDLE hMutex = NULL;

	if( hMutex == NULL )
	{
		CString sUniqueName;
		CreateUniqueName( pszGUID, sUniqueName, nMode );

		hMutex = CreateMutex( NULL, FALSE, sUniqueName );
		return ( GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED );
	}
	return FALSE;
}
