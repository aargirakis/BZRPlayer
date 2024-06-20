static DWORD RegGetInteger(LPTSTR lpszSubKey, LPTSTR lpszName, DWORD dwDefaultValue)
{
	HKEY hkey;
	DWORD dispo, buf, dwt, dws;
	if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, lpszSubKey, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dispo)) return dwDefaultValue;
	dws = 4;
	if (ERROR_SUCCESS != RegQueryValueEx(hkey, lpszName,NULL,&dwt,(LPBYTE)&buf, &dws) || dwt != REG_DWORD_LITTLE_ENDIAN)
	{
		buf = dwDefaultValue;
		RegSetValueEx(hkey, lpszName,0,REG_DWORD_LITTLE_ENDIAN,(LPBYTE)&buf, 4);
	}
	RegCloseKey(hkey);
	return buf;
}
