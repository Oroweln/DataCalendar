#include "framework.h"

HWND buttonmarks[3] = { 0 };
RECT buttonrectmk = { 0 };
HFONT localfont = { 0 };

RGBQUAD * dummydata = NULL;
LRESULT CALLBACK ButtonProcMk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TempBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void Timerproc(HWND hwnd, UINT message, UINT_PTR wParam, DWORD lParam);
void marksfilepresent(int bithsize, HANDLE hFile);
RECT buttonswindow(HWND hwnd, POINT mypt);
void DatesChangeWindow(HWND hwnd, POINT mypt, RECT TempRect2);
void ShapesChangeButton(int indexselected, HWND hwnd);
void RGBDatatxtUpdate(char* filebuffer, int realindex, int selectedcolor, int filesize, HANDLE hFile, OVERLAPPED overlapstruct);
RECT ButtonsBarFunction(RECT LocalRect, HDC hdc, HWND hwnd, int* step, RECT DataUpdateRect);
RECT ButtonsDataWipeandBelow(char* mytimedata, RECT LocalRect, RECT TempRect, HDC hdc, HBRUSH frameBrush, int* step, RECT DataUpdateRect);
COLORREF inputsignalcolor = RGB(255, 100, 50);

LRESULT CALLBACK MarkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int static cxClient = 0, cyClient = 0, cyChar=0, cxCaps=0, cxChar=0;
	char* filebuffer = NULL;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	HANDLE MarksFile = { 0 };
	HBRUSH hBrush = { 0 };
	static BOOL retardflag = FALSE;
	int yearmonthindex = 0;

	OVERLAPPED overlapstruct = { 0 };
	LPWSTR Marktxt[3] = { TEXT("Color selection"), TEXT("Shape Selection"), TEXT("Buttons")};
	switch (message)
	{
	case WM_CREATE:
		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);

		for (TrustedIndex = 0; TrustedIndex < 3; TrustedIndex++)
			buttonMkCreationRoutine(buttonmarks, hwnd, &Marktxt[TrustedIndex]);
		char tempstring[1000] = { 0 };
		sprintf_s(tempstring, 1000, "%sMarksData.txt", theworkingdirectory);
		MarksFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if(MarksFile!=NULL)
			CloseHandle(MarksFile);
		//creation and/or Read of RGBDATA.txt
		for (int zg = 0; zg < 1000; zg++)
			tempstring[zg] = '\0';
		sprintf_s(tempstring, 1000, "%sRGBData.txt", theworkingdirectory);
		HANDLE hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		int bithsize = GetFileSize(hFile, NULL);
		if (bithsize <= 0)//file not present, fill it with basic values
		{
			filebuffer = calloc(2000, sizeof(char));
			if (filebuffer == NULL)
			{
				CrashDumpFunction(101, 1);
				return FALSE;
			}
			ColorSelection[0] = RGB(255, 255, 255);
			ColorSelection[1] = RGB(255, 0, 0);
			ColorSelection[2] = RGB(0, 255, 0);
			ColorSelection[3] = RGB(0, 0, 255);
			ColorSelection[4] = RGB(0, 0, 0);
			for (int m = 5; m < 100; m++)
			{
				ColorSelection[m] = RGB(rand() % 256, rand() % 256, rand() % 256);
			}
			int charsnumber = 0;
			for (int i = 1; i < 101; i++)
			{
				charsnumber += sprintf_s(filebuffer + charsnumber, (1500 * sizeof(char))-charsnumber, "%d=%lu|", i, (DWORD)ColorSelection[i-1]);
			}
			OVERLAPPED overlapstructshit = { 0 };
			WriteFile(hFile, filebuffer, charsnumber, NULL, &overlapstructshit);
			free(filebuffer);
			filebuffer = NULL;
			CloseHandle(hFile);
		}
		else//filepresent
			marksfilepresent(bithsize, hFile);
		//Read all bitmaps and store them in an array of bitmaps
		HANDLE BitmapHandle = { 0 };
		char directoryplace[1000] = { 0 };
		int offset = 0;
		for (int i = 0; i < 100; i++)
		{
			offset = GetModuleFileNameA(NULL, directoryplace, 1000);
			for (int f = offset; f > 0 && directoryplace[f] != '\\'; f--)
			{
				directoryplace[f] = '\0';
				offset = f;
			}
			sprintf_s(directoryplace + offset, (size_t)(1000 - offset), "%i.bmp", i);
			BitmapHandle = LoadImageA(NULL, directoryplace, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);//Load image to get BITMAP handle
			if(GetLastError() == 0)
				MyBitmapsArray[i] = BitmapHandle;
			else
			{
				sprintf_s(directoryplace + offset, (size_t)(1000 - offset), "%i.ico", i);
				BitmapHandle = LoadImageA(NULL, directoryplace, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);//Load image to get BITMAP handle
				if (GetLastError() == 0)
					MyBitmapsArray[i] = BitmapHandle;
			}
		}
		return 0;

	case WM_SIZE:

		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		return 0;
	case WM_COMMAND://???
		yearmonthindex = LOWORD(wParam);
		for (int zg = 0; zg < 1000; zg++)
			tempstring[zg] = '\0';
		sprintf_s(tempstring, 1000, "%sMarksData.txt", theworkingdirectory);
		MarksFile = CreateFileA(tempstring, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		int filesize = GetFileSize(MarksFile, NULL);
		int returnval = 0;
		if(filesize>16)//enough data for dataset is present
		{
			static char* Marksbuffer = 0;
			Marksbuffer = calloc(filesize, sizeof(char));
			returnval = ReadFile(MarksFile, Marksbuffer, filesize, NULL, &overlapstruct);
			free(Marksbuffer);
			Marksbuffer = NULL;
			if (returnval == 0)
			{
				CrashDumpFunction(102, 0);
				return -1;
			}
			int datetosend = 0;
			int currentmonth = 0;
			int currentyear = 0;
			currentmonth = yearmonthindex;
			if (currentmonth > 12)
				currentmonth %= 12;
			if (currentmonth == 0)
				currentmonth = 12;
			if (yearmonthindex >= 12)
				currentyear = (yearmonthindex / 12) + yearzero;
			else
				currentyear = yearzero;
			datetosend = currentyear << 11;
			datetosend += currentmonth << 7;
			MarkPresenceCheck(MarksFile, filesize, datetosend, FALSE, NULL, NULL, NULL);//read all marks data and set according colors if color present
		}
		CloseHandle(MarksFile);

		return 0; 
	case MARK_DATESIGNAL:
		retardflag = TRUE;
		return 0;
	case WM_SHOWWINDOW:
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		RECT WindowSize = { 0,0,0,0 };
		GetClientRect(hwnd, &WindowSize);
		if (retardflag == TRUE)
		{
			hBrush = CreateSolidBrush(inputsignalcolor);
		}
		else
			hBrush = CreateHatchBrush(HS_BDIAGONAL,RGB(0,0,255));
		FillRect(hdc, &WindowSize, hBrush);
		DeleteObject(hBrush);
		RECT windorect = { 0 };
		RECT daterect = { 0 };
		RECT marksrect = { 0 };
		for (TrustedIndex = 0; TrustedIndex < 2; TrustedIndex++)
		{
			GetWindowRect(buttondates[0], &daterect);
			MapWindowPoints(NULL, DatesHwnd, (LPPOINT)&daterect, 2);
			GetWindowRect(buttonmarks[TrustedIndex], &windorect);
			MapWindowPoints(NULL, hwnd, (LPPOINT)&windorect, 2);
			GetWindowRect(hwnd, &marksrect);
			MoveWindow(buttonmarks[TrustedIndex], (daterect.right - daterect.left)*2+ (daterect.right - daterect.left)/3 + 2 *TrustedIndex * (daterect.right - daterect.left), windorect.top, (daterect.right - daterect.left) * 2, (daterect.bottom - daterect.top) * 100, TRUE);
			UpdateWindow(buttonmarks[TrustedIndex]);
		}
		MoveWindow(buttonmarks[2], 0, windorect.top, windorect.right / 3, (daterect.bottom - daterect.top) * 100, TRUE);

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
	RECT daterect = { 0 };
	GetClientRect(buttondates[0], &daterect);
	buttonrectmk.left = 0;
	buttonrectmk.top = 0;
	buttonrectmk.right = (daterect.right - daterect.left);
	buttonrectmk.bottom = (daterect.bottom - daterect.top)*100;
	markarray[TrustedIndex] = CreateButtonMk(*Marks, buttonrectmk, hwnd, TrustedIndex);

	if (markarray != NULL)
		return TRUE;

	else
	{
		CrashDumpFunction(103, 1);
		return FALSE;
	}
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
	buttonclassmk.cbWndExtra = sizeof(short int),
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
	RECT marksrect = { 0 };
	GetWindowRect(GetParent(hwnd), &marksrect);
	HWND hwndButtonmk;
	if (index == 0)
		Size.left=Size.left - Size.right/2;
	if (index == 1)
	{
		Size.left = Size.left - Size.right / 2;
	}
	//positioning here doesnt matter, we will do it right at WM_MOVE.
	hwndButtonmk = CreateWindow(TEXT("Button class mk"), szString, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_TEXT | BS_CENTER, 0, 0, Size.right * 2, Size.bottom, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
	RECT shutup = { 0 };
	GetWindowRect(hwndButtonmk, &shutup);
	Button_Enable(hwndButtonmk, TRUE);
	Button_SetText(hwndButtonmk, szString);
	SetWindowLongPtr(hwndButtonmk, GWLP_USERDATA, index);
	if (hwndButtonmk)
		return hwndButtonmk;
	else
	{
		CrashDumpFunction(95,1);
		return NULL;
	}
}

LRESULT CALLBACK ButtonProcMk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL faggot = TRUE;
	PAINTSTRUCT ps = { 0 };
	HDC hdc = NULL;
	RECT daterectn = { 0 };
	int WindowIndex = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	int static Walkamount = 0, step = 0;
	int xr, yr;
	WPARAM indexselected = 0;
	int static shapeselection = 0;
	int zDelta = 0;
	SYSTEMTIME sTime = { 0 };
	int static scrollamount[2] = { 0 };
	RECT LocalRect = { '\0 ' };
	static RECT DataUpdateRect = { 0 };
	COLORREF* colorspointer[20] = { 0 };
	switch (message)
	{
	case WM_CREATE:
		//GetClientRect(hwnd, &LocalRect);
		//Walkamount = LocalRect.bottom / 100;
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		HDC memdc = { 0 };
		HBRUSH hBrush = CreateSolidBrush(RGB(222, 255, 255));
		FillRect(hdc, &ps.rcPaint, hBrush);
		GetClientRect(hwnd, &LocalRect);
		Walkamount = LocalRect.bottom / 100;
		DeleteObject(hBrush);
		int k = 0;
		if (WindowIndex == 0)//shapes window
		{
			for (int i = (scrollamount[WindowIndex]*2); i < 100 && MyBitmapsArray[i] != NULL; i++)
			{
				if (i % 2 != 0)
				{
					//LocalRect.right = LocalRect.right / 2;
					LocalRect.left = LocalRect.right / 2;
				}
				SetStretchBltMode(hdc, HALFTONE);
				SetBrushOrgEx(hdc, 0, 0, NULL);
				memdc = CreateCompatibleDC(hdc);
				SelectObject(memdc, MyBitmapsArray[i]);
				hBrush = CreateSolidBrush(RGB(0, 100, 255));
				int g = 0;
				BITMAP mybitmap = { 0 };
				g = GetObject(MyBitmapsArray[i], sizeof(BITMAP), &mybitmap);
				g = StretchBlt(hdc, LocalRect.left, k * Walkamount, LocalRect.right/2, Walkamount, memdc, 0, 0, mybitmap.bmWidth, mybitmap.bmHeight, SRCCOPY);
				RECT temprect69 = { 0 };
				temprect69.left = LocalRect.left;
				temprect69.top = k * Walkamount;
				temprect69.right = LocalRect.right;
				temprect69.bottom = Walkamount + k * Walkamount;
				FrameRect(hdc, &temprect69, hBrush);
				DeleteObject(hBrush);
				DeleteObject(memdc);
				if (i % 2 != 0)
				{
					//LocalRect.right = LocalRect.right * 2;
					LocalRect.left = 0;
					k++;
				}
			}
		}
		if (WindowIndex == 1)//colors window
		{
			int a = 0;
			for (int i = scrollamount[WindowIndex]*2; i < 100; i++)
			{
				LocalRect.top = Walkamount * a;
				LocalRect.bottom = Walkamount * a + Walkamount;
				if (i % 2 != 0)
				{
					LocalRect.left = LocalRect.right / 2;
					LocalRect.right += LocalRect.left;
				}
				hBrush = CreateSolidBrush(ColorSelection[i]);
				FillRect(hdc, &LocalRect, hBrush);
				LocalRect.right -= LocalRect.left;
				DeleteObject(hBrush);
				if (i % 2 != 0)
				{
					LocalRect.left = 0;
					a++;
				}
			}
		}
		if (WindowIndex == 2)//Buttons bar nigga
			DataUpdateRect = ButtonsBarFunction(LocalRect, hdc, hwnd, &step, DataUpdateRect);
		EndPaint(hwnd, &ps);
		return 0;
	case TIMEUPDATE://timeupdateloop
		hdc = GetDC(hwnd);
		GetSystemTime(&sTime);
		SystemTimeToTzSpecificLocalTime(NULL, &sTime, &sTime);
		GetClientRect(hwnd, &LocalRect);
		LocalRect.bottom = LocalRect.bottom / 10/4;
		hBrush = CreateSolidBrush(RGB(40, 40, 40));
		FillRect(hdc, &LocalRect, hBrush);
		DeleteObject(hBrush);
		char mytimedata[1000] = { 0 };
		sprintf_s(mytimedata, 1000, " Year: %u\n Month: %u\n Day: %u\n Hour: %u\n Minute: %u\n Second: %u\n Milisecond: %u", sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond, sTime.wMilliseconds);
		SetBkColor(hdc, RGB(40, 40, 40));
		SetTextColor(hdc, RGB(150, 150, 150));
		DrawTextA(hdc, mytimedata, -1, &LocalRect, DT_TOP | DT_LEFT);
		ReleaseDC(hwnd, hdc);
		return 0;
	case DATEUPDATETICK:
		hdc = GetDC(hwnd);
		char  memdata[1000] = { 0 };
		memset(memdata, 0, sizeof(char) * 1000);
		hBrush = CreateSolidBrush(RGB(200, 120, 80));
		FillRect(hdc, &DataUpdateRect, hBrush);
		DeleteObject(hBrush);
		sprintf_s(memdata, 1000, "remaining: \n%u", TextHeapRemaining);
		DrawTextA(hdc, memdata, -1, &DataUpdateRect, DT_LEFT);
		//Data remaining
		memset(memdata, 0, sizeof(char) * 1000);
		LocalRect.top = DataUpdateRect.top + step + step / 2;
		LocalRect.bottom = DataUpdateRect.bottom + step + step / 2;
		LocalRect.left = DataUpdateRect.left;
		LocalRect.right = DataUpdateRect.right;
		hBrush = CreateSolidBrush(RGB(33, 200, 3));
		FillRect(hdc, &LocalRect, hBrush);
		DeleteObject(hBrush);
		sprintf_s(memdata, 1000, "avalible: \n%lld", pchinputbuffermemory);
		DrawTextA(hdc, memdata, -1, &LocalRect, DT_LEFT);
		ReleaseDC(hwnd, hdc);
		return 0;
	case WM_LBUTTONDOWN:
		xr = LOWORD(lParam);
		yr = HIWORD(lParam);
		GetClientRect(buttondates[0], &daterectn);
		indexselected = 0;
		if (yr > daterectn.bottom)
			indexselected = (WPARAM)((yr / daterectn.bottom) * 2);
		if (xr > daterectn.right)
			indexselected++;
		if (WindowIndex == 0)//shapes signaling
		{
			indexselected += (WPARAM)(scrollamount[WindowIndex] * 2 + 1);
			SendMessage(DatesHwnd, MARK_SIGNAL, selecteddate, indexselected);
		}
		if (WindowIndex == 1)//color signaling
		{
			indexselected += (WPARAM)(scrollamount[WindowIndex] * 2);
			WPARAM paramy = 0;
			GetClientRect(hwnd, &LocalRect);
			paramy = selecteddate + ((IDCOLORS+indexselected) << 24);
			SendMessage(DatesHwnd, MARK_SIGNAL, paramy, 0);
		}
		if (WindowIndex == 2)//buttons cool shit!
		{
			POINT mypt = { 0 };
			mypt.x = GET_X_LPARAM(lParam);
			mypt.y = GET_Y_LPARAM(lParam);
			RECT TempRect2 = { 0 };
			TempRect2 = buttonswindow(hwnd, mypt);
			//Current date selected, manual date selection
			TempRect2.top = TempRect2.bottom;
			TempRect2.bottom = TempRect2.bottom * 2;
			if (PtInRect(&TempRect2, mypt))
			{
				DatesChangeWindow(hwnd, mypt, TempRect2);//deals with dateschanges window in buttons part
			}
			TempRect2.top = TempRect2.bottom;
			TempRect2.bottom = TempRect2.top + step;
			if (PtInRect(&TempRect2, mypt))//data wipe
			{
				hdc = GetDC(hwnd);
				InvertRect(hdc, &TempRect2);
				ReleaseDC(hwnd, hdc);
				if (MessageBox(GetParent(DatesHwnd), TEXT("This will delete all data for the day, are you sure?"), TEXT("Data Wipe"), MB_OKCANCEL | MB_ICONWARNING) == IDOK)
				{
					int datedate1 = selecteddate;
					int year1 = datedate1 >> 11;
					int month1 = (datedate1 & 0x780) >> 7;
					int day1 = (datedate1 & 0x7F);
					if (selecteddate != 0)
						RangedDataWipe(month1, month1, year1, year1, day1, day1);//deletes only one day, the day selected.
				}
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
			}
			TempRect2.top += step;
			TempRect2.bottom += step;
			if (PtInRect(&TempRect2, mypt))//word wrap
			{
				hdc = GetDC(hwnd);
				InvertRect(hdc, &TempRect2);
				ReleaseDC(hwnd, hdc);
				if (MessageBox(GetParent(DatesHwnd), TEXT("This will warp the text, by appending newlines to the text at places where it leaves the textbox, are you sure you want to do this?"), TEXT("Text Warp"), MB_OKCANCEL | MB_ICONWARNING) == IDOK)
				{
					SendMessage(TextBoxHwnd, WM_SETFOCUS, 0, 0);
					SendMessage(TextBoxHwnd, WORDWRAP, 0, 0);//send message with flag that will send wordwrap function
				}
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
			}
			TempRect2.top += step;
			TempRect2.bottom += step;
			if (PtInRect(&TempRect2, mypt))//data save
			{
				hdc = GetDC(hwnd);
				InvertRect(hdc, &TempRect2);
				ReleaseDC(hwnd, hdc);
				if (selecteddate != 0)//
				{
					if (MessageBox(GetParent(DatesHwnd), TEXT("This will overwrite any data already present for the day selected, are you sure?"), TEXT("Save Data"), MB_OKCANCEL | MB_ICONWARNING) == IDOK)
					{
						SendMessage(TextBoxHwnd, WM_KEYDOWN, VK_CONTROL, 0);
						SendMessage(TextBoxHwnd, WM_KEYDOWN, 0x53, 0);
					}
				}
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
			}
		}
		return 0;
	case WM_RBUTTONDOWN://right click means change color or shape of a given window
		xr = LOWORD(lParam);
		yr = HIWORD(lParam);
		GetClientRect(buttondates[0], &daterectn);
		if (yr > daterectn.bottom)
		{
			indexselected = (WPARAM)((yr / daterectn.bottom) * 2);
		}
		if (xr > daterectn.right)
			indexselected++;
		if (WindowIndex == 0)//shapes change
		{
			ShapesChangeButton((int)indexselected, hwnd);
		}
		if (WindowIndex == 1)//color change
		{
			GetClientRect(hwnd, &LocalRect);
			COLORREF selectedcolor = { 0 };
			CHOOSECOLORA colordlg = {
			colordlg.lStructSize = sizeof(CHOOSECOLOR),
			colordlg.hwndOwner = GetParent(DatesHwnd),
			colordlg.hInstance = (HWND)GetModuleHandle(NULL),
			colordlg.rgbResult = selectedcolor,
			colordlg.lpCustColors = (void*)colorspointer,
			colordlg.Flags = CC_SOLIDCOLOR|CC_FULLOPEN,
			colordlg.lCustData = 0,
			colordlg.lpfnHook = NULL,
			colordlg.lpTemplateName = NULL
			};
			ChooseColorA(&colordlg);
			int realindex = 0;
			int returnval = 0;
			returnval = CommDlgExtendedError();
			if (returnval > 0)
				CrashDumpFunction(104,0);
			selectedcolor = colordlg.rgbResult;
			indexselected += (WPARAM)(scrollamount[WindowIndex] * 2);
			ColorSelection[indexselected] = selectedcolor;
			realindex = (int)indexselected + 1;
			HANDLE hFile = { 0 };
			char tempstring[1000] = { 0 };
			sprintf_s(tempstring, 1000, "%sRGBData.txt", theworkingdirectory);
			hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == 0)
			{
				CrashDumpFunction(105, 1);
				return 0;
			}
			int filesize = GetFileSize(hFile, NULL);
			OVERLAPPED overlapstruct = { 0 };
			char* filebuffer = calloc((size_t)(filesize + 1), sizeof(char));
			if (filebuffer == NULL)
			{
				CloseHandle(hFile);
				CrashDumpFunction(106, 1);
				return 0;
			}
			bool fag = ReadFile(hFile, filebuffer, filesize, NULL, &overlapstruct);
			if (fag == 0) CrashDumpFunction(120,0);
			//append the data update to rgbdata
			RGBDatatxtUpdate(filebuffer, realindex, selectedcolor, filesize, hFile, overlapstruct);
			InvalidateRect(GetParent(TextBoxHwnd), NULL, TRUE);
			UpdateWindow(TextBoxHwnd);
		}
		return 0;
	case WM_MOUSEWHEEL:
		zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (zDelta < 0 && scrollamount[WindowIndex]>0)
		{
			scrollamount[WindowIndex]--;
		}
		if (zDelta > 0 && scrollamount[WindowIndex]<100)
		{
			scrollamount[WindowIndex]++;
		}
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
		return 0;
	case WM_DESTROY:
		DeleteObject(localfont);
		return DestroyButton(hwnd);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

#pragma warning (disable: 4100)
void Timerproc(HWND hwnd, UINT message, UINT_PTR wParam, DWORD lParam)
{
	ButtonProcMk(hwnd, TIMEUPDATE, 0, 0);
}
#pragma warning(default: 4100)

LRESULT CALLBACK TempBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//GlobalIndex = 1 means year, 2 means month, 3 means day
	HDC hdc = { 0 };
	static char* mindexspace = { 0 };
	char static numberarray[100] = { 0 };
	int static cxChar = 0, cxCaps = 0, cyChar = 0, index = 0;
	int * GlobalIndexptr = 0;
	int datedate = 0;
	static int GlobalIndex = 0;
	CREATESTRUCTA * ca = { 0 };
	TEXTMETRIC tm = { 0 };
	static RECT boxsize = { 0 };
	RECT parentclient = { 0 };
	PAINTSTRUCT ps = { 0 };
	HBRUSH commonbrush = { 0 };
	HDC godgivemecancer = { 0 };
	RECT godkillme = { 0 };
	switch (message)
	{
	case WM_CREATE:
		mindexspace = calloc(yearrange, sizeof(char));
		ca = lParam;
		GlobalIndexptr = (uintptr_t)ca->lpCreateParams;
		GlobalIndex = GlobalIndexptr[0];
		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		SIZE sizeextent = { 0 };
		memset(numberarray, 0, sizeof(char) * 100);
		index = 0;
		ReleaseDC(hwnd, hdc);
		return 0;
	case WM_SIZE:
		GetClientRect(hwnd, &boxsize);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(GetParent(DatesHwnd), &godkillme);
		godgivemecancer = GetDC(GetParent(DatesHwnd));
		TextOutA(godgivemecancer, godkillme.right / 2, godkillme.bottom / 2, "Press Enter for change to take effect", 38);
		ReleaseDC(GetParent(DatesHwnd), godgivemecancer);
		SelectFont(hdc, localfont);
		SetBkColor(hdc, RGB(150, 150, 150));
		SetTextColor(hdc, RGB(0, 0, 255));
		commonbrush = CreateSolidBrush(RGB(10, 22, 70));
		FillRect(hdc, &boxsize, commonbrush);
		TextOutA(hdc, 0, 0, numberarray, 100);
		DeleteObject(commonbrush);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_KEYDOWN:
		if (wParam >= 0x30 && wParam <= 0x39 && index < 6)//its a numeric key
		{
			numberarray[index] = '0' + (char)wParam - 0x30;//puts number as a character
			index++;
		}
		if (wParam == VK_DELETE || wParam == VK_BACK && index>0)//delete
		{
			index--;
			numberarray[index] = '\0';
		}
		if (wParam == VK_RETURN)
		{
			SetFocus(GetParent(hwnd));
			return 0;
		}
		hdc = GetDC(hwnd);
		SelectFont(hdc, localfont);
		GetTextExtentPoint32A(hdc, numberarray + index, (int)strlen(numberarray + index), &sizeextent);
		boxsize.right += sizeextent.cx;
		ReleaseDC(hwnd, hdc);
		CopyRect(&parentclient, &boxsize);
		MapWindowPoints(hwnd, GetParent(hwnd), (LPPOINT)&parentclient, 2);
		MoveWindow(hwnd, parentclient.left, parentclient.top, parentclient.right - parentclient.left, parentclient.bottom - parentclient.top, FALSE);
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
		return 0;
	case WM_KILLFOCUS://apply change
		datedate = selecteddate;
		if(GlobalIndex == 1)//yearchange
		{
			int cmonth = (datedate & 0x780) >> 7;
			int oldyear = datedate >> 11;
			datedate = datedate & 0x7ff;//zerout the year
			int year = atoi(numberarray);
			datedate += year << 11;//put the year inside datedate;
			//selecteddate = datedate;//apply the change to selecteddate and our job is done
			//mindexspace[MonthYearIndex] = FALSE;
			if ((yearzero < year) && (yearzero + year-yearzero) < yearzero + yearrange / 12)//if its in scope of year-range
			{
				int newyearmonthsindex = (12 * (year - yearzero)) + cmonth;
				int hwndfornewmyindex = 0, zerothwindowpos = 0;
				//which button of hwnd's would properly correspond to newyearsmonthindex,to whom message will be sent before updating all other things:
				zerothwindowpos = (year - yearzero) * 12+1;
				hwndfornewmyindex = newyearmonthsindex - zerothwindowpos;
				for (int i = 0; i < buttonfactor; i++,zerothwindowpos++)
				{
					SetWindowLongPtr(buttonarray[i], GWLP_USERDATA, zerothwindowpos);
				}
				SendMessageA(buttonarray[hwndfornewmyindex], WM_LBUTTONDOWN, 0, 0);//update everything
			}
		}
		if (GlobalIndex == 2)//monthchange
		{
			datedate = datedate & 0xfff87f;//zeroout the month
			int month = atoi(numberarray);
			int oldyear = datedate >> 11;
			if((month<13 && month >0) && ((month + oldyear*12-yearzero*12)<yearrange))
			{
				int zerothwindowpos = 0;
				//which button of hwnd's would properly correspond to newyearsmonthindex,to whom message will be sent before updating all other things:
				zerothwindowpos = (oldyear - yearzero) * 12+1;
				for (int i = 0; i < buttonfactor; i++, zerothwindowpos++)
				{
					SetWindowLongPtr(buttonarray[i], GWLP_USERDATA, zerothwindowpos);
				}
				SendMessageA(buttonarray[month], WM_LBUTTONDOWN, 0, 0);//update everything
			}
		}
		if (GlobalIndex == 3)//daychange
		{
			//datedate = datedate & 0xFFFF80;//zerout the day
			int day = atoi(numberarray);
			if (day < 32 && day > 0)
			{
				//datedate += day;
				//selecteddate = datedate;
				SendMessage(buttondates[day-1], WM_LBUTTONDOWN, 0, 0);
			}
		}
		InvalidateRect(GetParent(DatesHwnd), NULL, TRUE);
		UpdateWindow(GetParent(DatesHwnd));
	case WM_DESTROY:
		memset(numberarray, 0, sizeof(char) * 100);
		index = 0;
		DestroyWindow(hwnd);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void marksfilepresent(int bithsize, HANDLE hFile)
{
	OVERLAPPED overlapstruct = { 0 };
	char * filebuffer = calloc(bithsize, sizeof(char));
	if (filebuffer == NULL)
	{
		CloseHandle(hFile);
		CrashDumpFunction(90, 1);
		return;
	}
	if (hFile == NULL)
		return;
	char* pEnd = NULL;
	int colorindex = 0;
	int rgbColor = 0;
	int firstpoint = 0;
	bool totaldeath = ReadFile(hFile, filebuffer, bithsize, NULL, &overlapstruct);
	if (totaldeath == 0) return;
	for (int i = 0; i < 100; i++)
	{
		for (int k = firstpoint; k < bithsize; k++)
		{
			if (filebuffer[k] == '=')
			{
				k++; //to skip equal sign
				colorindex = strtol(filebuffer + firstpoint, &pEnd, 10);
				if (colorindex != i + 1)//unordered number or index failure 
				{
					CloseHandle(hFile);
					free(filebuffer);
					CrashDumpFunction(91,0);
					return;
				}
				rgbColor = strtol(filebuffer + k, &pEnd, 10);
				ColorSelection[i] = rgbColor;
				if (rgbColor > 9)
					k += (int)log10((double)rgbColor);//depending on the log of 10 determines how much spaces does the number take and skip them.
				firstpoint = k + 2;//to bypass the '|' sign and pass the single number that log10 dont capture
				k = bithsize;//to quit the loop
			}
		}
	}
	free(filebuffer);
	CloseHandle(hFile);
	filebuffer = NULL;
}

RECT buttonswindow(HWND hwnd, POINT mypt)
{
	RECT LocalRect = { 0 };
	HGLOBAL clipmemory = NULL;
	char* cliptext = NULL;
	RECT TempRect2 = { 0 };
	GetClientRect(hwnd, &LocalRect);
	//check live-date first
	TempRect2.top = LocalRect.top;
	TempRect2.bottom = LocalRect.bottom / 40;
	TempRect2.left = LocalRect.left;
	int puskawetathr = 0;
	TempRect2.right = LocalRect.right;
	if (PtInRect(&TempRect2, mypt))
	{//if you click live date paste the live data, put live date on clipboard
		HDC hdc = { 0 };
		hdc = GetDC(hwnd);
		InvertRect(hdc, &TempRect2);
		ReleaseDC(hwnd, hdc);
		if (!(OpenClipboard(GetParent(TextBoxHwnd))))
		{
			CrashDumpFunction(97, 1);
			return TempRect2;
		}
		EmptyClipboard();
		//allocate data memory set for use by clipboard
		clipmemory = GlobalAlloc(GMEM_MOVEABLE, (sizeof(char)) * (1000));
		if (clipmemory == NULL)
		{
			CrashDumpFunction(98, 1);
			return TempRect2;
		}
		cliptext = GlobalLock(clipmemory);
		if (cliptext == NULL)
		{
			CrashDumpFunction(99, 1);
			return TempRect2;
		}
		SYSTEMTIME lTime = { 0 };
		GetSystemTime(&lTime);
		SystemTimeToTzSpecificLocalTime(NULL, &lTime, &lTime);
		sprintf_s(cliptext, 1000, " Year: %u\n Month: %u\n Day: %u\n Hour: %u\n Minute: %u\n Second: %u\n Milisecond: %u", lTime.wYear, lTime.wMonth, lTime.wDay, lTime.wHour, lTime.wMinute, lTime.wSecond, lTime.wMilliseconds);
		GlobalUnlock(clipmemory);
		SetClipboardData(CF_TEXT, clipmemory);
		CloseClipboard();
		GetClientRect(GetParent(hwnd), &LocalRect);
		HDC faggot = GetDC(GetParent(DatesHwnd));
		TextOutA(faggot, LocalRect.right / 2, LocalRect.top / 2, "Copied Live Time!", 18);
		ReleaseDC(GetParent(DatesHwnd), faggot);
		puskawetathr = GetLastError();
		
		InvalidateRect(hwnd, &TempRect2, TRUE);
		UpdateWindow(hwnd);
		return TempRect2;
	}
	return TempRect2;
}

void DatesChangeWindow(HWND hwnd, POINT mypt, RECT TempRect2)
{
	char tempbuffer10[100] = { 0 };
	TEXTMETRIC mtm = { 0 };
	HDC hdc = GetDC(hwnd);
	SelectFont(hdc, localfont);
	GetTextMetrics(hdc, &mtm);
	ReleaseDC(hwnd, hdc);
	int cyChar = mtm.tmHeight + mtm.tmExternalLeading;
	SIZE sizeextent = { 0 };
	WNDCLASSEX Tempclass =
	{
	Tempclass.cbSize = sizeof(WNDCLASSEX),
	Tempclass.style = CS_VREDRAW | CS_DBLCLKS | CS_HREDRAW,
	Tempclass.lpfnWndProc = TempBoxProc,//make this proc
	Tempclass.cbClsExtra = 0,
	Tempclass.cbWndExtra = 0,
	Tempclass.hInstance = GetModuleHandle(NULL),
	Tempclass.hIcon = LoadIcon(NULL, IDI_APPLICATION),
	Tempclass.hCursor = LoadCursor(NULL, IDC_ARROW),
	Tempclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
	Tempclass.lpszMenuName = NULL,
	Tempclass.lpszClassName = TEXT("Tempclass"),
	Tempclass.hIconSm = NULL,
	};
	HWND tempwindow = { 0 };
	int testvar = cyChar;
	RegisterClassEx(&Tempclass);
	TempRect2.top += testvar;//only year
	TempRect2.bottom = TempRect2.top + testvar;
	int val = 0;
	if (PtInRect(&TempRect2, mypt))
	{//Year
		sprintf_s(tempbuffer10, 100, "Year: ");

		hdc = GetDC(hwnd);
		SelectFont(hdc, localfont);
		GetTextExtentPoint32A(hdc, tempbuffer10, 6, &sizeextent);
		ReleaseDC(hwnd, hdc);
		val = 1;
		tempwindow = CreateWindowExA(0,"Tempclass", "TempBox", WS_CHILD | WS_VISIBLE, sizeextent.cx, TempRect2.top, sizeextent.cx, sizeextent.cy, hwnd, NULL, NULL, &val);
		ShowWindow(tempwindow, SW_SHOW);
		UpdateWindow(tempwindow);
		SetFocus(tempwindow);
	}
	TempRect2.top += testvar * 2;
	TempRect2.bottom += testvar * 2;
	if (PtInRect(&TempRect2, mypt))
	{//Month
		sprintf_s(tempbuffer10, 100, "Month: ");
		hdc = GetDC(hwnd);
		SelectFont(hdc, localfont);
		GetTextExtentPoint32A(hdc, tempbuffer10, 7, &sizeextent);
		ReleaseDC(hwnd, hdc);
		val = 2;
		tempwindow = CreateWindowExA(0, "Tempclass", "TempBox", WS_CHILD | WS_VISIBLE, sizeextent.cx, TempRect2.top, sizeextent.cx, sizeextent.cy, hwnd, NULL, NULL, &val);
		ShowWindow(tempwindow, SW_SHOW);
		UpdateWindow(tempwindow);
		SetFocus(tempwindow);
	}
	//Date
	TempRect2.top += testvar * 2;
	TempRect2.bottom += testvar * 2;
	if (PtInRect(&TempRect2, mypt))
	{
		sprintf_s(tempbuffer10, 100, "Day: ");
		hdc = GetDC(hwnd);
		SelectFont(hdc, localfont);
		GetTextExtentPoint32A(hdc, tempbuffer10, 5, &sizeextent);
		ReleaseDC(hwnd, hdc);
		val = 3;
		tempwindow = CreateWindowExA(0, "Tempclass", "TempBox", WS_CHILD | WS_VISIBLE, sizeextent.cx, TempRect2.top, sizeextent.cx, sizeextent.cy, hwnd, NULL, NULL, &val);
		ShowWindow(tempwindow, SW_SHOW);
		UpdateWindow(tempwindow);
		SetFocus(tempwindow);
	}
}

void ShapesChangeButton(int indexselected, HWND hwnd)
{
	char filenamestring[1000] = { 0 };
	char currentdirectory[1000] = { 0 };
	OPENFILENAMEA ofna = {
	ofna.lStructSize = sizeof(OPENFILENAME),
	ofna.hwndOwner = hwnd,
	ofna.hInstance = NULL,
	ofna.lpstrFilter = "*.bmp\0\0",
	ofna.lpstrCustomFilter = NULL,
	ofna.nMaxCustFilter = 0,
	ofna.nFilterIndex = 0,
	ofna.lpstrFile = filenamestring,
	ofna.nMaxFile = 1000,
	ofna.lpstrFileTitle = NULL,
	ofna.nMaxFileTitle = 0,
	ofna.lpstrInitialDir = NULL,
	ofna.lpstrTitle = NULL,
	ofna.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
	ofna.nFileOffset = 0,
	ofna.nFileExtension = 0,
	ofna.lpstrDefExt = NULL,
	ofna.lCustData = 0,
	ofna.lpfnHook = 0,
	ofna.lpTemplateName = 0,
	ofna.pvReserved = NULL,
	ofna.dwReserved = 0,
	ofna.FlagsEx = 0
	};
	GetOpenFileNameA(&ofna);
	int offset = 0;
	offset = GetModuleFileNameA(NULL, currentdirectory, 1000);
	for (int i = offset; i > 0 && currentdirectory[i] != '\\'; i--)
	{
		currentdirectory[i] = '\0';
		offset = i;
	}
	sprintf_s(currentdirectory + offset, (size_t)(1000 - offset), "%i.bmp", indexselected);
	CopyFileA(filenamestring, currentdirectory, FALSE);
	HANDLE BitmapHandle = { 0 };
	BitmapHandle = LoadImageA(NULL, filenamestring, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);//Load image to get BITMAP handle
	if (GetLastError() == 0)
	{
		MyBitmapsArray[indexselected] = BitmapHandle;
	}
	InvalidateRect(GetParent(TextBoxHwnd), NULL, TRUE);
	UpdateWindow(TextBoxHwnd);
}

void RGBDatatxtUpdate(char * filebuffer, int realindex, int selectedcolor, int filesize, HANDLE hFile, OVERLAPPED overlapstruct)
{
	if(hFile == NULL) return;
	int colorindex = 0;
	int rgbColor = 0;
	char* endptr = NULL;
	if (filebuffer == NULL)
		return;
	for (int i = 0; i < filesize; i++)
	{
		int firstpoint = 0;
#pragma warning(push)
#pragma warning(disable: 6001)
		if (filebuffer[i] == '=')
#pragma warning(pop)
		{
			for (int p = i - 1; p >= 0 && isdigit(filebuffer[p]); p--)
			{
				firstpoint = p;
			}
			colorindex = strtol(filebuffer + firstpoint, &endptr, 10);
			i++; //to skip equal sign
			if (colorindex == realindex)
			{//writeover
				//get old number
				rgbColor = strtol(filebuffer + i, &endptr, 10);
				int logoldnumber = (int)log10((double)rgbColor) + 1;//+1 to include 1st numeral in a technical way
				int lognewnumber = (int)log10((double)selectedcolor) + 1;
				if (rgbColor < 10)//this ensures that in case of log failure due to non 10 root
					logoldnumber = 1;
				if (selectedcolor < 10)
					lognewnumber = 1;
				int totaldifference = lognewnumber - logoldnumber;
				if (totaldifference < 0)//shrink
				{
					for (int q = i; q + abs(totaldifference) < filesize; q++)//remove unecessary memory, by shrinking the buffer at the number space
					{
						filebuffer[q] = filebuffer[q + abs(totaldifference)];
					}
					SetFilePointer(hFile, totaldifference, NULL, FILE_END);
					SetEndOfFile(hFile);//shrink the file accordingly
				}
				else if (totaldifference > 0)//enlarge
				{
					void* temppoint = NULL;
					temppoint = realloc(filebuffer, (size_t)(filesize + totaldifference + 10) * sizeof(char));
					if (temppoint == NULL)
					{
						CloseHandle(hFile);
						CrashDumpFunction(110, 1);
						return;
					}
					filebuffer = temppoint;
					for (int q = filesize + totaldifference; q > i + totaldifference; q--)
					{
						filebuffer[q] = filebuffer[q - totaldifference];
					}
				}
				//now simple writeover
				int cancertetard = 0;
				if (totaldifference > 0)
					cancertetard = totaldifference;
				sprintf_s(filebuffer + i, (size_t)(lognewnumber + 1), "%d", selectedcolor);
				filebuffer[lognewnumber + i] = '|';//to replace the impotent null thanks to sprintf
				overlapstruct.Offset = i;
#pragma warning(push)
#pragma warning(disable: 6001)
				if (hFile == INVALID_HANDLE_VALUE) return;
				WriteFile(hFile, filebuffer + i, filesize - i + totaldifference, NULL, &overlapstruct);
				CloseHandle(hFile);
#pragma warning(pop)
				free(filebuffer);
				i = filesize;//to quit the loop.
			}
		}
	}
}

RECT ButtonsBarFunction(RECT LocalRect, HDC hdc, HWND hwnd, int * step, RECT DataUpdateRect)
{
	static BOOL faggot = TRUE;
	SYSTEMTIME sTime = { 0 };
	HBRUSH hBrush = { 0 };
	RECT TempRect = { 0 };
	HBRUSH frameBrush = CreateSolidBrush(RGB(0, 255, 255));
	TempRect.top = LocalRect.top;
	TempRect.bottom = LocalRect.bottom / 10;
	TempRect.left = LocalRect.left;
	TempRect.right = LocalRect.right;
	hBrush = CreateSolidBrush(RGB(100, 100, 100));
	FillRect(hdc, &TempRect, hBrush);
	DeleteObject(hBrush);

	//Live Date presented
	GetSystemTime(&sTime);
	SystemTimeToTzSpecificLocalTime(NULL, &sTime, &sTime);
	TempRect.bottom = TempRect.bottom / 4;
	hBrush = CreateSolidBrush(RGB(40, 40, 40));
	FillRect(hdc, &TempRect, hBrush);
	DeleteObject(hBrush);
	char mytimedata[1000] = { 0 };
	sprintf_s(mytimedata, 1000, " Year: %u\n Month: %u\n Day: %u\n Hour: %u\n Minute: %u\n Second: %u\n Milisecond: %u", sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond, sTime.wMilliseconds);
	SetBkColor(hdc, RGB(40, 40, 40));
	SetTextColor(hdc, RGB(150, 150, 150));
	DrawTextA(hdc, mytimedata, -1, &TempRect, DT_TOP | DT_LEFT);
	FrameRect(hdc, &TempRect, frameBrush);
	if (faggot == TRUE)
	{
		SetTimer(hwnd, 1, USER_TIMER_MINIMUM / 10, Timerproc);
		faggot = FALSE;
	}

	//Date Selected
	memset(mytimedata, 0, sizeof(char) * 1000);
	TempRect.top = TempRect.bottom;
	TempRect.bottom = TempRect.bottom * 2;
	SetBkColor(hdc, RGB(150, 150, 150));
	SetTextColor(hdc, RGB(0, 0, 255));
	SaveDC(hdc);
	localfont = CreateFontA(0, TempRect.bottom / 30, 0, 0, FW_REGULAR, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_CHARACTER_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FIXED_PITCH, NULL);
	SelectObject(hdc, localfont);
	hBrush = CreateSolidBrush(RGB(150, 150, 150));
	FillRect(hdc, &TempRect, hBrush);
	DeleteObject(hBrush);
	int datedate = selecteddate;
	int year = datedate >> 11;
	int month = (datedate & 0x780) >> 7;
	int day = (datedate & 0x7F);
	sprintf_s(mytimedata, 1000, "Day Selected:\nYear: %i\n\nMonth: %i\n\nDay: %i", year, month, day);
	DrawTextA(hdc, mytimedata, -1, &TempRect, DT_LEFT);
	FrameRect(hdc, &TempRect, frameBrush);
	RestoreDC(hdc, -1);

	DataUpdateRect = ButtonsDataWipeandBelow(mytimedata, LocalRect, TempRect, hdc, frameBrush, step, DataUpdateRect);

	return DataUpdateRect;
}

RECT ButtonsDataWipeandBelow(char * mytimedata, RECT LocalRect, RECT TempRect, HDC hdc, HBRUSH frameBrush, int * step, RECT DataUpdateRect)
{
	HBRUSH hBrush = { 0 };
	//Data wipe
	memset(mytimedata, 0, sizeof(char) * 1000);
	TempRect.top = TempRect.bottom;
	TempRect.bottom = LocalRect.bottom / 17;
	hBrush = CreateSolidBrush(RGB(200, 20, 80));
	FillRect(hdc, &TempRect, hBrush);
	DeleteObject(hBrush);
	sprintf_s(mytimedata, 1000, "\nData Wipe");
	SetBkColor(hdc, RGB(200, 20, 80));
	SetTextColor(hdc, RGB(5, 20, 80));
	DrawTextA(hdc, mytimedata, -1, &TempRect, DT_CENTER | DT_LEFT);
	FrameRect(hdc, &TempRect, frameBrush);
	*step = TempRect.bottom - TempRect.top;
	//WordWrap
	memset(mytimedata, 0, sizeof(char) * 1000);
	TempRect.top = TempRect.bottom;
	TempRect.bottom = LocalRect.bottom;
	hBrush = CreateSolidBrush(RGB(5, 20, 80));
	FillRect(hdc, &TempRect, hBrush);
	DeleteObject(hBrush);
	sprintf_s(mytimedata, 1000, "\nWord Wrap");
	SetBkColor(hdc, RGB(5, 20, 80));
	SetTextColor(hdc, RGB(0, 255, 0));
	DrawTextA(hdc, mytimedata, -1, &TempRect, DT_CENTER | DT_LEFT);
	FrameRect(hdc, &TempRect, frameBrush);
	//Save
	memset(mytimedata, 0, sizeof(char) * 1000);
	TempRect.top += *step;
	TempRect.bottom += *step;
	hBrush = CreateSolidBrush(RGB(0, 20, 5));
	FillRect(hdc, &TempRect, hBrush);
	DeleteObject(hBrush);
	sprintf_s(mytimedata, 1000, "\nSave");
	SetBkColor(hdc, RGB(0, 20, 5));
	SetTextColor(hdc, RGB(200, 120, 80));
	DrawTextA(hdc, mytimedata, -1, &TempRect, DT_CENTER | DT_LEFT);
	FrameRect(hdc, &TempRect, frameBrush);
	//Data avalible
	memset(mytimedata, 0, sizeof(char) * 1000);
	TempRect.top += *step;
	TempRect.bottom += *step;
	DataUpdateRect.top = TempRect.top;
	DataUpdateRect.bottom = TempRect.bottom;
	DataUpdateRect.left = TempRect.left;
	DataUpdateRect.right = TempRect.right;
	hBrush = CreateSolidBrush(RGB(200, 120, 80));
	FillRect(hdc, &TempRect, hBrush);
	DeleteObject(hBrush);
	sprintf_s(mytimedata, 1000, "remaining bytes: \n\n%zd", TextHeapRemaining);
	SetBkColor(hdc, RGB(200, 120, 80));
	SetTextColor(hdc, RGB(0, 0, 255));
	DrawTextA(hdc, mytimedata, -1, &TempRect, DT_LEFT);
	FrameRect(hdc, &TempRect, frameBrush);
	//Data remaining
	memset(mytimedata, 0, sizeof(char) * 1000);
	TempRect.top += *step + *step / 2;
	TempRect.bottom += *step + *step / 2;
	hBrush = CreateSolidBrush(RGB(33, 200, 3));
	FillRect(hdc, &TempRect, hBrush);
	DeleteObject(hBrush);
	sprintf_s(mytimedata, 1000, "avalible bytes: \n\n%lld", pchinputbuffermemory);
	SetBkColor(hdc, RGB(33, 200, 3));
	SetTextColor(hdc, RGB(0, 0, 0));
	DrawTextA(hdc, mytimedata, -1, &TempRect, DT_LEFT);
	FrameRect(hdc, &TempRect, frameBrush);
	return DataUpdateRect;
}