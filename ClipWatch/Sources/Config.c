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

#pragma warning(suppress: 4100)
INT_PTR CALLBACK CWSettingsDialog(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam) {
	switch(Message) {
	case WM_INITDIALOG:
		SendMessageW(Dialog, WM_SETICON, ICON_BIG, (LPARAM)CW.UI.Icon);
		SendMessageW(Dialog, WM_SETICON, ICON_SMALL, (LPARAM)CW.UI.Icon);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		SetBkColor((HDC)WParam, CW.Config.BackgroundColour);
		return TRUE;

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

void CWLoadConfig(void) {
	HKEY restrict Key;
	RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ClipWatch", 0, NULL, 0, KEY_QUERY_VALUE, NULL, &Key, NULL);

	dword Size = sizeof(dword), CastingBuffer;
	if(!RegGetValueW(Key, NULL, L"Fade-In Duration", RRF_RT_DWORD, NULL, &CastingBuffer, &Size)) CW.Config.FadeInDuration = CastingBuffer > MAXWORD ? MAXWORD : (word)CastingBuffer;
	else CW.Config.FadeInDuration = 1250;

	Size = sizeof(dword);
	if(!RegGetValueW(Key, NULL, L"Mid-Fade Delay", RRF_RT_DWORD, NULL, &CastingBuffer, &Size)) CW.Config.MidFadeDelay = CastingBuffer > MAXWORD ? MAXWORD : (word)CastingBuffer;
	else CW.Config.MidFadeDelay = 500;

	Size = sizeof(dword);
	if(!RegGetValueW(Key, NULL, L"Fade-Out Duration", RRF_RT_DWORD, NULL, &CastingBuffer, &Size)) CW.Config.FadeOutDuration = CastingBuffer > MAXWORD ? MAXWORD : (word)CastingBuffer;
	else CW.Config.FadeOutDuration = 1250;

	Size = sizeof(dword);
	if(!RegGetValueW(Key, NULL, L"Text Colour", RRF_RT_DWORD, NULL, &CW.Config.TextColour, &Size)) CW.Config.TextColour = RGB(0xFF, 0xFF, 0xFF);

	Size = sizeof(dword);
	if(!RegGetValueW(Key, NULL, L"Background Colour", RRF_RT_DWORD, NULL, &CW.Config.BackgroundColour, &Size)) CW.Config.BackgroundColour = RGB(0x33, 0x33, 0x33);

	Size = sizeof(dword);
	if(!RegGetValueW(Key, NULL, L"Flags", RRF_RT_DWORD, NULL, &CastingBuffer, &Size)) CW.Config.Flags = (byte)CastingBuffer;
	else CW.Config.Flags = CW_CFG_POS_TO_NEAREST;
	
	NONCLIENTMETRICSW Metrics;
	Metrics.cbSize = sizeof(Metrics);
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(Metrics), &Metrics, 0);
	CW.Config.LogFont = Metrics.lfMessageFont;
	CW.UI.Font = CreateFontIndirectW(&CW.Config.LogFont);
	return;
}

void CWSaveConfig(void) {
	return;
}
