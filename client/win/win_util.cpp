// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#include "boinc_win.h"

#include "win_util.h"

#define OS_UNKNOWN          0
#define OS_WIN95            1
#define OS_WINNT            2

int OSVersion;

//////////
// Function:    UtilInitOSVersion
// arguments:	void
// returns:		int indicating error
// function:	sets global variable "OSVersion" to the current OS (Win95/NT/Unknown)
int UtilInitOSVersion( void )
{
	OSVERSIONINFO osinfo;

	osinfo.dwOSVersionInfoSize = sizeof(osinfo);
	if (!GetVersionEx( &osinfo ))
		return FALSE;

	if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		OSVersion = OS_WIN95;
	else if ( osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		OSVersion = OS_WINNT;
	else
		OSVersion = OS_UNKNOWN;

	return TRUE;

}

//////////
// Function:    UtilGetRegKey
// arguments:	name: name of key, keyval: where to store value of key
// returns:		int indicating error
// function:	reads string value in specified key
int UtilSetRegKey(char *name, DWORD value)
{
	LONG error;
	HKEY boinc_key;

	if ( OSVersion == OS_WIN95 ) {
		error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else if ( OSVersion == OS_WINNT ) {
		error = RegOpenKeyEx( HKEY_CURRENT_USER, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else {
		return -1;
	}

	error = RegSetValueEx( boinc_key, name, 0,
		REG_DWORD, (CONST BYTE *)&value, 4 );

	RegCloseKey( boinc_key );

	return 0;
}

//////////
// Function:    UtilGetRegKey
// arguments:	name: name of key, keyval: where to store value of key
// returns:		int indicating error
// function:	reads string value in specified key
int UtilGetRegKey(char *name, DWORD &keyval)
{
	LONG error;
	DWORD type = REG_DWORD;
	DWORD size = sizeof( DWORD );
	char str[256];
	DWORD value;
	HKEY boinc_key;

	strncpy( str, "SOFTWARE\\BOINC\\", sizeof(str) );
	strcat( str, name );

	if ( OSVersion == OS_WIN95 ) {
		error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else if ( OSVersion == OS_WINNT ) {
		error = RegOpenKeyEx( HKEY_CURRENT_USER, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else {
		return -1;
	}

	error = RegQueryValueEx( boinc_key, name, NULL,
		&type, (BYTE *)&value, &size );

	keyval = value;

	RegCloseKey( boinc_key );

	if ( error != ERROR_SUCCESS ) return -1;

	return 0;
}

//////////
// Function:    UtilSetRegStr
// arguments:	name: name of key, str: value of string to store
// returns:		int indicating error
// function:	sets string value in specified key
int UtilSetRegStr(char *name, char *str)
{
	LONG error;
	HKEY boinc_key;

	if ( OSVersion == OS_WIN95 ) {
		error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else if ( OSVersion == OS_WINNT ) {
		error = RegOpenKeyEx( HKEY_CURRENT_USER, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else {
		return -1;
	}

	error = RegSetValueEx( boinc_key, name, 0,
		REG_SZ, (CONST BYTE *)str, strlen(str) + 1);

	RegCloseKey( boinc_key );

	return 0;
}

//////////
// Function:    UtilGetRegStr
// arguments:	name: name of key, str: where to store value of key
// returns:		int indicating error
// function:	reads string value in specified key
int UtilGetRegStr(char *name, char *str)
{
	LONG error;
	DWORD type = REG_SZ;
	DWORD size = 128;
	HKEY boinc_key;

	*str = 0;

	if ( OSVersion == OS_WIN95 ) {
		error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else if ( OSVersion == OS_WINNT ) {
		error = RegOpenKeyEx( HKEY_CURRENT_USER, "SOFTWARE\\BOINC",  
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else {
		return -1;
	}

	error = RegQueryValueEx( boinc_key, name, NULL,
		&type, (BYTE*)str, &size );

	RegCloseKey( boinc_key );

	if ( error != ERROR_SUCCESS ) return -1;

	return 0;
}

//////////
// Function:    UtilGetRegStartupStr
// arguments:	name: name of key, str: value of string to store
//				if str is empty, attepts to delete the key
// returns:		int indicating error
// function:	sets string value in specified key in windows startup dir
int UtilGetRegStartupStr(char *name, char *str)
{
	LONG error;
	DWORD type = REG_SZ;
	DWORD size = 128;
	HKEY boinc_key;

	*str = 0;

	if ( OSVersion == OS_WIN95 ) {
		error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else if ( OSVersion == OS_WINNT ) {
		error = RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else {
		return -1;
	}

	error = RegQueryValueEx( boinc_key, name, NULL,
		&type, (BYTE*)str, &size );

	RegCloseKey( boinc_key );

	if ( error != ERROR_SUCCESS ) return -1;

	return ERROR_SUCCESS;
}

//////////
// Function:    UtilSetRegStartupStr
// arguments:	name: name of key, str: value of string to store
//				if str is empty, attepts to delete the key
// returns:		int indicating error
// function:	sets string value in specified key in windows startup dir
int UtilSetRegStartupStr(char *name, char *str)
{
	LONG error;
	HKEY boinc_key;

	if ( OSVersion == OS_WIN95 ) {
		error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else if ( OSVersion == OS_WINNT ) {
		error = RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
			0, KEY_ALL_ACCESS, &boinc_key );
		if ( error != ERROR_SUCCESS ) return -1;
	} else {
		return -1;
	}

	if(strlen(str) != 0) {
		error = RegSetValueEx( boinc_key, name, 0,
			REG_SZ, (CONST BYTE *)str, strlen(str) + 1);
	} else {
		error = RegSetValueEx( boinc_key, name, 0,
			REG_SZ, (CONST BYTE *)"", 1);
		error = RegDeleteValue(boinc_key, name);
	}

	RegCloseKey( boinc_key );

	return error;
}
