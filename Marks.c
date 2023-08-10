#include "framework.h"

HWND buttonmarks[2];
RECT buttonrectmk = { 0 };

LRESULT CALLBACK ButtonProcMk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MarkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int static cxClient = 0, cyClient = 0, cyChar=0, cxCaps=0, cxChar=0;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	LPWSTR Marktxt[2] = { TEXT("Color selection"), TEXT("Shape Selection")};
	switch (message)
	{
	case WM_CREATE:

		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);

		for (TrustedIndex = 0; TrustedIndex < 2; TrustedIndex++)
		{
			buttonMkCreationRoutine(buttonmarks, hwnd, &Marktxt[TrustedIndex]);
		}

	case WM_SIZE:

		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);


		return 0;
	case WM_SHOWWINDOW:
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		RECT WindowSize = { 0,0,0,0 };
		GetClientRect(hwnd, &WindowSize);
		HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 255));
		FillRect(hdc, &WindowSize, hBrush);
		DeleteObject(hBrush);

		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL buttonMkCreationRoutine(HWND* markarray, HWND hwnd, LPWSTR* Marks)
{
	buttonrectmk.left = MarksRect.right/10;
	buttonrectmk.top = MarksRect.bottom/5;
	buttonrectmk.right = MarksRect.right-MarksRect.right/5;
	buttonrectmk.bottom = MarksRect.bottom/8*2;
	markarray[TrustedIndex] = CreateButtonMk(*Marks, buttonrectmk, hwnd, TrustedIndex);

	if (markarray != NULL)
		return TRUE;

	else
		return FALSE;
}

HWND CreateButtonMk(LPWSTR szString, RECT Size, HWND hwnd, int index)
{
	static BOOL firsttime = TRUE;
	WNDCLASSEX buttonclassmk =
	{
	buttonclassmk.cbSize = sizeof(buttonclassmk),
	buttonclassmk.style = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW,
	buttonclassmk.lpfnWndProc = ButtonProcMk,
	buttonclassmk.cbClsExtra = 0,
	buttonclassmk.cbWndExtra = 0,
	buttonclassmk.hInstance = NULL,
	buttonclassmk.hIcon = NULL,
	buttonclassmk.hCursor = NULL,
	buttonclassmk.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
	buttonclassmk.lpszMenuName = NULL,
	buttonclassmk.lpszClassName = TEXT("Button class mk"),
	buttonclassmk.hIconSm = NULL,
	};

	if (firsttime == TRUE)
	{
		RegisterClassEx(&buttonclassmk);
		firsttime = FALSE;
	}
	HWND hwndButtonmk;
	//positioning here doesnt matter, we will do it right at WM_MOVE.

	hwndButtonmk = CreateWindow(TEXT("Button class mk"), szString, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_TEXT | BS_CENTER, Size.left, Size.bottom/2+Size.bottom*2*index, Size.right, Size.bottom, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
	Button_Enable(hwndButtonmk, TRUE);
	Button_SetText(hwndButtonmk, szString);
	if (hwndButtonmk)
		return hwndButtonmk;
	else
		return NULL;

}

LRESULT CALLBACK ButtonProcMk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps = { 0 };
	HDC hdc = NULL;
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		HBRUSH hBrush = CreateSolidBrush(RGB(55, 222, 120));
		FillRect(hdc, &ps.rcPaint, hBrush);
		EndPaint(hwnd, &ps);
		DeleteObject(hBrush);
		return 0;
	case WM_DESTROY:
		return DestroyButton(hwnd);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}