#include <ClipWatch.h>

void CWLoadConfig(void) {
	HKEY restrict Key;
	if(RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ClipWatch", 0, KEY_QUERY_VALUE, &Key) != ERROR_SUCCESS) {
	FailedReadingFontData:
		NONCLIENTMETRICSW NonClientMetrics;
		NonClientMetrics.cbSize = sizeof(NonClientMetrics);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NonClientMetrics), &NonClientMetrics, 0);
		CW.Config.LogicalFontData = NonClientMetrics.lfMessageFont;
		CW.UI.Font = CreateFontIndirectW(&NonClientMetrics.lfMessageFont);
		CW.UI.BackgroundColourBrush = CreateSolidBrush(CW.Config.BackgroundColour);
		wcscpy_s(CW.Config.Text, 33, L"Clipboard Updated!");
		return;
	}

	/* I don't like this, but what other options are there? */

	dword Value;
	dword Size = sizeof(Value);
	if(RegGetValueW(Key, NULL, L"Fade-In Duration", RRF_RT_DWORD, NULL, &Value, &Size) == ERROR_SUCCESS) CW.Config.FadeInDuration = (word)Value;
	Size = sizeof(Value);
	if(RegGetValueW(Key, NULL, L"Mid-Fade Delay", RRF_RT_DWORD, NULL, &Value, &Size) == ERROR_SUCCESS) CW.Config.MidFadeDelay = (word)Value;
	Size = sizeof(Value);
	if(RegGetValueW(Key, NULL, L"Fade-Out Duration", RRF_RT_DWORD, NULL, &Value, &Size) == ERROR_SUCCESS) CW.Config.FadeOutDuration = (word)Value;
	Size = sizeof(Value);
	if(RegGetValueW(Key, NULL, L"Text Colour", RRF_RT_DWORD, NULL, &Value, &Size) == ERROR_SUCCESS) CW.Config.TextColour = Value;
	Size = sizeof(Value);
	if(RegGetValueW(Key, NULL, L"Background Colour", RRF_RT_DWORD, NULL, &Value, &Size) == ERROR_SUCCESS) CW.Config.BackgroundColour = Value;
	Size = sizeof(Value);
	if(RegGetValueW(Key, NULL, L"Flags", RRF_RT_DWORD, NULL, &Value, &Size) == ERROR_SUCCESS) CW.Config.Flags = (byte)Value;
	Size = 33 * sizeof(wchar);
	if(RegGetValueW(Key, NULL, L"Text", RRF_RT_REG_SZ, NULL, CW.Config.Text, &Size) != ERROR_SUCCESS) wcscpy_s(CW.Config.Text, 33, L"Clipboard Updated!");
	Size = 92;
	if(RegGetValueW(Key, NULL, L"Logical Font Data", RRF_RT_REG_BINARY, NULL, &CW.Config.LogicalFontData, &Size) != ERROR_SUCCESS) {
		RegCloseKey(Key);
		goto FailedReadingFontData;
	}

	RegCloseKey(Key);
	CW.UI.Font = CreateFontIndirectW(&CW.Config.LogicalFontData);
	CW.UI.BackgroundColourBrush = CreateSolidBrush(CW.Config.BackgroundColour);
	return;
}

bool CWSaveConfig(void) {
	HKEY restrict Key;
	if(RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ClipWatch", 0, NULL, 0, KEY_SET_VALUE, NULL, &Key, NULL) != ERROR_SUCCESS) return true;

	dword Value = CW.Config.FadeInDuration;
	RegSetValueExW(Key, L"Fade-In Duration", 0, REG_DWORD, (byte*)&Value, sizeof(Value));
	Value = CW.Config.MidFadeDelay;
	RegSetValueExW(Key, L"Mid-Fade Delay", 0, REG_DWORD, (byte*)&Value, sizeof(Value));
	Value = CW.Config.FadeOutDuration;
	RegSetValueExW(Key, L"Fade-Out Duration", 0, REG_DWORD, (byte*)&Value, sizeof(Value));
	RegSetValueExW(Key, L"Text Colour", 0, REG_DWORD, (byte*)&CW.Config.TextColour, sizeof(CW.Config.TextColour));
	RegSetValueExW(Key, L"Background Colour", 0, REG_DWORD, (byte*)&CW.Config.BackgroundColour, sizeof(CW.Config.BackgroundColour));
	Value = CW.Config.Flags;
	RegSetValueExW(Key, L"Flags", 0, REG_DWORD, (byte*)&Value, sizeof(Value));
	RegSetValueExW(Key, L"Text", 0, REG_SZ, (byte*)CW.Config.Text, (dword)(wcslen(CW.Config.Text) + 1) * sizeof(wchar));
	RegSetValueExW(Key, L"Logical Font Data", 0, REG_BINARY, (byte*)&CW.Config.LogicalFontData, sizeof(CW.Config.LogicalFontData));

	return false;
}