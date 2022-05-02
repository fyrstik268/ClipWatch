/* ClipWatch - Configuration dialog box and registry code */
#include <ClipWatch.h>

/* WM_CTLCOLORSTATIC
wParam
	Handle to the device context for the static control window.
lParam
	Handle to the static control. */

/* ClipWatch Registry Keys (HKCU\Software\ClipWatch\)
- Text Colour (DWORD, COLORREF format)
- Background Colour (DWORD, COLORREF format)
- FadeIn duration (DWORD, milliseconds. Cast to WORD)
- Mid-Fade Delay (DWORD, milliseconds. Cast to WORD)
- FadeOut duration (DWORD, milliseconds. Cast to WORD)
- Flags (DWORD, see the CF_* enums)
- Font (Byte array of LOGFONTW struct, *only* 92 bytes.) */

HWND CW_PreviewRectControl, CW_PreviewTextControl;

#pragma warning(suppress: 4100)
intptr CALLBACK CWSettingsDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam) {
	switch(Message) {
	case WM_INITDIALOG:
		SendMessageW(Dialog, WM_SETICON, ICON_BIG, (LPARAM)CW.UI.Icon);
		SendMessageW(Dialog, WM_SETICON, ICON_SMALL, (LPARAM)CW.UI.Icon);

		CW_PreviewRectControl = GetDlgItem(Dialog, IDC_BG_PREVIEW);
		CW_PreviewTextControl = GetDlgItem(Dialog, IDC_TEXT_PREVIEW);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		if((HWND)LParam == CW_PreviewRectControl || (HWND)LParam == CW_PreviewTextControl) {
			SetBkColor((HDC)WParam, CW.Config.BackgroundColour);
			SetTextColor((HDC)WParam, CW.Config.TextColour);
			return (intptr)CW.UI.BackgroundColourBrush;
		}
		return false;

	case WM_COMMAND:
		switch(WParam) {
		case IDC_CHOOSE_FONT:
			CHOOSEFONTW FontInfo = {
				.lStructSize = sizeof(FontInfo),
				.hwndOwner = Dialog,
				.lpLogFont = &CW.Config.LogFont,
				.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS
			};
			if(ChooseFontW(&FontInfo)) {
				HFONT Font = CreateFontIndirectW(&CW.Config.LogFont);
				DeleteObject(CW.UI.Font);
				CW.UI.Font = Font;
			}
			return TRUE;

		default:
			return TRUE;
		}

	default:
		return FALSE;
	}
}

static inline void CWLoadConfigElement(HKEY const restrict Key, wchar* const restrict ValueName, dword* const restrict Value, const dword Default) {
	dword Size = sizeof(dword);
	if(RegGetValueW(Key, NULL, ValueName, RRF_RT_DWORD, NULL, &Value, &Size)) *Value = Default;
	return;
}

void CWLoadConfig(void) {
	HKEY restrict Key;
	RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ClipWatch", 0, NULL, 0, KEY_QUERY_VALUE, NULL, &Key, NULL);

	dword CastingBuffer;
	CWLoadConfigElement(Key, L"Fade-In Duration", &CastingBuffer, 1250);
	CW.Config.FadeInDuration = CastingBuffer > MAXWORD ? MAXWORD : (word)CastingBuffer;
	CWLoadConfigElement(Key, L"Mid-Fade Delay", &CastingBuffer, 500);
	CW.Config.MidFadeDelay = CastingBuffer > MAXWORD ? MAXWORD : (word)CastingBuffer;
	CWLoadConfigElement(Key, L"Fade-Out Duration", &CastingBuffer, 1250);
	CW.Config.FadeOutDuration = CastingBuffer > MAXWORD ? MAXWORD : (word)CastingBuffer;

	CWLoadConfigElement(Key, L"Text Colour", &CW.Config.TextColour, RGB(0xFF, 0xFF, 0xFF));
	CWLoadConfigElement(Key, L"Background Colour", &CW.Config.BackgroundColour, RGB(0x33, 0x33, 0x33));

	CWLoadConfigElement(Key, L"Flags", &CastingBuffer, CW_CFG_POS_TO_NEAREST);
	CW.Config.Flags = (byte)CastingBuffer;

	dword Size = sizeof(CW.Config.LogFont);
	if(RegGetValueW(Key, NULL, L"Font", RRF_RT_REG_BINARY, NULL, &CW.Config.LogFont, &Size)) {
		NONCLIENTMETRICSW Metrics;
		Metrics.cbSize = sizeof(Metrics);
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(Metrics), &Metrics, 0);
		CW.Config.LogFont = Metrics.lfMessageFont;
	}

	CW.UI.Font = CreateFontIndirectW(&CW.Config.LogFont);
	CW.UI.BackgroundColourBrush = CreateSolidBrush(CW.Config.BackgroundColour);
	return;
}

void CWSaveConfig(void) {
	return;
}
