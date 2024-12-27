#include "framework.h" //All header includes stuff is mashed up over there

/*
TODO: Its necessary to tie down the "textouts" and "reads", and their relativity to the index of the dynamic buttons itself..
*/
static HFONT myfont = { 0 };
int MonthYearIndex = 0;
HWND buttonarray[24] = { 0 };
float indexspace = 0; //always +24 is index space
COLORREF monthsbackground = RGB(0, 0, 255);
COLORREF monthsbuttoncolor = RGB(222, 10, 100);

LRESULT CALLBACK MonthsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static int cxClient = 0, cyClient = 0;
	int wheeldelta = 0;
	PAINTSTRUCT ps;
	TRIVERTEX triv[2] = {0};
	HWND localhwnd = { 0 };
	POINT localpoint = { 0 };
	GRADIENT_RECT grr = { 0 };
	SCROLLINFO si = { 0 };
	BOOL static MonthCreationFlag = TRUE;
	LPWSTR Months[24] = { TEXT("January"),TEXT("February"),TEXT("March"),TEXT("April"),TEXT("May"),TEXT("June"),TEXT("July"),TEXT("August"),TEXT("September"),TEXT("October"),TEXT("November"),TEXT("December"),TEXT("January1"),TEXT("February1"),TEXT("March1"),TEXT("April1"),TEXT("May1"),TEXT("June1"),TEXT("July1"),TEXT("August1"),TEXT("September1"),TEXT("October1"),TEXT("November1"),TEXT("December1") };
	switch (message)
	{
	case WM_CREATE:
		// To enumerate all styles of all fonts for the ANSI character set 
		buttonMCreationRoutine(buttonarray, hwnd, Months);
		return 0;

	case WM_SIZE:

		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		// Set the vertical scrolling range and page size
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_TRACKPOS;
		si.nMin = 0;
		si.nMax = buttonrect.bottom * yearrange;
		si.nPage = buttonrect.bottom;
		assert(yearange >= 0);
		buttonrect.left = MonthRect.right / 4;
		buttonrect.top = MonthRect.top;
		buttonrect.right = MonthRect.right;
		buttonrect.bottom = MonthRect.bottom/24;
		buttonrect.left = 0;
		return 0;

	case WM_COMMAND:
		if (MonthCreationFlag)
		{
			SYSTEMTIME sTime = { 0 };
			GetSystemTime(&sTime);
			SystemTimeToTzSpecificLocalTime(NULL, &sTime, &sTime);
			int indexvar = (startyear - yearzero) * 12;
			assert(indexvar >= 0);
			if (indexvar < 24)
				indexvar = 24;
			if (indexvar > yearrange-24)//this and above is in order to avoid problems with displaying months 
				indexvar = yearrange-24;
			if (startyear == 0)//startyear isnt initinalized
				indexvar = yearrange / 2;
			for (int i = 0; i < buttonfactor; i++)
			{
				if (buttonarray[i] == NULL)
				{
					CrashDumpFunction(200,1);
					return FALSE;
				}
				SetWindowLongPtrA(buttonarray[i], GWLP_USERDATA, (long long)((i)+1)+(long long)(indexvar));//sets the years shown, half of yearrange/12+yearzero.
			}
			//here put some shoddyfunction to get the things talked about in notes.
			InvalidateRectSafe(hwnd, NULL, TRUE);
		}
		assert(startmonth >= 0);
		if (startmonth == 0)
			startmonth++;
		(void)SendMessage(buttonarray[startmonth-1], WM_LBUTTONDOWN, 0, 0);
		if (startday == 0)
			startday++;
		(void)SendMessage(buttondates[startday-1], WM_LBUTTONDOWN, 0, 0);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		if (hdc == NULL)
			CrashDumpFunction(90, 0);
		HBRUSH hBrush = SafetCreateSolidBrush(monthsbackground);
		assert(hBrush != INVALID_HANDLE_VALUE);
		SafeSetBkColor(hdc, monthsbackground);
		SafeFillRect(hdc, &ps.rcPaint, hBrush);
		SafeDeleteObject(hBrush);
		(void)EndPaint(hwnd, &ps);
		RECT windorect = { 0 };
		//fading scheme below
		int ysize = buttonrect.bottom - buttonrect.top;
		((COLORREF)(((BYTE)(100) | ((WORD)((BYTE)(100)) << 8)) | (((DWORD)(BYTE)(100)) << 16)));
		triv[0].Alpha = 0; 
		triv[0].Red = GetRValue(monthsbackground)*255;
		triv[0].Blue = GetBValue(monthsbackground)*255;
		triv[0].Green = GetGValue(monthsbackground)*255;
		triv[1].Alpha = 0;
		triv[1].Red = GetRValue(monthsbuttoncolor) * 255;
		triv[1].Blue = GetBValue(monthsbuttoncolor) * 255;
		triv[1].Green = GetGValue(monthsbuttoncolor) * 255;
		grr.UpperLeft = 0;
		grr.LowerRight = 1;
		for (TrustedIndex = 0; TrustedIndex < 24; TrustedIndex++)
		{
			triv[0].x = buttonrect.left;
			triv[0].y = windorect.top - ysize / 2;
			triv[1].x = buttonrect.right;
			triv[1].y = windorect.top;
			triv[0].Red = GetRValue(monthsbackground) * 255;
			triv[0].Blue = GetBValue(monthsbackground) * 255;
			triv[0].Green = GetGValue(monthsbackground) * 255;
			triv[1].Red = GetRValue(monthsbuttoncolor) * 255;
			triv[1].Blue = GetBValue(monthsbuttoncolor) * 255;
			triv[1].Green = GetGValue(monthsbuttoncolor) * 255;
			assert(buttonarray != NULL);
			GetWindowRectSafe(buttonarray[TrustedIndex], &windorect);
			MapWindowPointsSafe(NULL, hwnd, (LPPOINT)&windorect, 2);
			if (FALSE == GradientFill(hdc, triv, 2, &grr, 1, GRADIENT_FILL_RECT_V))
			{
				CrashDumpFunction(100, 0);
			}
			triv[0].x = buttonrect.left;
			triv[0].y = windorect.bottom;
			triv[1].x = buttonrect.right;
			triv[1].y = windorect.bottom + ysize/2;
			triv[1].Red = GetRValue(monthsbackground) * 255;
			triv[1].Blue = GetBValue(monthsbackground) * 255;
			triv[1].Green = GetGValue(monthsbackground) * 255;
			triv[0].Red = GetRValue(monthsbuttoncolor) * 255;
			triv[0].Blue = GetBValue(monthsbuttoncolor) * 255;
			triv[0].Green = GetGValue(monthsbuttoncolor) * 255;
			if (FALSE == GradientFill(hdc, triv, 2, &grr, 1, GRADIENT_FILL_RECT_V))
			{
				CrashDumpFunction(100, 0);
			}
			MoveWindowSafe(buttonarray[TrustedIndex], buttonrect.left, windorect.top, buttonrect.right, windorect.bottom-windorect.top, TRUE);
			UpdateWindowSafe(buttonarray[TrustedIndex]);
		}
		if (GlobalDebugFlag == TRUE)
		{
			TEXTMETRICA textmetric = { 0 };
			HDC monthshdc = { 0 };
			monthshdc = safeGetDC(SafeGetParent(buttonarray[0]));
			assert(monthshdc != NULL);
			GetTextMetricsA(monthshdc, &textmetric);
			SafeReleaseDC(SafeGetParent(buttonarray[0]), monthshdc);
		}
		hdc = NULL;
		hdc = safeGetDC(hwnd);
		COLORREF invmonthsbackground = RGB(GetRValue(~monthsbackground), GetGValue(~monthsbackground), GetBValue(~monthsbackground));
		hBrush = SafetCreateSolidBrush(invmonthsbackground);
		RECT monthsrect = { 0 };
		monthsrect.right = cxClient;
		monthsrect.bottom = cyClient;
		monthsrect.left = monthsrect.right - monthsrect.right / 30;
		SafeFillRect(hdc, &monthsrect, hBrush);
		SafeDeleteObject(hBrush);
		SafeReleaseDC(hwnd, hdc);
		return 0;
	case WM_MOUSEWHEEL:
		localpoint.x = 1;
		localpoint.y = 1;
		localhwnd = ChildWindowFromPoint(hwnd, localpoint);
		assert(localhwnd != NULL);
		int firstbuttonid = 0;
		firstbuttonid = (int)SafeGetWindowLongPtr(localhwnd, GWLP_USERDATA);
		wheeldelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
		if ((firstbuttonid != 1 || wheeldelta != -1) && (wheeldelta != 1 || firstbuttonid+11 < yearrange))
		{
			DynamicScroll(wheeldelta, hwnd);
		}
		break;
	case WM_DESTROY:
		SafeDeleteObject(myfont);
		for (TrustedIndex = 0; TrustedIndex < 24; TrustedIndex++)
			DestroyButton(buttonarray[TrustedIndex]);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

HWND CreateButtonM(LPWSTR szString, RECT Size, HWND hwnd, int index)
{
	assert(hwnd != NULL);
	assert(szString != NULL);
	static BOOL firsttime = TRUE;
	WNDCLASSEX buttonclass =
	{
	buttonclass.cbSize = sizeof(buttonclass),
	buttonclass.style = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW,
	buttonclass.lpfnWndProc = ButtonProcM,
	buttonclass.cbClsExtra = 0,
	buttonclass.cbWndExtra = sizeof(short int),
	buttonclass.hInstance = NULL,
	buttonclass.hIcon = NULL,
	buttonclass.hCursor = NULL,
	buttonclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
	buttonclass.lpszMenuName = NULL,
	buttonclass.lpszClassName = TEXT("Button class"),
	buttonclass.hIconSm = NULL,
	};

	if (firsttime == TRUE)
	{
		RegisterClassEx(&buttonclass);
		firsttime = FALSE;
	}

	int ysize = Size.bottom - Size.top;
	int separationfactor = 2;

	HWND hwndButton = CreateWindow(TEXT("Button class"), szString, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_TEXT | BS_CENTER, Size.left, ysize * index * separationfactor, Size.right, Size.bottom, hwnd, NULL, (HINSTANCE)SafeGetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
	Button_Enable(hwndButton, TRUE);
	Button_SetText(hwndButton, szString);
	RECT dummyrect = { 0 };
	GetWindowRectSafe(hwndButton, &dummyrect);
	GetWindowRectSafe(hwnd, &dummyrect);
	SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)index + 1);
	if (hwndButton)
		return hwndButton;
	else
		return NULL;

}

LRESULT ButtonProcM(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/*
	-Hot state, on stack.
	-Cold state, on external data file.
	*/
	static int lastlocalint = 0;
	PAINTSTRUCT ps = { NULL };
	HDC hdc;
	HBRUSH hBrush;
	RECT localrect = { 0 };
	HANDLE hfile;
	BOOL static Firstbuttonflag = TRUE;
	static int monthindex = 1;
	static LPSTR month[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "Septemeber", "October", "November", "December" };
	static LPSTR year[2] = { 0 };
	int buttonindex = (int)SafeGetWindowLongPtr(hwnd, GWLP_USERDATA);
	static char* mindexspace = { 0 };

	switch (message)
	{
	case WM_CREATE:
		if (Firstbuttonflag == TRUE)
		{
			RECT temprect9 = { 0 };
			SafetGetClientRect(hwnd, &temprect9);
			myfont = CreateFontA(0, temprect9.bottom / 3, 0, 0, FW_REGULAR, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_CHARACTER_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FIXED_PITCH, NULL);
			assert(myfont != NULL);
			Firstbuttonflag = FALSE;
			mindexspace = SafeCalloc((size_t)yearrange + 100, sizeof(char));
		}
		return 0;
	case WM_LBUTTONDOWN: //memory leak happens befor buttondown message is even sent
		//something before buttondown happens?
		if (TextBoxCalFlag == FALSE)
		{
			TextBoxCalFlag = TRUE;
		}
		assert(TextBoxInput != NULL);
		Edit_Enable(TextBoxInput, FALSE);
		markflagmonth = TRUE; //check if there is a mark in the month where we going, so the painting loop can be done for it. 
		SafetGetClientRect(hwnd, &localrect);
		hdc = safeGetDC(hwnd);
		hBrush = SafetCreateSolidBrush(RGB(0, 0, 0));
		SafeFillRect(hdc, &localrect, hBrush);
		SafeReleaseDC(hwnd, hdc);
		SafeDeleteObject(hBrush);
		MonthYearIndex = (int)SafeGetWindowLongPtr(hwnd, GWLP_USERDATA);
		selecteddate = ((WORD)((selecteddate)) & 0x7FF) + ((WORD)(((selecteddate)) >> 23) << 23);//get year section to be 0 bits, that is 11th to 23rd bit. 
		int currentyear = yearzero + MonthYearIndex / 12;
		if (MonthYearIndex % 12 == 0)
			currentyear--;
		selecteddate += currentyear << 11; //update year data for selectedate
		assert(mindexspace != NULL);
		mindexspace[lastlocalint] = FALSE;
		mindexspace[MonthYearIndex] = TRUE;
		lastlocalint = MonthYearIndex;
		InvalidateRectSafe(SafeGetParent(hwnd), NULL, TRUE);
		(void)SendMessage(SafeGetParent(buttondates[0]), WM_COMMAND, MonthYearIndex, 0);//updates monthyear index
		(void)SendMessage(SafeGetParent(buttonmarks[0]), WM_COMMAND, MonthYearIndex, 0);//notify marks.c that the month-year has //memory leak happens before thischanged, so it can update the coloring/shapes for the dates present.
		startmonth = MonthYearIndex % 12;
		if (startmonth == 0)
			startmonth = 12;
		if (MonthYearIndex >= 12)
			startyear = (MonthYearIndex / 12) + yearzero;
		else startyear = 0;
		return 0;
	case WM_PAINT:
		assert(mindexspace != NULL);
		hdc = BeginPaint(hwnd, &ps);
		if (hdc == NULL)
			CrashDumpFunction(90, 0);
		OVERLAPPED overlapstruct = { 0 };
		overlapstruct.Offset = (((buttonindex * 150) + 6) - 150);
		int testint = (int)SafeGetWindowLongPtr(hwnd, GWLP_USERDATA);
		COLORREF invertedbcolor = RGB(GetRValue(~monthsbuttoncolor), GetGValue(~monthsbuttoncolor), GetBValue(~monthsbuttoncolor));
		if ((mindexspace[testint] == TRUE))
		{
			hBrush = SafetCreateSolidBrush(invertedbcolor);
			SafeSetBkColor(hdc, invertedbcolor);
			SafeSetTextColor(hdc, monthsbuttoncolor);
		}
		else
		{
			hBrush = SafetCreateSolidBrush(monthsbuttoncolor);
			SafeSetBkColor(hdc, monthsbuttoncolor);
			SafeSetTextColor(hdc, invertedbcolor);
		}
		SafetGetClientRect(hwnd, &ps.rcPaint);
		SafeFillRect(hdc, &ps.rcPaint, hBrush);
		SafeDeleteObject(hBrush);
		char charbuffer[3] = { 0 };
		char tempstring[1000] = { 0 };
		if(-1==sprintf_s(tempstring, 1000, "%sLocaldata.txt", theworkingdirectory))
		{
			CrashDumpFunction(50, 0);
		}
		hfile = CreateFileA(tempstring, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		assert(hfile != INVALID_HANDLE_VALUE);
		if (FALSE == ReadFile(hfile, charbuffer, 2, NULL, &overlapstruct))
		{
			SafeCloseHandle(hfile);
			CrashDumpFunction(201, 0);
			return -1;
		}
		monthindex = mystringtoint(charbuffer);
		BOOL randflag1 = FALSE;
		SafeSelectObject(hdc, myfont);
		SafeTextOutA(hdc, (LONG)ps.rcPaint.right / 3, (LONG)ps.rcPaint.bottom / 2, month[monthindex - 1], (int)strlen(month[monthindex - 1]));
		//put a , year at the end
		overlapstruct.Offset -= 6;
		if (overlapstruct.Offset == 1800)
			randflag1 = TRUE;
		char yearbuffer[7] = { 0 };
		if (FALSE == ReadFile(hfile, yearbuffer, 4, NULL, &overlapstruct))
		{
			SafeCloseHandle(hfile);
			CrashDumpFunction(204, 0);
			return -1;
		}
		year[0] = yearbuffer;
		SafeTextOutA(hdc, (LONG)ps.rcPaint.left, (LONG)ps.rcPaint.top, year[0], (int)strlen(year[0]));
		overlapstruct.Offset += 6;
		assert(buttonarray != NULL);
		if (GlobalDebugFlag == TRUE)
		{
			char* printstring = SafeCalloc(50, sizeof(char));
			int m = 0;
			for (m = 0; hwnd != buttonarray[m] && m < buttonfactor; m++)
				;
			if(-1==sprintf_s(printstring, 50, "%d, %d", buttonindex, m))
			{
				CrashDumpFunction(50, 0);
			}
			SafeTextOutA(hdc, (LONG)ps.rcPaint.right / 2, (LONG)ps.rcPaint.top, printstring, (int)strlen(printstring));
			free(printstring);
		}
		SafeCloseHandle(hfile);
		(void)EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		free(mindexspace);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

//pops the firstwindow by putting it to lastwindow pos
int PopChildMonth(HWND hwnd, int lastwindow)
{
	RECT Temprect;
	assert(buttonarray != NULL);
	GetWindowRectSafe(buttonarray[lastwindow], &Temprect);
	MapWindowPointsSafe(HWND_DESKTOP, SafeGetParent(hwnd), (LPPOINT)&Temprect, 2);
	MoveWindowSafe(hwnd, Temprect.left, Temprect.top, Temprect.right-Temprect.left, Temprect.bottom-Temprect.top, TRUE);//pops the window by moving to the lastwindow pos
	int lindex = (int)SafeGetWindowLongPtr(hwnd, GWLP_USERDATA);
	lindex += 24;
	assert(hwnd != NULL);
	SetWindowLongPtr(hwnd, GWLP_USERDATA, lindex);
	return 0;
}

int PushChildMonth(HWND hwnd, int firstwindow)
{
	assert(hwnd != NULL);
	assert(buttonarray != NULL);
	RECT Temprect;
	GetWindowRectSafe(buttonarray[firstwindow], &Temprect);
	MapWindowPointsSafe(HWND_DESKTOP, SafeGetParent(hwnd), (LPPOINT)&Temprect, 2);
	MoveWindowSafe(hwnd, Temprect.left, Temprect.top, Temprect.right - Temprect.left, Temprect.bottom - Temprect.top, TRUE);//pushes the window by moving to the firstwindow pos
	int lindex = (int)SafeGetWindowLongPtr(hwnd, GWLP_USERDATA);
	lindex -= 24;
	SetWindowLongPtr(hwnd, GWLP_USERDATA, lindex);
	return 0;
}

//3rd consequent run of dynamicscroll starts incrementally updating things the same!!!
int DynamicScroll(int scrollamount, HWND hwnd)
{
	assert(hwnd != NULL);
	assert(buttonarray != NULL);
	RECT monthsbuttonrect = { 0 };
	static int firstwindow = 0, lastwindow = buttonfactor-1;//firstwindow is window that can be poped, lastwindow is window that can be pushed
	SafetGetClientRect(buttonarray[0], &monthsbuttonrect);
	int distancebetweenboxes = monthsbuttonrect.bottom * 2;//this is equivalent to two y sizes of the buttom
	// If the position has changed, scroll window and update it.
	if (scrollamount > 0)//wheel moved forward
	{
		PopChildMonth(buttonarray[firstwindow], lastwindow);//pop the window
		lastwindow = firstwindow;//firstwindow pops and is put in the place of the lastwindow
		firstwindow++;//firstwindow becomes the first window below the last window, and it is always the firstwindow's id + 1
		if (firstwindow > buttonfactor - 1)//in case firstwindow's id is 23, then the the firstwindow+1, that is window below it, must have id of 0.
			firstwindow = 0;
		for (int i = firstwindow, k = -2; i != lastwindow && i<buttonfactor*2 && k< buttonfactor * 2; i++, k=i-1)//we here beging with i = firstwindow cause we need to move firstwindow to first place, that is now empty after the last one popped,
			//k is -2 as a signal that firstwindow need to be puts at its place, that is relative to the 0y
			//after the first iteration k is to be i-1, for GetRect calculation in order to position the target window relative to the window above it.
		{
			if (i == buttonfactor)
			{
				i = 0;
				if (i == lastwindow)
					return TRUE;
			}
			if (k == -1)//if k == -1 it means that one above the i-1 is 23, set k to 23, as we moving k down, just one place behind i.
			{
				k = buttonfactor-1;
			}
			if(k>-1 && k<buttonfactor)
			{
				GetWindowRectSafe(buttonarray[k], &monthsbuttonrect);
				MapWindowPointsSafe(HWND_DESKTOP, SafeGetParent(hwnd), (LPPOINT)&monthsbuttonrect, 2);
			}
			if (k == -2)//the first iteration, set the monthsbuttonrect to be the first button relative to the upper edge of the monthwindow
			{
				monthsbuttonrect.top = -(distancebetweenboxes);//doing this so MoveWindowSafe would set monthsbuttonrect.top to 0 when plused with distancebetweenboxes
				k = i - 1;//set k to k-1, and it will be for all future iterations tailing i so it can be used as relative value
			}
			MoveWindowSafe(buttonarray[i], 0, monthsbuttonrect.top+(distancebetweenboxes), monthsbuttonrect.right, distancebetweenboxes/2, TRUE);//+distancebetweentwoboxes will put the window below always relatively two buttons worth height below
		}
	}
	if (scrollamount < 0)//wheel moved backward
	{
		PushChildMonth(buttonarray[lastwindow], firstwindow);//push the window
		firstwindow = lastwindow;//first is pushed to the position of lastwindow
		lastwindow--;//lastwindow becomes the window before the lastwindow 
		if (lastwindow < 0)//in case lastwindow's id is -1, then the the lastwindow-1, that is window above it, must have id of 23.
			lastwindow = buttonfactor - 1;
		for (int i = firstwindow, k = -2; i != lastwindow && i >= 0 && i < buttonfactor * 2 && k < buttonfactor * 2; i++, k = i - 1)//we here beging with i = firstwindow cause we need to move firstwindow to first place, that is now empty after the last one popped,
			//k is -2 as a signal that firstwindow need to be puts at its place, that is relative to the 0y
			//after the first iteration k is to be i-1, for GetRect calculation in order to position the target window relative to the window above it.
		{
			if (i == buttonfactor)
			{
				i = 0;
				if (i == lastwindow)
					return TRUE;
			}
			if (k == -1)//if k == -1 it means that one above the i-1 is 23, set k to 23, as we moving k down, just one place behind i.
			{
				k = buttonfactor - 1;
			}
			if (k > -1 && k < buttonfactor)
			{
				GetWindowRectSafe(buttonarray[k], &monthsbuttonrect);
				MapWindowPointsSafe(HWND_DESKTOP, SafeGetParent(hwnd), (LPPOINT)&monthsbuttonrect, 2);
			}
			if (k == -2)//the first iteration, set the monthsbuttonrect to be the first button relative to the upper edge of the monthwindow
			{
				monthsbuttonrect.top = -(distancebetweenboxes);//doing this so MoveWindowSafe would set monthsbuttonrect.top to 0 when plused with distancebetweenboxes
				k = i - 1;//set k to k-1, and it will be for all future iterations tailing i so it can be used as relative value
			}
			MoveWindowSafe(buttonarray[i], 0, monthsbuttonrect.top + (distancebetweenboxes), monthsbuttonrect.right, distancebetweenboxes / 2, TRUE);//+distancebetweentwoboxes will put the window below always relatively two buttons worth height below
		}
		return TRUE;
	}
	return TRUE;
}

BOOL buttonMCreationRoutine(HWND* buttonarrayd, HWND hwnd, LPWSTR* Months)
{
	assert(buttonarrayd != NULL);
	assert(hwnd != NULL);
	buttonrect.left = MonthRect.right / 4;
	buttonrect.top = MonthRect.top;
	buttonrect.right = buttonrect.left * 2;
	buttonrect.bottom = MonthRect.bottom/16;
	for (TrustedIndex = 0; TrustedIndex < 24; TrustedIndex++)
		buttonarrayd[TrustedIndex] = CreateButtonM(Months[TrustedIndex], buttonrect, hwnd, TrustedIndex);

	if (buttonarrayd == NULL)
	{
		CrashDumpFunction(203,1);
		return TRUE;
	}

	else
		return FALSE;
}

int mystringtoint(LPCSTR string) //accepts up to 99.
{
	assert(string != NULL);
	int returnval = 0;
	int intarray[3] = { 0 };
	for (int i = 0; i < 3; i++)
	{
		intarray[i] = string[i] - '0';
	}
	if (isdigit(string[1]))
	{
		returnval = ((intarray[0] * 10) + intarray[1]);
	}
	else
		returnval = intarray[0];

	return returnval;
}