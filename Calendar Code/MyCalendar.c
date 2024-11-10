/*
************

MyCalendar.c, This is the main WinMain located.
- 
************
*/

#include "framework.h" //All header includes stuff is mashed up over theres

int ordereddatasave = 0;//when on will re-arrange existing data into ordered state.
char* macrolist[256] = {0};
RECT buttonrect;//month button rect
RECT buttonrectd;//day buttons rect, declared multiple times, due to some unknown bug, you need to change all of them for it to work properly
int pusenjekurca = 0;//used for buttonrectd.bottom declarations, due to the fact they are declared multiple times cause WIN32 is made by good guys
HANDLE myHeap = { 0 };//Used as handle for inputbuf heap
char theworkingdirectory[1000] = { 0 };//stores the working
char datasource[2000] = { 0 };//stores the data source location that is being read and appended to
HWND mainHwnd = { 0 };//the apps main hwnd is stored here
int yearzero = 0, yearrange = 0;
HWND TextBoxInput = { 0 };
int startyear = 0, startmonth = 0, startday = 0;
char specialchar[2] = { 0 };
BOOL RTForTXT = 0;//data will be imported/exported in either .rtf or .txt file/format

//Sizes of the 4 main child windows, used by only this logical unit
RECT MonthRect = { 0 };
RECT DatesRect = { 0 };
RECT StringRect = { 0 };
RECT MarksRect = { 0 };
BOOL GlobalDebugFlag = FALSE;//used when checking the debug box, to represent debug data
int TrustedIndex = 0;//mittigating "Speculative Execution Side Channel hardware vulnerabilities" for some i loops.
long long pchinputbuffermemory = 30 * 500;
BOOL TextBoxFlag = FALSE;//Signal, its only on and only when you click dates or month
BOOL TextBoxCalFlag = FALSE;//Signal, its only on and only on when you click Month

char* ScriptFormat(char* Inputbuffer, HWND parenthwnd);
INT_PTR CALLBACK DlgSettingsProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK BufTextProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgHelpAndInfoProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgScriptedInput(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgScriptMacros(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgScriptDates(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MarkBoxInputSrc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ListBoxSrc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgColorWindows(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgMonthsRange(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL RangedDataWipe(int monthup, int monthdown, int yearup, int yeardown, int dayup, int daydown);
char* DateInput(int mflag, int filesize, char* readbuffer, int* appendindexlocation, char* dateset, int i);
char* yearange(char* readbuffer, int filesize, char* dateset, int* appendindexlocation);
char* monthrange(int filesize, char* readbuffer, char* dateset, int* appendindexlocation);
BOOL CalendarCreate(BOOL RealorCustom, int startyear, int newyearrange);
char* whcararryinputroutine(char* wchararray, int* dateprint, int monthtype, int y);
char* monthtypegen(char* wchararray, int dateyear, int datemonth, int * monthtype, int* thirty, int* leap, int* thirtyone);
char* MacroFormating(char* macroformated, HWND parenthwnd, BOOL firstrun);
int NumbersFunction(int* symbolsarray, int maxsymbols, char* macroformated, int numbersbegginingindex, BOOL FloatFlag, int * lastbyteindex);
BOOL comparestrings(char* string1, char* string2);

char* DataSaveReordering(char* readbuffer);
int NearestDate(int mflag, int filesize, char* readbuffer, int* appendindexlocation, char* dateset, int i);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstances, _In_ LPWSTR pCmdLine, _In_ int nShowCmd)
{
	HWND hwnd = { 0 };
	MSG msg;
	static TCHAR MenuName[] = TEXT("MyMenu");

	int myoffset = GetModuleFileNameA(NULL, theworkingdirectory, 1000);
	for (int f = myoffset; f > 0 && theworkingdirectory[f] != '\\'; f--)
	{
		theworkingdirectory[f] = '\0';
		myoffset = f;
	}

	WNDCLASSEX wndclass =
	{
	wndclass.cbSize = sizeof(WNDCLASSEX),
	wndclass.style = CS_VREDRAW | CS_DBLCLKS | CS_HREDRAW,
	wndclass.lpfnWndProc = WndProc,
	wndclass.cbClsExtra = 0,
	wndclass.cbWndExtra = 0,
	wndclass.hInstance = hInstance,
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION),
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW),
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
	wndclass.lpszMenuName = NULL,
	wndclass.lpszClassName = TEXT("Calendar"),
	wndclass.hIconSm = NULL,
	};

	RegisterClassEx(&wndclass);

	HMENU hMenu = { 0 };
	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(MYMENU));

	hwnd = CreateWindowEx(0, TEXT("Calendar"), TEXT("Calendar"), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, hMenu, hInstance, NULL);
	mainHwnd = hwnd;
	if (hwnd == NULL)
	{
		CrashDumpFunction(1, 0);
		return FALSE;
	}

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DestroyWindow(hwnd);
	return 0;
}//s

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int cxClient, cyClient, xMeasure, yMeasure, cyChar;
	TEXTMETRIC tm = { 0 };
	static HWND hWMonths = NULL, hWTextBox = NULL, hWMarks = NULL, hWDates = NULL;
	static BOOL CreationFlag = FALSE; //False if childwindows have not been created, true if they where.
	static HINSTANCE hInstance;
	char* pusikurac = { 0 };
	PAINTSTRUCT ps = { 0 };
	HDC hdc = 0;
	if(GlobalDebugFlag == TRUE)
		ShowMessage(hwnd, cxClient, cyClient, message);
	if (CreationFlag == TRUE)
	{
		if (TextBoxCalFlag == TRUE)
		{
			EnableWindow(hWDates, 1);
		}
		if (TextBoxFlag == TRUE && TextBoxCalFlag == FALSE)//CREATION!!!!!!!
		{
			EnableWindow(TextBoxHwnd, 1);
			TextBoxFlag = FALSE;
		}
		if (TextBoxCalFlag == TRUE && TextBoxFlag == FALSE)//FUCKKOFFF
		{
			EnableWindow(TextBoxHwnd, 0);
			TextBoxCalFlag = FALSE;
		}
	}
	switch (message)
	{

	case WM_CREATE:
		hdc = GetDC(hwnd);
		LoadLibrary(TEXT("Riched20.dll"));
		GetTextMetrics(hdc, &tm);
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);
		hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		myHeap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 15000, 0);
		if (myHeap == 0)
		{
			CrashDumpFunction(2, 1);
		}
		TextHeapRemaining = 15000;//Default 15000 bytes allocation, TextHeap will also account for y pointers.
		//load window colors
		HFILE hFile = { 0 };
		char* tempstring = calloc(1000, sizeof(char));
		sprintf_s(tempstring, 1000, "%sCalendar.dat", theworkingdirectory);
		hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == -1)
			hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		int filesize = GetFileSize(hFile, NULL);
		if(filesize > 0)//read data file contents and update the global variables necessary
		{//use the numeration for windows colors dialog to order the colors
			/*1 - monthbutton
			3-daybutton
			4-monthsbackground
			5-daysbackground
			6-textbox color
			7- input signal color*/
			int templength = 0;
			tempstring = realloc(tempstring, filesize + 10);
			memset(tempstring, 0, filesize+5);
			ReadFile(hFile, tempstring, filesize, &filesize, NULL);
			//the windowscolorsdat separates datasets by newlines
			char* tempstring2 = calloc(filesize+100, sizeof(char));
			_memccpy(tempstring2, tempstring, '\n', filesize);
			monthsbuttoncolor = StrToIntA(tempstring2);
			templength = strlen(tempstring2);
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring+templength, '\n', filesize-templength);
			datesbuttoncolor = StrToIntA(tempstring2);
			templength += strlen(tempstring2);
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			monthsbackground = StrToIntA(tempstring2);
			templength += strlen(tempstring2);
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			datesbackground = StrToIntA(tempstring2);
			templength += strlen(tempstring2);
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			textbackground = StrToIntA(tempstring2);
			templength += strlen(tempstring2);
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			inputsignalcolor = StrToIntA(tempstring2);
			templength += strlen(tempstring2)+1;//+1 to include the 2nd delimiting newline as we are moving towards yearzero and yearrange
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			yearzero = StrToIntA(tempstring2);
			templength += strlen(tempstring2);
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			yearrange = StrToIntA(tempstring2)*12-yearzero*12;
			templength += strlen(tempstring2)+1;//+1 due to double newlines
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			startday = StrToIntA(tempstring2+4);
			templength += strlen(tempstring2);
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			startmonth = StrToIntA(tempstring2+6);
			templength += strlen(tempstring2);
			memset(tempstring2, 0, filesize);
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			startyear = StrToIntA(tempstring2+5);
			templength += strlen(tempstring2)+1;//+1 due to double newlines
			memset(tempstring2, 0, filesize);
			int maxcount = min(filesize, 2000);
			_memccpy(datasource, tempstring + templength, '\n', maxcount);
			int stringlength20 = strlen(datasource);
			datasource[stringlength20 -1] = 0;
			templength += stringlength20;
			_memccpy(specialchar, tempstring + templength, '\n', filesize - templength);
			specialchar[1] = 0;
			templength += strlen(specialchar)+1;//zbog kurcine
			_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
			ordereddatasave = StrToIntA(tempstring2);
			free(tempstring2);
		}
		else//if dat file doesnt exist
		{
			SYSTEMTIME mytime = { 0 };
			GetSystemTime(&mytime); char* sourcedata;
			yearzero = mytime.wYear - 50;
			yearrange = 1200;//yearrange is expressed in months
			sprintf_s(datasource, 2000, "%sTextdata.txt", theworkingdirectory);//by default datasource is at the working directory
			specialchar[0] = '*';
			memset(tempstring, 0, 1000);
			sprintf_s(tempstring, 1000, "%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear, datasource, specialchar, ordereddatasave);
			WriteFile(hFile, tempstring, strlen(tempstring), &filesize, NULL);
		}
		free(tempstring);
		if (hFile > 0)
			CloseHandle(hFile);
		CalendarCreate(0, 0, 0);//creates the refference calender "LocalData.txt" for use by the program to generate proper calander in itself.
		return 0;
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		xMeasure = cxClient / 10;
		yMeasure = cyClient / 10;
		MonthRect.left = 0;
		MonthRect.top = 0;
		MonthRect.right = 2 * xMeasure;
		MonthRect.bottom = cyClient;

		StringRect.left = 2 * xMeasure;
		StringRect.top = 8 * yMeasure;
		StringRect.right = cxClient - 4 * xMeasure;
		StringRect.bottom = cyClient - 8 * yMeasure;

		MarksRect.left = cxClient - 4 * xMeasure;
		MarksRect.top = 8 * yMeasure;
		MarksRect.right = cxClient - 8 * xMeasure;
		MarksRect.bottom = cyClient - 8 * yMeasure;

		DatesRect.left = 2 * xMeasure;
		DatesRect.top = 0;
		DatesRect.right = cxClient - 2 * xMeasure;
		DatesRect.bottom = 8 * yMeasure;

		SendMessage(DatesHwnd, WM_SIZE, 0, 0);

		return 0;
	case WM_MOVE:
		SendMessage(DatesHwnd, WM_SIZE, 0, 0);
		return 0;
	case WM_PAINT:
		if (CreationFlag == FALSE)
		{
			if (ChildCreationFunction())
			{
				hWMonths = CreateWindowEx(0, TEXT("Months class"), TEXT("Months"), WS_CHILD, 0, 0, 0,0, hwnd, NULL, NULL, NULL);
				//hWTextBox = CreateWindowEx(0, TEXT("EDIT"), NULL, WS_CHILD | ES_LEFT | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL | WS_DISABLED | ES_AUTOVSCROLL, 0,0,0,0, hwnd, NULL, NULL, NULL);
				hWTextBox = CreateDialogW(GetModuleHandle(NULL), TEXTBOXDIALOG, hwnd, TextBoxProc);
				hWDates = CreateWindowEx(0, TEXT("Dates Class"), TEXT("Dates"), WS_CHILD | WS_DISABLED, 0,0,0,0, hwnd, NULL, NULL, NULL);
				buttonrectd.left = 0;//very dump shit dont ask removing it will break more shit
				buttonrectd.top = DatesRect.top;
				buttonrectd.right = DatesRect.right / 20;
				pusenjekurca = buttonrectd.bottom = DatesRect.bottom / 12 + DatesRect.bottom / 200;//determines the leftmost marksbuttons and the dates buttons
				hWMarks = CreateWindowEx(0, TEXT("MarkClass"), TEXT("Marks"), WS_CHILD, 0,0,0,0, hwnd, NULL, NULL, NULL);

				ShowWindow(hWMonths, SW_SHOW);
				UpdateWindow(hWMonths);
				SendMessage(hWMonths, WM_COMMAND, 0, 0);//centers the calendar to present date
				ShowWindow(hWTextBox, SW_SHOW);
				UpdateWindow(hWTextBox);
				ShowWindow(hWMarks, SW_SHOW);
				UpdateWindow(hWMarks);
				ShowWindow(hWDates, SW_SHOW);
				UpdateWindow(hWDates);
				CreationFlag = TRUE;
				TextBoxInput = GetDlgItem(hWTextBox, TEXTBOXINPUT);
				Edit_LimitText(TextBoxInput, 15000);
				EnableWindow(TextBoxHwnd, 0);
			}
		}
		if (CreationFlag == TRUE)
		{
			if (hWMonths != NULL)
			{
				MoveWindow(hWMonths, 0, 0, 2 * xMeasure, cyClient, TRUE);
				UpdateWindow(hWMonths);
			}

			if (hWTextBox != NULL)
			{
				MoveWindow(hWTextBox, 2 * xMeasure, 7 * yMeasure, cxClient - 2 * xMeasure, cyClient-7*yMeasure, TRUE);
				UpdateWindow(hWTextBox);
				RECT TextBoxRect = { 0 };
				GetClientRect(hWTextBox, &TextBoxRect);
				MoveWindow(TextBoxInput, 0, 0, TextBoxRect.right, TextBoxRect.bottom, TRUE);
			}

			if (hWMarks != NULL)
			{
				MoveWindow(hWMarks, 8 * xMeasure-xMeasure/2, 0, cxClient - 2 * xMeasure+xMeasure/2, 7 * yMeasure, TRUE);
				UpdateWindow(hWMarks);
			}

			if (hWDates != NULL)
			{
				MoveWindow(hWDates, 2 * xMeasure, 0, 8 * xMeasure - xMeasure / 2-2*xMeasure, 7 * yMeasure, TRUE);
				buttonrectd.left = 0;//very dump shit dont ask removing it will break more shit
			buttonrectd.top = DatesRect.top;
				buttonrectd.right = DatesRect.right / 20;
				buttonrectd.bottom = pusenjekurca;
				UpdateWindow(hWDates);
			}
		}
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		hdc = NULL;
		hdc = GetDC(hwnd);
		HBRUSH hBrush = { 0 };
		COLORREF invmonthsbackground = RGB(GetRValue(~monthsbackground), GetGValue(~monthsbackground), GetBValue(~monthsbackground));
		hBrush = CreateSolidBrush(invmonthsbackground);
		RECT fillrect = { 0 };
		fillrect.right = 2 * xMeasure;
		fillrect.bottom = cyClient;
		fillrect.left = fillrect.right - fillrect.right / 30;
		FillRect(hdc, &fillrect, hBrush);
		DeleteObject(hBrush);
		ReleaseDC(hwnd, hdc);
		
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
	/*	case :
			// Send WM_UNDO only if there is something to be undone. 

			if (SendMessage(hwndEdit, EM_CANUNDO, 0, 0))
				SendMessage(hwndEdit, WM_UNDO, 0, 0);
			else
			{
				MessageBox(hwndEdit,
					L"Nothing to undo.",
					L"Undo notification",
					MB_OK);
			}
			break;

		case IDM_EDCUT:
			SendMessage(hwndEdit, WM_CUT, 0, 0);
			break;

		case IDM_EDCOPY:
			SendMessage(hwndEdit, WM_COPY, 0, 0);
			break;

		case IDM_EDPASTE:
			SendMessage(hwndEdit, WM_PASTE, 0, 0);
			break;

		case IDM_EDDEL:
			SendMessage(hwndEdit, WM_CLEAR, 0, 0);
			break;*/
		case ID_SETTINGS:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_SETTINGS), hwnd, DlgSettingsProc);
			return 0;
		case ID_HELP:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_INFOHELPBOX), hwnd, DlgHelpAndInfoProc);
		default:
			return 0;
		}


	case WM_DESTROY:
		pusikurac = calloc(1000, sizeof(char));
		sprintf_s(pusikurac, 1000, "%sCalendar.dat", theworkingdirectory);
		hFile = CreateFileA(pusikurac, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == -1)
			hFile = CreateFileA(pusikurac, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
		char* writebuffer = calloc(3000, sizeof(char));
		sprintf_s(writebuffer, 3000, "%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear,datasource, specialchar, ordereddatasave);
		int* retardbudala = { 0 };
		WriteFile(hFile, writebuffer, strlen(writebuffer), retardbudala, NULL);
		CloseHandle(hFile);
		DestroyWindow(hWTextBox);
		HeapDestroy(myHeap);
		DestroyWindow(hWMarks);
		DestroyWindow(hWDates);
		DestroyWindow(hWMonths);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL ChildCreationFunction(void)
{
	WNDCLASSEX MarkClass = {
	MarkClass.cbSize = sizeof(WNDCLASSEX),
	MarkClass.style = CS_VREDRAW | CS_DBLCLKS | CS_HREDRAW,
	MarkClass.lpfnWndProc = MarkProc,
	MarkClass.cbClsExtra = 0,
	MarkClass.cbWndExtra = 0,
	MarkClass.hInstance = NULL,
	MarkClass.hIcon = LoadIcon(NULL, IDI_APPLICATION),
	MarkClass.hCursor = LoadCursor(NULL, IDC_ARROW),
	MarkClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
	MarkClass.lpszMenuName = NULL,
	MarkClass.lpszClassName = TEXT("MarkClass"),
	MarkClass.hIconSm = NULL,};
	RegisterClassEx(&MarkClass);

	WNDCLASSEX DatesClass = {
	DatesClass.cbSize = sizeof(WNDCLASSEX),
	DatesClass.style = CS_VREDRAW | CS_DBLCLKS | CS_HREDRAW,
	DatesClass.lpfnWndProc = DatesProc,
	DatesClass.cbClsExtra = 0,
	DatesClass.cbWndExtra = 0,
	DatesClass.hInstance = NULL,
	DatesClass.hIcon = LoadIcon(NULL, IDI_APPLICATION),
	DatesClass.hCursor = LoadCursor(NULL, IDC_ARROW),
	DatesClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
	DatesClass.lpszMenuName = NULL,
	DatesClass.lpszClassName = TEXT("Dates Class"),
	DatesClass.hIconSm = NULL,};
	RegisterClassEx(&DatesClass);

	WNDCLASSEX MonthsClass = {
	MonthsClass.cbSize = sizeof(WNDCLASSEX),
	MonthsClass.style = CS_VREDRAW | CS_DBLCLKS | CS_HREDRAW | CS_OWNDC,
	MonthsClass.lpfnWndProc = MonthsProc,
	MonthsClass.cbClsExtra = 0,
	MonthsClass.cbWndExtra = 0,
	MonthsClass.hInstance = NULL,
	MonthsClass.hIcon = LoadIcon(NULL, IDI_APPLICATION),
	MonthsClass.hCursor = LoadCursor(NULL, IDC_ARROW),
	MonthsClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
	MonthsClass.lpszMenuName = NULL,
	MonthsClass.lpszClassName = TEXT("Months class"),
	MonthsClass.hIconSm = NULL,};
	RegisterClassEx(&MonthsClass);

	if (&MonthsClass != FALSE && &DatesClass != FALSE && &MarkClass != FALSE)
		return TRUE;
	CrashDumpFunction(3, 1);
	return FALSE;
}

BOOL ShowMessage(HWND hwnd, int XClient, int YClient, UINT message)//shows debug messages when debug mode is set.
{
	char szString[100] = "Hi!";
	HDC ihdc = GetDC(hwnd);
	if (GlobalDebugFlag == TRUE)
		TextOutA(ihdc, XClient / 2, YClient / 2, szString, sprintf_s(szString,100, "%u", message));
	memset(szString, 0, 100);
	_i64toa_s(pchinputbuffermemory, szString, 100,10);
	StrCatA(szString, ", pchinputbuffermemory");
	size_t length = 0;
	if(S_OK == StringCchLengthA(szString, 100, &length))
		TextOutA(ihdc, XClient / 2, (YClient / 2)+16, szString, (int)length);
	memset(szString, 0, 100);
	_i64toa_s(TextHeapRemaining, szString, 100 ,10);
	StrCatA(szString, ", TextHeapRemaining");
	if(S_OK == StringCchLengthA(szString, 100, &length))
		TextOutA(ihdc, XClient / 2, (YClient / 2) + 32, szString, (int)length);
	memset(szString, 0, 100);
	_i64toa_s(xAllocamount, szString, 100,10);
	StrCatA(szString, ", xAllocamount");
	if (S_OK == StringCchLengthA(szString, 100, &length))
		TextOutA(ihdc, XClient / 2, (YClient / 2) + 16*3, szString, (int)length);
	memset(szString, 0, 100);
	_i64toa_s(yAllocamount, szString, 100,10);
	StrCatA(szString, ", yAllocamount");
	if (S_OK == StringCchLengthA(szString, 100, &length))
		TextOutA(ihdc, XClient / 2, (YClient / 2) + 16*4, szString, (int)length);
	ReleaseDC(hwnd, ihdc);


	return FALSE;
}

//destroys month buttons when destroying the month window
BOOL DestroyButton(HWND hwnd)
{
	if (hwnd == NULL)
	{
		hwnd = NULL;
		return FALSE;
	}
	else
	{
		DestroyWindow(hwnd);
		return TRUE;
	}
}

INT_PTR CALLBACK DlgSettingsProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND BuftxtProc = { 0 };
	static int thehehhe = 0, cyChar = 0;
	int jebanjeubulju = 0;
	static long long monthup = 0, monthdown = 0, yearup = 0, yeardown = 0, dayup = 0, daydown = 0;
	char* dummy2 = NULL;
	int bolimukarc = 0;
	HDC hdc = { 0 };
	BOOL failcheck = TRUE;
	int kurcina231523256 = 0;
	TEXTMETRIC tm = { 0 };
	HWND bufedit = { 0 };
	int intermittentvar = 0;
	static HWND hwndDlgMain = { 0 };//When calling controls, you will have hwnddlgs being control handles, and for refference to send message to others controls, you can use this handle which will refference the dialog itself.
	thehehhe = LOWORD(wParam);
	switch (message)
	{
	case WM_INITDIALOG:
		if (RTForTXT == 0)
		{
			Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO2), 0);
			Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO3), 1);
		}
		if(RTForTXT == 1)
		{
			Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO2), 1);
			Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO3), 0);
		}
		Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO4), ordereddatasave);
		hdc = GetDC(hwndDlg);
		GetTextMetrics(hdc, &tm);
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwndDlg, hdc);
		if (GlobalDebugFlag == FALSE)
			CheckDlgButton(hwndDlg, IDDEBUG, BST_UNCHECKED);
		else
			CheckDlgButton(hwndDlg, IDDEBUG, BST_CHECKED);
		char dummy[100] = { 0 };
		sprintf_s(dummy, sizeof(char) * 100, "%lli", pchinputbuffermemory);
		SetDlgItemTextA(hwndDlg, IDBUFEDIT, dummy);
		SetDlgItemTextA(hwndDlg, EDITSCHAR, specialchar);
		hwndDlgMain = hwndDlg;
		return TRUE;
		// Place message cases here. 
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDDEBUG:
			if (GlobalDebugFlag == FALSE)
				GlobalDebugFlag = TRUE;
			else
				GlobalDebugFlag = FALSE;
			break;
		case IDBUFCHANGE:
			if (MessageBox(GetParent(DatesHwnd), TEXT("Before doing this, ensure you backup your current calendar textdata, failure to do can result in irretrivable loss of data"), TEXT("PROCEED WITH CAUTION"), MB_OKCANCEL | MB_ICONWARNING) == IDOK)
			{//here you change the amount of memory in the heap.
				BOOL failornot = TRUE;
				int memoryused = Edit_GetTextLength(TextBoxInput);
				pchinputbuffermemory = GetDlgItemInt(hwndDlgMain, IDBUFEDIT, &failornot, FALSE);//since HeapCreate allows for us to grow the heap, these logical checks like textheapremaining and pchinputbuffermemory are what actually limit the input and protect from any input memory overflows
				if (failornot == FALSE)
				{
					CrashDumpFunction(5, 0);
					return FALSE;
				}
				Edit_LimitText(TextBoxInput, pchinputbuffermemory);
				TextHeapRemaining = pchinputbuffermemory - memoryused;
				if (TextHeapRemaining < 0)
					CrashDumpFunction(10000, 1);//this means you allocated less memory then you are using.
				InvalidateRect(buttonmarks[2], NULL, TRUE);
				UpdateWindow(buttonmarks[2]);
			}
			break;
		case IDDSCRIPTINPUT:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDINPUTSCRIPTBX), hwndDlgMain, DlgScriptedInput);
			break;
		case IDWINDOWCOLOR:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_COLORWINDOWS), hwndDlgMain, DlgColorWindows);
			break;
		case IDCHANGEMONTHSRG:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDMONTHSRANGE), hwndDlgMain, DlgMonthsRange);
			break;
		case IDOK:
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		case IDWORDWRAP://sendsmessage to TEXTBOX to commit wordwrappign
			SendMessage(TextBoxHwnd, WM_SETFOCUS, 0, 0);
			SendMessage(TextBoxHwnd, WORDWRAP, 0, 0);//send message with flag that will send wordwrap function
			break;
		case IDDAYDOWN:
			daydown = GetDlgItemInt(hwndDlgMain, IDDAYDOWN, &failcheck, FALSE);
			break;
		case IDDAYUP:
			dayup = GetDlgItemInt(hwndDlgMain, IDDAYUP, &failcheck, FALSE);
			break;
		case IDMONTHUP:
			monthup = GetDlgItemInt(hwndDlgMain, IDMONTHUP, &failcheck, FALSE);
			break;
		case IDMONTHDOWN:
			monthdown = GetDlgItemInt(hwndDlgMain, IDMONTHDOWN, &failcheck, FALSE);
			break;
		case IDYEARUP:
			yearup = GetDlgItemInt(hwndDlgMain, IDYEARUP, &failcheck, FALSE);
			break;
		case IDYEARDOWN:
			yeardown = GetDlgItemInt(hwndDlgMain, IDYEARDOWN, &failcheck, FALSE);
			break;
		case IDDATAWIPE:
			dummy2 = calloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(16, 0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			SendDlgItemMessageA(hwndDlg, IDDAYUP, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(6, 0);
				return FALSE;
			}
			StrToInt64ExA(dummy2, STIF_DEFAULT, &dayup);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = calloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(7,0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			SendDlgItemMessageA(hwndDlg, IDDAYDOWN, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(8,0);
				return FALSE;
			}
			StrToInt64ExA(dummy2, STIF_DEFAULT, &daydown);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = calloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
				return FALSE;
			dummy2[0] = '3';
			dummy2[1] = '0';
			SendDlgItemMessageA(hwndDlg, IDMONTHUP, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(9,0);
				return FALSE;
			}
			StrToInt64ExA(dummy2, STIF_DEFAULT, &monthup);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = calloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(10,0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			SendDlgItemMessageA(hwndDlg, IDMONTHDOWN, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(11,0);
				return FALSE;
			}
			StrToInt64ExA(dummy2, STIF_DEFAULT, &monthdown);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = calloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(12,0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			SendDlgItemMessageA(hwndDlg, IDYEARUP, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(13,0);
				return FALSE;
			}
			StrToInt64ExA(dummy2, STIF_DEFAULT, &yearup);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = calloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(14,0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			SendDlgItemMessageA(hwndDlg, IDYEARDOWN, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(15,0);
				return FALSE;
			}
			StrToInt64ExA(dummy2, STIF_DEFAULT, &yeardown);
			free(dummy2);
			dummy2 = NULL;
			RangedDataWipe((int)monthup, (int)monthdown, (int)yearup, (int)yeardown, (int)dayup, (int)daydown);
			break;
		case IDCHANGEDATASOURCE:
			intermittentvar = 1;
			OPENFILENAMEA ofna = {
			ofna.lStructSize = sizeof(OPENFILENAME),
			ofna.hwndOwner = hwndDlg,
			ofna.hInstance = NULL,
			ofna.lpstrFilter = "*.txt\0\0",
			ofna.lpstrCustomFilter = NULL,
			ofna.nMaxCustFilter = 0,
			ofna.nFilterIndex = 1,
			ofna.lpstrFile = datasource,
			ofna.nMaxFile = 2000,
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
			int kurcina24142 = 0;
			char* tempstring2 = calloc(2000, sizeof(char));
			sprintf_s(tempstring2, 2000, "%sCalendar.dat", theworkingdirectory);
			HANDLE hFile = CreateFileA(tempstring2, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			memset(tempstring2, 0, 2000);
			sprintf_s(tempstring2, 1000, "%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear, datasource, specialchar, ordereddatasave);
			WriteFile(hFile, tempstring2, strlen(tempstring2), &kurcina24142, NULL);
			CloseHandle(hFile);
			free(tempstring2);
			break;
		case IDSPECIALCHAR:
			dummy2 = calloc(2, sizeof(char));
			GetDlgItemTextA(hwndDlgMain, EDITSCHAR, dummy2, 2);
			if (IDYES == MessageBox(hwndDlgMain, "Do you want to change all special characters within the data set to the new special character?", "Special Character change", MB_YESNO))
			{
				HANDLE tmpFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				int fsize = GetFileSize(tmpFile, NULL);
				char* readbuffer = calloc(fsize, sizeof(char));
				int fagnumber = 0;
				ReadFile(tmpFile, readbuffer, fsize, &fagnumber, NULL);
				for (int i = 0; i < fagnumber; i++)
				{
					if(readbuffer[i] == specialchar[0])
					{
						readbuffer[i] = dummy2[0];
					}
				}
				WriteFile(tmpFile, readbuffer, strlen(readbuffer), &intermittentvar, NULL);
				CloseHandle(tmpFile);
			}
			specialchar[0] = dummy2[0];
			char* CRKNIKURVO = calloc(2000, sizeof(char));
			sprintf_s(CRKNIKURVO, 2000, "%sCalendar.dat", theworkingdirectory);
			HANDLE PUSKURAC = CreateFileA(CRKNIKURVO, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			memset(CRKNIKURVO, 0, 2000);
			sprintf_s(CRKNIKURVO, 1000, "%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear, datasource, specialchar, ordereddatasave);
			WriteFile(PUSKURAC, CRKNIKURVO, strlen(CRKNIKURVO), &intermittentvar, NULL);
			CloseHandle(PUSKURAC);
			free(dummy2);
			break;
		default: 
			break;
		}
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			switch (LOWORD(wParam))
			{
			case IDC_RADIO2://txt button
				RTForTXT = 1;
				Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO3), 0);//rtf format
				Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO2), 1);//txt format
				char* tempstring2 = calloc(2000, sizeof(char));
				sprintf_s(tempstring2, 2000, "%sCalendar.dat", theworkingdirectory);
				HANDLE hFile = CreateFileA(tempstring2, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				memset(tempstring2, 0, 2000);
				sprintf_s(tempstring2, 1000, "%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear, datasource, specialchar, ordereddatasave);
				WriteFile(hFile, tempstring2, strlen(tempstring2), &bolimukarc, NULL);
				CloseHandle(hFile);
				free(tempstring2);
				break;
			case IDC_RADIO3://rtf button
				RTForTXT = 0;
				Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO3), 1);
				Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO2), 0);
				char* tempstring25215213 = calloc(2000, sizeof(char));
				sprintf_s(tempstring25215213, 2000, "%sCalendar.dat", theworkingdirectory);
				hFile = CreateFileA(tempstring25215213, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				memset(tempstring25215213, 0, 2000);
				sprintf_s(tempstring25215213, 1000, "%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear, datasource, specialchar, ordereddatasave);
				WriteFile(hFile, tempstring25215213, strlen(tempstring25215213), &bolimukarc, NULL);
				CloseHandle(hFile);
				free(tempstring25215213);
				break;
			case IDC_RADIO4://ordereddatasave
				ordereddatasave = Button_GetCheck(GetDlgItem(hwndDlg, IDC_RADIO4));
				int static flag27 = 1;
				if (ordereddatasave == TRUE)
				{
					Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO4), 0);
					ordereddatasave = 0;
				}
				else if (ordereddatasave == FALSE && flag27 == 1)
				{
					flag27 = 0;
					if (IDYES == MessageBoxA(NULL, "This will re-order current calendar data structure to oldest-to-earliest date", "Re-order data saving", MB_YESNO))
					{
						HANDLE PusiKurac = { 0 };
						PusiKurac = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						char* myreadbuffer = { 0 };
						int stringlength19 = GetFileSize(PusiKurac, NULL);
						myreadbuffer = calloc(stringlength19 +10, sizeof(char));
						int sonofabitch = 0;
						ReadFile(PusiKurac, myreadbuffer, stringlength19, &sonofabitch, NULL);
						myreadbuffer = DataSaveReordering(myreadbuffer);
						OVERLAPPED ofn = { 0 };
						ofn.Offset = 0;
						WriteFile(PusiKurac, myreadbuffer, stringlength19, &sonofabitch, &ofn);
						Button_SetCheck(GetDlgItem(hwndDlg, IDC_RADIO4), 1);
						ordereddatasave = 1;
					}
					flag27 = 1;
				}
				char * tempstring = calloc(1000, sizeof(char));
				sprintf_s(tempstring, 1000, "%sCalendar.dat", theworkingdirectory);
				hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				int filesize = GetFileSize(hFile, NULL);
				char* writebuffer = calloc(3000, sizeof(char));
				SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
				SetEndOfFile(hFile);
				sprintf_s(tempstring, 1000, "%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear, datasource, specialchar, ordereddatasave);
				WriteFile(hFile, writebuffer, strlen(writebuffer), &filesize, NULL);
				if (hFile > 0)
					CloseHandle(hFile);
				free(tempstring);
				break;
			default: break;
			}
		default: break;
		}
		return TRUE;
	case WM_CTLCOLOREDIT:
		BuftxtProc = (HWND)wParam;
		//make the buffer show current memory reserved for the txtbuffers, default is 30*500
		return TRUE;
	default:
		return FALSE;
	}
}

INT_PTR CALLBACK BufTextProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hwnd);
			return TRUE;
		}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

//monthdown/yeardown - the end date
//monthup/yearup - the beginning date
BOOL RangedDataWipe(int monthup, int monthdown, int yearup, int yeardown, int dayup, int daydown)
{
	HANDLE hFile = { 0 };
	size_t amountread = 0;
	OVERLAPPED overlapstruct = { 0 };
	int beginninglocation = 0, endlocation = 0;
	char startdate[50] = { 0 }, enddate[50] = { 0 };

	size_t oldstringlength = 0;
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	amountread = GetFileSize(hFile, NULL);
	char* readbuffer = calloc((DWORD)amountread, sizeof(char));
	if (readbuffer == NULL)
		return FALSE;
	if (FALSE == ReadFile(hFile, readbuffer, (DWORD)amountread, NULL, &overlapstruct))
	{
		int errorvalue = GetLastError();
		if ((amountread != 0) && (errorvalue != 38))//if amountread is 0, that means file is empty, hence why error was thrown.
		{
			CloseHandle(hFile);
			return FALSE;
		}
		else if (errorvalue == 38 && amountread > 0)//the amountread is not empty but the date itself is empty and standing at the border of EOF, effectively offset is at EOF, we fix this by doing nothing 
		{
			;
		}
	}
	for (oldstringlength = 0; oldstringlength < amountread && readbuffer[oldstringlength] != '\0'; oldstringlength++)//calculating length of the string.
	{
		;
	}
	//format startdate for beggininglocaton calc
	StringCbCatExA((LPSTR)startdate, 24, "*00:00:    *", NULL, NULL, 0);
	_itoa_s(dayup, startdate + 1, 3, 10);
	if (dayup < 10)
		startdate[2] = ' ';
	_itoa_s(monthup, startdate + 4, 3, 10);
	if (monthup < 10)
		startdate[5] = ' ';
	_itoa_s(yearup, startdate + 7, 5, 10);
	startdate[3] = ':';
	startdate[6] = ':';
	startdate[11] = specialchar[0];
	//format enddate for endlocation calc
	StringCbCatExA((LPSTR)enddate, 24, "*00:00:    *", NULL, NULL, 0);
	_itoa_s(daydown, enddate + 1, 3, 10);
	if (daydown < 10)
		enddate[2] = ' ';
	_itoa_s(monthdown, enddate + 4, 3, 10);
	if (monthdown < 10)
		enddate[5] = ' ';
	_itoa_s(yeardown, enddate + 7, 5, 10);
	enddate[3] = ':';
	enddate[6] = ':';
	enddate[11] = specialchar[0];
	if (endlocation == beginninglocation)
	{//in case we are deleting just one date, run findnearest date only once, and put endlocation at the end of text of the same date.
		findthenearestdate((int)oldstringlength, readbuffer, startdate, &beginninglocation, 0);
		endlocation = beginninglocation + 12;
		for (; readbuffer[endlocation] != specialchar[0] && endlocation != '\0' && (unsigned)endlocation < oldstringlength; endlocation++)
			;//this will ensure that endlocation will include the data for the sameself date
	}
	else
	{
		//calculate beginninglocation
		findthenearestdate((int)oldstringlength, readbuffer, startdate, &beginninglocation, 0);
		//calculate the endlocation
		findthenearestdate((int)oldstringlength, readbuffer, enddate, &endlocation, 0);
		endlocation += 12;//so we could delete enddate too!
	}
	//wipe all data from begininglocation to endlocation
	for (int i = endlocation, l = beginninglocation; (unsigned)i < oldstringlength && l < endlocation; i++, l++)
	{
		readbuffer[l] = readbuffer[i];
	}
	overlapstruct.Offset = beginninglocation;
	UINT_PTR fbeginninglocation = beginninglocation;
	if ((unsigned)endlocation == oldstringlength)//simply shrink the file to beginning location
	{
		SetFilePointer(hFile, beginninglocation, (PLONG)fbeginninglocation, FILE_BEGIN);
		SetEndOfFile(hFile);
	}
	else 
		WriteFile(hFile, readbuffer + beginninglocation, (DWORD)(oldstringlength - (size_t)((DWORD)endlocation - beginninglocation)), NULL, &overlapstruct);

	CloseHandle(hFile);
	free(readbuffer);
	return TRUE;
}

//used by textbox.c and mycalendar.c to properly figure out the range of dataset present
//
char * findthenearestdate(int filesize, char * readbuffer, char * dateset, int * appendindexlocation, int flag)
{
	int mflag = 0, i = 0, escapeflag = 0;

	//scan for the year dataset range
	int yearchars = strlen(dateset + 7)-1;//-1 removes the star at the end
	int yeardate = StrToIntA(dateset + 7);
	int monthdate = StrToIntA(dateset + 4);
	int daydate = StrToIntA(dateset + 1);
	for (int k = 0; k < filesize; k++)//find year
	{
		if (readbuffer[k] == specialchar[0])//found a date
		{
			i = k;
			char tempyearstring[20] = { 0 };
			int yeartemp = StrToIntA(readbuffer + k + 7);
			_itoa_s(yeartemp, tempyearstring, 20, 10);
			int yeartemplength = strlen(tempyearstring);
			int monthtemp = StrToIntA(readbuffer + k + 4);
			int daytemp = StrToIntA(readbuffer + k + 1);
			for (int l = 7; yeartemplength == yearchars && readbuffer[k + l] == dateset[l] && dateset[l] != specialchar[0]; l++)
				mflag++;
			if (mflag == yearchars)
			{
				i = k;//make the global pointer point to 
				k = filesize;//quit the year loop, as year has been found
			}
			else//no result move to next dateset
				for (int j = k + 1; j < filesize; j++)
					if (readbuffer[j] == specialchar[0])
					{
						mflag = 0;
						if (yeartemp > yeardate)
							k = filesize;//next date is larger
						else if (yeartemp == yeardate && monthtemp > monthdate)
							k = filesize;
						else if (yeartemp == yeardate && monthtemp == monthdate && daytemp > daydate)
							k = filesize;
						else
							k = j;//and k will iterate once more and point away from the 2nd star
						j = filesize;
					}
		}
	}

	//scan for the month data set range in the given year data set range
	if (mflag == yearchars)
	{
		for (int k = i; k < filesize; k++)
		{
			i = k;
			int yeartemp = StrToIntA(readbuffer + k + 7);
			int monthtemp = StrToIntA(readbuffer + k + 4);
			int daytemp = StrToIntA(readbuffer + k + 1);
			if (readbuffer[k] == specialchar[0])//found a date
			{
				for (int l = 4; readbuffer[k + l] == dateset[l] && l < 6; l++)
				{
					mflag++;
				}
				if (mflag == 2+yearchars)
				{
					i = k;
					k = filesize;
				}
				else//no result move to next dateset
					mflag = yearchars;
					for (int j = k + 1; j < filesize; j++)
						if (readbuffer[j] == specialchar[0])//load when it hits the 2nd star of the dateset to evaluate if its the first bigger
						{
							if (yeartemp > yeardate)
								k = filesize;//next date is larger, stop iterations i will be set behind the given tempdate
							else if (yeartemp == yeardate && monthtemp > monthdate)
								k = filesize;
							else if (yeartemp == yeardate && monthtemp == monthdate && daytemp > daydate)
								k = filesize;
							else
								k = j;//and k will iterate once more and point away from the 2nd star
							j = filesize;
						}
			}
		}
	}

	//scan for the dates present
	if (mflag == 2 + yearchars)
	{
		for (int k = i; k < filesize; k++)
		{
			if (readbuffer[k] == specialchar[0])
			{
				i = k;
				int yeartemp = StrToIntA(readbuffer + k + 7);
				int monthtemp = StrToIntA(readbuffer + k + 4);
				int daytemp = StrToIntA(readbuffer + k + 1);
				for (int l = 1; readbuffer[k + l] == dateset[l] && l < 3; l++)
				{
					mflag++;
				}
				if (mflag == 4+yearchars)
				{
					i = k;
					k = filesize;
				}
				else//no result move to next dateset
					mflag = yearchars + 2;
					for (int j = k+1; j < filesize; j++)
						if (readbuffer[j] == specialchar[0])
						{
							if (yeartemp > yeardate)
								k = filesize;//next date is larger
							else if (yeartemp == yeardate && monthtemp > monthdate)
								k = filesize;
							else if (yeartemp == yeardate && monthtemp == monthdate && daytemp > daydate)
								k = filesize;
							else
								k = j;//and k will iterate once more and point away from the 2nd star
							j = filesize;
						}
			}
		}
	}
	//input the date at the right place, in between the nearest larger and nearest smaller date.	
	if(flag == 0)
		return DateInput(mflag, filesize, readbuffer, appendindexlocation, dateset, i);//for ranged data wipe
	if(flag == 1)
		return NearestDate(mflag, filesize, readbuffer, appendindexlocation, dateset, i);//for ordered data inpu
}

//Return value will be set by functions and handled then case-by-case
void CrashDumpFunction(int return_value, int fatality)
{
	SYSTEMTIME lt = { 0 };//local time
	HANDLE hFile = { 0 };
	int stringlength = 0;
	DWORD filelength = 0;
	OVERLAPPED Overlapped = { 0 };
	char crashtext[1000] = { 0 };
	GetLocalTime(&lt);

	char tempstring[1000] = { 0 };
	sprintf_s(tempstring, 1000, "%scrashdump.dmp", theworkingdirectory);
	hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileSize(hFile, &filelength);
	Overlapped.Offset = filelength;

	memset(tempstring, 0, sizeof(char) * 1000);
	sprintf_s(tempstring, 16, "Error code: %i", return_value);
	MessageBoxA(GetParent(DatesHwnd), tempstring, "Fatal Error!", MB_OK | MB_ICONWARNING);
	memset(tempstring, 0, sizeof(char) * 1000);
	stringlength = wsprintfA(crashtext, "%u:%u:%u:%u:%u:%u:%u === Error Code: %i\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds, return_value);
	if (stringlength < 0)
		WndProc(GetParent(TextBoxHwnd), WM_DESTROY, 0, 0);
	WriteFile(hFile, crashtext, stringlength, NULL, &Overlapped);

	if (fatality == 1)
		SendMessage(mainHwnd, WM_CLOSE, 0, 0);

	return;
}

INT_PTR CALLBACK DlgHelpAndInfoProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwndDlg, 0);
			return TRUE;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
}

char* DateInput(int mflag, int filesize, char * readbuffer, int * appendindexlocation, char * dateset, int i)
{
	int yearchars = strlen(dateset + 7) - 1;//-1 removes the star at the end 
	if (mflag == 0)
	{
		//no date alike is present, input to the closest year range
		yearange(readbuffer, filesize, dateset, appendindexlocation);
	}
	if (mflag == yearchars)
	{
		//only appropriate year is present, input to the closest month range
		monthrange(filesize, readbuffer, dateset, appendindexlocation);
	}
	if (mflag == yearchars+2)
	{
		//only appropriate month is present, input to the closest day range
		for (int k = 0; k < filesize; k++)
		{
			if (readbuffer[k] == specialchar[0])//load the day and check if its larger
			{
				char tempstring[12] = { 0 };
				char tempstring2[12] = { 0 };
				long long inputday = 0, presentday = 0;
				for (int l = 1, p = 0; readbuffer[k + l] && l < 3; l++, p++)
				{
					tempstring[p] = readbuffer[k + l];
					tempstring2[p] = dateset[l];
				}
				StrToInt64ExA(tempstring, 0, &presentday);
				StrToInt64ExA(tempstring2, 0, &inputday);
				if (presentday > inputday)
				{
					//we found the first date larger then, write the date.
					char* pholder = NULL;
					pholder = realloc(readbuffer, sizeof(char) * ((size_t)filesize + 13));//make space for date input
					if (pholder == NULL)
						return 0;//memory failure
					readbuffer = pholder;
					for (int l = filesize; l < filesize + 13 && l >= k; l--)//move all character forwards by 12
						readbuffer[l + 12] = readbuffer[l];
					for (int l = 0; l < 12; l++)
					{
						readbuffer[l + k] = dateset[l];
					}
					*appendindexlocation = k;
					return readbuffer;
				}
				else
					k += 12;//pass the date and skip the second star
			}
		}
	}
	if (mflag == yearchars+4)//date found, point to it
	{
		*appendindexlocation = i;
		return readbuffer;
	}
	return readbuffer;
}

char* yearange(char * readbuffer, int filesize, char * dateset, int * appendindexlocation)
{
	char tempstring[12] = { 0 };
	char tempstring2[12] = { 0 };
	for (int k = 0; k < filesize; k++)
	{
		if (readbuffer[k] == specialchar[0])//load the year and check if its larger
		{
			long long inputyear = 0, presentyear = 0;
			for (int l = 7; readbuffer[k + l] && l < 11; l++)
			{
				tempstring[l] = readbuffer[k + l];
				tempstring2[l] = dateset[l];
			}
			StrToInt64ExA(tempstring, 0, &presentyear);
			StrToInt64ExA(tempstring2, 0, &inputyear);
			if (presentyear > inputyear)
			{
				//we found the first date larger then, write the date.
				char* pholder = NULL;
				pholder = realloc(readbuffer, sizeof(char) * ((size_t)filesize + 13));//make space for date input
				if (pholder == NULL)
					return 0;//memory failure
				readbuffer = pholder;
				for (int l = filesize; l < filesize + 13 && l >= k; l--)//move all character forwards by 12
					readbuffer[l] = readbuffer[l + 12];
				for (int l = 0; l < 13; l++)
				{
					readbuffer[l + k] = dateset[l];
				}
				*appendindexlocation = k;
				return readbuffer;
			}
			else
				k += 12;//pass the date and skip the second star
		}
	}
	return readbuffer;
}
char* monthrange(int filesize, char * readbuffer, char * dateset, int * appendindexlocation)
{
	char tempstring[12] = { 0 };
	char tempstring2[12] = { 0 };
	for (int k = 0; k < filesize; k++)
	{
		if (readbuffer[k] == specialchar[0])//load the month and check if its larger
		{
			long long inputmonth = 0, presentmonth = 0;
			for (int l = 4; readbuffer[k + l] && l < 6; l++)
			{
				tempstring[l] = readbuffer[k + l];
				tempstring2[l] = dateset[l];
			}
			StrToInt64ExA(tempstring, 0, &presentmonth);
			StrToInt64ExA(tempstring2, 0, &inputmonth);
			if (presentmonth > inputmonth)
			{
				//we found the first date larger then, write the date.
				char* pholder = NULL;
				pholder = realloc(readbuffer, sizeof(char) * ((size_t)filesize + 13));//make space for date input
				if (pholder == NULL)
					return 0;//memory failure
				readbuffer = pholder;
				for (int l = filesize; l < filesize + 13 && l >= k; l--)//move all character forwards by 12
					readbuffer[l] = readbuffer[l + 12];
				for (int l = 0; l < 13; l++)
				{
					readbuffer[l + k] = dateset[l];
				}
				*appendindexlocation = k;
				return readbuffer;
			}
			else
				k += 12;//pass the date and skip the second star
		}
	}
	return readbuffer;
}

//creates the LocalData.txt used as refference for calendar emulation within the program.
BOOL CalendarCreate(BOOL RealorCustom, int startyear, int newyearrange)
{
	int thirtyone[] = { 1,3,5,7,8,10,12 };
	int thirty[] = { 4,6,9,11 };
	int leap[] = { 2 };
	OVERLAPPED dummyoverlap = { 0 };
	HANDLE datafile = NULL;
	SYSTEMTIME temptime = { 0 };
	char* wchararray = calloc(150, sizeof(char));
	if (wchararray == NULL)
	{
		CrashDumpFunction(20, 1);
		return 0;
	}
	memset(wchararray, ' ', sizeof(char) * 150);
	static LPSTR CreationBuffer = { 0 };
	dummyoverlap.Offset = 0xFFFFFFFF;
	dummyoverlap.OffsetHigh = 0xFFFFFFFF;//these two thing set the "write" to be done at the end of file.
	char tempstring[1000] = { 0 };
	sprintf_s(tempstring, 1000, "%sLocaldata.txt", theworkingdirectory);
	datafile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	int dateprint = 1, monthtype = 0, datemonth = 0, dateyear = 0;
	SetEndOfFile(datafile);
	GetSystemTime(&temptime);
	if (RealorCustom == 0)//always call this if ur during WM_CREATE of the programs main winproc
	{
		dateyear = yearzero;
	}
	else//create a localdata.txt based on new year range, call this when changing localdate from button!
	{
		yearrange = newyearrange;
		yearzero = dateyear = startyear;
		SetFilePointer(datafile, 0, NULL, FILE_BEGIN);//empty the file
		SetEndOfFile(datafile);
	}
	for (int z = 0; z < yearrange; z++)
	{
		datemonth++;
		if (z >= 12 && z % 12 == 0)
		{
			datemonth = 1;
			dateyear++;
		}
		for (int y = 0; y < 7; y++)
		{
			if (y == 0)
			{
				wchararray = monthtypegen(wchararray, dateyear, datemonth, &monthtype, thirty, leap, thirtyone);
			}
			else if (y == 6) //append RGB and shape value at the end
			{
				CreationBuffer = "RGB: TBD, SHAPE: TBD";
				StringCbCatExA(&wchararray[5 * 24], 24, CreationBuffer, NULL, NULL, 0);
			}
			else
				wchararray = whcararryinputroutine(wchararray, &dateprint, monthtype, y);
		}
		wchararray[149] = '\r';
		WriteFile(datafile, wchararray, 150, NULL, &dummyoverlap);//wtf shit about asynchronous I/O no idea why botherin me with that stuf??
		for (int i = 0; i < 149; i++)//empty the buffer/s
			wchararray[i] = ' ';
	}
	CloseHandle(datafile);
	return 1;
}

//takes hold of inputting and sorting data into the refference calendar buffer, before writing it into the datafile
char * whcararryinputroutine(char * wchararray, int * dateprint, int monthtype, int y)
{
	for (int i = 0; i < 19; i += 3)//+3 cause two from decimal value(even non decimals have it reserved for simplicity sake, and 1 for empty space.
	{
		if (y == 1)
		{
			_itoa_s((*dateprint)++, wchararray + (i + y * 24), 3, 10);//puts digit in
			wchararray[(i + y * 24) + 2] = ' ';
			wchararray[(i + y * 24) + 1] = ' ';
		}
		else if (y < 5)
		{
			_itoa_s(((7 * y) - 7) + (*dateprint)++, wchararray + (i + y * 24), 3, 10);//puts digit in
			wchararray[(i + y * 24) + 2] = ' ';
			if (((7 * y) + (*dateprint) - 7) < 11)
				wchararray[(i + y * 24) + 1] = ' ';
		}
		else if (y == 5)
		{
			BOOL shitflag = FALSE;
			_itoa_s((7 * y) - 7 + (*dateprint)++, wchararray + (i + y * 24), 3, 10);//puts digit i;
			wchararray[(i + y * 24) + 2] = ' ';
			if ((monthtype == 1) && ((7 * y) + *dateprint - 7) > 28)
			{
				shitflag = TRUE;
			}
			if ((monthtype == 2) && ((7 * y) + *dateprint - 7) > 30)
			{
				shitflag = TRUE;
			}
			if ((monthtype == 3) && ((7 * y) + *dateprint - 7) > 31)
			{
				shitflag = TRUE;
			}
			if (shitflag == TRUE)
				i = 19;//breakthestupidloop
		}
	}
	wchararray[24 * y - 1] = '\r';
	*dateprint = 1;
	return wchararray;
}

//deduces the type of month for calendar generation
char* monthtypegen(char * wchararray, int dateyear, int datemonth, int * monthtype, int * thirty, int * leap, int * thirtyone)
{
	_itoa_s(dateyear, wchararray, 5, 10);
	wchararray[4] = ',';
	wchararray[5] = ' ';
	_itoa_s(datemonth, wchararray + 6, 3, 10);
	wchararray[8] = ' ';
	for (int i = 0; i < 7; i++)
	{
		if (i < 1)
		{
			if (datemonth == leap[i])
				*monthtype = 1;
		}
		if (i < 4)
		{
			if (datemonth == thirty[i])
				*monthtype = 2;
		}
		if (i < 20)
		{
			if (datemonth == thirtyone[i])
				*monthtype = 3;
		}
	}
	if (datemonth < 10)
		wchararray[7] = ' ';
	wchararray[24] = '\r';
	return wchararray;
}
/*	int dateyear = selecteddate >> 11;
	int datemonthindex = (selecteddate & 0x780) >> 7;
	int datedayindex = (selecteddate & 0x7F);*/
HWND pickamaterina = 0;
static long long monthf = 1, monthl = 1, yearf = 2024, yearl = 2025, dayf = 1, dayl = 1, dayspassed = 0, monthspassed = 0, yearspassed = 0, newlinespassed = 0, charspassed = 0;;//global variables for selection of scripted dates for sake of ease with dealing with them. l stands for last f for first
INT_PTR CALLBACK DlgScriptedInput(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int result = 0, appendlocationindex = 0, amountread = 0;
	char * ScriptString = { 0 };
	BOOL datepresent = FALSE;
	int originaldateselected = selecteddate;
	OVERLAPPED overlapstruct = { 0 };
	int monthsinaday[13] = {0, 31,28,31,30,31,30,31,31, 30,31,30,31 };
	switch (message)
	{
	case WM_INITDIALOG:
		SendDlgItemMessageA(hwndDlg, TXTSCRIPTINPUT, EM_SETLIMITTEXT, pchinputbuffermemory/2, 0);//limits the editbox input to 1/2 of textbox input buffer
		CreateDialogA(GetModuleHandle(NULL), MAKEINTRESOURCE(IDSCRIPTMACRO), hwndDlg, DlgScriptMacros);
		DestroyWindow(pickamaterina);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		case IDOK:
			
			ScriptString = calloc(pchinputbuffermemory / 2, sizeof(char));
			GetDlgItemTextA(hwndDlg, TXTSCRIPTINPUT, ScriptString, pchinputbuffermemory / 2);

			//below code pastes the ScriptString to all days within range of selection.
			for(int i = yearf; i<=yearl; i++)
			{
				//leap year mechanism
				monthsinaday[2] = 28;
				if (yearf % 4 == 0)
					if (yearf % 100)
					{
						if (yearf % 400)
							monthsinaday[2] = 29;
					}
					else
						monthsinaday[2] = 29;
				//end of leapyear mechanism
				int l = 1;
				if (i == yearf)
					l = monthf;
				else if (i == yearl)
					l = monthl;
				selecteddate = selecteddate & 0x7ff;//zerout the year
				selecteddate += i << 11;//put the year inside selecteddate;
				for(; l <= monthl && l <= 12; l++)
				{
					int k = 1;
					if (i == yearf && l == monthf)
						k == dayf;//begging with the starting date at the starting month
					else if (i == yearl && l == monthl)
						monthsinaday[l] = dayl;
					if(yearl == yearf && monthl == monthf)
						monthsinaday[l] = dayl;//if the months and years are the same, then the max days is basically dayl
					selecteddate = selecteddate & 0xfff87f;//zeroout the month
					selecteddate += l << 7;//implant the month
					for(; k <= monthsinaday[l] ; k++)
					{
						selecteddate = selecteddate & 0xFFFF80;//zeroout the day
						selecteddate += k;//the day is the lowest bit henceforth we can add it like this
						//scriptformatfunction
						ScriptFormat(ScriptString, hwndDlg);
						DateTestBufferLoad(&amountread, &overlapstruct, &appendlocationindex, &datepresent);
						char* readbuffer = calloc((size_t)(amountread)+strlen(ScriptString)+10, sizeof(char));
						//concatenate the datetext and scriptstring
						if(datepresent == 1)
						{
							OVERLAPPED puskurac = { 0 };
							puskurac.Offset = appendlocationindex+overlapstruct.Offset;//the beggining of datemark + datemarklength is the beggining of text
							HANDLE hFile = { 0 };
							hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
							ReadFile(hFile, readbuffer, amountread, NULL, &puskurac);//load the datetext
							CloseHandle(hFile);
						}
						_memccpy(readbuffer + amountread, ScriptString, 0, pchinputbuffermemory / 2);
						savingFunction(&appendlocationindex, readbuffer, &overlapstruct, &amountread, datepresent);
						charspassed += strlen(ScriptString);
						dayspassed++;
						for (int mz = 0; mz <= charspassed; mz++)
						{
							if (ScriptString[mz] == '\n')
								newlinespassed++;
						}
						datepresent = appendlocationindex = datepresent = 0;
						free(readbuffer);
					}
					monthspassed++;
				}
				yearspassed++;
			}
			selecteddate = originaldateselected;//return original date selected
			charspassed = 0;
			newlinespassed = 0;
			return TRUE;
		case IDMACROSSI:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDSCRIPTMACRO), hwndDlg, DlgScriptMacros);
			return TRUE;
		case IDDATESSI:
			result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDSCRIPTDATES), hwndDlg, DlgScriptDates);
			return TRUE;
		case IDHELPSI:
			return TRUE;
		case TXTSCRIPTINPUT:
			return TRUE;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return FALSE;
}
//macrosetup dialog
INT_PTR CALLBACK DlgScriptMacros(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND ParenthwndDlg = { 0 }, listbox = { 0 }, textstring2 = { 0 };
	char macrosymbol = 0;
	HANDLE hFile = { 0 };
	char* tempstring = { 0 }, * readbuffer = { 0 }, * dataitem = { 0 };
	BOOL gayflag = FALSE;
	int fileszie = 0, readnumber=0;
	int static selection = 0;
	switch (message)
	{
	case WM_INITDIALOG:
		textstring2 = GetDlgItem(hwndDlg, TXTSCRIPTINPUT2);
		SetWindowSubclass(textstring2, MarkBoxInputSrc, 0, 0);
		SetWindowSubclass(GetDlgItem(hwndDlg, IDC_LIST1), ListBoxSrc, 0, 0);
		for (int i = 0; i < 256; i++)
			macrolist[i] = calloc(101, sizeof(char));
		pickamaterina = ParenthwndDlg = hwndDlg;
		SendDlgItemMessageA(hwndDlg, TXTSCRIPTINPUT2, EM_SETLIMITTEXT, pchinputbuffermemory/10, 0);//limits the editbox input to 1/10 of textbox input buffer
		SendDlgItemMessageA(hwndDlg, TXTSCRIPTINPUT3, EM_SETLIMITTEXT, 1, 0);//limits the editbox input to 1 bytes.
		SendDlgItemMessageA(hwndDlg, IDC_LIST1, LB_SETCOLUMNWIDTH, 40, 0);//limits the editbox input to 1 bytes.
		tempstring = calloc(1000, sizeof(char));
		sprintf_s(tempstring, 1000, "%sScriptMacros.txt", theworkingdirectory);
		hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		free(tempstring);
		fileszie = GetFileSize(hFile, NULL);
		readbuffer = calloc(fileszie, sizeof(char));
		readnumber = 0;
		int listindex = 0;
		ReadFile(hFile, readbuffer, fileszie, &readnumber, NULL);
		for (int i = 1, l = 3; i < 256 && fileszie>4 && l < fileszie; i++)//max 255 items on the list
		{
			//grab listindex
			listindex = (int)readbuffer[l - 2];//converts char into number used for listindex
			//then grab the text
			for (int m = l; readbuffer[l] != specialchar[0] && l < fileszie; l++)
			{
				macrolist[listindex][l - m] = readbuffer[l];
				if (readbuffer + 1 == fileszie)
				{
					fileszie = 4;//break the loop
				}
			}
			l += 4;
		}
		listbox = GetDlgItem(ParenthwndDlg, IDC_LIST1);
		for (int i = 0, b = 0; i < 256; i++)
		{//check if index is valid
			if (macrolist[i][0] != '\0')
			{//if its valid send the appropriate message to signal it to set the data associated with it
				char signalstring[3] = { 0 };
				signalstring[0] = '%';
				signalstring[1] = i;
				//int d = SendMessageA(listbox, LB_SETITEMDATA, b, macrolist[i]);
				//now set its text that will be shown in the list
				SendMessageA(listbox, LB_ADDSTRING, b, signalstring);
				b++;
				char* fagshit = calloc(1000, sizeof(char));
				//SendDlgItemMessageA(ParenthwndDlg, IDC_LIST1, fagshit, 1000);
			}
		}//with this you inititalize the scriptstringsmacros file of ours
		SetFocus(listbox);
		CloseHandle(hFile);
		free(readbuffer);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		case IDOK:
			return TRUE;
		case IDINFOSCRIPT:
			return TRUE;
		case IDREMOVESCRIPT:
			return TRUE;
		case IDMACROSET://click on button "macroset", sets a macro symbol and adds it to macrolist or modifies the preexisting macro
			//now append this data to our macrostrings file
			selection = 0;
			char * bullshitcancer = calloc(20, sizeof(char));
			Edit_GetText(GetDlgItem(ParenthwndDlg, TXTSCRIPTINPUT3), bullshitcancer, 2);
			selection = bullshitcancer[0];
			if (selection == 0)
			{
				free(bullshitcancer);
				return FALSE;
			}
			tempstring = calloc(1000, sizeof(char));
			char macrosymboldata[4] = { 0 };
			sprintf_s(tempstring, 1000, "%sScriptMacros.txt", theworkingdirectory);
			hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			fileszie = GetFileSize(hFile, NULL);
			macrosymboldata[0] = '%';
			macrosymboldata[1] = selection;
			macrosymboldata[2] = specialchar[0];
			char * readbuffer = calloc(fileszie+10, sizeof(char));
			OVERLAPPED overlapstruct = { 0 };
			ReadFile(hFile, readbuffer, fileszie, NULL, &overlapstruct);
			unsigned char intextflag = FALSE;
			BOOL seekfront = FALSE;
			BOOL symbolexists = FALSE;
			int firstoffset = 0 , frontoffset = 0;
			for(int i = 0; i < fileszie; i++)
			{
				if (readbuffer[i] == specialchar[0])
				{
					if (seekfront == TRUE && intextflag == FALSE)
					{
						frontoffset = i-2;//position of macro in front of the selected macro
						i = fileszie;//terminate the loop
					}
					if (intextflag == FALSE && i - 1 > 0 && readbuffer[i-1] == selection)
					{
						firstoffset = i-2;//position of the beggining of macro ie its % symbol
						seekfront = TRUE;//datapos
						symbolexists = TRUE;
					}
					intextflag = ~intextflag;
				}
				else if (i!=fileszie)
					frontoffset = i;
			} 
			if (symbolexists == FALSE)
				firstoffset = fileszie;
			overlapstruct.Offset = firstoffset;
			int length = GetWindowTextLengthA(textstring2)+1;
			char* newstring = calloc(length+2, sizeof(char));
			GetWindowTextA((textstring2), (newstring), (length));
			char * memoryvariable = calloc(length+fileszie+10, sizeof(char));
			memset(memoryvariable, 0, length + fileszie + 10);
			_memccpy(memoryvariable, macrosymboldata, 0, 3);
			_memccpy(memoryvariable + 3, newstring, 0, length);
			memoryvariable[3 + length - 1] = specialchar[0];//close the dataset with the star
			_memccpy(memoryvariable + 3 + length, readbuffer + frontoffset+2, 0, fileszie - frontoffset);
			int datachange = (frontoffset - firstoffset) - (length+1);
			int amounttowrite = fileszie - firstoffset - datachange;
			WriteFile(hFile, memoryvariable, amounttowrite, NULL, &overlapstruct);
			if (datachange > 0)
			{//shrinkfile
				SetFilePointer(hFile, -datachange, 0, FILE_END);
				SetEndOfFile(hFile);
			}
			//update the macrolist
			if (macrolist[selection][0] == 0)//macro not present, add it
			{
				char* charsstring = calloc(5, sizeof(char));
				charsstring[0] = '%';
				charsstring[1] = selection;
				SendMessageA(listbox, LB_ADDSTRING, 0, charsstring);
				free(charsstring);
			}
			memset(macrolist[selection], 0, 101);//empty the macro
			_memccpy(macrolist[selection], newstring, 0, length + 2);
			CloseHandle(hFile);
			free(tempstring);
		//	free(absoluteretard);
			free(bullshitcancer);
			free(newstring);
			return TRUE;
		case TXTSCRIPTINPUT2://DEFINITION MACROBOX
			return TRUE;
		case TXTSCRIPTINPUT3://ONE-LETTER MACROBOX
			if (HIWORD(wParam) == EN_UPDATE)
			{
				char* Remaints = calloc(100, sizeof(char));
				Edit_GetText(GetDlgItem(ParenthwndDlg, TXTSCRIPTINPUT3), Remaints, 2);
				selection = (int)Remaints[0];
				free(Remaints);
				SetWindowTextA(textstring2, macrolist[selection]);
			}
			return TRUE;
		case TXTSCRIPTSHOW:
			return TRUE;
		case IDC_LIST1:
			switch(HIWORD(wParam))
			{
			case LBN_DBLCLK:
				return TRUE;
			case LBN_ERRSPACE:
				return TRUE;
			case LBN_KILLFOCUS:
				return TRUE;
			case LBN_SELCANCEL:
				return TRUE;
			case LBN_SELCHANGE:
				selection = SendDlgItemMessageA(ParenthwndDlg, IDC_LIST1, LB_GETCARETINDEX, 0, 0);
				char* indexname = calloc(4, sizeof(char));
				SendDlgItemMessageA(ParenthwndDlg, IDC_LIST1, LB_GETTEXT, selection, indexname);
				selection = (int)indexname[1];
				SetWindowTextA(textstring2, macrolist[selection]);
				SetWindowTextA(GetDlgItem(hwndDlg, TXTSCRIPTINPUT3), indexname+1);
				free(indexname);
				return TRUE;
			case LBN_SETFOCUS:
				return TRUE;
			}
			return TRUE;
		}
	default:
		return FALSE;
	}
}
//daterange for the macros setup
INT_PTR CALLBACK DlgScriptDates(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL failcheck = TRUE;
	static HWND hwndDlgMain = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		hwndDlgMain = hwndDlg;
		SetDlgItemInt(hwndDlgMain, IDMONTHF, monthf, FALSE);
		SetDlgItemInt(hwndDlgMain, IDMONTHL, monthl, FALSE);
		SetDlgItemInt(hwndDlgMain, IDYEARF, yearf, FALSE);
		SetDlgItemInt(hwndDlgMain, IDYEARL, yearl, FALSE);
		SetDlgItemInt(hwndDlgMain, IDDAYF, dayf, FALSE);
		SetDlgItemInt(hwndDlgMain, IDDAYL, dayl, FALSE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		case IDOK:
			monthf = GetDlgItemInt(hwndDlgMain, IDMONTHF, &failcheck, FALSE);
			monthl = GetDlgItemInt(hwndDlgMain, IDMONTHL, &failcheck, FALSE);
			yearf = GetDlgItemInt(hwndDlgMain, IDYEARF, &failcheck, FALSE);
			yearl = GetDlgItemInt(hwndDlgMain, IDYEARL, &failcheck, FALSE);
			dayf = GetDlgItemInt(hwndDlgMain, IDDAYF, &failcheck, FALSE);
			dayl = GetDlgItemInt(hwndDlgMain, IDDAYL, &failcheck, FALSE);
			//code below ensures that 0 when inputted turns to one
			if (dayl == 0)
				dayl = 1;
			if (dayf == 0)
				dayf = 1;
			if (yearl == 0)
				yearl = 1;
			if (yearf == 0)
				yearf = 1;
			if (monthl == 0)
				monthl = 1;
			if (monthf == 0)
				monthf = 1;

			EndDialog(hwndDlg, 0);
			return TRUE;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
}
//Extracts the macros
char* ScriptFormat(char* Inputbuffer, HWND parenthwnd)
{
	char* returnstring = { 0 };
	int stringlength = 0;
	BOOL firstrun = TRUE;
	stringlength = strlen(Inputbuffer);
	for (int i = 0; i < stringlength; i++)
	{
		if (Inputbuffer[i] == '%')
		{
			if (i + 1 < stringlength)
			{
				i += 1;//point to character for macro symbol
				int macro = (int)Inputbuffer[i];
				i -= 1;//point to '%'
				if (macro < 255 && macro>-1 && macrolist[macro] != NULL)
				{
					_memccpy(Inputbuffer + i, Inputbuffer + i +2, 0, stringlength - 2);//removes '% and following character"
					int mstringlength = strlen(macrolist[macro]);
					char* macroformatted = calloc(mstringlength+4, sizeof(char));
					_memccpy(macroformatted, macrolist[macro], 0, mstringlength);
					macroformatted = MacroFormating(macroformatted, parenthwnd, firstrun);
					mstringlength = strlen(macroformatted);
					if (i + mstringlength >= 1000)//larger then scriptstirngbuffer
					{
						CrashDumpFunction(8989, 0);
						free(macroformatted);
						return Inputbuffer;
					}
					_memccpy(Inputbuffer + i + mstringlength, Inputbuffer + 2, 0, stringlength - i);//moves the string forward to make space for macrostringformatted, +2 excludes the % and symbol that follows it
					_memccpy(Inputbuffer + i, macroformatted, 0, mstringlength);//pastes macro into the string
					free(macroformatted);
					stringlength = stringlength + mstringlength;
					memset(Inputbuffer + stringlength, 0, 8);//set delimiting nulls
					charspassed += strlen(Inputbuffer);
				}
			}
		}
	}
	return Inputbuffer;
}

//expands the macro to its form
char* MacroFormating(char* macroformated, HWND parenthwnd, BOOL firstrun)
{
	int length = 0, templength = 0, oldselecteddate = 0, daysempty = 0;
	int* number = calloc(100, sizeof(int));
	int lastbyte = 0;
	length = strlen(macroformated);
	char scripts[50][100] = { 
		"Date",
		"Month",
		"Year",
		"TotalTextAmount",
		"TotalLines",
		"TotalCharacters",
		"TotalDateTextAmount",
		"TotalDateLines",
		"TotalDateCharacters",
		"TotalMemoryRemaining",
		"PassedDates",
		"PassedMonths",
		"PassedYears",
		"EmptyFlag",
		"EmptyDatesWithinAMonth",
		"EmptyDatesWithinAYear",
		"TotalDateSelectedTextAmount",
		"TotalDateSelectedLines",
		"TotalDateSelectedCharacters",
		"PassedChars",
		"PassedLines",
		"TotalPassedTextData",
		"sqrt",
		"Exponent",
		"Multiplication",
		"Plus",
		"Minus",
		"Number",
		"FloatNumber",
		"Logarithm",
		"Division",
		"Sum",
		"StringApply",
		"StringFalse",
		"If",
		"Equivalent",
		"BeginIf",
		"EndIf"
	};
	int monthsinaday[13] = { 0, 31,28,31,30,31,30,31,31, 30,31,30,31 };
	int scriptslength = 0, numberindex = 0, day = (selecteddate & 0x7F), month = (selecteddate & 0x780) >> 7, year = selecteddate >> 11, amountread = 0, appendindexlocation = 0, ifnumber = 1, ifsign = 0, macrolength = 0, difference = 0;
	int dummyint = 0;
	BOOL myswitch = FALSE;
	double dummyintfloat = 0, ifstatement1 = 0, ifstatement2 = 0;
	int static startyear = 0, startmonth = 0, startday = 0, oldmonth = 0, symbolindex = 0;
	int static timesinvoked = 0;
	static char signed stateflag[100] = { 0 };//used for arithemtic flags and if statements, capable of nesting them.
	/*Formating is thus like this, symbolsymbolsymbol, since there is 19 symbols each number represent a symbol*/
	/*
	* 0 - sqrt
	* 1 - Exponent
	* 2 - Multiplication
	* 3 - Plus
	* 4 - Minus
	* 5 - Logarithm
	* 6 - Division
	* 7 - Sum
	* 8 - If
	* 9 - Equivalent
	* -1 - LargerThen
	* -2 - SmallerThen
	* -3 - LargerorEqual
	* -4 - SmallerorEqual
	* -5 - NotEquivalent
	*/
	HANDLE hFile = { 0 };
	HWND dummy = { 0 };
	OVERLAPPED overlapstruct = { 0 };
	BOOL datepresent = { 0 }, ifflag = FALSE, foundstring = FALSE;
	char* tempvariable = { 0 };
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(firstrun == TRUE)
	{
		startyear = selecteddate >> 11;//these 3 never change afterwards
		startmonth = (selecteddate & 0x780) >> 7;
		startday = (selecteddate & 0x7F);
	}
	firstrun = FALSE;
	for (int i = 0; i < length; i++)
	{
		foundstring = FALSE;
		if (macroformated[i] == '!')
		{
			for(int n = 0; n < 38 && foundstring == FALSE; n++)
				if (0 == comparestrings(macroformated+(i+1), scripts[n]))
				{
					foundstring = TRUE;
					macrolength = strlen(scripts[n]+1);//length of the macro including ! sign 
					switch (n)
					{
					case 0://!Date ie. day
						scriptslength = 69;
						tempvariable = calloc(10, sizeof(char));
						sprintf_s(tempvariable, 10,"%i", day);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 1://month
						scriptslength = 69;
						tempvariable = calloc(10, sizeof(char));
						sprintf_s(tempvariable, 10,"%i", month);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 2://year
						scriptslength = 69;
						tempvariable = calloc(10, sizeof(char));
						sprintf_s(tempvariable, 10,"%i", year);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 3://!TotalTextAmount, Sum of the data within the data source
						dummyint = GetFileSize(hFile, NULL);
						tempvariable = calloc(20, sizeof(char));
						sprintf_s(tempvariable, 10,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 4://!TotalLines, Sum of lines of the data within the data source
						dummyint = GetFileSize(hFile, NULL);
						tempvariable = calloc(dummyint, sizeof(char));
						ReadFile(hFile, tempvariable, dummyint, &dummyint, NULL);
						for (int i = 0; i < dummyint; i++)
						{
							if (tempvariable[i] == '\n')
								numberindex++;
						}
						memset(tempvariable, 0, 50);
						sprintf_s(tempvariable, 50,"%i", numberindex);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 5://!TotalCharacters, Sum of characters of data within the data source
						dummyint = GetFileSize(hFile, NULL);
						tempvariable = calloc(100, sizeof(char));
						sprintf_s(tempvariable, 100,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 6://!TotalDateTextAmount, Sum of data within a Date
						dummyint = GetFileSize(hFile, NULL);
						tempvariable = calloc(100, sizeof(char));
						sprintf_s(tempvariable, 100,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 7://!TotalDateLines, Sum of lines within a Date
						DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
						tempvariable = calloc(amountread+10, sizeof(char));
						overlapstruct.Offset = appendindexlocation;
						ReadFile(hFile, tempvariable, amountread, NULL, &overlapstruct);
						for (int i = 0; i < amountread; i++)
						{
							if (tempvariable[i] == '\n')
								dummyint++;
						}
						memset(tempvariable, 0, amountread);
						sprintf_s(tempvariable, amountread+10,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 8://!TotalDateCharacters, Sum of characters within a Date
						DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
						tempvariable = calloc(50, sizeof(char));
						sprintf_s(tempvariable, 50,"%i", amountread);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 9://!TotalMemoryRemaining, memory remaining in bytes 
						dummyint = GetWindowTextLengthA(TextBoxHwnd);
						dummyint = SendMessageA(TextBoxHwnd, EM_GETLIMITTEXT, 0, 0) - dummyint;
						tempvariable = calloc(50, sizeof(char));
						sprintf_s(tempvariable, 50,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 10://!PassedDates, amount of dates passed within the runtime of the script
						dummyint = dayspassed;
						tempvariable = calloc(50, sizeof(char));
						sprintf_s(tempvariable, 50,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 11://!PassedMonths, amount of months passed within the runtime of the script 
						dummyint = monthspassed;
						tempvariable = calloc(50, sizeof(char));
						sprintf_s(tempvariable, 50,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 12://!PassedYears, amount of years passed within the runtime of the script
						dummyint = yearspassed;
						tempvariable = calloc(50, sizeof(char));
						sprintf_s(tempvariable, 50,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 13://!EmptyFlag, Yes if there is no data in date, No otherwise
						DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
						tempvariable = calloc(4, sizeof(char));
						sprintf_s(tempvariable, 4,"%i", datepresent);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 14://!EmptyDatesWithinAMonth, How many dates within the month are empty
						scriptslength = 69;
						oldselecteddate = selecteddate;
						//leap year mechanism
						monthsinaday[2] = 28;
						if (year % 4 == 0)
							if (year % 100)
							{
								if (year % 400)
									monthsinaday[2] = 29;
							}
							else
								monthsinaday[2] = 29;
						//end of leapyear mechanism
						daysempty = 0;
						for (int i = 1; i <= monthsinaday[month]; i++)
						{
							selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
							selecteddate += i + 1;//set the day bits
							DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
							if (datepresent == FALSE)
								daysempty++;
						}
						tempvariable = calloc(10, sizeof(char));
						sprintf_s(tempvariable, 10,"%i", daysempty);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 15://!EmptyDatesWithinAYear, How many dates within the year are empty
						scriptslength = 69;
						oldselecteddate = selecteddate;
						//leap year mechanism
						monthsinaday[2] = 28;
						if (year % 4 == 0)
							if (year % 100)
							{
								if (year % 400)
									monthsinaday[2] = 29;
							}
							else
								monthsinaday[2] = 29;
						//end of leapyear mechanism
						daysempty = 0;
						for(int k = 1; k<=12;k++)
						{
							selecteddate = selecteddate & 0xfff87f;//zeroout the month
							selecteddate += k << 7;//implant the month
							for (int i = 1; i <= monthsinaday[k]; i++)
							{
								selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
								selecteddate += i + 1;//set the day bits
								DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
								if (datepresent == FALSE)
									daysempty++;
							}
						}
						tempvariable = calloc(10, sizeof(char));
						sprintf_s(tempvariable, 10,"%i", daysempty);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 16://!TotalDateSelectedTextAmount, Amount of total data within the text selected for the script input
						dummyint = 0;
						oldselecteddate = selecteddate;
						daysempty = 0;
						for (int l = yearf; l <= yearl; l++)
						{
							selecteddate = selecteddate & 0x7ff;//zerout the year
							selecteddate += i << 11;//put the year inside selecteddate;
							//leap year mechanism
							monthsinaday[2] = 28;
							if (l % 4 == 0)
								if (l % 100)
								{
									if (l % 400)
										monthsinaday[2] = 29;
								}
								else
									monthsinaday[2] = 29;
							//end of leapyear mechanism
							for (int k = monthf; k <= monthl; k++)
							{
								selecteddate = selecteddate & 0xfff87f;//zeroout the month
								selecteddate += k << 7;//implant the month
								for (int i = 1; i <= monthsinaday[k]; i++)
								{
									selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
									selecteddate += i + 1;//set the day bits
									DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
									dummyint += amountread;
								}
							}
						}
						tempvariable = calloc(10, sizeof(char));
						sprintf_s(tempvariable, 10,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 17://!TotalDateSelectedLines, Amount of total lines within the text selected for script input
						dummyint = 0;
						oldselecteddate = selecteddate;
						daysempty = 0;
						for (int l = yearf; l <= yearl; l++)
						{
							selecteddate = selecteddate & 0x7ff;//zerout the year
							selecteddate += i << 11;//put the year inside selecteddate;
							//leap year mechanism
							monthsinaday[2] = 28;
							if (l % 4 == 0)
								if (l % 100)
								{
									if (l % 400)
										monthsinaday[2] = 29;
								}
								else
									monthsinaday[2] = 29;
							//end of leapyear mechanism
							for (int k = monthf; k <= monthl; k++)
							{
								selecteddate = selecteddate & 0xfff87f;//zeroout the month
								selecteddate += k << 7;//implant the month
								for (i = 1; i <= monthsinaday[k]; i++)
								{
									selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
									selecteddate += i + 1;//set the day bits
									DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
									int sizeoffile = GetFileSize(hFile, NULL);
									char* tempbuffer = calloc(sizeoffile, sizeof(char));
									for (int z = overlapstruct.Offset; z < amountread; z++)
									{
										if (tempbuffer[z] == '\n')
											dummyint++;
									}
								}
							}
						}
						tempvariable = calloc(10, sizeof(char));
						sprintf_s(tempvariable, 10,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 18://!TotalDateSelectedCharacters, Amount of total characters within the text selected for script input, tbd with compatibility update for wide characters, right now same as the data 
						dummyint = 0;
						oldselecteddate = selecteddate;
						daysempty = 0;
						for (int l = yearf; l <= yearl; l++)
						{
							selecteddate = selecteddate & 0x7ff;//zerout the year
							selecteddate += i << 11;//put the year inside selecteddate;
							//leap year mechanism
							monthsinaday[2] = 28;
							if (l % 4 == 0)
								if (l % 100)
								{
									if (l % 400)
										monthsinaday[2] = 29;
								}
								else
									monthsinaday[2] = 29;
							//end of leapyear mechanism
							for (int k = monthf; k <= monthl; k++)
							{
								selecteddate = selecteddate & 0xfff87f;//zeroout the month
								selecteddate += k << 7;//implant the month
								for (int i = 1; i <= monthsinaday[k]; i++)
								{
									selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
									selecteddate += i + 1;//set the day bits
									DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
									dummyint += amountread;
								}
							}
						}
						tempvariable = calloc(10, sizeof(char));
						sprintf_s(tempvariable, 10,"%i", dummyint);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 19://!PassedChars, Amount of chars typed in the runtime of the script
						dummyint = 0;
						tempvariable = calloc(30, sizeof(char));
						sprintf_s(tempvariable, 30,"%i", charspassed);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 20://!PassedLines, Amount of lines typed in the runtime of the script
						dummyint = 0;
						tempvariable = calloc(30, sizeof(char));
						sprintf_s(tempvariable, 30,"%i", newlinespassed);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 21://!TotalPassedTextData, Total amount of data inputted within the runtime of the script, same as passed chars, to be updated when we make compatibility with widechars
						dummyint = 0;
						tempvariable = calloc(30, sizeof(char));
						sprintf_s(tempvariable, 30,"%i", charspassed);
						templength = strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, templength);
							memset(macroformated + length + difference, 0, -difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 22://!sqrt, Sqaure root of a number: !sqrt!Number10
						stateflag[symbolindex++] = 0;
						break;
					case 23://!Exponent, Exponent of a number: !Exponent!Number10
						stateflag[symbolindex++] = 1;
						break;
					case 24://!Multiplication, Multiplication of a number: !Multiplication!Number100!Number100
						stateflag[symbolindex++] = 2;
						break;
					case 25://!Plus, Add a number to a number: !Plus!Number10!Number5
						stateflag[symbolindex++] = 3;
						break;
					case 26://!Minus, Minus a number by a number: !Minus!Number10!Number5
						stateflag[symbolindex++] = 4;
						break;
					case 27://!Number, Declares that following string will be numbers and to be treated as numbers by the program. It will consider number any decimal integer, once it finds a non decimal integer it will stop the number classification. Thus !Number100 is translated as number 100 within the program, !Numbered will be translated as invalid number, d is not a number. !Number52 100, will translate 52 as a number.
						for (int p = 0; p < symbolindex; p++)
						{
							dummyint = NumbersFunction(stateflag, symbolindex, macroformated, i, FALSE, &lastbyte);
							//uses nesting system ie. !sqrt!sum!Number50!Number80!Plus!Logarithm!Number20!Number50, you nest these to do multiple operations in an order of desire, once !Number appears, the nest stops and operation happen
						}
						tempvariable = calloc(30, sizeof(char));
						sprintf_s(tempvariable, 30,"%i", dummyint);
						templength = strlen(tempvariable);
						//replace the ! macro members with results, move index i to the first position of the result
						for (int l = symbolindex-1; l >= 0; l--)
						{
							switch (stateflag[l])
							{
							case 1://exponent
								i -= 9;
								break;
							case 0://sqrt
								i -= 5;
								break;
							case 5://Logarithm, number2 determines base, number determines the argument
								i -= 10;
								break;
							case 6://Division, number is dividend, number2 divisor
								i -= 9;
								break;
							case 2://Multiplication
								i -= 15;
								break;
							case 3://Plus, //get number 2 and sum it up with number 1 basically
								i -= 5;
								break;
							case 4://Minus
								i -= 6;
								break;
							case 7://Sum
								i -= 4;
								break;
							default:
								break;
							}
						}
						//input the data
						difference = (lastbyte - i) - templength;
						if (difference < 0)
						{//input is larger, push data forward
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							memset(macroformated + length + difference, 0, -difference);//push data forwards to make space
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if (difference > 0)
						{
							//shrinkg the string now
							_memccpy(macroformated + i + templength, macroformated + lastbyte, 0, length - lastbyte);
							//zerout the excess bytes if necessary
							memset(macroformated + difference + i, 0, length - difference);//zerout excess bytes
						}
						free(tempvariable);
						if (ifflag == TRUE)
						{
							if (ifnumber == 1)
							{
								ifstatement1 = dummyint;
							}
							else if (ifnumber == 2)
							{
								int ifresult = 0;
								ifstatement2 = dummyint;
								ifnumber = 1;
								//determine if the if statement is true, then keep going properly within the bound of ifstatement till end if, else stop and skip everything till after !endif
								switch (ifsign)
								{
								case 1://!Equivalent
									if (ifstatement1 == ifstatement2)
										ifresult = 1;
									break;
								case 2://!LargerThen
									if (ifstatement1 > ifstatement2)
										ifresult = 1;
									break;
								case 3://!SmallerThen
									if (ifstatement1 < ifstatement2)
										ifresult = 1;
									break;
								case 4://LargerorEqual
									if (ifstatement1 >= ifstatement2)
										ifresult = 1;
									break;
								case 5://SmallerorEqual
									if (ifstatement1 <= ifstatement2)
										ifresult = 1;
									break;
								case 6://NotEquivalent
									if (ifstatement1 != ifstatement2)
										ifresult = 1;
									break;
								default:
									break;
								}
								//below is skipping of endif in case if returned false
								BOOL myswitch = FALSE;
								if (ifresult == 0)//skip everything till next endif
								{
									for (; i < length && myswitch == FALSE; i++)
									{
										if (macroformated[i] == '!')
										{
											{
												for (; n < 38 && myswitch == FALSE; n++)
													if (0 == comparestrings(macroformated[i + 1], "!EndIf"))
														myswitch = TRUE;//kill loop sitting at EndIf
											}
										}
									}
								}
							}
						}
						break;
					case 28://!FloatNumber, Will translate number as a float ie if you type !Number100.200 will only give you number 100, !FloatNumber100.200 however will give you 100.200 as a number.
						for (int p = 0; p <= symbolindex; p++)
						{
							dummyintfloat = NumbersFunction(stateflag, symbolindex, macroformated, i, TRUE, &lastbyte);
							//uses nesting system ie. !sqrt!sum!Number50!Number80!Plus!Logarithm!Number20!Number50, you nest these to do multiple operations in an order of desire, once !Number appears, the nest stops and operation happen
						}
						tempvariable = calloc(30, sizeof(char));
						sprintf_s(tempvariable, 30,"%ld", dummyintfloat);
						templength = strlen(tempvariable);
						for (int l = symbolindex-1; l >= 0; l--)
						{
							switch (stateflag[l])
							{
							case 1://exponent
								i -= 8;
								break;
							case 0://sqrt
								i -= 4;
								break;
							case 5://Logarithm, number2 determines base, number determines the argument
								i -= 9;
								break;
							case 6://Division, number is dividend, number2 divisor
								i -= 8;
								break;
							case 2://Multiplication
								i -= 14;
								break;
							case 3://Plus, //get number 2 and sum it up with number 1 basically
								i -= 4;
								break;
							case 4://Minus
								i -= 5;
								break;
							case 7://Sum
								i -= 3;
								break;
							default:
								break;
							}
						}//wipe out the macroformated up the beggining of the first symbol of the mathemathical shit
						//input the data
						difference = (lastbyte - i) - templength;
						if (difference < 0)
						{//input is larger, push data forward
							macroformated = realloc(macroformated, sizeof(char) * difference + 1);
							memset(macroformated + length + difference, 0, -difference);//push data forwards to make space
						}
						_memccpy(macroformated + i, tempvariable, 0, templength);
						if(difference>0)
						{
							//shrinkg the string now
							_memccpy(macroformated + i + templength, macroformated + lastbyte, 0, length - lastbyte);
							//zerout the excess bytes if necessary
							memset(macroformated + difference + i, 0, length - difference);//zerout excess bytes
						}
						free(tempvariable);
						if (ifflag == TRUE)
						{
							if (ifnumber == 1)
							{
								ifstatement1 = dummyintfloat;
							}
							else if (ifnumber == 2)
							{
								int ifresult = 0;
								ifstatement2 = dummyintfloat;
								ifnumber = 1;
								//determine if the if statement is true, then keep going properly within the bound of ifstatement till end if, else stop and skip everything till after !endif
								switch (ifsign)
								{
								case 1://!Equivalent
									if (ifstatement1 == ifstatement2)
										ifresult = 1;
									break;
								case 2://!LargerThen
									if (ifstatement1 > ifstatement2)
										ifresult = 1;
									break;
								case 3://!SmallerThen
									if (ifstatement1 < ifstatement2)
										ifresult = 1;
									break;
								case 4://LargerorEqual
									if (ifstatement1 >= ifstatement2)
										ifresult = 1;
									break;
								case 5://SmallerorEqual
									if (ifstatement1 <= ifstatement2)
										ifresult = 1;
									break;
								case 6://NotEquivalent
									if (ifstatement1 != ifstatement2)
										ifresult = 1;
									break;
								default:
									break;
								}
								//below is skipping of endif in case if returned false
								if (ifresult == 0)//skip everything till next endif
								{
									for (; i < length && myswitch == FALSE; i++)
									{
										if (macroformated[i] == '!')
										{
											{
												for (; n < 38 && myswitch == FALSE; n++)
													if (0 == comparestrings(macroformated[i + 1], "!EndIf"))
														myswitch = TRUE;//kill loop sitting at EndIf
											}
										}
									}
								}
							}
						}
						break;
					case 29://!Logarithm, !Logarithm!Number10!Number4, First number is the log value and second number log input.
						stateflag[symbolindex++] = 5;
						break;
					case 30://!Division, !Division!Number8!Number2 will give you 4
						stateflag[symbolindex++] = 6;
						break;
					case 31://!Sum, !Sum!Number4!Number-4!Number2 will give you 2, it allows for summing line full of !Number 
						stateflag[symbolindex++] = 7;
						break;
					case 32://!StringApply, Applies a String, used to terminate !StringFalse statement essentially
						break;
					case 33://!StringFalse, Will not apply a string, till next !StringApply
						for (; i < length && myswitch == FALSE; i++)
						{
							if (macroformated[i] == '!')
							{
								{
									for (; n < 38 && myswitch == FALSE; n++)
										if (0 == comparestrings(macroformated[i + 1], "!StringApply"))
											myswitch = TRUE;//kill loop at !StringApply
								}
							}
						}
						break;
					case 34://!If, an if statement, !If(!Number2!Equivalent!MacroZ), if if statement is true it will continue down the script, and not otherwise.
						if (ifflag == FALSE)
							ifflag = TRUE;
						break;
					case 35://!Equivalent, Serves as a comparison statement within the if statement
						if (ifflag == TRUE && ifnumber == 1)
						{
							ifsign = 1;
							ifnumber = 2;
						}
						break;
					case 36://!EndIf, end of if statement, terminates the !If
						if (ifflag == TRUE && ifnumber == 2)
						{
							ifflag=0;
							ifnumber = 1;
						}
						break;
					case 37://!LargerThen
						if (ifflag == TRUE && ifnumber == 1)
						{
							ifsign = 2;
							ifnumber = 2;
						}
						break;
					case 38://!SmallerThen
						if (ifflag == TRUE && ifnumber == 1)
						{
							ifsign = 3;
							ifnumber = 3;
						}
						break;
					case 39://!LargerorEqual
						if (ifflag == TRUE)
						{
							ifsign = 4;
							ifnumber = 4;
						}
						break;
					case 40://!SmallerorEqual
						if (ifflag == TRUE && ifnumber == 1)
						{
							ifsign = 5;
							ifnumber = 5;
						}
						break;
					case 41://!NotEquivalent
						if (ifflag == TRUE && ifnumber == 1)
						{
							ifsign = 6;
							ifnumber = 6;
						}
						break;
					default:
						break;
					}
				}
				length = strlen(macroformated);
		}
	}
	CloseHandle(hFile);
	return macroformated;
}

INT_PTR CALLBACK MarkBoxInputSrc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CHAR:
		if (wParam == specialchar[0])
		{
			return FALSE;//dont type in start
		}
	}
	return DefSubclassProc(hwnd, message, wParam, lParam);
}

INT_PTR CALLBACK ListBoxSrc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case LB_ADDSTRING:
		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);
		return DefSubclassProc(hwnd, message, wParam, lParam);
	case LB_SETITEMDATA:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	case LBN_DBLCLK:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	case LBN_ERRSPACE:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	case LBN_KILLFOCUS:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	case LBN_SELCANCEL:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	case LBN_SELCHANGE:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	case LBN_SETFOCUS:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	}

	return DefSubclassProc(hwnd, message, wParam, lParam);
}
//Deals with arithmetics, capable of nesting
int NumbersFunction(int * symbolsarray, int maxsymbols, char * macroformated, int numbersbegginingindex, BOOL FloatFlag, int * lastbyteindex)
{
	int returnvalue = 0, number = 0, index = 0, length = 0, number2 = 0, sumarray[100] = { 0 }, number2factor = 0;//number2factor is incremented each time a number 2, number in front of number the called the function is called. Once incremented it uses another number in front as number 2, while the old number 2 is simply assumed as part of already calculated number. If there is no more number2 then it will be -1 and any math that necessiates it will fail
	char* tempvariable = NULL;
	long double numberf1 = 0, numberf2 = 0;
	number = StrToIntA(macroformated+(numbersbegginingindex + 7));
		for (int i = maxsymbols-1; i >= 0; i--)
		{
			if(FloatFlag == FALSE)
			{
				switch (symbolsarray[i])
				{
				case 1://exponent
					number = exp(number);
					break;
				case 0://sqrt
					number = sqrt(number);
					break;
				case 5://Logarithm, number2 determines base, number determines the argument
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), "!Number")))
					{
						number2 = StrToIntA(macroformated+(index + 7));
						number2factor++;
						number = log10(number) / log10(number2);
						numbersbegginingindex += index + 7;
					}
					else
					{
						number2 = -1;
						return number;
					}
					break;
				case 6://Division, number is dividend, number2 divisor
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), "!Number")))
					{
						number2 = StrToIntA(macroformated + (index + 7));
						number2factor++;
						number = number / number2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						number2 = -1;
						return number;
					}
					break;
				case 2://Multiplication
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), "!Number")))
					{
						number2 = StrToIntA(macroformated + (index + 7));
						number2factor++;
						number = number * number2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						number2 = -1;
						return number;
					}
					break;
				case 3://Plus, //get number 2 and sum it up with number 1 basically
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + index, "!Number")))
					{
						number2 = StrToIntA(macroformated + (index + 7));
						number2factor++;
						number = number + number2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						number2 = -1;
						return number;
					}
					break;
				case 4://Minus
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), "!Number")))
					{
						number2 = StrToIntA(macroformated + (index + 7));
						number2factor++;
						number = number - number2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						number2 = -1;
						return number;
					}
					break;
				case 7://Sum
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length; index++)
					{
						if ((0 == comparestrings(macroformated + (index), "!Number")))
						{
							number2 = StrToIntA(macroformated + (index + 7));
							number2factor++;
							number = number * number2;
							numbersbegginingindex += index + 7;
						}
						else if (isdigit(macroformated + (index)) == FALSE)
						{
							number2 = -1;
							return number;
						}
					}
					break;
				default:
					break;
				}
			}
			if (FloatFlag == TRUE)
			{
				switch (symbolsarray[i])
				{
				case 1://exponent
					numberf1 = expl(numberf1);
					break;
				case 0://sqrt
					numberf1 = sqrtl(numberf1);
					break;
				case 5://Logarithm, number2 determines base, number determines the argument
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), "!Number")))
					{
						numberf1 = strtod(macroformated + (index + 7), NULL);
						number2factor++;
						numberf1 = log10l(numberf1) / log10l(numberf2);
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return numberf1;
					}
					break;
				case 6://Division, number is dividend, number2 divisor
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), "!Number")))
					{
						numberf2 = strtod(macroformated + (index + 7), NULL);
						number2factor++;
						numberf1 = numberf1 / numberf2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return numberf1;
					}
					break;
				case 2://Multiplication
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), "!Number")))
					{
						numberf2 = strtod(macroformated+(index + 7), NULL);
						number2factor++;
						numberf1 = numberf1 * numberf2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return numberf1;
					}
					break;
				case 3://Plus
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), "!Number")))
					{
						numberf2 = strtod(macroformated + (index + 7), NULL);
						number2factor++;
						numberf1 = numberf1 + numberf2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return numberf1;
					}
					break;
				case 4://Minus
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), "!Number")))
					{
						numberf2 = strtod(macroformated + (index + 7), NULL);
						number2factor++;
						numberf1 = numberf1 - numberf2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return numberf1;
					}
					break;
				case 7://Sum
					length = strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length; index++)
					{
						if ((0 == comparestrings(macroformated + (index), "!Number")))
						{
							numberf2 = strtod(macroformated + (index + 7), NULL);
							number2factor++;
							numberf1 = numberf1 * numberf2;
							numbersbegginingindex += index + 7;
						}
						else if (isdigit(macroformated[numbersbegginingindex + index]) == FALSE)
						{
							numberf2 = -1;
							return numberf1;
						}
					}
					break;
				default:
					break;
				}
			}
		}
		int kl = 0;
		//lastbyteindex calculation is all about calculating lass position covered by the algorithm, and remove it.
		if (FloatFlag == FALSE)
		{
			for (kl = 0; isdigit(macroformated[7 + index + kl]); kl++)
				;
			*lastbyteindex = index + 7+kl;
			return number;
		}
		else
		{
			for (kl = 0; isdigit(macroformated[7 + index + kl]); kl++)
				;
			*lastbyteindex = index + 7+kl;
			return numberf1;
		}
}

int ifstatmentfunction()
{

	return TRUE;
}
;//beggining to the end linear simple string comparing
BOOL comparestrings(char* string1, char* string2)
{
	int string1length = strlen(string1);
	int string2length = strlen(string2);
	for (int i = 0; i < string1length && i < string2length; i++)
	{
		if (string1[i] != string2[i])
			return TRUE;
	}
	if (string1length < string2length)
		return TRUE;
	return FALSE;
}

INT_PTR CALLBACK DlgColorWindows(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND colorbutton = { 0 };
	static COLORREF * button8color = { 0 }, customcolors[20] = { 0 };
	HDC tempDC = { 0 };
	static HWND oghwnd = { 0 };
	static int tripval = 0, filesize = 0; 
	static HFILE hFile = { 0 };
	static char* tempstring = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		oghwnd = hwndDlg;
		colorbutton = GetDlgItem(hwndDlg, IDC_BUTTON8);
		break;
	case WM_DRAWITEM:
		if (wParam == IDC_BUTTON8)
		{
			DRAWITEMSTRUCT * drawstruck = { 0 };
			drawstruck = lParam;
			HBRUSH tempbrush = { 0 };
			if (button8color != NULL)
				tempbrush = CreateSolidBrush(*button8color);
			FillRect(drawstruck->hDC, &drawstruck->rcItem, tempbrush);
			DeleteObject(tempbrush);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON8://saves the colors
			if (tripval != 0)
			{
				CHOOSECOLORA choosecolor = { 0 };
				choosecolor.lStructSize = sizeof(CHOOSECOLOR);
				choosecolor.hwndOwner = oghwnd;
				choosecolor.hInstance = (HWND)GetModuleHandle(NULL);
				choosecolor.rgbResult = *button8color;
				choosecolor.Flags = CC_ANYCOLOR | CC_RGBINIT | CC_SOLIDCOLOR;
				choosecolor.lCustData = 0;
				choosecolor.lpfnHook = NULL;
				choosecolor.lpTemplateName = NULL;
				choosecolor.lpCustColors = (void*)customcolors;
				ChooseColorA(&choosecolor);
				HWND updatelist[10] = { 0 };
				updatelist[1] = GetParent(buttonarray[0]);;
				updatelist[3] = DatesHwnd;
				updatelist[4] = GetParent(buttonarray[0]);
				updatelist[5] = DatesHwnd;
				updatelist[6] = TextBoxHwnd;
				updatelist[7] = GetParent(buttonmarks[0]);
				*button8color = choosecolor.rgbResult;
				if (tripval == 6)
				{
					SendMessage(TextBoxHwnd, TEXTBOXCOLORCHANGE, 0, 0);
				}
				InvalidateRect(updatelist[tripval], NULL, TRUE);
				UpdateWindow(updatelist[tripval]);
				tempstring = calloc(1000, sizeof(char));
				sprintf_s(tempstring, 1000, "%sCalendar.dat", theworkingdirectory);
				hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				filesize = GetFileSize(hFile, NULL);
				char* writebuffer = calloc(3000, sizeof(char));
				SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
				SetEndOfFile(hFile);
				sprintf_s(tempstring, 1000, "%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear, datasource, specialchar, ordereddatasave);
				WriteFile(hFile, writebuffer, strlen(writebuffer), &filesize, NULL);
				if(hFile > 0)
					CloseHandle(hFile);
				free(tempstring);
			}
			break;
		case IDC_BUTTON1://month button colors
			button8color = &monthsbuttoncolor;
			InvalidateRect(colorbutton, NULL, FALSE);
			UpdateWindow(colorbutton);
			tripval = 1;
			break;
		case IDC_BUTTON3://day button color
			button8color = &datesbuttoncolor;
			InvalidateRect(colorbutton, NULL, TRUE);
			UpdateWindow(colorbutton);
			tripval = 3;
			break;
		case IDC_BUTTON4://months background colors
			button8color = &monthsbackground;
			InvalidateRect(colorbutton, NULL, TRUE);
			UpdateWindow(colorbutton);
			tripval = 4;
			break;
		case IDC_BUTTON5://days background colors
			button8color = &datesbackground;
			InvalidateRect(colorbutton, NULL, TRUE);
			UpdateWindow(colorbutton);
			tripval = 5;
			break;
		case IDC_BUTTON6://textbox background color
			button8color = &textbackground;
			InvalidateRect(colorbutton, NULL, TRUE);
			UpdateWindow(colorbutton);
			tripval = 6;
			break;
		case IDC_BUTTON7://input signal color
			button8color = &inputsignalcolor;
			InvalidateRect(colorbutton, NULL, TRUE);
			UpdateWindow(colorbutton);
			tripval = 7;
			break;
		case IDOK:
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return TRUE;
}
//changesmonths range
INT_PTR CALLBACK DlgMonthsRange(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL failcheck = TRUE;
	static HWND hwndDlgMain = { 0 };
	int filesize = 0, rangeyearf = 0, rangeyearl = 0;
	static char* tempstring = { 0 };
	char* readbuffer = { 0 };
	HFILE hFile = { 0 };
	static OVERLAPPED overlapped = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		hwndDlgMain = hwndDlg;
		SetDlgItemInt(hwndDlgMain, IDYEARF, yearzero, FALSE);
		SetDlgItemInt(hwndDlgMain, IDYEARL, yearrange/12+yearzero, FALSE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		case IDOK://TODO EMPTY AND CREATE NEW LOCALDATA.tXT With new range.
			rangeyearf = GetDlgItemInt(hwndDlgMain, IDYEARF, &failcheck, FALSE);
			rangeyearl = GetDlgItemInt(hwndDlgMain, IDYEARL, &failcheck, FALSE)*12-(rangeyearf*12);
			//code below ensures that 0 when inputted turns to one
			if (rangeyearl == 0)
				rangeyearl = 1;
			if (rangeyearf == 0)
				rangeyearf = 1;
			int DUVAJKURAC = 0;
			CalendarCreate(1, rangeyearf, rangeyearl);
			tempstring = calloc(1000, sizeof(char));
			sprintf_s(tempstring, 1000, "%sCalendar.dat", theworkingdirectory);
			hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
			SetEndOfFile(hFile);
			char* writebuffer = calloc(3000, sizeof(char));
			sprintf_s(tempstring, 1000, "%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear, datasource, specialchar, ordereddatasave);
			WriteFile(hFile, writebuffer, strlen(writebuffer), NULL, &overlapped);
			CloseHandle(hFile);
			int zerothwindowpos = (yearrange/2)+1;
			for (int lk = 0; lk < buttonfactor; lk++, zerothwindowpos++)
			{
				SetWindowLongPtr(buttonarray[lk], GWLP_USERDATA, zerothwindowpos);
			}
			SendMessageA(buttonarray[0], WM_LBUTTONDOWN, 0, 0);//update everything
			InvalidateRect(GetParent(buttonarray[0]), NULL, TRUE);
			UpdateWindow(GetParent(buttonarray[0]));
			EndDialog(hwndDlg, 0);
			return TRUE;
		default:
			return FALSE;
		}
	case WM_DESTROY:
		free(tempstring);
	default:
		return FALSE;
	}
}

typedef struct dateanddata
{
	char date[20];
	char* data;
}dateanddata, **lpdateanddata, *fagdata;

typedef struct date
{
	int year;
	int month;
	int day;
}date, ** ppdate, * pdate;

//reorders data save into ordered state
/*
1. Check the sync between mydatasets and mydatesarrays, the rearrangers seem to separate the months/year/days in a way from their original state and export different ones somewhere along the way
*/
char* DataSaveReordering(char* readbuffer)
{
	lpdateanddata mydataset = calloc(1, sizeof(fagdata));
	int datesamount = 0;
	int stringlength = strlen(readbuffer);
	//1st get all dates and data
	for (int i = 0; i < stringlength; i++)
	{
		if (readbuffer[i] == specialchar[0])
		{//allocate the struct
			//allocate the pointers to the array of struct pointers
			mydataset = realloc(mydataset, sizeof(fagdata)*(datesamount+2));
			//initialize the pointer necessary
			mydataset[datesamount] = calloc(1, sizeof(dateanddata));
			//set the date of the struct
			mydataset[datesamount]->date[0] = '*';//
			i++;//to skip the first star
			_memccpy(mydataset[datesamount]->date+1, readbuffer + i, specialchar[0], 20);
			//get the datalength
			int tempdatalength = 0;
			for (int m = i + strlen(mydataset[datesamount]->date)-1; m < stringlength && readbuffer[m] != specialchar[0]; m++)//-1 cause we plused it before
				tempdatalength++;
			mydataset[datesamount]->data = calloc(tempdatalength + 10, sizeof(char));
			//paste the data
			_memccpy(mydataset[datesamount]->data, readbuffer + i + strlen(mydataset[datesamount]->date)-1, specialchar[0], tempdatalength);
			i += tempdatalength + strlen(mydataset[datesamount]->date)-1;//to liquidite the +1 we had manually added
			datesamount++;
			i--;//to avoid skipping the '*' upon i++ at the end of the loop
		}
	}

	//now memset and apply new data
	memset(readbuffer, 0, stringlength);
	int bpos = 0;
	pdate datesarray = calloc(datesamount, sizeof(date));
	char* tempdate = calloc(20, sizeof(char));
	//arrange the years, all data arrange from smallest to largest
	for (int i = 0; i < datesamount; i++)
	{
		memset(tempdate, 0, 20);
		_memccpy(tempdate, mydataset[i]->date + 7, '*', 20);
		datesarray[i].year = StrToIntA(tempdate);
		for (int k = i;k > 0 && datesarray[k - 1].year > datesarray[k].year;k--)
		{//data in back is larger
			int temp = datesarray[k].year;//save the i
			datesarray[k].year = datesarray[k - 1].year;//put the larger data forward
			datesarray[k - 1].year = temp;//put the smaller data back
		}
	}
	//arrange the months
	for (int i = 0; i < datesamount; i++)
	{//year are already arranged!!
		memset(tempdate, 0, 20);
		_memccpy(tempdate, mydataset[i]->date + 7, ':', 20);
		int loadingyear = StrToIntA(tempdate);
		memset(tempdate, 0, 20);//get the month
		_memccpy(tempdate, mydataset[i]->date + 4, ':', 20);
		int loadingmonth = StrToIntA(tempdate);
		int yearset = 0;
		for (int m = datesamount-1; m >= 0; m--)
		{
			if (datesarray[m].year == loadingyear)
			{
				yearset = m;//set the yearset to m
				//check if there is more same years, and insert it in m-1 pos if there is, this is to avoid overwriting and ensuring 
				for (int z = m; datesarray[z].year == loadingyear && z >= 0; z--)
				{
					if (datesarray[z].month == 0)
					{
						datesarray[z].month = loadingmonth;//insert the month were there is no data, to avoid overwriting!
						z = 0;//break the loop
					}
				}
				m = 0;//break the loop
			}
		}

		//datesarray[i].month = StrToIntA(tempdate);
		//memset(tempdate, 0, 20);
		for (int n = yearset; n > 0 && datesarray[n - 1].year == datesarray[n].year; n--)//make sure its month of a given year range
		{
			for (int k = n; datesarray[k - 1].month > datesarray[k].month && datesarray[k - 1].year == datesarray[k].year; k++)
			{//data in back is larger, push it in front of all data that is smaller
				int temp = datesarray[k].month;//save the i
				datesarray[k].month = datesarray[k - 1].month;//put the larger data forward
				datesarray[k - 1].month = temp;//put the smaller data back
				n = 0;//break the loop
			}
			
		}
	}

	//arrange the days
	memset(tempdate, 0, 20);
	for (int i = 0; i < datesamount; i++)
	{//months are already arranged!!
		memset(tempdate, 0, 20);
		_memccpy(tempdate, mydataset[i]->date + 7, ':', 20);
		int loadingyear = StrToIntA(tempdate);
		memset(tempdate, 0, 20);//get the month
		_memccpy(tempdate, mydataset[i]->date + 4, ':', 20);
		int loadingmonth = StrToIntA(tempdate);
		memset(tempdate, 0, 20);
		_memccpy(tempdate, mydataset[i]->date + 1, ':', 20);
		int loadingday = StrToIntA(tempdate);
		int monthset;
		int yearset;
		for (int m = datesamount - 1; m >= 0; m--)
		{
			if (datesarray[m].year == loadingyear)
			{
				yearset = m;//set the yearset to m
				for(int j = m; j>=0;j--)//get the monthset
				{
					if (datesarray[j].month == loadingmonth)
					{
						monthset = j;
						//check if there is more same years, and insert it in m-1 pos if there is, this is to avoid overwriting and ensuring 
						for (int z = j; datesarray[z].month == loadingmonth && z >= 0; z--)
						{
							if (datesarray[z].day == 0)
							{
								datesarray[z].day = loadingday;//insert the month were there is no data, to avoid overwriting!
								z = 0;//break the loop
							}
						}
						m = 0;//break the loop
						j = 0;//break the loop
					}
				}
			}
		}
		for (int n = monthset; n > 0 && datesarray[n - 1].year == datesarray[n].year && datesarray[n-1].month == datesarray[n].month; n--)//make sure its month of a given year range
		{
			for (int k = n; datesarray[k - 1].day > datesarray[k].day && datesarray[k - 1].year == datesarray[k].year && datesarray[k - 1].month == datesarray[k].month; k++)
			{//data in back is larger
				int temp = datesarray[k].day;//save the i
				datesarray[k].day = datesarray[k - 1].day;//put the larger data forward
				datesarray[k - 1].day = temp;//put the smaller data back
			}
		}
	}

	memset(readbuffer, 0, stringlength);
	char* date = calloc(20, sizeof(char));
	for (int i = 0; i < datesamount; i++)
	{
		//get the first date

		if(datesarray[i].day < 10 && datesarray[i].month < 10)
			sprintf_s(date, 20, "%d :%d :%d", datesarray[i].day, datesarray[i].month, datesarray[i].year);
		else if (datesarray[i].day < 10)
			sprintf_s(date, 20, "%d :%d:%d", datesarray[i].day, datesarray[i].month, datesarray[i].year);
		else if (datesarray[i].month < 10)
			sprintf_s(date, 20, "%d:%d :%d", datesarray[i].day, datesarray[i].month, datesarray[i].year);
		else
			sprintf_s(date, 20, "%d:%d:%d", datesarray[i].day, datesarray[i].month, datesarray[i].year);

		//char* temparray = calloc(20, sizeof(char));
		int datavalue = 0;
		for (int k = 0; k < datesamount; k++)
		{
			int l = 0;
			datavalue = StrToIntA(mydataset[k]->date + 7);//get the year
			if (datavalue == datesarray[i].year)
			{
				l++;//year 
				//now find month
				datavalue = StrToIntA(mydataset[k]->date + 4);
				if (datavalue == datesarray[i].month && l == 1)
				{//now find day
					l++;
					datavalue = StrToIntA(mydataset[k]->date + 1);
					if (datavalue == datesarray[i].day && l == 2)
					{
						_memccpy(readbuffer + strlen(readbuffer), mydataset[k]->date, 0, stringlength);
						_memccpy(readbuffer + strlen(readbuffer), mydataset[k]->data, 0, stringlength - strlen(readbuffer));
						k = datesamount;//date found break the loop
					}
				}
			}
		}
		memset(date, 0, 20);

	}
	free(date);
	free(datesarray);
	free(tempdate);
	for (int i = 0; i < datesamount; i++)
	{
		free(mydataset[i]->data);
		free(mydataset[i]);
	}
	free(mydataset);
	return readbuffer;
}

int NearestDate(int mflag, int filesize, char* readbuffer, int* appendindexlocation, char* dateset, int i)
{
	int yearchars = strlen(dateset + 7) - 1;//-1 removes the star at the end 
	int switchflag = 0;//when it switches means like 
	char tempstring[12] = { 0 };
	char tempstring2[12] = { 0 };
	if (mflag == 0)
	{
		//no date alike is present, input to the closest year range
		for (int k = 0; k < filesize; k++)
		{
			if (readbuffer[k] == specialchar[0])//load the year and check if its larger
			{
				long long inputyear = 0, presentyear = 0;
				for (int l = 7, m = 0; readbuffer[k + l] && l < 11; l++, m++)
				{
					tempstring[m] = readbuffer[k + l];
					tempstring2[m] = dateset[l];
				}
				StrToInt64ExA(tempstring, 0, &presentyear);
				StrToInt64ExA(tempstring2, 0, &inputyear);
				if (presentyear > inputyear)
				{
					if (switchflag == 1)
					{
						*appendindexlocation = k;
						k = filesize;
					}
					switchflag = 2;
				}
				else if(presentyear < inputyear)
				{
					if (switchflag == 2)
					{
						*appendindexlocation = k;
						k = filesize;
					}
					switchflag = 1;
				}
				else
					k += 12;//pass the date and skip the second star
			}
		}
	}
	if (mflag == yearchars)
	{
		for (int k = 0; k < filesize; k++)
		{
			if (readbuffer[k] == specialchar[0])//load the month and check if its larger
			{
				long long inputmonth = 0, presentmonth = 0;
				for (int l = 4, m = 0; readbuffer[k + l] && l < 6; l++,m++)
				{
					tempstring[m] = readbuffer[k + l];
					tempstring2[m] = dateset[l];
				}
				StrToInt64ExA(tempstring, 0, &presentmonth);
				StrToInt64ExA(tempstring2, 0, &inputmonth);
				if (presentmonth > inputmonth)
				{
					if (presentmonth > inputmonth)
					{
						if (switchflag == 1)
						{
							*appendindexlocation = k;
							k = filesize;
						}
						switchflag = 2;
					}
					else if (presentmonth < inputmonth)
					{
						if (switchflag == 2)
						{
							*appendindexlocation = k;
							k = filesize;
						}
						switchflag = 1;
					}
				}
				else
					k += 12;//pass the date and skip the second star
			}
		}
	}
	if (mflag == yearchars+2)
	{
		//only appropriate month is present, input to the closest day range
		for (int k = 0; k < filesize; k++)
		{
			if (readbuffer[k] == specialchar[0])//load the day and check if its larger
			{
				char tempstring[12] = { 0 };
				char tempstring2[12] = { 0 };
				long long inputday = 0, presentday = 0;
				for (int l = 1, p = 0; readbuffer[k + l] && l < 3; l++, p++)
				{
					tempstring[p] = readbuffer[k + l];
					tempstring2[p] = dateset[l];
				}
				StrToInt64ExA(tempstring, 0, &presentday);
				StrToInt64ExA(tempstring2, 0, &inputday);
				if (presentday > inputday)
				{
						if (presentday > inputday)
						{
							if (switchflag == 1)
							{
								*appendindexlocation = k;
								k = filesize;
							}
							switchflag = 2;
						}
						else if (presentday < inputday)
						{
							if (switchflag == 2)
							{
								*appendindexlocation = k;
								k = filesize;
							}
							switchflag = 1;
						}
				}
				else
					*appendindexlocation = k += 12;//pass the date and skip the second star
			}
		}
	}
	if (mflag == yearchars+4)//date found, point to it
	{
		*appendindexlocation = i;
		return readbuffer;
	}
	return *appendindexlocation;
}