#include "framework.h"

void * SafeCalloc(size_t size, int bytes)
{
	assert(bytes > 0);
	void* pointer = { 0 };
	if (pointer != NULL)
		CrashDumpFunction(66, 1);
	pointer = calloc(size, (size_t)bytes);
	if (pointer == NULL)
		CrashDumpFunction(67, 1);
	return pointer;
}

void GetWindowRectSafe(HWND hwnd, RECT* rectval)
{
	DWORD myflag = FALSE;
	SetLastError(0);
	myflag = (DWORD)GetWindowRect(hwnd, rectval);
	if (myflag == FALSE)
	{
		myflag = GetLastError();
		CrashDumpFunction(920, 0);
	}
}

void MapWindowPointsSafe(HWND hWndFrom, HWND hWndTo,LPPOINT lpPoints,UINT cPoints)
{
	SetLastError(0);
	long long returnval = MapWindowPoints(hWndFrom, hWndTo, lpPoints, cPoints);
	if (returnval == FALSE)
	{
		returnval = (int)GetLastError();
		if (returnval != 0)
			CrashDumpFunction(927, 0);
	}
}

void UpdateWindowSafe(HWND hwnd)
{
	if (FALSE == UpdateWindow(hwnd))
		CrashDumpFunction(33, 0);
}

void MoveWindowSafe(HWND hwnd, int x, int y, int width, int height, BOOL repaint)
{
	if(FALSE == MoveWindow(hwnd, x, y, width, height, repaint))
	{
		DWORD fgdg = GetLastError();
		if(fgdg > 0)
			CrashDumpFunction(40, 0);
	}
}

void InvalidateRectSafe(HWND hwnd, RECT * lpRect, BOOL bErase)
{
	if (InvalidateRect(hwnd, lpRect, bErase) == FALSE)
	{
		CrashDumpFunction(48, 0);
	}
}

HDC safeGetDC(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	if (hdc == NULL)
		CrashDumpFunction(56, 0);
	return hdc;
}

void SafewriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumbrofBytesToWrite, LPDWORD lpNumberofbytestobewritten, LPOVERLAPPED lpoverlapped)
{
	if(FALSE == WriteFile(hFile, lpBuffer, nNumbrofBytesToWrite, lpNumberofbytestobewritten, lpoverlapped))
	{
		CrashDumpFunction(63, 0);
	}
}

void SafeCloseHandle(HANDLE handle)
{
	if (CloseHandle(handle) == FALSE)
		CrashDumpFunction(71, 0);
}

void SafetGetClientRect(HWND hwnd, LPRECT lpRect)
{
	if (0 == GetClientRect(hwnd, lpRect))
		CrashDumpFunction(77, 0);
}

HBRUSH SafetCreateSolidBrush(COLORREF rgb)
{
	HBRUSH brush = CreateSolidBrush(rgb);
	if (brush == NULL)
		CrashDumpFunction(84, 0);
	return brush;
}

void SafeFillRect(HDC hdc, RECT * lprc, HBRUSH hbr)
{
	if (0 == FillRect(hdc, lprc, hbr))
		CrashDumpFunction(88, 0);
}

void SafeDeleteObject(HGDIOBJ ho)
{
	if (0 == DeleteObject(ho))
		CrashDumpFunction(97, 0);
}

void SafeReleaseDC(HWND hwnd, HDC hdc)
{
	if (0 == ReleaseDC(hwnd, hdc))
		CrashDumpFunction(103, 0);
}

int SafeGetWindowLongPtr(HWND hwnd, int nindex)
{
	SetLastError(0);
	LONG_PTR returnval = GetWindowLongPtrW(hwnd, nindex);
	DWORD lasterror =  GetLastError();
	if (returnval == 0 && lasterror != 0)
		CrashDumpFunction(109, 0);
	return (int)returnval;
}

void SafeSetBkColor(HDC hdc, COLORREF color)
{
	if (CLR_INVALID == SetBkColor(hdc, color))
		CrashDumpFunction(115, 0);
}

void SafeSetTextColor(HDC hdc, COLORREF color)
{
	if (CLR_INVALID == SetTextColor(hdc, color))
		CrashDumpFunction(115, 0);
}

void SafeSelectObject(HDC hdc, HGDIOBJ h)
{
	if (NULL == SelectObject(hdc, h) || HGDI_ERROR == SelectObject(hdc,h))
		CrashDumpFunction(127, 0);
}

void SafeTextOutA(HDC hdc, int x, int y, LPCSTR lpString, int c)
{
	if (0 == TextOutA(hdc, x, y, lpString, c))
		CrashDumpFunction(133, 0);
}

void SafeFrameRect(HDC hdc, RECT* myrect, HBRUSH hBrush)
{
	if (0 == FrameRect(hdc, myrect, hBrush))
		CrashDumpFunction(139, 0);
}

void SafeSetFocus(HWND hwnd)
{
	DWORD error = 0;
	SetLastError(0);
	if (NULL == SetFocus(hwnd))
	{
		error = GetLastError();
		if(error != 0)
			CrashDumpFunction(145, 0);
	}
}

HWND SafeGetParent(HWND hwnd)
{
	HWND returnval = GetParent(hwnd);
	if (returnval == NULL)
		CrashDumpFunction(151, 0);
	return returnval;
}

void safe_itoa_s(int num, char* buff, size_t buffercount, int radix)
{
	if (EINVAL == _itoa_s(num, buff, buffercount, radix))
		CrashDumpFunction(157, 0);
}

HWND SafeGetDlgItem(HWND hwnd, int DLGITEMID)
{
	HWND returnval = GetDlgItem(hwnd, DLGITEMID);
	if (returnval == NULL)
		CrashDumpFunction(163, 0);
	else
		return returnval;
}

void DrawTextASafe(HDC hdc, LPCSTR lpString, int c, LPRECT lprc, UINT format)
{
	if (0 == DrawTextA(hdc, lpString, c, lprc, format))
		CrashDumpFunction(169, 0);
}

void SafeDestroyWindow(HWND hwnd)
{
	if (0 == DestroyWindow(hwnd))
		CrashDumpFunction(175, 0);
}

long Safestrtol(char* str, char** endptr, int base)
{
	long returnval = strtol(str, endptr, base);
	if (((long long)endptr-(long long)str) > 0)
		CrashDumpFunction(181, 0);
	return returnval;
}

void SafeSetEndOfFile(HANDLE hFile)
{
	if (0 == SetEndOfFile(hFile))
		CrashDumpFunction(190, 0);
}

UINT SafeGetDlgItemInt(HWND hwnd, int DLGITEMID, BOOL* lpTranslated, BOOL bSigned)
{
	UINT val = 0;
	val = GetDlgItemInt(hwnd, DLGITEMID, lpTranslated, bSigned);
	if (FALSE == lpTranslated)
		CrashDumpFunction(198, 0);
	return val;
}

void SafeStrToInt64ExA(char* str, int dwFlags, LONGLONG* pll)
{
	if (0 == StrToInt64ExA(str, dwFlags, pll) && (str[0] != 0))
	{
		CrashDumpFunction(205, 0);
	}
}

void SafeSetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod))
		CrashDumpFunction(211, 0);
}

void SafeSetWindowTextA(HWND hwnd, LPCSTR lpString)
{
	if (0 == SetWindowTextA(hwnd, lpString))
		CrashDumpFunction(217, 0);
}

void SafeSetDlgItemInt(HWND hwnd, int DLGITEMID, UINT uValue, BOOL bSigned)
{
	if (0 == SetDlgItemInt(hwnd, DLGITEMID, uValue, bSigned))
		CrashDumpFunction(223, 0);
}

void SafeEndDialog(HWND hwnd, int nResult)
{
	if (0 == EndDialog(hwnd, nResult))
		CrashDumpFunction(229, 0);
}

LONG_PTR SafeSetWindowLongPtr(HWND hwnd, int nindex, LONG_PTR dwNewLong)
{
	LONG_PTR returnval = 0;
	SetLastError(0);
	if ((0 == SetWindowLongPtrA(hwnd, nindex, dwNewLong)) && (GetLastError() != 0))
		CrashDumpFunction(235, 0);
	else
		return returnval;
}