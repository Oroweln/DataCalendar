#include "framework.h"


LOGFONT** Fontdata = { 0 };
HWND buttondates[32] = { 0 };//stored hwnd of all 32 buttons for dates
HWND DatesHwnd = { 0 };//dates hwnd
int selecteddate = 0;//lowest 7 bits are day, then followed by 4 bits that are month, then from 11th to 23rd are the year(reserved for calcs), those be always 0 tho
int datemonthindex = 0; //used for calculating selecteddate
int datindice = 0;//used for date calcuations
int TrustedIndex3 = 0;//made to avoid sctreloadmitigation
BOOL markflagmonth = TRUE; //when true it will do a markcheck, otherwise it wont.
static HFONT datefont = NULL;//used to store font
COLORREF datesbackground = RGB(0, 255, 255);
COLORREF datesbuttoncolor = RGB(55, 60, 120);
HWND FontBoxHwnd = { 0 };
RGBData colorsdata[170] = { 0 };

LRESULT CALLBACK ButtonProcD(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FontBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
int CALLBACK EnumFontFamilyProc(const LOGFONT* lpelfe, const void* lpntme, DWORD      FontType, LPARAM     lParam);

HWND datetickfunction(int datetick, char* filebuffer, int * i);
BOOL DateColoring(int colorid, HWND refferentdate, HDC hdc, char* filebuffer, int* i);
BOOL DateShaping(int shapeid, char* filebuffer, int* i, int datetick);
HWND ColorRemovalRoutine(int offsetpresent, int shiuze, OVERLAPPED fuckshit, HANDLE hFile, HWND refferentdate, int datetick);
HWND MarkRemoveRoutine(int offsetpresent, int shiuze, OVERLAPPED fuckshit, HANDLE hFile, int datetick, HWND refferentdate);
char* marksbuffermodifier(int shapeid, int colorid, char* filebuffer, int datetick, int* i, int presentshapeval, HWND refferentdate, HDC hdc, int presentcolorval);
bool markspaint(BOOL* dateflags);
bool marksmonthcheck(char* marksbuffer, int filesize);
char* tempbuffermarkchecker(BOOL* flag69, int* checker, char* tempbuffer2, char* TempBuffer, BOOL lswitch, int* day, BOOL* mydflags, int* i, int month, int* indexplace, int* colorvalue, int* shapevalue);
HWND colorshapepresent(HWND refferentbutton, BOOL* mydflags, int shape, int color, int day);
char* dataovverridingsmarks(char* tempbuffer2, char* TempBuffer, BOOL* flag69, int day, int* indexplace, int* colorvalue, int* shapevalue, int* checker, int* i);
int ColorsMaker(RGBData mycolors[]);


/*32 child button windows */
LRESULT CALLBACK DatesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DatesHwnd = hwnd;//even before this
	HDC hdc = 0;
	static RECT fontrect = { 0 };
	int shiuze = 0;
	int cxClient = 0, cyClient = 0, cxChar = 0, cxCaps = 0, cyChar = 0, shapeid = 0, datetick=0;
	WPARAM colorid = 0;
	PAINTSTRUCT ps;
	TRIVERTEX triv[2] = { 0 };
	GRADIENT_RECT grr = { 0 };
	LPWSTR dates[32] = { TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9"), TEXT("10"), TEXT("11"), TEXT("12"), TEXT("13"), TEXT("14"), TEXT("15"), TEXT("16"), TEXT("17"), TEXT("18"), TEXT("19"), TEXT("20"), TEXT("21"), TEXT("22"), TEXT("23"), TEXT("24"), TEXT("25"),TEXT("26"),TEXT("27"),TEXT("28"),TEXT("29"),TEXT("30"),TEXT("31") };
	TEXTMETRIC tm;

	switch (message)//memory leak is before this even
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

		ColorsMaker(colorsdata);
		CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(FONTBOXDIALOG), hwnd, FontBoxProc);

		return 0;

	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		RECT temprect = { 0 };
		POINT fontupleft = { 0 };
		POINT fontdownright = { 0 };
		GetWindowRect(buttondates[30], &temprect);
		fontupleft.x = temprect.right+4;
		GetWindowRect(buttondates[27], &temprect);
		fontupleft.y = temprect.bottom+4;
		GetWindowRect(DatesHwnd, &temprect);
		fontdownright.x = temprect.right;
		fontdownright.y = temprect.bottom;

		MapWindowPoints(NULL, NULL, &fontupleft,1);
		MapWindowPoints(NULL, NULL, &fontdownright,1);


		fontrect.left = fontupleft.x;
		fontrect.top = fontupleft.y;
		fontrect.right = fontdownright.x;
		fontrect.bottom = fontdownright.y;

		(buttondates[11], &temprect);
		fontupleft.x = temprect.left;
		GetWindowRect(buttondates[11], &temprect);
		fontupleft.y = temprect.top;
		GetWindowRect(DatesHwnd, &temprect);
		fontdownright.x = temprect.right;
		fontdownright.y = temprect.bottom;

		int stupidnigga = MoveWindow(FontBoxHwnd, fontrect.left, fontrect.top, fontrect.right - fontrect.left, fontrect.bottom - fontrect.top, TRUE);
		UpdateWindow(FontBoxHwnd);
		ShowWindow(FontBoxHwnd, SW_SHOW);

		return 0;

	case WM_COMMAND://update month for 
		datemonthindex = (int)wParam;
		datemonthindex %= 12;
		if (datemonthindex == 0)
			datemonthindex = 12;
		selecteddate = (selecteddate & 0x7F) + (((selecteddate) >> 11) << 11);//clear the month space first before update
		selecteddate += (datemonthindex) << 7;
		SendMessage(buttondates[0], WM_COMMAND, 1488, 888);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case MARK_SIGNAL: //here the data is appended upon clicking the color after selecting the desired date. We must make it so its able to "overwrite" presentdata, instead of spamming same date like an idiot.
		shapeid = (int)lParam;
		colorid = wParam;
		HWND refferentdate = 0;
		datetick = colorid & 0xffffff;
		colorid = colorid >> 24;
		HANDLE hFile = 0;
		OVERLAPPED fuckshit = { 0 };
		char tempstring[1000] = { 0 };
		sprintf_s(tempstring, 1000, "%sMarksData.txt", theworkingdirectory);
		hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			CrashDumpFunction(31, 0);
			return 0;
		}
		if (INVALID_FILE_SIZE == ((shiuze = GetFileSize(hFile, NULL))))
		{
			CrashDumpFunction(30, 0);
			shiuze = 0;
		}
		int offsetpresent = 0;
		int presentcolorval = 0;
		int presentshapeval = 0;
		offsetpresent = MarkPresenceCheck(hFile, shiuze, datetick, TRUE, &presentshapeval, &presentcolorval, NULL);//check if there is a mark at the selected date, overwrite it and apply new dataset.
		if (offsetpresent < 0)
		{
			fuckshit.Offset = shiuze;//this causes line spam, and avoid re-reading from provided data set.
			if (shiuze > 0)
				fuckshit.Offset -= 0;//to mitigate the NULL coming from overlap when there is one byte its gonna point to 1st place, but that one byte is at 0th place, to point to that byte we must -1 it.
		}//Create a routine in which the shuize actually point to an existing dataset if such exists, instead of re-creating it billion times
		else
			fuckshit.Offset = offsetpresent;
		if (shapeid == IDCOLORREMOVE)//color removal routine
		{
			refferentdate = ColorRemovalRoutine(offsetpresent, shiuze, fuckshit, hFile, refferentdate, datetick);
			CloseHandle(hFile);
			return TRUE;
		}
		else if (shapeid == IDMARKSREMOVE)
		{
			refferentdate = MarkRemoveRoutine(offsetpresent, shiuze, fuckshit, hFile, datetick, refferentdate);
			CloseHandle(hFile);
			return TRUE;
		}
		char* filebuffer = calloc(40, sizeof(char));
		if (filebuffer == NULL)
		{
			CrashDumpFunction(47, 0);
			return 0;
		}
		int i = 0;
		filebuffer[i++] = '|';
		refferentdate = datetickfunction(datetick, filebuffer, &i);
		hdc = GetDC(refferentdate);
		filebuffer[i++] = specialchar[0];
		filebuffer = marksbuffermodifier(shapeid, (int)colorid, filebuffer, datetick, &i, presentshapeval, refferentdate, hdc, presentcolorval);
		WriteFile(hFile, filebuffer, 18, NULL, &fuckshit);
		CloseHandle(hFile);
		free(filebuffer);
		filebuffer = NULL;
		return 0;
	case WM_SHOWWINDOW:
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);//even before this
		RECT WindowSize = { 0,0,0,0 };
		GetClientRect(hwnd, &WindowSize);
		HBRUSH sbrush = CreateSolidBrush(datesbackground);
		FillRect(hdc, &WindowSize, sbrush);
		DeleteObject(sbrush);
		int currentmonth = datemonthindex;
		if (datemonthindex > 11)
			currentmonth = datemonthindex - 11;
		GetWindowRect(hwnd, &WindowSize);
		int ysize = buttonrectd.bottom - buttonrectd.top;
		triv[0].Blue = triv[1].Red = USHRT_MAX;
		triv[0].Green = triv[0].Alpha = triv[0].Red = triv[1].Alpha = triv[1].Blue = triv[1].Green = 0;
		grr.UpperLeft = 0;
		grr.LowerRight = 1;
		RECT windorect = { 0 };
		datindice = 0;//code below causes memory leak?
		for (TrustedIndex = 0; TrustedIndex < 31; TrustedIndex++)
		{
			if (TrustedIndex % 7 == 0 && TrustedIndex != 0)
				datindice += 3;
			if (buttondates[TrustedIndex] != 0)
			{
				int linefact = TrustedIndex / 7;
				GetWindowRect(buttondates[TrustedIndex], &windorect);
				MapWindowPoints(NULL, hwnd, &windorect, 2);
				triv[0].x = windorect.left;
				triv[0].y = windorect.top - ysize / 20;
				triv[1].x = windorect.right;
				triv[1].y = windorect.top;
				triv[0].Green = USHRT_MAX;
				triv[0].Blue = USHRT_MAX;
				triv[1].Red = 55 * 257;
				triv[1].Green = 60 * 257;
				triv[1].Blue = 120 * 257;
				GradientFill(hdc, triv, 2, &grr, 1, GRADIENT_FILL_RECT_V);
				triv[0].x = windorect.left;
				triv[0].y = windorect.bottom;
				triv[1].x = windorect.right;
				triv[1].y = windorect.bottom + ysize / 20;
				triv[0].Red = 55 * 257;
				triv[0].Green = 60 * 257;
				triv[0].Blue = 120 * 257;//55, 60, 120
				triv[1].Green = USHRT_MAX;
				triv[1].Blue = USHRT_MAX;
				GradientFill(hdc, triv, 2, &grr, 1, GRADIENT_FILL_RECT_V);
				triv[0].x = windorect.left;
				triv[0].y = windorect.top;
				triv[1].x = windorect.left - ysize / 20;
				triv[1].y = windorect.bottom;
				GradientFill(hdc, triv, 2, &grr, 1, GRADIENT_FILL_RECT_H);
				triv[0].x = windorect.right;
				triv[0].y = windorect.top;
				triv[1].x = windorect.right + ysize / 20;
				triv[1].y = windorect.bottom;
				GradientFill(hdc, triv, 2, &grr, 1, GRADIENT_FILL_RECT_H);
				HBRUSH niggabrush = 0;
				niggabrush = CreateSolidBrush(RGB(72, 41, 250));
				FrameRect(hdc, &windorect, niggabrush);
				MoveWindow(buttondates[TrustedIndex], buttonrectd.left + (TrustedIndex - linefact * 7) * buttonrectd.right * 2, linefact * buttonrectd.bottom * 2 + buttonrectd.bottom / 2, buttonrectd.right, buttonrectd.bottom, TRUE);
				UpdateWindow(buttondates[TrustedIndex]);
				DeleteObject(niggabrush);
			}
			datindice += 3;
		}
		MoveWindow(FontBoxHwnd, fontrect.left, fontrect.top, fontrect.right - fontrect.left, fontrect.bottom - fontrect.top, TRUE);
		UpdateWindow(FontBoxHwnd);
		ShowWindow(FontBoxHwnd, SW_SHOW);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		DeleteObject(datefont);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL buttonDCreationRoutine(int cyChar, HWND* buttonarrayz, HWND hwnd, LPWSTR* Dates)
{
	buttonrectd.left = 0;//he has autism needs to die from thats why we put the values  here
	buttonrectd.top = DatesRect.top;
	buttonrectd.right = DatesRect.right / 20;
	buttonrectd.bottom = DatesRect.bottom/14;//this determines heigh of marks buttons!!!
	buttonarrayz[TrustedIndex] = CreateButtonD(*Dates, hwnd, TrustedIndex);

	if (buttonarrayz != NULL)
		return TRUE;

	else
	{
		CrashDumpFunction(50, 1);
		return FALSE;
	}
}


HWND CreateButtonD(LPWSTR szString, HWND hwnd, int index)
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

	hwndButtond = CreateWindow(TEXT("Button class d"), szString, WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_TEXT | BS_CENTER, buttonrectd.left+(index-linefact*7)* buttonrectd.right*2, linefact* buttonrectd.bottom*2+ buttonrectd.bottom/2, buttonrectd.right, buttonrectd.bottom, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
	Button_Enable(hwndButtond, TRUE);
	Button_SetText(hwndButtond, szString);
	SetWindowLongPtr(hwndButtond, GWLP_USERDATA, lindex);
	lindex = (int)GetWindowLongPtr(hwndButtond, GWLP_USERDATA);
	lindex++;
	if (hwndButtond)
		return hwndButtond;
	else
	{
		CrashDumpFunction(51, 1);
		return NULL;
	}

}

LRESULT CALLBACK ButtonProcD(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps = { 0 };//before even procD it happens
	HDC hdc = NULL;
	HANDLE hFile = 0;
	DWORD paramy = 0;
	char charbuffer[4] = { 0 };
	RECT localrect = { 0 };
	static BOOL dateflags[32], Firstbuttonflag = TRUE;
	HBRUSH hBrush;
	char* pusikurac = { 0 };
	SYSTEMTIME temptime = { 0 };
	static int lindex = 0;

	switch (message)
	{
	case WM_CREATE:
		if (Firstbuttonflag == TRUE)
		{
			RECT temprect9 = { 0 };
			GetClientRect(hwnd, &temprect9);
			datefont = CreateFontA(0, temprect9.bottom / 3, 0, 0, FW_REGULAR, FALSE, TRUE, FALSE, ANSI_CHARSET, OUT_CHARACTER_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, FIXED_PITCH, NULL);
			Firstbuttonflag = FALSE;
		}
		return 0;
	case WM_LBUTTONDOWN:
		if(TextBoxFlag == FALSE)
			TextBoxFlag = TRUE;
		selecteddate -= ((WORD)((selecteddate)) & 0x7F);//get date section of 7 bits to be 0, by minusing the part covered by seven bits to extract and update the day properly
		selecteddate += (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		int monthvalue = 0;
		GetClientRect(hwnd, &localrect);
		hdc = GetDC(hwnd);
		hBrush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hdc, &localrect, hBrush);
		ReleaseDC(hwnd, hdc);
		DeleteObject(hBrush);
		dateflags[lindex] = FALSE;
		startday = lindex = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);//after counting 12 one is skipped apperently.
		dateflags[lindex] = TRUE;
		InvalidateRect(GetParent(hwnd), NULL, TRUE);
		GetSystemTime(&temptime);
		int TheYearNigga = selecteddate>>11;
		DWORD32 messagevalue = 0;
		if (datemonthindex % 12 == 0)
		{
			messagevalue = 12;
			monthvalue = 12;
		}
		else
		{
			messagevalue = datemonthindex % 12;
			monthvalue = messagevalue;
		}
		messagevalue += lindex << 16;
		DWORD32 lmessagevalue = 12120;//flag for setfocus to set date stamp
		lmessagevalue += (TheYearNigga) << 16; //store year in highword lparam for sendmessage below
		datechangecheck = TRUE;
		SendMessageA(TextBoxInput, IDDATECHANGE, 0, 0);
		SetFocus(TextBoxInput);
		Edit_Enable(TextBoxInput, TRUE);
		HWND retard = GetParent(buttonmarks[0]);
		SendMessageA(retard, MARK_DATESIGNAL, 241, 11);
		InvalidateRect(GetParent(buttonmarks[2]), NULL, TRUE);
		UpdateWindow(GetParent(buttonmarks[2]));
		return 0;
	case WM_RBUTTONDOWN://delete color for the date
		paramy = selecteddate;
		SendMessage(GetParent(hwnd), MARK_SIGNAL, paramy, IDCOLORREMOVE);
		return 0;
	case WM_MBUTTONDOWN://delete marks for the date
		paramy = selecteddate;
		SendMessage(GetParent(hwnd), MARK_SIGNAL, paramy, IDMARKSREMOVE);
		return 0;

	case WM_PAINT://date guy unpaints itself like a dumbass
		hdc = BeginPaint(hwnd, &ps);//the leak is happening inbefore
		if (buttondates[30] == hwnd || buttondates[26] == hwnd)
		{
			SendMessage(GetParent(hwnd), WM_SIZE, 0, 0);
		}
		COLORREF inverttextbclr = RGB(GetRValue(~datesbuttoncolor), GetGValue(~datesbuttoncolor), GetBValue(~datesbuttoncolor));
		OVERLAPPED overlapstruct = { 0 };
		overlapstruct.Offset = max(0, ((datemonthindex * 150) - 150))+24+datindice;
		hBrush = CreateSolidBrush(datesbuttoncolor);
		FillRect(hdc, &ps.rcPaint, hBrush);
		char myfilename[1000] = { 0 };
		int myoffset = GetModuleFileNameA(NULL, myfilename, 1000);
		for (int f = myoffset; f > 0 && myfilename[f] != '\\'; f--)
		{
			myfilename[f] = '\0';
			myoffset = f;
		}
		char tempstring[1000] = { 0 };
		sprintf_s(tempstring, 1000, "%sLocaldata.txt", theworkingdirectory);
		hFile = CreateFileA(tempstring, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (FALSE == ReadFile(hFile, charbuffer, 3, NULL, &overlapstruct))
		{
			CloseHandle(hFile);
			CrashDumpFunction(60, 0);
			return 0;
		}
		DeleteObject(hBrush);
		int i = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (dateflags[i] == TRUE)
		{
			SetBkColor(hdc, inverttextbclr);
			SetTextColor(hdc, datesbuttoncolor);
			hBrush = CreateSolidBrush(inverttextbclr);
			FillRect(hdc, &ps.rcPaint, hBrush);
			DeleteObject(hBrush);
		}
		else
		{
			SetBkColor(hdc, datesbuttoncolor);
			SetTextColor(hdc, inverttextbclr);
		}
		SelectObject(hdc, datefont);
		CloseHandle(hFile);
		for (int qt = 0; qt < 1000; qt++)
		{
			tempstring[qt] = '\0';
		}
		TextOutA(hdc, ps.rcPaint.right / 7, ps.rcPaint.bottom / 3, charbuffer, 3);
		sprintf_s(tempstring, 1000, "%sMarksData.txt", theworkingdirectory);
		hFile = CreateFileA(tempstring, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);;
		int filesize = GetFileSize(hFile, NULL);
		char* marksbuffer = calloc(filesize, sizeof(char));
		overlapstruct.Offset = 0;
		if (FALSE == ReadFile(hFile, marksbuffer, filesize, NULL, &overlapstruct))
			markflagmonth = FALSE;
		else
		{
			markflagmonth = marksmonthcheck(marksbuffer, filesize);
		}
		free(marksbuffer);
		marksbuffer = NULL;
		CloseHandle(hFile);
		//repaint the marks through rereading the marksdatafile if its in the space of the current month page.
		if(markflagmonth == TRUE)
		{
			markspaint(dateflags);
		}
		EndPaint(hwnd, &ps);
		return 0;
	case MARK_CHECK_SIGNAL:
		hdc = GetDC(hwnd);
		int color = 0, shape = 0;
		shape = (int)wParam;
		color = (int)lParam;
		RECT localrect2 = { 0 };
		GetClientRect(hwnd, &localrect2);
		if(color > 0)
		{
			HBRUSH localbrush = NULL;
			localbrush = CreateSolidBrush(ColorSelection[color-100]);
			if (localbrush != NULL && hdc != NULL)
			{
				FillRect(hdc, &localrect2, localbrush);
			}
			if(localbrush != NULL)
				DeleteObject(localbrush);
		}
		if (shape > 0)
		{
			shape -= 1;
			SetStretchBltMode(hdc, HALFTONE);
			SetBrushOrgEx(hdc, 0, 0, NULL);
			HDC memdc = CreateCompatibleDC(hdc);
			SelectObject(memdc, MyBitmapsArray[shape]);
			int g = 0;
			BITMAP mybitmap = { 0 };
			g = GetObject(MyBitmapsArray[shape], sizeof(BITMAP), &mybitmap);
			g = StretchBlt(hdc, 0, 0, localrect2.right, localrect2.bottom, memdc, 0, 0, mybitmap.bmWidth, mybitmap.bmHeight, SRCCOPY);
			DeleteObject(memdc);
		}
		ReleaseDC(hwnd, hdc);
		return 0;

	case WM_COMMAND:
		if(wParam == 1488 && lParam == 888)
		{
			for (int k = 0; k < 32; k++)
				dateflags[k] = FALSE;//this will ensure date is graphically unselected upon month change.
		}
		return 0;
	case WM_DESTROY:
		return DestroyButton(hwnd);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

//checks the offset of the mark in marksdata.txt
int MarkPresenceCheck(HANDLE hFile, int filelength, int dateticks, BOOL lswitch, int * shapevalue, int * colorvalue, BOOL * mydflags)//make a switch for mark presence check to be able to be used both as day reader and month reader. Switch should be TRUE when doing "checking" to override existing data" false when reading it just to color shit.
{
	OVERLAPPED localoverlapstruct = { 0 };
	char* TempBuffer = calloc(filelength, sizeof(char));
	if (FALSE == ReadFile(hFile, TempBuffer, filelength, NULL, &localoverlapstruct))
	{
		CrashDumpFunction(80,0);
		return -1;
	}
	int day = 0, month = 0, year = 0, i = 0;
	year = dateticks >> 11;
	month = ((dateticks & 0x7ff) >> 7);
	day = dateticks & 0x7f;//will be used if lswitch is true, otherwise it will be updated later on to a value from the markdata buffer.
	int indexplace = 0;
	int checker = 0;
	BOOL flag69 = FALSE;
	char tempbuffer2[10];
	if (TempBuffer == NULL)
	{
		CrashDumpFunction(81,0);
		return -1;
	}
	while ((TempBuffer[indexplace] == '|') && (indexplace < filelength))
	{
		i = indexplace + 1;//go to indexplace and skip '|'
		indexplace += 18;
		int x = 0;
		_itoa_s(year, tempbuffer2, 5, 10);
		checker = 1;
		while ((tempbuffer2[x] == TempBuffer[i + x]) && (x < 10))//you reading a year
		{
			x++;
			flag69 = TRUE;
			if (x > 4 || (tempbuffer2[x+1] == TempBuffer[i + x] && x<3))
			{
				checker -= 1;
				flag69 = FALSE;
			}
		}
		if (x != 4)
		{
			checker -= 1;
			flag69 = FALSE;
		}
		i += x;
		if (TempBuffer[i++] == ',' && flag69 == TRUE)
		{
			if (NULL == (TempBuffer = tempbuffermarkchecker(&flag69, &checker, tempbuffer2, TempBuffer, lswitch, &day, mydflags, &i, month, &indexplace, colorvalue, shapevalue)))
				return indexplace;
		}
	}
	if (TempBuffer != NULL)
	{
		free(TempBuffer);
		TempBuffer = NULL;
	}
	return -2;
}

HWND datetickfunction(int datetick, char * filebuffer, int * i)
{
	HWND refferentdate = { 0 };
	if (datetick > 0)//here we get hwnd and hdc
	{
		int year = 0, month = 0, day = 0;
		year = datetick >> 11;
		_itoa_s(year, filebuffer + (*i), 5, 10);
		(*i) += 4;
		filebuffer[(*i)++] = ',';
		month = ((datetick & 0x7ff) >> 7);
		if (month < 10)
		{
			filebuffer[(*i)++] = '0';
			_itoa_s(month, filebuffer + (*i), 2, 10);
		}
		else
		{
			_itoa_s(month, filebuffer + (*i), 3, 10);
			++*i;
		}
		++*i;
		filebuffer[(*i)++] = ',';
		day = datetick & 0x7f;
		TrustedIndex3 = day;//to avoid spectreloadmitigation
		if (TrustedIndex3 < 10)
		{
			filebuffer[(*i)++] = '0';
			_itoa_s(TrustedIndex3, filebuffer + (*i), 2, 10);
		}
		else
		{
			_itoa_s(TrustedIndex3, filebuffer + (*i), 3, 10);
			++*i;
		}
		(*i) += 1;
		if (day < 33)
			refferentdate = buttondates[day - 1];
		return refferentdate;
	}
	CrashDumpFunction(75,0);
	return NULL;
}

BOOL DateColoring(int colorid, HWND refferentdate, HDC hdc, char * filebuffer, int * i)
{
	HBRUSH hBrush = { 0 };
	hBrush = CreateSolidBrush(ColorSelection[colorid-100]);
	RECT localrect = { 0 };
	if (refferentdate == NULL)
	{
		CrashDumpFunction(82,0);
		return 0;
	}
	GetClientRect(refferentdate, &localrect);
	FillRect(hdc, &localrect, hBrush);
	ReleaseDC(refferentdate, hdc);
	_itoa_s(colorid, filebuffer + *i, 4, 10);
	*i += 3;
	DeleteObject(hBrush);
	return 1;
}

BOOL DateShaping(int shapeid, char* filebuffer, int* i, int datetick)
{
	int day = datetick & 0x7f;
	int shape = 0;
	HDC hdc = 0;
	if (day > 0 && day < 32)
		hdc = GetDC(buttondates[day - 1]);
	else return FALSE;
	HDC memdc = CreateCompatibleDC(hdc);//create compatible DC memory
	RECT DateRect = { 0 };
	GetWindowRect(buttondates[day], &DateRect);//all dates have the same size
	shape = shapeid-1;
	SetStretchBltMode(hdc, HALFTONE);
	SetBrushOrgEx(hdc, 0, 0, NULL);
	memdc = CreateCompatibleDC(hdc);
	SelectObject(memdc, MyBitmapsArray[shape]);
	int cx = DateRect.right - DateRect.left;
	int cy = DateRect.bottom - DateRect.top;
	int g = 0;
	BITMAP mybitmap = { 0 };
	g = GetObject(MyBitmapsArray[shape], sizeof(BITMAP), &mybitmap);
	g = StretchBlt(hdc, 0, 0, cx, cy, memdc, 0, 0, mybitmap.bmWidth, mybitmap.bmHeight, SRCCOPY);
	DeleteObject(memdc);
	ReleaseDC(buttondates[day], hdc);
	if (shapeid < 10)
	{
		filebuffer[*i] = '0';
		*i += 1;
		_itoa_s(shapeid, filebuffer + *i, 2, 10);
		*i += 1;
		return TRUE;
	}
	else
	{
		_itoa_s(shapeid, filebuffer + *i, 3, 10);
		*i += 2;
		return TRUE;
	}
}

HWND ColorRemovalRoutine(int offsetpresent, int shiuze, OVERLAPPED fuckshit, HANDLE hFile, HWND refferentdate, int datetick)
{
	OVERLAPPED localoffset = { 0 };
	localoffset.Offset = fuckshit.Offset;
	if (offsetpresent >= 0)
	{
		char markdata[40] = { 0 };
		int sizetoedit = shiuze - fuckshit.Offset;
		char* filebuffer = calloc(sizetoedit, sizeof(char));
		int returnval = 0;
		if (ReadFile(hFile, filebuffer, sizetoedit, &returnval, &localoffset) == FALSE || filebuffer == NULL)
		{
			returnval = GetLastError();
			free(filebuffer);
			CrashDumpFunction(40, 0);
			return refferentdate;
		}
		int i = 1;
		refferentdate = datetickfunction(datetick, markdata, &i);
		if (*(filebuffer + 12) == '0' && *(filebuffer + 13) == '0')//shapeid is empty, hence delete whole dateset
		{
			WriteFile(hFile, filebuffer + 18, sizetoedit - 18, NULL, &localoffset);
			SetFilePointer(hFile, -18, NULL, FILE_END);
			SetEndOfFile(hFile);
			free(filebuffer);
			InvalidateRect(refferentdate, NULL, TRUE);
			UpdateWindow(refferentdate);
			return refferentdate;
		}
		else
		{
			markdata[0] = '|';
			markdata[i++] = specialchar[0];
			markdata[i] = filebuffer[i];//load shapes from buffer
			i++;
			markdata[i] = filebuffer[i];
			i++;
			markdata[i++] = specialchar[0];
			markdata[i++] = '0';
			markdata[i++] = '0';
			markdata[i++] = '0';
			WriteFile(hFile, markdata, 18, NULL, &localoffset);
			free(filebuffer);
			InvalidateRect(refferentdate, NULL, TRUE);
			UpdateWindow(refferentdate);
			return refferentdate;
		}
	}
	return refferentdate;
}

HWND MarkRemoveRoutine(int offsetpresent, int shiuze, OVERLAPPED fuckshit, HANDLE hFile, int datetick, HWND refferentdate)
{
	if (offsetpresent >= 0)
	{
		char markdata[40] = { 0 };
		int sizetoedit = shiuze - fuckshit.Offset;
		char* filebuffer = calloc(sizetoedit, sizeof(char));
		int returnval = 0;
		if ((ReadFile(hFile, filebuffer, sizetoedit, NULL, &fuckshit) == FALSE) || filebuffer == NULL)
		{
			free(filebuffer);
			CrashDumpFunction(42, 0);
			return refferentdate;
		}
		int i = 1;
		refferentdate = datetickfunction(datetick, markdata, &i);
		if (*(filebuffer + 15) == '0')//colors are empty thus delete whole markdata set
		{
			WriteFile(hFile, filebuffer + 18, sizetoedit - 18, NULL, &fuckshit);
			SetFilePointer(hFile, -18, NULL, FILE_END);
			SetEndOfFile(hFile);
			free(filebuffer);
			InvalidateRect(refferentdate, NULL, TRUE);
			UpdateWindow(refferentdate);
			return refferentdate;
		}
		else
		{
			markdata[0] = '|';
			markdata[i++] = specialchar[0];
			markdata[i++] = '0';
			markdata[i++] = '0';
			markdata[i++] = specialchar[0];
			for (; i < 18; i++)
				markdata[i] = filebuffer[i];//paste the color data
			WriteFile(hFile, markdata, 18, NULL, &fuckshit);
			free(filebuffer);
			InvalidateRect(refferentdate, NULL, TRUE);
			UpdateWindow(refferentdate);
			return refferentdate;
		}
	}
	return refferentdate;
}

char * marksbuffermodifier(int shapeid, int colorid, char * filebuffer, int datetick, int * i, int presentshapeval, HWND refferentdate, HDC hdc, int presentcolorval)
{
	if (shapeid > 0)//shapes
	{
		if (FALSE == DateShaping(shapeid, filebuffer, i, datetick))
		{
			CrashDumpFunction(45, 0);
			filebuffer = NULL;
			return NULL;
		}
	}
	if (shapeid == 0)
	{
		if (presentshapeval < 10)
			filebuffer[(*i)++] = '0';
		_itoa_s(presentshapeval, filebuffer + *i, (size_t)(40 - *i), 10);
		(*i)++;
	}
	filebuffer[(*i)++] = specialchar[0];
	if (colorid > 0) //colors
	{
		if (FALSE == DateColoring(colorid, refferentdate, hdc, filebuffer, i))
		{
			CrashDumpFunction(46, 0);
			filebuffer = NULL;
		}
	}
	else
	{
		if (presentcolorval == 0)
		{
			filebuffer[(*i)++] = '0';
			filebuffer[(*i)++] = '0';
		}
		_itoa_s(presentcolorval, filebuffer + *i, (size_t)(40 - *i), 10);
	}
	return filebuffer;
}

//checks if there is marksdata on selected month
#pragma warning(disable:4702)//idiot warning supression
bool marksmonthcheck(char * marksbuffer, int filesize)
{
	int lyear = 0;
	int lmonth = 0;
	int lflag1 = 0;
	char localbuffer[6] = { 0 };
	if (MonthYearIndex >= 12)
		lyear = (MonthYearIndex / 12) + yearzero;
	else
		lyear = yearzero;
	lmonth = MonthYearIndex % 12;
	if (lmonth == 0)
		lmonth = 12;
	if (marksbuffer != NULL)
	{
		for (int qn = 0; (marksbuffer[qn] == '|') && (qn < filesize); qn += 18)
		{
			lflag1 = 0;
			int z = qn;
			z++;
			_itoa_s(lyear, localbuffer, 5, 10);
			for (int g = 0; marksbuffer[z] == localbuffer[g] && g < 4; g++, z++)
			{
				if (g == 2 && marksbuffer[z + 1] == localbuffer[g + 1])
				{
					lflag1 = 1;
				}
			}
			z++;//go past the ','
			if (lflag1 == 1)
			{
				if (lmonth < 10)
				{
					_itoa_s(lmonth, localbuffer + 1, 2, 10);
					localbuffer[0] = '0';
				}
				else
				{
					_itoa_s(lmonth, localbuffer, 3, 10);
				}
				localbuffer[2] = '\0';
				for (int g = 0; marksbuffer[z] == localbuffer[g] && g < 2; g++, z++)
				{
					if (g == 0 && marksbuffer[z + 1] == localbuffer[g + 1])
					{
						lflag1 += 1;
					}
				}
			}
			if (lflag1 == 2)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}
#pragma warning(default:4702)//idiot warning supression

//paints the date if marks present
bool markspaint(BOOL* dateflags)
{
	OVERLAPPED overlapstruct = { 0 };
	HANDLE hFile = { 0 };
	int filesize = 0;
	char tempstring1[1000] = { 0 };
	sprintf_s(tempstring1, 1000, "%sMarksData.txt", theworkingdirectory);
	hFile = CreateFileA(tempstring1, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	filesize = GetFileSize(hFile, NULL);
	char* marksbuffer = calloc(filesize, sizeof(char));
	overlapstruct.Offset = 0;
	if (FALSE == ReadFile(hFile, marksbuffer, filesize, NULL, &overlapstruct))
	{
		CloseHandle(hFile);
		free(marksbuffer);
		marksbuffer = NULL;
		CrashDumpFunction(61, 0);
		return FALSE;
	}

	MarkPresenceCheck(hFile, filesize, selecteddate, FALSE, NULL, NULL, dateflags);
	CloseHandle(hFile);
	free(marksbuffer);
	marksbuffer = NULL;
	return TRUE;
}

char * dataovverridingsmarks(char * tempbuffer2, char * TempBuffer, BOOL * flag69, int day, int * indexplace, int * colorvalue, int * shapevalue, int * checker, int * i)
{
	(*i)--;//to get position to the beggining of the day
	*flag69 = TRUE;
	if (day >= 10)
		_itoa_s(day, tempbuffer2, 3, 10);
	if (day < 10)
	{
		_itoa_s(day, tempbuffer2 + 1, 3, 10);
		tempbuffer2[0] = '0';
	}
	tempbuffer2[2] = '\0';
	if (TempBuffer[(*i)++] == ',')//lastly check if dates are equal, if they are, return overriding signal by sending position of the data to be overriden
	{
		*checker = 0;
		for (int x = 0; (TempBuffer[(*i) + x] == tempbuffer2[x] && *flag69 == TRUE) && (x < 10); x++)
		{
			if (x > 1)
			{
				*flag69 = FALSE;//break the loop data is wack or corrupt!
			}
			(*checker)++;

		}
		if ((*flag69 == TRUE) && (*checker == 2))
		{
			//get indexplace, colorvalue and symbolvalue
			//get colorvalue
			tempbuffer2[3] = '\0';
			tempbuffer2[2] = TempBuffer[*indexplace - 1];
			tempbuffer2[1] = TempBuffer[*indexplace - 2];
			tempbuffer2[0] = TempBuffer[*indexplace - 3];
			*colorvalue = strtol(tempbuffer2, NULL, 10);
			tempbuffer2[2] = '\0';
			tempbuffer2[1] = TempBuffer[*indexplace - 5];
			tempbuffer2[0] = TempBuffer[*indexplace - 6];
			*shapevalue = strtol(tempbuffer2, NULL, 10);
			free(TempBuffer);
			TempBuffer = NULL;
			*indexplace = *indexplace - 18;
		}
	}
	return TempBuffer;
}

HWND colorshapepresent(HWND refferentbutton, BOOL * mydflags, int shape, int color, int day)
{
	RECT myrect = { 0 };
	HDC hdc = { 0 };
	HANDLE tmpfile = { 0 };
	HBRUSH hBrush = { 0 };
	hdc = GetDC(refferentbutton);
	GetClientRect(refferentbutton, &myrect);
	int rp = (int)GetWindowLongPtr(refferentbutton, GWLP_USERDATA);
	OVERLAPPED overlapstruct = { 0 };
	int mydateindice = (day - 1) * 3 + ((day - 1) / 7) * 3;
	overlapstruct.Offset = max(0, ((datemonthindex * 150) - 150)) + 24 + mydateindice;
	char charbuffer[500] = { 0 };
	char tempstring[1000] = { 0 };
	COLORREF nigger2 = RGB(GetRValue(~ColorSelection[color - 100]), GetGValue(~ColorSelection[color - 100]), GetBValue(~ColorSelection[color - 100]));
	sprintf_s(tempstring, 1000, "%sLocaldata.txt", theworkingdirectory);
	tmpfile = CreateFileA((LPCSTR)tempstring, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FALSE == ReadFile(tmpfile, charbuffer, 3, NULL, &overlapstruct))
	{
		CloseHandle(tmpfile);
		CrashDumpFunction(70,0);
		return NULL;
	}
	SendMessage(refferentbutton, MARK_CHECK_SIGNAL, shape, color);
	if (mydflags != NULL && mydflags[rp] == TRUE)
	{
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, nigger2);
		hBrush = CreateSolidBrush(datesbuttoncolor);
		PatBlt(hdc, myrect.left, myrect.top, myrect.right - myrect.left, myrect.bottom - myrect.top, DSTINVERT);
		FrameRect(hdc, &myrect, hBrush);
		DeleteObject(hBrush);
	}
	else
	{
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, nigger2);
	}
	SelectObject(hdc, datefont);
	TextOutA(hdc, myrect.right / 7, myrect.bottom / 3, charbuffer, 3);
	ReleaseDC(refferentbutton, hdc);
	CloseHandle(tmpfile);
	if(hBrush != NULL)
		DeleteObject(hBrush);

	return refferentbutton;
}

char* tempbuffermarkchecker(BOOL * flag69, int *checker, char * tempbuffer2, char * TempBuffer, BOOL lswitch, int * day, BOOL * mydflags, int * i, int month, int * indexplace, int * colorvalue, int * shapevalue)
{
	*flag69 = FALSE;
	*checker += 1;
	int x = 0;
	if (month >= 10)
		_itoa_s(month, tempbuffer2, 3, 10);
	if (month < 10)
	{
		_itoa_s(month, tempbuffer2 + 1, 2, 10);
		tempbuffer2[0] = '0';
	}
	tempbuffer2[2] = '\0';
	while ((tempbuffer2[x] == TempBuffer[*i + x]) && (x < 10))//you reading a month
	{
		x++;
		*flag69 = TRUE;
		if (x > 2)
		{
			*checker -= 1;
			*flag69 = FALSE;
		}
	}
	*i += x;
	x = 0;
	if (TempBuffer[(*i)++] == ',' && *flag69 == TRUE)
	{
		if (lswitch == FALSE)//reading data for painting
		{
			*flag69 = FALSE;
			*checker += 1;
			*day = atoi(&(TempBuffer[*i]));
			if (*day >= 10)
				_itoa_s(*day, tempbuffer2, 3, 10);
			if (*day < 10)
			{
				_itoa_s(*day, tempbuffer2 + 1, 2, 10);
				tempbuffer2[0] = '0';
			}
			tempbuffer2[2] = '\0';
			HWND refferentbutton = NULL;
			refferentbutton = buttondates[*day - 1];//-1 cause logically dates begin with 1 being 0, albeit visuallty 0 is 1.
			*i += 2;
			if ((refferentbutton != NULL) && (TempBuffer[(*i)++] == specialchar[0]))//pack the color and shape to wparam and lparam, sned it out to the said button
			{
				int shape = 0, color = 0;
				//read shape
				shape = atoi(&(TempBuffer[*i]));
				//read color;
				(*i) += 3;
				color = atoi(&(TempBuffer[*i]));
				if (color > 0 || shape > 0)
				{
					refferentbutton = colorshapepresent(refferentbutton, mydflags, shape, color, *day);
					if (refferentbutton == NULL)
					{
						CrashDumpFunction(73, 0);
						return TempBuffer;
					}
				}
			}
		}
		if (lswitch == TRUE)//reading data for overriding
		{
			if (NULL == (TempBuffer = dataovverridingsmarks(tempbuffer2, TempBuffer, flag69, *day, indexplace, colorvalue, shapevalue, checker, i)))
				return NULL;
		}
	}
	return TempBuffer;
}

WCHAR** fontlist = { 0 };
int fontamount = 0;


INT_PTR CALLBACK FontBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndtemp = 0, hwndFont = 0, controlhwnd = 0;
	int value = 0;
	int static fontvalue = 0, fuckyounigger = 0;
	CHARFORMAT cf = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		FontBoxHwnd = hwnd;
		//fill the font type list
		HDC hdc = GetDC(hwnd);
		LOGFONT logfont = { 0 };
		EnumFontFamiliesExW(hdc, &logfont, EnumFontFamilyProc, 0, 0);
		controlhwnd = GetDlgItem(hwnd, IDC_FONTTYPE);
		for (int i = 0; i < fontamount - 1; i++)
		{
			ComboBox_InsertString(controlhwnd, i, fontlist[i]);
		}
		hwndFont = GetDlgItem(hwnd, IDC_EDITFONT);
		char tempstring[12] = { 0 };
		fontvalue = 8;
		_itoa_s(8, tempstring, 12, 10);
		SetWindowTextA(hwndFont, tempstring);
		controlhwnd = GetDlgItem(hwnd, IDC_FONTCOLOR);
		for (int i = 0; i < 138; i++)
		{
			((int)(DWORD)SendMessageA((controlhwnd), 0x014A, (WPARAM)(int)(i), (LPARAM)(LPCTSTR)(colorsdata[i].colorname)));
		}
		controlhwnd = GetDlgItem(hwnd, IDC_FONTBKG);
		for (int i = 0; i < 138; i++)
		{
			((int)(DWORD)SendMessageA((controlhwnd), 0x014A, (WPARAM)(int)(i), (LPARAM)(LPCTSTR)(colorsdata[i].colorname)));
		}
		controlhwnd = GetDlgItem(hwnd, IDC_FUCOLOR);//
		for (int i = 0; i < 149-133; i++)
		{
			((int)(DWORD)SendMessageA((controlhwnd), 0x014A, (WPARAM)(int)(i), (LPARAM)(LPCTSTR)(colorsdata[133+i].colorname)));
		}
		controlhwnd = GetDlgItem(hwnd, IDC_FUNDERLINE);
		char tempstring22[200] = "Cf1,Underline,Invert,Dash,DashDot,DashDotDot,Dotted,Double,DoubleWave,Hairline,HeavyWave,LongDash,None,Thick,ThickDash,ThickDashDot,ThickDashDotDot,ThickDotted,ThickLongDash,Wave,Word";
		for (int i = 0, m = 0; i < 21; i++)
		{
			char* stringtosend[30] = { 0 };
			_memccpy(stringtosend, tempstring22 + m, ',', 30);
			m += strlen(stringtosend);
			((int)(DWORD)SendMessageA((controlhwnd), 0x014A, (WPARAM)(int)(i), (LPARAM)(LPCTSTR)(stringtosend)));
		}
		ReleaseDC(hwnd, hdc);
		break;
	case WM_COMMAND:
		hwndtemp = GetDlgItem(hwnd, LOWORD(wParam));
		CHARFORMAT2 cfrm = { 0 };
		switch (LOWORD(wParam))
		{//RADIOS
		case IDC_RBOLD:
			value = Button_GetCheck(hwndtemp);
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				if (value == BST_UNCHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_BOLD;
					cfrm.dwEffects = CFE_BOLD;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_BOLD;
					cfrm.dwEffects = 0;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
			}
			break;
		case IDC_RITALIC:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				value = Button_GetCheck(hwndtemp);
				if (value == BST_UNCHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_ITALIC;
					cfrm.dwEffects = CFM_ITALIC;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_ITALIC;
					cfrm.dwEffects = 0;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
			}
			break;
		case IDC_RUNDERLINE:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				value = Button_GetCheck(hwndtemp);
				if (value == BST_UNCHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_UNDERLINE;
					cfrm.dwEffects = CFM_UNDERLINE;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_UNDERLINE;
					cfrm.dwEffects = 0;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
			}
			break;
		case IDC_RSTRIKEOUT:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				value = Button_GetCheck(hwndtemp);
				if (value == BST_UNCHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_STRIKEOUT;
					cfrm.dwEffects = CFM_STRIKEOUT;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_STRIKEOUT;
					cfrm.dwEffects = 0;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
			}
			break;
		case IDC_RSUPERSCRIPT:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				value = Button_GetCheck(hwndtemp);
				if (value == BST_UNCHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_OFFSET | CFM_SIZE;
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.yHeight /= 2;
					cfrm.yOffset = 100;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_OFFSET | CFM_SIZE;
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.yHeight *= 2;
					cfrm.yOffset = 0;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
			}
			break;
		case IDC_SUBSCRIPT:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				value = Button_GetCheck(hwndtemp);
				if (value == BST_UNCHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_OFFSET | CFM_SIZE;
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.yHeight /= 2;
					cfrm.yOffset = -100;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_OFFSET | CFM_SIZE;
					SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.yHeight *= 2;
					cfrm.yOffset = 0;
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
			}
			break;
			//radios
		case IDC_EDITFONT:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				value = GetDlgItemInt(FontBoxHwnd, IDC_EDITFONT, 0, FALSE);
				SendMessage(TextBoxHwnd, EM_SETFONTSIZE, -(fontvalue - value), 0);
				fontvalue = value;
			}
			break;
		case IDC_FONTTYPE:
			switch (HIWORD(wParam))
			{
			case CBN_SELENDOK:
				value = SendMessage(hwndtemp, CB_GETCURSEL, 0, 0);
				//HFONT hFont = CreateFontIndirectA(Fontdata[value]);
				cfrm.cbSize = sizeof(CHARFORMAT2);
				cfrm.dwMask = CFM_FACE;
				wmemcpy_s(cfrm.szFaceName, 32, fontlist[value], 32);
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
			}
			break;
		case IDC_FONTCOLOR:
			switch (HIWORD(wParam))
			{
			case CBN_SELENDOK:
				value = SendMessage(hwndtemp, CB_GETCURSEL, 0, 0);
				cfrm.cbSize = sizeof(CHARFORMAT2);
				cfrm.dwMask = CFM_COLOR;
				cfrm.crTextColor = colorsdata[value].RGBcolor;
				cfrm.dwEffects = 0;
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				break;
			}
			break;
		case IDC_FONTBKG:
			switch (HIWORD(wParam))
			{
			case CBN_SELENDOK:
				value = SendMessage(hwndtemp, CB_GETCURSEL, 0, 0);
				cfrm.cbSize = sizeof(CHARFORMAT2);
				cfrm.dwMask = CFM_BACKCOLOR;
				cfrm.crBackColor = colorsdata[value].RGBcolor;
				cfrm.dwEffects = 0;
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
			}
			break;
		case IDC_FUNDERLINE:
			switch (HIWORD(wParam))
			{
			case CBN_SELENDOK:
				value = SendMessage(hwndtemp, CB_GETCURSEL, 0, 0);//enumerate underline types
				cfrm.cbSize = sizeof(CHARFORMAT2);
				cfrm.dwMask = CFM_UNDERLINETYPE;
				switch(value)
				{
				case 0:
					cfrm.bUnderlineType = CFU_CF1UNDERLINE;
					break;
				case 1:
					cfrm.bUnderlineType = CFU_INVERT;
					break;
				case 2:
					cfrm.bUnderlineType = CFU_UNDERLINE;
					break;
				case 3:
					cfrm.bUnderlineType = CFU_UNDERLINEDASH;
					break;
				case 4:
					cfrm.bUnderlineType = CFU_UNDERLINEDASHDOT;
					break;
				case 5:
					cfrm.bUnderlineType = CFU_UNDERLINEDASHDOTDOT;
					break;
				case 6:
					cfrm.bUnderlineType = CFU_UNDERLINEDOTTED;
					break;
				case 7:
					cfrm.bUnderlineType = CFU_UNDERLINEDOUBLE;
					break;
				case 8:
					cfrm.bUnderlineType = CFU_UNDERLINEDOUBLEWAVE;
					break;
				case 9:
					cfrm.bUnderlineType = CFU_UNDERLINEHAIRLINE;
					break;
				case 10:
					cfrm.bUnderlineType = CFU_UNDERLINEHEAVYWAVE;
					break;
				case 11:
					cfrm.bUnderlineType = CFU_UNDERLINELONGDASH;
					break;
				case 12:
					cfrm.bUnderlineType = CFU_UNDERLINENONE;
					break;
				case 13:
					cfrm.bUnderlineType = CFU_UNDERLINETHICK;
					break;
				case 14:
					cfrm.bUnderlineType = CFU_UNDERLINETHICKDASH;
					break;
				case 15:
					cfrm.bUnderlineType = CFU_UNDERLINETHICKDASHDOT;
					break;
				case 16:
					cfrm.bUnderlineType = CFU_UNDERLINETHICKDASHDOTDOT;
					break;
				case 17:
					cfrm.bUnderlineType = CFU_UNDERLINETHICKDOTTED;
					break;
				case 18:
					cfrm.bUnderlineType = CFU_UNDERLINETHICKLONGDASH;
					break;
				case 19:
					cfrm.bUnderlineType = CFU_UNDERLINEWAVE;
					break;
				case 20:
					cfrm.bUnderlineType = CFU_UNDERLINEWORD;
					break;
				default:
					break;
				}
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
			}
			break;
		case IDC_FUCOLOR:
			switch (HIWORD(wParam))
			{
			case CBN_SELENDOK:
				value = SendMessage(hwndtemp, CB_GETCURSEL, 0, 0);
				cfrm.cbSize = sizeof(CHARFORMAT2);
				cfrm.dwMask = CFM_BACKCOLOR;
				cfrm.bUnderlineColor = colorsdata[value].RGBcolor;//fix this read documentation about it
				cfrm.dwEffects = 0;
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
				SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				break;
			}
		default:
			return FALSE;
		}
		return FALSE;
	}
	return 0;
}


int CALLBACK EnumFontFamilyProc(const LOGFONT* lpelfe, const void* lpntme, DWORD      FontType, LPARAM     lParam)
{
	static int amount = 3;
	static int index = 0;
	WCHAR fontname[100] = { 0 };
	//*fontname = lpelfe->lfFaceName;
	wmemcpy_s(fontname, 100,lpelfe->lfFaceName, 32);
	if (fontname[0] == '\0')
		return 0;
	if (fontlist == NULL)
	{
		fontlist = calloc(sizeof(WCHAR*), 2);
		Fontdata = calloc(sizeof(LOGFONT*), 2);
	}
	else
	{
		fontlist = realloc(fontlist, sizeof(WCHAR*) * amount);
		Fontdata = realloc(Fontdata, sizeof(LOGFONT*) * amount);
	}
	fontlist[index] = calloc(32, sizeof(WCHAR));
	wmemcpy_s(fontlist[index], 100, fontname, 32);
	Fontdata[index] = lpelfe;
	//wmemcpy_s(Fontdata[index], sizeof(LOGFONT), lpelfe, sizeof(LOGFONT));
	index++;
	amount++;
	fontamount = index+1;
	return 1;
}

int ColorsMaker(RGBData mycolors[])
{
	char* colornames = "Black,White,Red,Green,Blue,Yellow,Cyan(Aqua),Magenta(Fuchsia),Silver,Gray,Maroon,Olive,DarkGreen,Purple,Teal,Navy,DarkRed,Brown,Firebrick,Crimson,Tomato,Coral,IndianRed,LightCoral,DarkSalmon,Salmon,LightSalmon,OrangeRed,DarkOrange,Orange,Gold,DarkGoldenrod,Goldenrod,PaleGoldenrod,DarkKhaki,Khaki,YellowGreen,DarkOliveGreen,OliveDrab,LawnGreen,Chartreuse,GreenYellow,DarkGreen,ForestGreen,LimeGreen,LightGreen,PaleGreen,DarkSeaGreen,MediumSpringGreen,SpringGreen,SeaGreen,MediumAquamarine,MediumSeaGreen,LightSeaGreen,DarkCyan,LightCyan,DarkTurquoise,Turquoise,MediumTurquoise,PaleTurquoise,Aquamarine,PowderBlue,CadetBlue,SteelBlue,CornflowerBlue,DeepSkyBlue,DodgerBlue,LightBlue,SkyBlue,LightSkyBlue,MidnightBlue,DarkBlue,MediumBlue,RoyalBlue,BlueViolet,Indigo,DarkSlateBlue,SlateBlue,MediumSlateBlue,MediumPurple,DarkMagenta,DarkViolet,DarkOrchid,MediumOrchid,Thistle,Plum,Violet,Orchid,MediumVioletRed,PaleVioletRed,DeepPink,HotPink,LightPink,Pink,AntiqueWhite,Beige,Bisque,BlanchedAlmond,Wheat,Cornsilk,LemonChiffon,LightGoldenrodYellow,LightYellow,SaddleBrown,Sienna,Chocolate,Peru,SandyBrown,Burlywood,Tan,RosyBrown,Moccasin,NavajoWhite,PeachPuff,MistyRose,LavenderBlush,Linen,OldLace,PapayaWhip,SeaShell,MintCream,SlateGray,LightSlateGray,LightSteelBlue,Lavender,FloralWhite,AliceBlue,GhostWhite,Honeydew,Ivory,Azure,Snow,DimGray,DarkGray,LightGray,Gainsboro,WhiteSmoke,Black,Blue,Aqua,Lime,Fuchsia,Red,Yellow,White,Navy,Teal,Green,Purple,Maroon,Olive,DkGray,LtGray";

	COLORREF colorset[160] = { 0x000000, 0xFFFFFF, 0x0000FF, 0x00FF00, 0xFF0000, 0xFFFF00, 0x00FFFF, 0xFF00FF, 0xC0C0C0, 0x808080, 0x800000, 0x808000, 0x008000, 0x800080, 0x008080, 0x000080, 0x00008B, 0x2A2A8B, 0x2222B2, 0x3C14DC, 0x47FF63, 0x50FF7F, 0x5C5CCD, 0x80F0F0, 0x7A96E9, 0x72FA80, 0x7AFFA0, 0x4500FF, 0x8C00FF, 0xA500FF, 0xD700FF, 0x0B86B8, 0x520DAA, 0xAAEEE8, 0xB76BDB, 0x8CE0F0, 0x32CD9A, 0x2F6B55, 0x238E6B, 0x00FC7C, 0x00FF7F, 0x2E8B57, 0xAACD66, 0x371BC3, 0xB2AA20, 0x8B8B00, 0xFFFFE0, 0xD1CE00, 0xD0E040, 0xCC48D1, 0xEAEFAF, 0xD4DFFF, 0xE0D6B0, 0xA09E5F, 0xB4689E, 0xED6495, 0xBFFF00, 0x90E01E, 0xE4A0CD, 0xFA87CE, 0x970919, 0x8B0000, 0xCD0000, 0xE16941, 0xE2BE8A, 0x2D4B00, 0x8B3D48, 0xCD6A5A, 0xEE68B7, 0xDB7093, 0x8B008B, 0xD30094, 0xCC3299, 0xD3BA55, 0xD8BFD8, 0xDDA0DD, 0xEE82EE, 0xD670DA, 0x855C7D, 0x9300B0, 0x93DDF2, 0xB4F7F1, 0xF1F5F1, 0xD4E4FF, 0xB5E4C4, 0xD9FAE6, 0xB3F0E4, 0xA0E2D6, 0x8BADDC, 0x9B91C0, 0xD6E9FF, 0xE6F1F0, 0xF0F5F6, 0x8B4513, 0x2D52A0, 0x1E2691, 0x3F853F, 0x4604F4, 0xB8E4E8, 0xB48E2D, 0xB8D9B0, 0x8C4B9E, 0xB5E4B5, 0xDEADFF, 0xD9FFD1, 0xE1E4E4, 0xF0F5F0, 0xFAEBD7, 0xF5F5DC, 0xE4C4FF, 0xEBC3FF, 0xD3B5E7, 0xD1E6E6, 0xD6F8F5, 0xE6F5E0, 0xF5E3E0, 0xFAFAF0, 0x909070, 0x899799, 0xDEB0C4, 0xFAE6E6, 0xF0FFFA, 0xF0F8FF, 0xF8F8FF, 0xF0FFF0, 0xFFFFF0, 0xF0FFFF, 0xFAFAFF, 0x696969, 0xA9A9A9, 0xD3D3D3, 0xDCDCDC, 0xF5F5F5, 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};

	char* temppointer = colornames;
	int m = 0;
	for (int i = 0; i < 150; i++)
	{
		_memccpy(mycolors[i].colorname, colornames+m, ',', 50);
		mycolors[i].RGBcolor = colorset[i];
		int g = strlen(mycolors[i].colorname);
		mycolors[i].colorname[g-1] = 0;
		m += g;
	}

	

	return 0;
}