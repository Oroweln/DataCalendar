#include "framework.h"

RECT buttonrectd = { 0 };
HWND buttondates[32] = { 0 };
int datemonthindex = 0;
int datindice = 0;

LRESULT CALLBACK ButtonProcD(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/*32 child button windows */
LRESULT CALLBACK DatesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int cxClient = 0, cyClient = 0, cxChar = 0, cxCaps = 0, cyChar = 0;
	PAINTSTRUCT ps;
	LPWSTR dates[32] = { TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9"), TEXT("10"), TEXT("11"), TEXT("12"), TEXT("13"), TEXT("14"), TEXT("15"), TEXT("16"), TEXT("17"), TEXT("18"), TEXT("19"), TEXT("20"), TEXT("21"), TEXT("22"), TEXT("23"), TEXT("24"), TEXT("25"),TEXT("26"),TEXT("27"),TEXT("28"),TEXT("29"),TEXT("30"),TEXT("31") };
	TEXTMETRIC tm;



	switch (message)
	{
	case WM_CREATE:

		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);

		for (TrustedIndex = 0; TrustedIndex < 31; TrustedIndex++)
		{
			buttonDCreationRoutine(cyChar, buttondates, hwnd, &dates[TrustedIndex]);
		}

		return 0;

	case WM_SIZE:

		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		return 0;
	case WM_COMMAND:
		datemonthindex = wParam;
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	case WM_SHOWWINDOW:
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		RECT WindowSize = { 0,0,0,0 };
		GetClientRect(hwnd, &WindowSize);
		HBRUSH hBrush = CreateSolidBrush(RGB(0, 255, 255));
		FillRect(hdc, &WindowSize, hBrush);
		DeleteObject(hBrush);
		int currentmonth = datemonthindex;
		if (datemonthindex > 11)
			currentmonth = datemonthindex - 11;
		EndPaint(hwnd, &ps);
		datindice = 0;
		for (TrustedIndex = 0; TrustedIndex < 31; TrustedIndex++)
		{
			if (TrustedIndex % 7 == 0 && TrustedIndex != 0)
				datindice += 3;
			if (buttondates[TrustedIndex] != 0)
				SendMessage(buttondates[TrustedIndex], WM_PAINT, 0, 0);
			datindice += 3;
			int fuckkillyourself = datindice + 24;
		}

		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL buttonDCreationRoutine(int cyChar, HWND* buttonarray, HWND hwnd, LPWSTR* Dates)
{

	buttonrectd.left = 0;
	buttonrectd.top = DatesRect.top;
	buttonrectd.right = DatesRect.right/28;
	buttonrectd.bottom = cyChar * 3;
	buttonarray[TrustedIndex] = CreateButtonD(*Dates, buttonrectd, hwnd, TrustedIndex);

	if (buttonarray != NULL)
		return TRUE;

	else
		return FALSE;
}


HWND CreateButtonD(LPWSTR szString, RECT Size, HWND hwnd, int index)
{
	static BOOL firsttime = TRUE;
	int static lindex = 1;
	WNDCLASSEX buttonclassd =
	{
	buttonclassd.cbSize = sizeof(buttonclassd),
	buttonclassd.style = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW,
	buttonclassd.lpfnWndProc = ButtonProcD,
	buttonclassd.cbClsExtra = 0,
	buttonclassd.cbWndExtra = sizeof(int),
	buttonclassd.hInstance = NULL,
	buttonclassd.hIcon = NULL,
	buttonclassd.hCursor = NULL,
	buttonclassd.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
	buttonclassd.lpszMenuName = NULL,
	buttonclassd.lpszClassName = TEXT("Button class d"),
	buttonclassd.hIconSm = NULL,
	};

	if (firsttime == TRUE)
	{
		RegisterClassEx(&buttonclassd);
		firsttime = FALSE;
	}
	int linefact = 0;
	if(index != 0)
		linefact = index / 7;
	HWND hwndButtond;
	//positioning here doesnt matter, we will do it right at WM_MOVE.

	hwndButtond = CreateWindow(TEXT("Button class d"), szString, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_TEXT | BS_CENTER, Size.left+(index-linefact*7)*Size.right*2, linefact*Size.bottom*2, Size.right, Size.bottom, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
	Button_Enable(hwndButtond, TRUE);
	Button_SetText(hwndButtond, szString);
	SetWindowLongPtr(hwndButtond, GWLP_USERDATA, lindex);
	lindex = GetWindowLongPtr(hwndButtond, GWLP_USERDATA);
	lindex++;
	if (hwndButtond)
		return hwndButtond;
	else
		return NULL;

}

LRESULT CALLBACK ButtonProcD(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps = { 0 };
	HDC hdc = NULL;
	HFILE hFile;
	char charbuffer[4] = { 0 };
	RECT localrect = { 0 };
	static BOOL dateflags[32];
	HBRUSH hBrush;
	SYSTEMTIME temptime = { 0 };
	static int lindex = 0;

	switch (message)
	{
	case WM_LBUTTONDOWN:
		GetClientRect(hwnd, &localrect);
		hdc = GetDC(hwnd);
		hBrush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hdc, &localrect, hBrush);
		ReleaseDC(hwnd, hdc);
		DeleteObject(hBrush);
		dateflags[lindex] = FALSE;
		lindex = GetWindowLongPtr(hwnd, GWLP_USERDATA);//after counting 12 one is skipped apperently.
		dateflags[lindex] = TRUE;
		InvalidateRect(GetParent(hwnd), NULL, TRUE);
		GetSystemTime(&temptime);
		int dateyear = temptime.wYear - 50;
		DWORD32 messagevalue = 0;
		if (datemonthindex % 12 == 0)
			messagevalue = 12;
		else
			messagevalue = datemonthindex % 12;
		messagevalue += lindex << 16;
		DWORD32 lmessagevalue = 12120;
		lmessagevalue += ((datemonthindex-1) / 12 + dateyear) << 16; //store year in highword lparam for sendmessage below
		datechangecheck = TRUE;
		SendMessage(TextBoxHwnd, WM_SETFOCUS, messagevalue, lmessagevalue);


		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		OFSTRUCT oFStruct = { 0 };
		OVERLAPPED overlapstruct = { 0 };
		overlapstruct.Offset = max(0, ((datemonthindex * 150) - 150))+24+datindice;

		hBrush = CreateSolidBrush(RGB(55, 60, 120));
		FillRect(hdc, &ps.rcPaint, hBrush);
		hFile = OpenFile("Localdata.txt", &oFStruct, OF_READ);
		ReadFile(hFile, charbuffer, 3, NULL, &overlapstruct);
		TextOutA(hdc, 0, 0, charbuffer, 3);
		DeleteObject(hBrush);
		int i = GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (dateflags[i] == TRUE)
		{
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			FillRect(hdc, &ps.rcPaint, hBrush);
			DeleteObject(hBrush);
		}

		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		return DestroyButton(hwnd);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}