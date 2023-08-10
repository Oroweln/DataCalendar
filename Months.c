#include "framework.h" //All header includes stuff is mashed up over there

/*
TODO: Its necessary to tie down the "textouts" and "reads", and their relativity to the index of the dynamic buttons itself..
*/
int poppushindex = 0;
int popushdynamic = 0;
extern winloc windowdataorg[buttonfactor];
int distancebetweenboxes = 0;
BOOL mindexspace[1200] = { 0 };
float indexspace = 0; //always +24 is index space


LRESULT CALLBACK MonthsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static int cxClient = 0, cyClient = 0, cyChar = 0, cxChar = 0, cxCaps = 0;
	int lastpos = 0;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	static int offset = 0;//for ensuring moving relative to the amount being shown at the given moment
	static HWND buttonarray[24] = { 0 };
	SCROLLINFO si = { 0 };
	LPWSTR Months[24] = { TEXT("January"),TEXT("February"),TEXT("March"),TEXT("April"),TEXT("May"),TEXT("June"),TEXT("July"),TEXT("August"),TEXT("September"),TEXT("October"),TEXT("November"),TEXT("December"),TEXT("January1"),TEXT("February1"),TEXT("March1"),TEXT("April1"),TEXT("May1"),TEXT("June1"),TEXT("July1"),TEXT("August1"),TEXT("September1"),TEXT("October1"),TEXT("November1"),TEXT("December1") };
	switch (message)
	{
	case WM_CREATE:

		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);
		buttonMCreationRoutine(cyChar, buttonarray, hwnd, Months);
		return 0;

	case WM_SIZE:

		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		// Set the vertical scrolling range and page size
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_TRACKPOS;
		si.nMin = 0;
		si.nMax = buttonrect.bottom * 1200;
		si.nPage = buttonrect.bottom;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		//bunch of buttons to be created just before they are shown, and then destroyed when they are far from being show
		return 0;

	case WM_VSCROLL:

		lastpos = ScrollingFunction(hwnd, wParam, SB_VERT);
		offset = DynamicScroll(lastpos, hwnd, si, offset, buttonarray);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_SHOWWINDOW:
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 255));
		FillRect(hdc, &ps.rcPaint, hBrush);
		DeleteObject(hBrush);
		ShowMessage(hwnd, cxClient / 2, cyClient / 2, offset);
		EndPaint(hwnd, &ps);
		for (TrustedIndex = 0; TrustedIndex < 24; TrustedIndex++)
		{
			if (buttonarray[TrustedIndex] != 0)
				SendMessage(buttonarray[TrustedIndex], WM_PAINT, 0, 0);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		for (TrustedIndex = 0; TrustedIndex < 24; TrustedIndex++)
			DestroyButton(buttonarray[TrustedIndex]);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

HWND CreateButtonM(LPWSTR szString, RECT Size, HWND hwnd, int index)
{
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

	HWND hwndButton = CreateWindow(TEXT("Button class"), szString, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_TEXT | BS_CENTER, Size.left, ysize * index * separationfactor, Size.right, Size.bottom, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
	Button_Enable(hwndButton, TRUE);
	Button_SetText(hwndButton, szString);
	SetWindowLongPtr(hwndButton, GWLP_USERDATA, index + 1);
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

	static int paintcall = 0, cxClient = 0, cyClient = 0, lastlocalint = 0, localint = 0;
	PAINTSTRUCT ps = { NULL };
	HDC hdc;
	HBRUSH hBrush;
	RECT localrect = { 0 };
	HFILE hfile;
	static int monthindex = 1;
	static LPSTR month[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "Septemeber", "October", "November", "December" };
	static LPSTR year[2] = { 0 };
	int buttonindex = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_LBUTTONDOWN:

		GetClientRect(hwnd, &localrect);
		hdc = GetDC(hwnd);
		hBrush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hdc, &localrect, hBrush);
		ReleaseDC(hwnd, hdc);
		DeleteObject(hBrush);
		localint = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		mindexspace[lastlocalint] = FALSE;
		mindexspace[localint] = TRUE;
		lastlocalint = localint;				
		InvalidateRect(GetParent(hwnd), NULL, TRUE);
		SendMessage(GetParent(buttondates[0]), WM_COMMAND, buttonindex, 0);
		//datactionswitch(3, );//upon clicking send message to dataformating.c. Shall we create datafile first?
		return 0;
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		return 0;

	case WM_PAINT:
			paintcall++;
			hdc = BeginPaint(hwnd, &ps);
			
			OFSTRUCT oFStruct = { 0 };
			OVERLAPPED overlapstruct = { 0 };
			overlapstruct.Offset = (((buttonindex * 150) + 6) - 150);
			int testint = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if ((mindexspace[testint] == TRUE))
				hBrush = CreateSolidBrush(RGB(0, 0, 0));
			else
				hBrush = CreateSolidBrush(RGB(222, 10, 100));
			GetClientRect(hwnd, &ps.rcPaint);
			DWORD fPointer = 0;
			FillRect(hdc, &ps.rcPaint, hBrush);
			char charbuffer[3] = { 0 };
			hfile = OpenFile("Localdata.txt", &oFStruct, OF_READ);
			ReadFile(hfile, charbuffer, 2, NULL, &overlapstruct);
			monthindex = mystringtoint(charbuffer);
			BOOL randflag1 = FALSE;
			TextOutA(hdc, (LONG)ps.rcPaint.left, (LONG)ps.rcPaint.bottom/2, month[monthindex-1], (int)strlen(month[monthindex-1]));

			//put a , year at the end
			//overlapstruct.Offset = ((monthindex * 150) + 6) + (150 * poppushindex);
			overlapstruct.Offset -= 6;
			if (overlapstruct.Offset == 1800)
			{
				randflag1 = TRUE;
			}
			char yearbuffer[7] = { 0 };
			ReadFile(hfile, yearbuffer, 4, NULL, &overlapstruct);

			year[0] = yearbuffer;
			TextOutA(hdc, (LONG)ps.rcPaint.left, (LONG)ps.rcPaint.top, year[0], (int)strlen(year[0]));
			overlapstruct.Offset += 6;

			char intstringb[33] = { 0 };
			LPSTR stringbptr = intstringb;

			_itoa_s(buttonindex, intstringb, 19, 10);
			TextOutA(hdc, (LONG)ps.rcPaint.right/2, (LONG)ps.rcPaint.top, stringbptr, (int)strlen(stringbptr));


			CloseHandle(hfile);
			EndPaint(hwnd, &ps);
			DeleteObject(hBrush);
			return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	}


	return DefWindowProc(hwnd, message, wParam, lParam);
}


BOOL CALLBACK EnumMonthsLocation(HWND hwnd, LPARAM lParam)
{
	TrustedIndex++;
	lParam = 10;
	GetWindowRect(hwnd, &windowdataorg[TrustedIndex].wcrect);
	windowdataorg[TrustedIndex].chwnd = hwnd;
	if (hwnd == FALSE)
		return FALSE;
	return TRUE;
}

//there are made to deal with 13 children
//WE SHOULD MAKE IT MORE VERSATILE
//wtf are we gonna do if he scrolls multiple buttons in one call? perhaps just scroll in fraps[x]
RECT PopChildMonth(HWND hwnd[], int offset)
{
	poppushindex++;
	popushdynamic = 1;
	RECT Temprect = { 0 };
	POINT high[2] = { 0 };
	//LPRECT fixthing = &Temprect;
	for (TrustedIndex = 0; TrustedIndex < buttonfactor; TrustedIndex++)
	{
		GetClientRect(hwnd[TrustedIndex], &Temprect);
		high[1].y = Temprect.bottom;
		MapWindowPoints(hwnd[TrustedIndex], GetParent(hwnd[TrustedIndex]), high, 2);
		if (high[1].y <= 0)//finds which child needs to be pushed, essentially the one that gone past the 0y of the parent
		{
			MoveWindow(hwnd[TrustedIndex], buttonrect.left, ((buttonfactor * (buttonrect.bottom*2)) - offset - buttonrect.bottom*2), buttonrect.right, buttonrect.bottom, TRUE);//moves the window to empty space created by scrolling
			/*Temprect.left = buttonrect.left;
			Temprect.top = max(buttonfactor * buttonrect.bottom, 1)-offset;
			Temprect.right = buttonrect.right;
			Temprect.bottom = buttonrect.bottom;*/
			int lindex = (int)GetWindowLongPtr(hwnd[TrustedIndex], GWLP_USERDATA);
			lindex += 24;
			SetWindowLongPtr(hwnd[TrustedIndex], GWLP_USERDATA, lindex);
			return Temprect;

		}

	}
	return Temprect;
}

RECT PushChildMonth(HWND hwnd[], int offset)
{
	poppushindex--;
	popushdynamic = -1;
	RECT Temprect = { 0 };
	POINT high[2] = { 0 };
	//LPRECT fixthing = &Temprect;
	for (TrustedIndex = 0; TrustedIndex < buttonfactor; TrustedIndex++)
	{
		GetClientRect(hwnd[TrustedIndex], &Temprect);
		high[1].y = Temprect.bottom;
		MapWindowPoints(hwnd[TrustedIndex], GetParent(hwnd[TrustedIndex]), high, 2);
		if (high[1].y >= buttonfactor * (buttonrect.bottom*2))//finds which child needs to be popped(essentially: offset + buttonheight+buttondistance, the *2 is so we could also include the distance between buttons)
		{
			MoveWindow(hwnd[TrustedIndex], buttonrect.left, buttonrect.top + offset, buttonrect.right, buttonrect.bottom, TRUE);//moves the window to empty space created by scrolling
			Temprect.left = buttonrect.left;
			Temprect.top = buttonrect.top + offset;
			Temprect.right = buttonrect.right;
			Temprect.bottom = buttonrect.bottom;
			int lindex = (int)GetWindowLongPtr(hwnd[TrustedIndex], GWLP_USERDATA);
			lindex -= 24;
			SetWindowLongPtr(hwnd[TrustedIndex], GWLP_USERDATA, lindex);//after counting 12 one is skipped apperently.
			return Temprect;
		}
	}
	return Temprect;
}

int DynamicScroll(int lastpos, HWND hwnd, SCROLLINFO si, int offset, HWND* buttonarray)
{
	RECT scrollvalue = { 0 };
	si.fMask = SIF_POS;
	GetScrollInfo(hwnd, SB_VERT, &si);
	distancebetweenboxes = buttonrect.bottom * 2;
	// If the position has changed, scroll window and update it.
	if (si.nPos != lastpos)
	{
		ScrollWindowEx(hwnd, 0, 1 * (lastpos - si.nPos), NULL, NULL, NULL, &scrollvalue, SW_SCROLLCHILDREN);
		int static scrollamount = 0;
		scrollamount += scrollvalue.bottom - scrollvalue.top;
		/*if (scrollamount >= 2 * distancebetweenboxes)
		{
			int fuckoff = 1;
			//si_thumbtrack sends huge values when sscrolling to fast due to it sending it in timeframes instead of tick by tick.
			//here we can notice when this happens, it messes up the offset arithmetic completely, as offset becomes larger then distance betweenboxes
			//when this happens the pop or push function cant compensate for it properly through its offset.

		}
		*/

		if (si.nPos > lastpos)
		{
			while (scrollamount >= distancebetweenboxes)
			{
				offset = scrollamount - distancebetweenboxes;
				PopChildMonth(buttonarray, offset);
				scrollamount -= distancebetweenboxes;
			}
		}
		if (si.nPos < lastpos)
		{
			while (scrollamount >= distancebetweenboxes)
			{
				offset = scrollamount - distancebetweenboxes;
				PushChildMonth(buttonarray, offset);
				scrollamount -= distancebetweenboxes;
			}

		}		//send createbutton signal, and according, deletebutton signal when scroll has moved enough that it is necessiated!
	}
	return offset;
}

BOOL buttonMCreationRoutine(int cyChar, HWND* buttonarray, HWND hwnd, LPWSTR* Months)
{
	buttonrect.left = MonthRect.right / 4;
	buttonrect.top = MonthRect.top;
	buttonrect.right = buttonrect.left * 2;
	buttonrect.bottom = cyChar * 3;
	for (TrustedIndex = 0; TrustedIndex < 24; TrustedIndex++)
		buttonarray[TrustedIndex] = CreateButtonM(Months[TrustedIndex], buttonrect, hwnd, TrustedIndex);

	if (buttonarray != NULL)
		return TRUE;

	else
		return FALSE;
}

int ScrollingFunction(HWND hwnd, WPARAM wParam, int type)
{
	int lastpos = 0;
	// Get all the vertial scroll bar information.
	SCROLLINFO si = {
	si.cbSize = sizeof(SCROLLINFO),
	si.fMask = SIF_ALL,
	};
	GetScrollInfo(hwnd, type, &si);

	// Save the position for comparison later on.
	lastpos = si.nPos;
	switch (LOWORD(wParam))
	{

		// User clicked the HOME keyboard key.
	case SB_TOP:
		si.nPos = si.nMin;
		indexspace = 0;

		break;

		// User clicked the END keyboard key.
	case SB_BOTTOM:
		si.nPos = si.nMax;
		indexspace = 1200 - 24;
		break;

		// User clicked the top arrow.
	case SB_LINEUP:
		si.nPos -= si.nPage / 2;
		if (si.nPos != 0)
			indexspace -= 0.5;
		break;

		// User clicked the bottom arrow.
	case SB_LINEDOWN:
		si.nPos += si.nPage / 2;
		if (si.nPos != si.nMax)
			indexspace += 0.5;
		break;

		// User clicked the scroll bar shaft above the scroll box.
	case SB_PAGEUP:
		si.nPos -= si.nPage;
		if (si.nPos != 0)
			indexspace +=1;
		break;

		// User clicked the scroll bar shaft below the scroll box.
	case SB_PAGEDOWN:
		si.nPos += si.nPage;
		if (si.nPos != 0)
			indexspace -= 1;
		break;

		// User dragged the scroll box.
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		//Note: even if we divide si.nTrackPos by 1000 to ensure it never goes over bottomrect, it still cause hilarious bugs, whenever thumbtrack is touched.
		break;

	default:
		break;
	}

	SetScrollInfo(hwnd, type, &si, TRUE);

	return lastpos;
}

int mystringtoint(LPCSTR string) //accepts up to 99.
{
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