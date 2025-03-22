#include "framework.h" //All header includes stuff is mashed up over theres

int ordereddatasave = 0;//when on will re-arrange existing data into ordered state.
char* macrolist[256] = {0};//stores macros of the macrobox
RECT buttonrect;//month button rect
RECT buttonrectd;//day buttons rect, declared multiple times, due to some unknown bug, you need to change all of them for it to work properly
int shoddyvar = 0;//used for buttonrectd.bottom declarations, due to the fact they are declared multiple times cause WIN32 is made by good guys
char* theworkingdirectory;//stores the working
char datasource[2000] = { 0 };//stores the data source location that is being read and appended to
HWND mainHwnd = { 0 };//the apps main hwnd is stored here
int yearzero = 0, yearrange = 0;//yearzero is the beggining year, yearrange is amount of years avalible following yearzero.
HWND TextBoxInput = { 0 };//Stored the textbox window
int startyear = 0, startmonth = 0, startday = 0;
char specialchar[2] = { 0 };
BOOL RTForTXT = 0;//data will be imported/exported in either .rtf or .txt file/format

//Sizes of the 4 main child windows, used by only this logical unit
RECT MonthRect = { 0 };
RECT DatesRect = { 0 };
BOOL GlobalDebugFlag = FALSE;//used when checking the debug box, to represent debug data
int TrustedIndex = 0;//mittigating "Speculative Execution Side Channel hardware vulnerabilities" for some i loops.
long long pchinputbuffermemory = 30 * 500;
BOOL TextBoxFlag = FALSE;//Signal, its only on and only when you click dates or month
BOOL TextBoxCalFlag = FALSE;//Signal, its only on and only on when you click Month
BOOL CreationFlag = FALSE; //False if childwindows have not been created, true if they where.
INT_PTR CALLBACK DlgMacroInfoProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
void SaveSettings();

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstances, _In_ LPWSTR pCmdLine, _In_ int nShowCmd)
{
	HWND hwnd = { 0 };
	MSG msg;

	theworkingdirectory = (char*)SafeCalloc(1000, sizeof(char));
	//assert(theworkingdirectory != NULL);
	int myoffset = (int)GetModuleFileNameA(NULL, (LPSTR)theworkingdirectory, 1000);
	if (myoffset == 0)
	{
		CrashDumpFunction(437, 0);
	}
	for (int f = myoffset; f > 0 && theworkingdirectory[f] != '\\' && f < 1000; f--)
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
	assert(hMenu != NULL);

	hwnd = CreateWindowEx(0, TEXT("Calendar"), TEXT("Calendar"), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, hMenu, hInstance, NULL);
	mainHwnd = hwnd;
	if (hwnd == NULL)
	{
		CrashDumpFunction(471, 0);
		return FALSE;
	}

	(void)ShowWindow(hwnd, nShowCmd);
	UpdateWindowSafe(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}//s

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int cxClient, cyClient, xMeasure, yMeasure;
	static HWND hWMonths = NULL, hWTextBox = NULL, hWMarks = NULL, hWDates = NULL;
	BOOL readflag = FALSE;
	static HINSTANCE hInstance;
	char* CalDatBuf = { 0 };
	HANDLE hFile = { 0 };
	if(GlobalDebugFlag == TRUE)
		ShowMessage(hwnd, cxClient, cyClient, message);
	if (CreationFlag == TRUE)
	{
		if (TextBoxCalFlag == TRUE)
		{
			EnableWindow(hWDates, 1);
			assert(hWDates != NULL);
		}
		if (TextBoxFlag == TRUE && TextBoxCalFlag == FALSE)//CREATION!!!!!!!
		{
			EnableWindow(TextBoxHwnd, 1);
			TextBoxFlag = FALSE;
		}
		if (TextBoxCalFlag == TRUE && TextBoxFlag == FALSE)
		{
			EnableWindow(TextBoxHwnd, 0);
			TextBoxCalFlag = FALSE;
		}
		assert(TextBoxHwnd != NULL);
	}
	switch (message)
	{

	case WM_CREATE:
	{
		if (NULL == LoadLibrary(TEXT("Riched20.dll")))
		{
			CrashDumpFunction(4123, 0);
		}
		hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		TextHeapRemaining = 15000;//Default 15000 bytes allocation, TextHeap will also account for y pointers.
		//load window colors
		char* tempstring = { 0 };
		tempstring = (char*)SafeCalloc(1000, sizeof(char));
		if (-1 == sprintf_s(tempstring, 1000, "%sCalendar.dat", theworkingdirectory))
		{
			CrashDumpFunction(50, 0);
		}
		hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD filesize = GetFileSize(hFile, NULL);
		if (filesize == INVALID_FILE_SIZE)
		{
			CrashDumpFunction(2, 0);
		}
		assert(hFile != NULL);
		if (filesize > 0)//read data file contents and update the global variables necessary
		{
			LoadConfigData(filesize, tempstring, readflag, hFile);
		}
		else//if dat file doesnt exist
		{
			SYSTEMTIME mytime = { 0 };
			GetSystemTime(&mytime);
			yearzero = mytime.wYear - 50;
			yearrange = 1200;//yearrange is expressed in months
			if (-1 == sprintf_s(datasource, 2000, "%sTextdata.txt", theworkingdirectory))//by default datasource is at the working directory
			{
				CrashDumpFunction(50, 0);
			}
			specialchar[0] = '*';
			memset(tempstring, 0, 1000);
			if (-1 == sprintf_s(tempstring, 1000, "%lu\n%lu\n%lu\n%lu\n%lu\n%lu\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d\n%d\n", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, (int)yearzero, (int)(yearzero + yearrange / 12), startday, startmonth, startyear, datasource, specialchar, ordereddatasave, RTForTXT))
			{
				CrashDumpFunction(50, 0);
			}
			SafewriteFile(hFile, tempstring, (DWORD)strlen(tempstring), &filesize, NULL);
		}
		free(tempstring);
		if (hFile != INVALID_HANDLE_VALUE)
			SafeCloseHandle(hFile);
		CalendarCreate(0, 0, 0);//creates the refference calender "LocalData.txt" for use by the program to generate proper calander in itself.
		return 0;
	}
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		xMeasure = cxClient / 10;
		yMeasure = cyClient / 10;
		MonthRect.left = 0;
		MonthRect.top = 0;
		MonthRect.right = 2 * xMeasure;
		MonthRect.bottom = cyClient;

		DatesRect.left = 2 * xMeasure;
		DatesRect.top = 0;
		DatesRect.right = cxClient - 2 * xMeasure;
		DatesRect.bottom = 8 * yMeasure;

		if(DatesHwnd != NULL)
		{
			(void)SendMessage(DatesHwnd, WM_SIZE, 0, 0);
		}

		return 0;
	case WM_MOVE:
		if (DatesHwnd != NULL)
		{
			(void)SendMessage(DatesHwnd, WM_SIZE, 0, 0);
		}
		return 0;
	case WM_PAINT:
	{
		if (CreationFlag == FALSE)
		{
			//CreationFlag = TRUE;
			if (ChildCreationFunction())
			{
				hWMonths = CreateWindowEx(0, TEXT("Months class"), TEXT("Months"), WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
				hWTextBox = CreateDialogW(GetModuleHandle(NULL), MAKEINTRESOURCE(TEXTBOXDIALOG), hwnd, TextBoxProc);
				hWDates = CreateWindowEx(0, TEXT("Dates Class"), TEXT("Dates"), WS_CHILD | WS_DISABLED | WS_CLIPCHILDREN, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
				buttonrectd.left = 0;//very dump shit dont ask removing it will break more shit
				buttonrectd.top = DatesRect.top;
				buttonrectd.right = DatesRect.right / 20;
				shoddyvar = buttonrectd.bottom = DatesRect.bottom / 12 + DatesRect.bottom / 200;//determines the leftmost marksbuttons and the dates buttons
				hWMarks = CreateWindowEx(0, TEXT("MarkClass"), TEXT("Marks"), WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
				assert(hWMarks != NULL && hWDates != NULL && hWTextBox != NULL && hWMonths != NULL);
				(void)ShowWindow(hWMonths, SW_SHOW);
				UpdateWindowSafe(hWMonths);
				(void)SendMessage(hWMonths, WM_COMMAND, 0, 0);//centers the calendar to present date
				(void)ShowWindow(hWTextBox, SW_SHOW);
				UpdateWindowSafe(hWTextBox);
				(void)ShowWindow(hWMarks, SW_SHOW);
				UpdateWindowSafe(hWMarks);
				(void)ShowWindow(hWDates, SW_SHOW);
				UpdateWindowSafe(hWDates);
				CreationFlag = TRUE;
				TextBoxInput = SafeGetDlgItem(hWTextBox, TEXTBOXINPUT);
				assert(TextBoxInput != NULL);
				Edit_LimitText(TextBoxInput, 15000);
				EnableWindow(TextBoxHwnd, 0);
				CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(FONTBOXDIALOG), hwnd, FontBoxProc);
				MoveWindowSafe(FontBoxHwnd, FontBoxRect.left, FontBoxRect.top, FontBoxRect.right - FontBoxRect.left, FontBoxRect.bottom - FontBoxRect.top, TRUE);
				UpdateWindowSafe(FontBoxHwnd);
				(void)ShowWindow(FontBoxHwnd, SW_SHOW);
			}
			else
				CrashDumpFunction(4225, 0);
		}
		if (CreationFlag == TRUE)
		{
			if (hWMonths != NULL)
			{
				MoveWindowSafe(hWMonths, 0, 0, 2 * xMeasure, cyClient, TRUE);
				UpdateWindowSafe(hWMonths);
			}
			if (hWTextBox != NULL)
			{
				MoveWindowSafe(hWTextBox, 2 * xMeasure, 7 * yMeasure, cxClient - 2 * xMeasure, cyClient - 7 * yMeasure, TRUE);
				UpdateWindowSafe(hWTextBox);
				RECT TextBoxRect = { 0 };
				SafetGetClientRect(hWTextBox, &TextBoxRect);
				MoveWindowSafe(TextBoxInput, 0, 0, TextBoxRect.right, TextBoxRect.bottom, TRUE);
			}
			if (hWMarks != NULL)
			{
				MoveWindowSafe(hWMarks, 8 * xMeasure - xMeasure / 2, 0, cxClient - 2 * xMeasure + xMeasure / 2, 7 * yMeasure, TRUE);
				UpdateWindowSafe(hWMarks);
			}
			if (hWDates != NULL)
			{
				MoveWindowSafe(hWDates, 2 * xMeasure, 0, 8 * xMeasure - xMeasure / 2 - 2 * xMeasure, 7 * yMeasure, TRUE);
				buttonrectd.left = 0;//very dump shit dont ask removing it will break more shit
				buttonrectd.top = DatesRect.top;
				buttonrectd.right = DatesRect.right / 20;
				buttonrectd.bottom = shoddyvar;
				UpdateWindowSafe(hWDates);
			}
		}
		return 0;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SETTINGS:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_SETTINGS), hwnd, DlgSettingsProc);
			return 0;
		case ID_HELP:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_INFOHELPBOX), hwnd, DlgHelpAndInfoProc);
			return 0;
		default:
			return 0;
		}


	case WM_DESTROY:
		SaveSettings();
		SafeDestroyWindow(hWTextBox);
		SafeDestroyWindow(hWMarks);
		SafeDestroyWindow(hWDates);
		SafeDestroyWindow(hWMonths);
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
	assert(MarkClass.hbrBackground != NULL);
	if (0 == RegisterClassEx(&MarkClass))
	{
		CrashDumpFunction(3, 1);
		return TRUE;
	}

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
	assert(DatesClass.hbrBackground != NULL);
	if (0 == RegisterClassEx(&DatesClass))
	{
		CrashDumpFunction(3, 1);
		return TRUE;
	}

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
	assert(MonthsClass.hbrBackground != NULL);
	if (0 == RegisterClassEx(&MonthsClass))
	{
		CrashDumpFunction(3, 1);
		return TRUE;
	}
	return TRUE;

}

BOOL ShowMessage(HWND hwnd, int XClient, int YClient, UINT message)//shows debug messages when debug mode is set.
{
	char szString[100] = "Hi!";
	HDC ihdc = safeGetDC(hwnd);
	assert(ihdc != NULL);
	int returnval = sprintf_s(szString, 100, "%u", message);
	if (returnval == -1)
	{
		CrashDumpFunction(50, 0);
	}
	if (GlobalDebugFlag == TRUE)
		SafeTextOutA(ihdc, XClient / 2, YClient / 2, szString, returnval);
	memset(szString, 0, 100);
	assert(pchinputbuffermemory >= 0);
	if(0 != _i64toa_s(pchinputbuffermemory, szString, 100,10))
	{
		CrashDumpFunction(4406, 0);
	}
	StrCatA(szString, ", pchinputbuffermemory");
	size_t length = 0;
	if(S_OK == StringCchLengthA(szString, 100, &length))
		SafeTextOutA(ihdc, XClient / 2, (YClient / 2)+16, szString, (int)length);
	memset(szString, 0, 100);
	assert(szString != NULL);
	if(0!=_i64toa_s((long long)TextHeapRemaining, szString, 100 ,10))
	{
		CrashDumpFunction(4416, 0);
	}
	StrCatA(szString, ", TextHeapRemaining");
	if(S_OK == StringCchLengthA(szString, 100, &length))
		SafeTextOutA(ihdc, XClient / 2, (YClient / 2) + 32, szString, (int)length);
	memset(szString, 0, 100);
	SafeReleaseDC(hwnd, ihdc);


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
		SafeDestroyWindow(hwnd);
		return TRUE;
	}
}

INT_PTR CALLBACK DlgSettingsProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static long long monthup = 0, monthdown = 0, yearup = 0, yeardown = 0, dayup = 0, daydown = 0;
	char* dummy2 = NULL;
	DWORD TempDword1 = 0;
	HDC hdc = { 0 };
	BOOL failcheck = TRUE;
	DWORD intermittentvar = 0;
	HANDLE hFile = { 0 };
	static HWND hwndDlgMain = { 0 };//When calling controls, you will have hwnddlgs being control handles, and for refference to send message to others controls, you can use this handle which will refference the dialog itself.
	if (lParam == 0)
		TempDword1 = 0;
	switch (message)
	{
	case WM_INITDIALOG:
	{
		if (RTForTXT == 0)
		{
			Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO2), 0);
			Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO3), 1);
		}
		if (RTForTXT == 1)
		{
			Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO2), 1);
			Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO3), 0);
		}
		Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO4), ordereddatasave);
		hdc = safeGetDC(hwndDlg);
		SafeReleaseDC(hwndDlg, hdc);
		if (GlobalDebugFlag == FALSE)
			CheckDlgButton(hwndDlg, IDDEBUG, BST_UNCHECKED);
		else
			CheckDlgButton(hwndDlg, IDDEBUG, BST_CHECKED);
		char dummy[100] = { 0 };
		if (-1 == sprintf_s(dummy, sizeof(char) * 100, "%lli", pchinputbuffermemory))
		{
			CrashDumpFunction(50, 0);
		}
		assert(pchinputbuffermemory > 0);
		if (FALSE == SetDlgItemTextA(hwndDlg, IDBUFEDIT, dummy))
		{
			CrashDumpFunction(4482, 0);
		}
		if (FALSE == SetDlgItemTextA(hwndDlg, EDITSCHAR, specialchar))
		{
			CrashDumpFunction(4486, 0);
		}
		hwndDlgMain = hwndDlg;
		return TRUE;
		// Place message cases here. 
	}
	case WM_ERASEBKGND:
	{
		return 0;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDDEBUG:
			if (GlobalDebugFlag == FALSE)
				GlobalDebugFlag = TRUE;
			else
				GlobalDebugFlag = FALSE;
			break;
		case IDBUFCHANGE:
			if (MessageBox(SafeGetParent(DatesHwnd), TEXT("Before doing this, ensure you backup your current calendar textdata, failure to do can result in irretrivable loss of data"), TEXT("PROCEED WITH CAUTION"), MB_OKCANCEL | MB_ICONWARNING) == IDOK)
			{//here you change the amount of memory in the heap.
				BOOL failornot = TRUE;
				int memoryused = Edit_GetTextLength(TextBoxInput);
				pchinputbuffermemory = SafeGetDlgItemInt(hwndDlgMain, IDBUFEDIT, &failornot, FALSE);//since HeapCreate allows for us to grow the heap, these logical checks like textheapremaining and pchinputbuffermemory are what actually limit the input and protect from any input memory overflows
				if (failornot == FALSE)
				{
					CrashDumpFunction(5, 0);
					return FALSE;
				}
				Edit_LimitText(TextBoxInput, pchinputbuffermemory);
				TextHeapRemaining = (size_t)(pchinputbuffermemory - memoryused);
				__int64 heapdifference = pchinputbuffermemory - memoryused;
				if (heapdifference < 0)
					CrashDumpFunction(10000, 1);//this means you allocated less memory then you are using.
				assert(buttonmarks[2] != NULL);
				InvalidateRectSafe(buttonmarks[2], NULL, TRUE);
				UpdateWindowSafe(buttonmarks[2]);
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
			SafeEndDialog(hwndDlg, 0);
			return TRUE;
		case IDWORDWRAP://sendsmessage to TEXTBOX to commit wordwrappign
			(void)SendMessage(TextBoxHwnd, WM_SETFOCUS, 0, 0);
			(void)SendMessage(TextBoxHwnd, WORDWRAP, 0, 0);//send message with flag that will send wordwrap function
			break;
		case IDDAYDOWN:
			daydown = SafeGetDlgItemInt(hwndDlgMain, IDDAYDOWN, &failcheck, FALSE);
			break;
		case IDDAYUP:
			dayup = SafeGetDlgItemInt(hwndDlgMain, IDDAYUP, &failcheck, FALSE);
			break;
		case IDMONTHUP:
			monthup = SafeGetDlgItemInt(hwndDlgMain, IDMONTHUP, &failcheck, FALSE);
			break;
		case IDMONTHDOWN:
			monthdown = SafeGetDlgItemInt(hwndDlgMain, IDMONTHDOWN, &failcheck, FALSE);
			break;
		case IDYEARUP:
			yearup = SafeGetDlgItemInt(hwndDlgMain, IDYEARUP, &failcheck, FALSE);
			break;
		case IDYEARDOWN:
			yeardown = SafeGetDlgItemInt(hwndDlgMain, IDYEARDOWN, &failcheck, FALSE);
			break;
		case IDDATAWIPE:
			dummy2 = (char*)SafeCalloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(16, 0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			(void)SendDlgItemMessageA(hwndDlg, IDDAYUP, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(6, 0);
				return FALSE;
			}
			SafeStrToInt64ExA(dummy2, STIF_DEFAULT, &dayup);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = (char*)SafeCalloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(7, 0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			(void)SendDlgItemMessageA(hwndDlg, IDDAYDOWN, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(8, 0);
				return FALSE;
			}
			SafeStrToInt64ExA(dummy2, STIF_DEFAULT, &daydown);
			assert(daydown >= 0);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = (char*)SafeCalloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
				return FALSE;
			dummy2[0] = '3';
			dummy2[1] = '0';
			(void)SendDlgItemMessageA(hwndDlg, IDMONTHUP, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(9, 0);
				return FALSE;
			}
			SafeStrToInt64ExA(dummy2, STIF_DEFAULT, &monthup);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = (char*)SafeCalloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(10, 0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			(void)SendDlgItemMessageA(hwndDlg, IDMONTHDOWN, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(11, 0);
				return FALSE;
			}
			SafeStrToInt64ExA(dummy2, STIF_DEFAULT, &monthdown);
			assert(monthdown >= 0);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = (char*)SafeCalloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(12, 0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			(void)SendDlgItemMessageA(hwndDlg, IDYEARUP, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(13, 0);
				return FALSE;
			}
			SafeStrToInt64ExA(dummy2, STIF_DEFAULT, &yearup);
			free(dummy2);
			dummy2 = NULL;
			dummy2 = (char*)SafeCalloc((size_t)(30), sizeof(char));
			if (dummy2 == NULL)
			{
				CrashDumpFunction(14, 0);
				return FALSE;
			}
			dummy2[0] = '3';
			dummy2[1] = '0';
			(void)SendDlgItemMessageA(hwndDlg, IDYEARDOWN, EM_GETLINE, 0, (LPARAM)dummy2);
			if (dummy2 == NULL)
			{
				CrashDumpFunction(15, 0);
				return FALSE;
			}
			assert(yeardown >= 0);
			SafeStrToInt64ExA(dummy2, STIF_DEFAULT, &yeardown);
			free(dummy2);
			dummy2 = NULL;
			if (FALSE == RangedDataWipe((int)monthup, (int)monthdown, (int)yearup, (int)yeardown, (int)dayup, (int)daydown))
			{
				CrashDumpFunction(5661, 0);
			}
			break;
		case IDCHANGEDATASOURCE:
		{
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
			if (FALSE != CommDlgExtendedError())
			{
				CrashDumpFunction(5694, 0);
			}
			SaveSettings();
			break;
		}
		case IDSPECIALCHAR:
		{
			dummy2 = (char*)SafeCalloc(2, sizeof(char));
			GetDlgItemTextA(hwndDlgMain, EDITSCHAR, dummy2, 2);
			if (0 != GetLastError())
			{
				CrashDumpFunction(5719, 0);
			}
			if (IDYES == MessageBoxA(hwndDlgMain, "Do you want to change all special characters within the data set to the new special character?", "Special Character change", MB_YESNO))
			{
				HANDLE tmpFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				assert(tmpFile != NULL);
				int fsize = (int)GetFileSize(tmpFile, NULL);
				if (fsize == INVALID_FILE_SIZE)
				{
					CrashDumpFunction(1813, 0);
				}
				char* readbuffer = (char*)SafeCalloc((size_t)fsize, sizeof(char));
				DWORD tempvar100 = 0;
				if (FALSE == ReadFile(tmpFile, readbuffer, (DWORD)fsize, &tempvar100, NULL))
					CrashDumpFunction(1814, 1);
				for (DWORD i = 0; i < tempvar100 && i < MAXINT; i++)
				{
					if (readbuffer[i] == specialchar[0])
					{
						readbuffer[i] = dummy2[0];
					}
				}
				OVERLAPPED tempov = { 0 };
				tempov.Offset = 0;
				SafewriteFile(tmpFile, readbuffer, (DWORD)strlen(readbuffer), &intermittentvar, &tempov);
				SafeCloseHandle(tmpFile);
			}
			specialchar[0] = dummy2[0];
			SaveSettings();
			free(dummy2);
			break;
		}
		default:
			break;
		}
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			switch (LOWORD(wParam))
			{
			case IDC_RADIO2://txt button
			{
				RTForTXT = 1;
				Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO3), 0);//rtf format
				Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO2), 1);//txt format
				SaveSettings();
				break;
			}
			case IDC_RADIO3://rtf button
			{
				RTForTXT = 0;
				Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO3), 1);
				Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO2), 0);
				SaveSettings();
				break;
			}
			case IDC_RADIO4://ordereddatasave
			{
				ordereddatasave = Button_GetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO4));
				int static flag27 = 1;
				if (ordereddatasave == TRUE)
				{
					Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO4), 0);
					ordereddatasave = 0;
				}
				else if (ordereddatasave == FALSE && flag27 == 1)
				{
					flag27 = 0;
					if (IDYES == MessageBoxA(NULL, "This will re-order current calendar data structure to oldest-to-earliest date", "Re-order data saving", MB_YESNO))
					{
						HANDLE FileHandle2 = { 0 };
						FileHandle2 = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						assert(FileHandle2 != NULL);
						char* myreadbuffer = { 0 };
						int stringlength19 = (int)GetFileSize(FileHandle2, NULL);
						if (stringlength19 == INVALID_FILE_SIZE)
						{
							CrashDumpFunction(1890, 0);
						}
						myreadbuffer = (char*)SafeCalloc((size_t)stringlength19 + 10, sizeof(char));
						DWORD DwordTemp3 = 0;
						DwordTemp3 = (DWORD)ReadFile(FileHandle2, myreadbuffer, (DWORD)stringlength19, &DwordTemp3, NULL);
						if (DwordTemp3 == 0)
						{
							CrashDumpFunction(1900, 0);
							free(myreadbuffer);
							return false;
						}
						myreadbuffer = DataSaveReordering(myreadbuffer);
						if (myreadbuffer == NULL)
							CrashDumpFunction(4855, 0);
						OVERLAPPED ofn = { 0 };
						ofn.Offset = 0;
						SafewriteFile(FileHandle2, myreadbuffer, (DWORD)stringlength19, &DwordTemp3, &ofn);
						Button_SetCheck(SafeGetDlgItem(hwndDlg, IDC_RADIO4), 1);
						ordereddatasave = 1;
					}
					flag27 = 1;
				}
				SaveSettings();
				break;
			}
			default: break;
			}
		default: break;
		}
		return TRUE;
	}
	default:
		return FALSE;
	}
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
	assert(hFile != NULL);
	amountread = GetFileSize(hFile, NULL);
	if (amountread == INVALID_FILE_SIZE)
	{
		CrashDumpFunction(1938, 0);
	}
	char* readbuffer = (char*)SafeCalloc(  (DWORD)amountread,    sizeof(char));
	if (FALSE == ReadFile(hFile, readbuffer, (DWORD)amountread, NULL, &overlapstruct))
	{
		int errorvalue = (int)GetLastError();
		if ((amountread != 0) && (errorvalue != 38))//if amountread is 0, that means file is empty, hence why error was thrown.
		{
			SafeCloseHandle(hFile);
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
	assert(specialchar != NULL);
	//format startdate for beggininglocaton calc
	HRESULT vresult = StringCbCatExA((LPSTR)startdate, 24, "*00:00:    *", NULL, NULL, 0);
	if(FAILED(vresult))
	{
		CrashDumpFunction(4950, 0);
	}
	safe_itoa_s(dayup, startdate + 1, 3, 10);
	if (dayup < 10)
		startdate[2] = ' ';
	safe_itoa_s(monthup, startdate + 4, 3, 10);
	if (monthup < 10)
		startdate[5] = ' ';
	safe_itoa_s(yearup, startdate + 7, 5, 10);
	startdate[3] = ':';
	startdate[6] = ':';
	startdate[11] = specialchar[0];
	//format enddate for endlocation calc
	vresult = StringCbCatExA((LPSTR)enddate, 24, "*00:00:    *", NULL, NULL, 0);
	if (FAILED(vresult))
	{
		CrashDumpFunction(4966, 0);
	}
	safe_itoa_s(daydown, enddate + 1, 3, 10);
	if (daydown < 10)
		enddate[2] = ' ';
	safe_itoa_s(monthdown, enddate + 4, 3, 10);
	if (monthdown < 10)
		enddate[5] = ' ';
	safe_itoa_s(yeardown, enddate + 7, 5, 10);
	enddate[3] = ':';
	enddate[6] = ':';
	enddate[11] = specialchar[0];
	if (strcmp(startdate, enddate) == 0)
	{//in case we are deleting just one date, run findthedate, and put endlocation at the end of text of the same date.
		Findthedate((int)oldstringlength, readbuffer, startdate, &beginninglocation, 0);
		endlocation = beginninglocation + 12;
		for (; readbuffer[endlocation] != specialchar[0] && endlocation != '\0' && (unsigned)endlocation < oldstringlength && endlocation<MAXINT; endlocation++)
			;//this will ensure that endlocation will include the data for the sameself date
	}
	else
	{
		//calculate beginninglocation
		findthenearestdate((int)oldstringlength, readbuffer, startdate, &beginninglocation, 0);
		//calculate the endlocation
		findthenearestdate((int)oldstringlength, readbuffer, enddate, &endlocation, 0);
		endlocation += 12;//so we could delete enddate too! WHAT IS E NDDATE NIGGER?`
	}
	//wipe all data from begininglocation to endlocation
	for (int i = endlocation, l = beginninglocation; (unsigned)i < oldstringlength && l < endlocation && i < MAXINT && l < MAXINT; i++, l++)
	{
		readbuffer[l] = readbuffer[i];
	}
	overlapstruct.Offset = (DWORD)beginninglocation;
	if ((unsigned)endlocation == oldstringlength)//simply shrink the file to beginning location
	{
		SafeSetFilePointer(hFile, beginninglocation, NULL, FILE_BEGIN);
		SafeSetEndOfFile(hFile);
	}
	else 
	{
		int faggot = oldstringlength-beginninglocation;
		DWORD byteswritten = 0;
		SafewriteFile(hFile, readbuffer + beginninglocation, faggot, &byteswritten, &overlapstruct);
		int filendmove = (int)((DWORD)oldstringlength - (endlocation-beginninglocation));//endlocation-beggininglocation = sumdata being removed, is removed against oldstringlength, setting new filesize
		SafeSetFilePointer(hFile, filendmove, NULL, FILE_BEGIN);
		SafeSetEndOfFile(hFile);
	}

	SafeCloseHandle(hFile);
	free(readbuffer);
	return TRUE;
}

//used by textbox.c and mycalendar.c to properly figure out the range of dataset present. 
//USE THIS ONLY WHEN ORDEREDDATASAVE=1
void findthenearestdate(int filesize, char * readbuffer, char * dateset, int * appendindexlocation, int flag)
{
	int mflag = 0, i = 0;

	//scan for the year dataset range
	int yearchars = (int)strlen(dateset + 7)-1;//-1 removes the star at the end
	int yeardate = StrToIntA(dateset + 7);
	int monthdate = StrToIntA(dateset + 4);
	int daydate = StrToIntA(dateset + 1);
	for (int k = 0; k < filesize && k < MAXINT; k++)//find year
	{
		if (readbuffer[k] == specialchar[0])//found a date
		{
			i = k;
			char tempyearstring[20] = { 0 };
			int yeartemp = StrToIntA(readbuffer + k + 7);
			safe_itoa_s(yeartemp, tempyearstring, 20, 10);
			int yeartemplength = (int)strlen(tempyearstring);
			int monthtemp = StrToIntA(readbuffer + k + 4);
			int daytemp = StrToIntA(readbuffer + k + 1);
			for (int l = 7; yeartemplength == yearchars && readbuffer[k + l] == dateset[l] && dateset[l] != specialchar[0] && l < MAXINT; l++)
				mflag++;
			assert(yeartemp >= 0 && monthtemp >= 0 && daytemp >= 0);
			if (mflag == yearchars)
			{
				i = k;//make the global pointer point to 
				k = filesize;//quit the year loop, as year has been found
			}
			else//no result move to next dateset
			{
				for (int j = k + 1; j < filesize && j < MAXINT; j++)
				{
					if ((readbuffer[j] == specialchar[0]))//this one is faster in case of datasaving is ordered
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
		}
	}

	//scan for the month data set range in the given year data set range
	if (mflag == yearchars)
	{
		for (int k = i; k < filesize && k < MAXINT; k++)
		{
			i = k;
			int yeartemp = StrToIntA(readbuffer + k + 7);
			int monthtemp = StrToIntA(readbuffer + k + 4);
			int daytemp = StrToIntA(readbuffer + k + 1);
			assert(yeartemp >= 0 && monthtemp >= 0 && daytemp >= 0);
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
				{
					mflag = yearchars;
					for (int j = k + 1; j < filesize && j < MAXINT; j++)
					{
						if ((readbuffer[j] == specialchar[0]))//load when it hits the 2nd star of the dateset to evaluate if its the first bigger
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
		}
	}

	//scan for the dates present
	if (mflag == 2 + yearchars)
	{
		for (int k = i; k < filesize && k <MAXINT; k++)
		{
			if (readbuffer[k] == specialchar[0])
			{
				i = k;
				int yeartemp = StrToIntA(readbuffer + k + 7);
				int monthtemp = StrToIntA(readbuffer + k + 4);
				int daytemp = StrToIntA(readbuffer + k + 1);
				assert(yeartemp >= 0 && monthtemp >= 0 && daytemp >= 0);
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
				{
					mflag = yearchars + 2;
					for (int j = k + 1; j < filesize && j < MAXINT; j++)
					{
						if ((readbuffer[j] == specialchar[0]))
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
		}
	}
	//input the date at the right place, in between the nearest larger and nearest smaller date.	
	if(flag == 0)
		DateInput(mflag, filesize, readbuffer, appendindexlocation, dateset, i);//for ranged data wipe
	else
	{
		NearestDate(mflag, filesize, readbuffer, appendindexlocation, dateset, i);//for ordered data input
		//return readbuffer + *appendindexlocation;
	}
}

void Findthedate(int filesize, char* readbuffer, char* dateset, int* appendindexlocation, int flag)
{
	int i = 0;
	//compare dates
	for (int k = 0; k < filesize && k < MAXINT; k++)//find a date
	{
		if (readbuffer[k] == specialchar[0])//found a date
		{
			i = k;
			char tempdatestring[20] = { 0 };
			memcpy_s(tempdatestring, 20, readbuffer + k, 12);
			if (0 == strcmp(tempdatestring, dateset))
			{
				*appendindexlocation = k;
				return;
			}

		}
	}
	return;
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
	assert(lt.wYear >= 0 && lt.wMonth >= 0 && lt.wDay >= 0 && lt.wHour >= 0 && lt.wMinute >= 0 && lt.wSecond >= 0 && lt.wMilliseconds >= 0);

	char tempstring[1000] = { 0 };
	if (CreationFlag == FALSE)
	{
		if(fatality == 1)
		{
			sprintf_s(tempstring, 1000, "Fatal Crash During Initialization, error code: %d", return_value);
		}
		else
		{
			sprintf_s(tempstring, 1000, "Crash During Initialization, error code: %d", return_value);
		}
		FatalAppExitA(0, tempstring);

	}
	if(-1==sprintf_s(tempstring, 1000, "%scrashdump.dmp", theworkingdirectory))
	{
		return;
	}
	hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileSize(hFile, &filelength);
	if (filelength == INVALID_FILE_SIZE)
	{
		return;
	}
	Overlapped.Offset = filelength;

	memset(tempstring, 0, sizeof(char) * 1000);
	if (-1 == sprintf_s(tempstring, 1000, "Error code: %i", return_value))
	{
		return;
	}
	if(CreationFlag == TRUE)
		switch (return_value)
		{
		case 10000:
			MessageBoxA(mainHwnd, "To little memory allocated, buffer to large." ,tempstring, MB_OK | MB_ICONWARNING);
			break;
		default:
			MessageBoxA(mainHwnd, tempstring, "Fatal Error!", MB_OK | MB_ICONWARNING);
		}
	memset(tempstring, 0, sizeof(char) * 1000);
	stringlength = wsprintfA(crashtext, "%u:%u:%u:%u:%u:%u:%u === Error Code: %i\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds, return_value);
	if (stringlength < 0)
		WndProc(mainHwnd, WM_DESTROY, 0, 0);
	WriteFile(hFile, crashtext, (DWORD)stringlength, NULL, &Overlapped);
	assert(mainHwnd != NULL);
	if (fatality == 1)
		(void)SendMessage(mainHwnd, WM_CLOSE, 0, 0);

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
			SafeEndDialog(hwndDlg, 0);
			return TRUE;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
}
void DateInput(int mflag, int filesize, char * readbuffer, int * appendindexlocation, char * dateset, int i)
{
	int yearchars = (int)strlen(dateset + 7) - 1;//-1 removes the star at the end 
	assert(yearchars>=0);
	assert(readbuffer != NULL);
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
		for (int k = 0; k < filesize && k < MAXINT; k++)
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
				SafeStrToInt64ExA(tempstring, 0, &presentday);
				SafeStrToInt64ExA(tempstring2, 0, &inputday);
				if (presentday > inputday)
				{
					//we found the first date larger then, write the date.
					char* pholder = NULL;
					pholder = (char*)realloc(readbuffer, sizeof(char) * ((size_t)filesize + 13));//make space for date input
					if (pholder == NULL)
						return;//memory failure
					readbuffer = pholder;
					for (int l = filesize; l < filesize + 13 && l >= k; l--)//move all character forwards by 12
						readbuffer[l + 12] = readbuffer[l];
					for (int l = 0; l < 12; l++)
					{
						readbuffer[l + k] = dateset[l];
					}
					*appendindexlocation = k;
					return;
				}
				else
					k += 12;//pass the date and skip the second star
			}
		}
	}
	if (mflag == yearchars+4)//date found, point to it
	{
		*appendindexlocation = i;
		return;
	}
	return;
}

void yearange(char * readbuffer, int filesize, char * dateset, int * appendindexlocation)
{
	char tempstring[12] = { 0 };
	char tempstring2[12] = { 0 };
	assert(filesize > 0);
	assert(readbuffer != NULL);
	for (int k = 0; k < filesize && k < MAXINT; k++)
	{
		if (readbuffer[k] == specialchar[0])//load the year and check if its larger
		{
			long long inputyear = 0, presentyear = 0;
			for (int l = 7; readbuffer[k + l] && l < 11; l++)
			{
				tempstring[l] = readbuffer[k + l];
				tempstring2[l] = dateset[l];
			}
			SafeStrToInt64ExA(tempstring, 0, &presentyear);
			SafeStrToInt64ExA(tempstring2, 0, &inputyear);
			if (presentyear > inputyear)
			{
				//we found the first date larger then, write the date.
				char* pholder = NULL;
				pholder = (char*)realloc(readbuffer, sizeof(char) * ((size_t)filesize + 13));//make space for date input
				if (pholder == NULL)
					return;//memory failure
				readbuffer = pholder;
				for (int l = filesize; l < filesize + 13 && l >= k; l--)//move all character forwards by 12
					readbuffer[l] = readbuffer[l + 12];
				for (int l = 0; l < 13; l++)
				{
					readbuffer[l + k] = dateset[l];
				}
				*appendindexlocation = k;
				return;
			}
			else
				k += 12;//pass the date and skip the second star
		}
	}
	return;
}
void monthrange(int filesize, char * readbuffer, char * dateset, int * appendindexlocation)
{
	char tempstring[12] = { 0 };
	char tempstring2[12] = { 0 };
	assert(readbuffer != NULL);
	assert(filesize > 0);
	for (int k = 0; k < filesize && k < MAXINT; k++)
	{
		if (readbuffer[k] == specialchar[0])//load the month and check if its larger
		{
			long long inputmonth = 0, presentmonth = 0;
			for (int l = 4; readbuffer[k + l] && l < 6; l++)
			{
				tempstring[l] = readbuffer[k + l];
				tempstring2[l] = dateset[l];
			}
			SafeStrToInt64ExA(tempstring, 0, &presentmonth);
			SafeStrToInt64ExA(tempstring2, 0, &inputmonth);
			if (presentmonth > inputmonth)
			{
				//we found the first date larger then, write the date.
				char* pholder = NULL;
				pholder = (char*)realloc(readbuffer, sizeof(char) * ((size_t)filesize + 13));//make space for date input
				if (pholder == NULL)
					return;//memory failure
				readbuffer = pholder;
				for (int l = filesize; l < filesize + 13 && l >= k; l--)//move all character forwards by 12
					readbuffer[l] = readbuffer[l + 12];
				for (int l = 0; l < 13; l++)
				{
					readbuffer[l + k] = dateset[l];
				}
				*appendindexlocation = k;
				return ;
			}
			else
				k += 12;//pass the date and skip the second star
		}
	}
	return;
}

//creates the LocalData.txt used as refference for calendar emulation within the program.
BOOL CalendarCreate(BOOL RealorCustom, int localstartyear, int newyearrange)
{
	int thirtyone[] = { 1,3,5,7,8,10,12 };
	int thirty[] = { 4,6,9,11 };
	int leap[] = { 2 };
	OVERLAPPED dummyoverlap = { 0 };
	HANDLE datafile = NULL;
	SYSTEMTIME temptime = { 0 };
	char* wchararray = (char*)SafeCalloc(  150,    sizeof(char));
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
	if(-1==sprintf_s(tempstring, 1000, "%sLocaldata.txt", theworkingdirectory))
	{
		CrashDumpFunction(50, 0);
	}
	datafile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(datafile != INVALID_HANDLE_VALUE);
	int dateprint = 1, monthtype = 0, datemonth = 0, dateyear = 0;
	SafeSetEndOfFile(datafile);
	GetSystemTime(&temptime);
	if (RealorCustom == 0)//always call this if ur during WM_CREATE of the programs main winproc
	{
		dateyear = yearzero;
	}
	else//create a localdata.txt based on new year range, call this when changing localdate from button!
	{
		assert(newyearrange > 0);
		yearrange = newyearrange;
		yearzero = dateyear = localstartyear;
		SafeSetFilePointer(datafile, 0, NULL, FILE_BEGIN);//empty the file
		SafeSetEndOfFile(datafile);
	}
	for (int z = 0; z < yearrange && z<MAXINT16; z++)
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
				CreationBuffer = (LPSTR)"RGB: TBD, SHAPE: TBD";
				StringCbCatExA(&wchararray[5 * 24], 24, CreationBuffer, NULL, NULL, 0);
			}
			else
				wchararray = whcararryinputroutine(wchararray, &dateprint, monthtype, y);
		}
		wchararray[149] = '\r';
		SafewriteFile(datafile, wchararray, 150, NULL, &dummyoverlap);//wtf shit about asynchronous I/O no idea why botherin me with that stuf??
		for (int i = 0; i < 149; i++)//empty the buffer/s
			wchararray[i] = ' ';
	}
	SafeCloseHandle(datafile);
	return 1;
}

//takes hold of inputting and sorting data into the refference calendar buffer, before writing it into the datafile
char * whcararryinputroutine(char * wchararray, int * dateprint, int monthtype, int y)
{
	assert(wchararray != NULL);
	assert(dateprint != NULL);
	for (int i = 0; i < 19; i += 3)//+3 cause two from decimal value(even non decimals have it reserved for simplicity sake, and 1 for empty space.
	{
		if (y == 1)
		{
			safe_itoa_s((*dateprint)++, wchararray + (i + y * 24), 3, 10);//puts digit in
			wchararray[(i + y * 24) + 2] = ' ';
			wchararray[(i + y * 24) + 1] = ' ';
		}
		else if (y < 5)
		{
			safe_itoa_s(((7 * y) - 7) + (*dateprint)++, wchararray + (i + y * 24), 3, 10);//puts digit in
			wchararray[(i + y * 24) + 2] = ' ';
			if (((7 * y) + (*dateprint) - 7) < 11)
				wchararray[(i + y * 24) + 1] = ' ';
		}
		else if (y == 5)
		{
			BOOL shitflag = FALSE;
			safe_itoa_s((7 * y) - 7 + (*dateprint)++, wchararray + (i + y * 24), 3, 10);//puts digit i;
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
	assert(wchararray != NULL);
	assert(monthtype != NULL);
	safe_itoa_s(dateyear, wchararray, 5, 10);
	wchararray[4] = ',';
	wchararray[5] = ' ';
	safe_itoa_s(datemonth, wchararray + 6, 3, 10);
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
HWND GlocalHwnd = 0;
static long long monthf = 1, monthl = 1, yearf = 2024, yearl = 2025, dayf = 1, dayl = 1, dayspassed = 0, monthspassed = 0, yearspassed = 0, newlinespassed = 0, charspassed = 0;;//global variables for selection of scripted dates for sake of ease with dealing with them. l stands for last f for first
INT_PTR CALLBACK DlgScriptedInput(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int appendlocationindex = 0, amountread = 0;
	char * ScriptString = { 0 };
	BOOL datepresent = FALSE;
	int originaldateselected = selecteddate;
	OVERLAPPED overlapstruct = { 0 };
	int monthsinaday[13] = {0, 31,28,31,30,31,30,31,31, 30,31,30,31 };
	switch (message)
	{
	case WM_INITDIALOG:
		(void)SendDlgItemMessageA(hwndDlg, TXTSCRIPTINPUT, EM_SETLIMITTEXT, (WPARAM)pchinputbuffermemory/2, 0);//limits the editbox input to 1/2 of textbox input buffer
		CreateDialogW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDSCRIPTMACRO), hwndDlg, DlgScriptMacros);
		assert(GlocalHwnd != 0);
		SafeDestroyWindow(GlocalHwnd);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			SafeEndDialog(hwndDlg, 0);
			return TRUE;
		case IDOK:
			
			ScriptString = (char*)SafeCalloc(  (size_t)(pchinputbuffermemory / 2),    sizeof(char));
			if (ScriptString == NULL)
			{
				CrashDumpFunction(1466,0);
				SafeEndDialog(hwndDlg, 0);
				return TRUE;
			}
			GetDlgItemTextA(hwndDlg, TXTSCRIPTINPUT, ScriptString, (int)pchinputbuffermemory / 2);
			if(GetLastError() != 0)
			{
				CrashDumpFunction(4549, 0);
				SafeEndDialog(hwndDlg, 0);
				return TRUE;
			}
			//below code pastes the ScriptString to all days within range of selection.
			for(int i = (int)yearf; i<=yearl && i < MAXINT16; i++)
			{
				//leap year mechanism
				monthsinaday[2] = 28;
				assert(yearf>=0);
				if (yearf % 4 == 0)
				{
					if (yearf % 100)
					{
						if (yearf % 400)
							monthsinaday[2] = 29;
					}
					else
					{
						monthsinaday[2] = 29;
					}
				}
				//end of leapyear mechanism
				int l = 1;
				if (i == yearf)
					l = (int)monthf;
				else if (i == yearl)
					l = (int)monthl;
				selecteddate = selecteddate & 0x7ff;//zerout the year
				selecteddate += i << 11;//put the year inside selecteddate;
				assert(selecteddate >= 0);
				int delimit = 12;
				if (yearl == i)//makes sure that month wont go pass the last year month limit
					delimit = monthl;
				for (; l <= delimit; l++)
				{
					int k = 1;
					if (i == yearf && l == monthf)
						k = (int)dayf;//beginning with the starting date at the starting month
					else if (i == yearl && l == monthl)
						monthsinaday[l] = (int)dayl;
					if(yearl == yearf && monthl == monthf)
						monthsinaday[l] = (int)dayl;//if the months and years are the same, then the max days is basically dayl
					selecteddate = selecteddate & 0xfff87f;//zeroout the month
					selecteddate += l << 7;//implant the month
					for(; k <= monthsinaday[l] && k<MAXINT16 ; k++)
					{
						selecteddate = selecteddate & 0xFFFF80;//zeroout the day
						selecteddate += k;//the day is the lowest bit henceforth we can add it like this
						//scriptformatfunction
						ScriptFormat(ScriptString);
						DateTestBufferLoad(&amountread, &overlapstruct, &appendlocationindex, &datepresent);
						char* readbuffer = (char*)SafeCalloc((size_t)(((size_t)amountread)+strlen(ScriptString)+(size_t)10),    sizeof(char));
						//concatenate the datetext and scriptstring
						if(datepresent == 1)
						{
							OVERLAPPED puskurac = { 0 };
							puskurac.Offset = appendlocationindex+overlapstruct.Offset;//the beggining of datemark + datemarklength is the beggining of text
							HANDLE hFile = { 0 };
							hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
							if (hFile == INVALID_HANDLE_VALUE)
							{
								CrashDumpFunction(41608, 0);
							}
							if (FALSE == ReadFile(hFile, readbuffer, (DWORD)amountread, NULL, &puskurac))//load the datetext
							{
								CrashDumpFunction(11602, 0);
							}
							SafeCloseHandle(hFile);
						}
						_memccpy(readbuffer + amountread, ScriptString, 0, (size_t)(pchinputbuffermemory / 2));
						if(FALSE == savingFunction(&appendlocationindex, readbuffer, &overlapstruct, &amountread, datepresent))
						{
							CrashDumpFunction(41619, 0);
						}
						charspassed += strlen(ScriptString);
						dayspassed++;
						for (int mz = 0; mz <= charspassed; mz++)
						{
							if (ScriptString[mz] == '\n')
								newlinespassed++;
						}
						datepresent = appendlocationindex = 0;
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
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDSCRIPTDATES), hwndDlg, DlgScriptDates);
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
	HANDLE hFile = { 0 };
	char* tempstring = { 0 }, * readbuffer = { 0 };
	int fileszie = 0;
	DWORD readnumber = 0;
	int static selection = 0;
	switch (message)
	{
	case WM_INITDIALOG:
	{
		textstring2 = SafeGetDlgItem(hwndDlg, TXTSCRIPTINPUT2);
		if (FALSE == SetWindowSubclass(textstring2, (SUBCLASSPROC)MarkBoxInputSrc, 0, 0))
		{
			CrashDumpFunction(41672, 0);
		}
		if (FALSE == SetWindowSubclass(SafeGetDlgItem(hwndDlg, IDC_LIST1), (SUBCLASSPROC)ListBoxSrc, 0, 0))
		{
			CrashDumpFunction(41676, 0);
		}
		for (int i = 0; i < 256; i++)
			macrolist[i] = (char*)SafeCalloc(101, sizeof(char));
		GlocalHwnd = ParenthwndDlg = hwndDlg;
		(void)SendDlgItemMessageA(hwndDlg, TXTSCRIPTINPUT2, EM_SETLIMITTEXT, (WPARAM)(pchinputbuffermemory / 10), 0);//limits the editbox input to 1/10 of textbox input buffer
		(void)SendDlgItemMessageA(hwndDlg, TXTSCRIPTINPUT3, EM_SETLIMITTEXT, 1, 0);//limits the editbox input to 1 bytes.
		(void)SendDlgItemMessageA(hwndDlg, IDC_LIST1, LB_SETCOLUMNWIDTH, 40, 0);//limits the editbox input to 1 bytes.
		tempstring = (char*)SafeCalloc(1000, sizeof(char));
		if (-1 == sprintf_s(tempstring, 1000, "%sScriptMacros.txt", theworkingdirectory))
		{
			CrashDumpFunction(50, 0);
		}
		hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		assert(hFile != INVALID_HANDLE_VALUE);
		free(tempstring);
		fileszie = (int)GetFileSize(hFile, NULL);
		if (fileszie == INVALID_FILE_SIZE)
		{
			CrashDumpFunction(11676, 0);
		}
		readbuffer = (char*)SafeCalloc((size_t)fileszie, sizeof(char));
		readnumber = 0;
		int listindex = 0;
		if (FALSE == ReadFile(hFile, readbuffer, (DWORD)fileszie, &readnumber, NULL))
		{
			CrashDumpFunction(11676, 0);
		}
		for (int i = 1, l = 3; i < 256 && fileszie>4 && l < fileszie; i++)//max 255 items on the list
		{
			//grab listindex
			listindex = (int)readbuffer[l - 2];//converts char into number used for listindex
			//then grab the text
			for (int m = l; macrolist[listindex] != NULL && readbuffer[l] != specialchar[0] && l < fileszie && l < MAXINT; l++)
			{
				macrolist[listindex][l - m] = readbuffer[l];
				if (l + 1 == fileszie)
				{
					fileszie = 4;//break the loop
				}
			}
			l += 4;
		}
		listbox = SafeGetDlgItem(ParenthwndDlg, IDC_LIST1);
		assert(listbox != NULL);
		for (int i = 0, b = 0; macrolist[i] != NULL && i < 256; i++)
		{//check if index is valid
			if (macrolist[i][0] != '\0')
			{//if its valid send the appropriate message to signal it to set the data associated with it
				char signalstring[3] = { 0 };
				signalstring[0] = '%';
				signalstring[1] = (char)i;
				//now set its text that will be shown in the list
				(void)SendMessageA(listbox, LB_ADDSTRING, (WPARAM)b, (LPARAM)signalstring);
				b++;
			}
		}//with this you inititalize the scriptstringsmacros file of ours
		SafeSetFocus(listbox);
		SafeCloseHandle(hFile);
		free(readbuffer);
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDINFOSCRIPT:
			MessageBoxA(hwndDlg, "Macro values would presented in Info button that would show a dialog box with text listing all the macros we would list here.\nEach pre-defined macro would be followed by a !, typing two !! in a row would allow for typing ""!"" itself\n- !Date, Date of input\n- !Month, Month of Input\n- !Year, Year of Input\n- !TotalTextAmount, Sum of the data within the data source\n- !TotalLines, Sum of lines of the data within the data source\n- !TotalCharacters, Sum of characters of data within the data source\n- !TotalDateTextAmount, Sum of data within a Date\n- !TotalDateLines, Sum of lines within a Date\n- !TotalDateCharacters, Sum of characters within a Date\n- !TotalMemoryRemaining, memory remaining in bytes \n- !PassedDates, amount of dates passed within the runtime of the script\n- !PassedMonths, amount of months passed within the runtime of the script \n- !PassedYears, amount of years passed within the runtime of the script\n- !EmptyFlag, Yes if there is no data in date, No otherwise\n- !EmptyDatesWithinAMonth, How many dates within the month are empty\n- !EmptyDatesWithinAYear, How many dates within the year are empty\n- !TotalDateSelectedTextAmount, Amount of total data within the text selected for the script input\n- !TotalDateSelectedLines, Amount of total lines within the text selected for script input\n- !TotalDateSelectedCharacters, Amount of total characters within the text selected for script input\n- !PassedChars, Amount of chars typed in the runtime of the script\n- !PassedLines, Amount of lines typed in the runtime of the script\n- !TotalPassedTextData, Total amount of data inputted within the runtime of the script\n- !sqrt, Sqaure root of a number: !sqrt!Number10\n- !Exponent, Exponent of a number: !Exponent!Number10\n- !Multiplication, Multiplication of a number: !Multiplication!Number100!Number100\n- !Plus, Add a number to a number: !Plus!Number10!Number5\n- !Minus, Minus a number by a number: !Minus!Number10!Number5\n- !Number, Declares that following string will be numbers and to be treated as numbers by the program. It will consider number any decimal integer, once it finds a non decimal integer it will stop the number classification. Thus !Number100 is translated as number 100 within the program, !Numbered will be translated as invalid number, d is not a number. !Number52 100, will translate 52 as a number.\n- !FloatNumber, Will translate number as a float ie if you type !Number100.200 will only give you number 100, !FloatNumber100.200 however will give you 100.200 as a number.\n- !Logarithm, !Logarithm!Number10!Number4, First number is the log value and second number log input.\n- !Division, !Division!Number8!Number2 will give you 4\n- !Sum, !Sum!Number4!Number-4!Number2 will give you 2, it allows for summing line full of !Number\n- !StringApply, Applies a String, used as return point of a string\n- !StringFalse, Will not apply a string\n- !If, an if statement, !If!Number2!Equivalent!MacroZ!EndIf, if if statement is true it will continue down the script, and not otherwise.\n- !Equivalent, Serves as a comparison statement within the if statement \n- !EndIf, end of if statement, terminates the !If\n- !(, bracket open, used to manipulate order of priority\n- !), bracket close, used to manipulate order of priority", "Info", 1);
			return TRUE;
		case IDCANCEL:
			SafeEndDialog(hwndDlg, 0);
			return TRUE;
		case IDOK:
			return TRUE;
		case IDREMOVESCRIPT:
			return TRUE;
		case IDMACROSET://click on button "macroset", sets a macro symbol and adds it to macrolist or modifies the preexisting macro
			//now append this data to our macrostrings file
		{
			selection = 0;
			LPWSTR bullshitcancer = (LPWSTR)SafeCalloc(20, sizeof(char));
			Edit_GetText(SafeGetDlgItem(ParenthwndDlg, TXTSCRIPTINPUT3), bullshitcancer, 2);
			selection = bullshitcancer[0];
			if (selection == 0)
			{
				free(bullshitcancer);
				return FALSE;
			}
			tempstring = (char*)SafeCalloc(1000, sizeof(char));
			char macrosymboldata[4] = { 0 };
			if (-1 == sprintf_s(tempstring, 1000, "%sScriptMacros.txt", theworkingdirectory))
			{
				CrashDumpFunction(50, 0);
			}
			hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			assert(hFile != INVALID_HANDLE_VALUE);
			fileszie = (int)GetFileSize(hFile, NULL);
			if (fileszie == INVALID_FILE_SIZE)
			{
				CrashDumpFunction(11753, 0);
			}
			assert(fileszie >= 0);
			macrosymboldata[0] = '%';
			macrosymboldata[1] = (char)selection;
			macrosymboldata[2] = specialchar[0];
			char* readbuffer1 = (char*)SafeCalloc((size_t)fileszie + 10, sizeof(char));
			OVERLAPPED overlapstruct = { 0 };
			if (FALSE == ReadFile(hFile, readbuffer1, (DWORD)fileszie, NULL, &overlapstruct))
			{
				CrashDumpFunction(11753, 0);
			}
			unsigned char intextflag = FALSE;
			BOOL seekfront = FALSE;
			BOOL symbolexists = FALSE;
			int firstoffset = 0, frontoffset = 0;
			for (int i = 0; i < fileszie && i < MAXINT; i++)
			{
				if (readbuffer1[i] == specialchar[0])
				{
					if (seekfront == TRUE && intextflag == FALSE)
					{
						frontoffset = i - 2;//position of macro in front of the selected macro
						i = fileszie;//terminate the loop
					}
					if (intextflag == FALSE && i - 1 > 0 && readbuffer1[i - 1] == selection)
					{
						firstoffset = i - 2;//position of the beggining of macro ie its % symbol
						seekfront = TRUE;//datapos
						symbolexists = TRUE;
					}
					intextflag = (unsigned char)~intextflag;
				}
				else if (i != fileszie)
					frontoffset = i;
			}
			if (symbolexists == FALSE)
				firstoffset = fileszie;
			overlapstruct.Offset = (DWORD)firstoffset;
			int length = GetWindowTextLengthA(textstring2) + 1;
			if (GetLastError() != 0)
			{
				CrashDumpFunction(41813, 0);
			}
			char* newstring = (char*)SafeCalloc((size_t)length + 2, sizeof(char));
			GetWindowTextA((textstring2), (newstring), (length));
			if (GetLastError() != 0)
			{
				CrashDumpFunction(41819, 0);
			}
			char* memoryvariable = (char*)SafeCalloc((size_t)length + fileszie + 10, sizeof(char));
			memset(memoryvariable, 0, (size_t)length + fileszie + 10);
			_memccpy(memoryvariable, macrosymboldata, 0, 3);
			_memccpy(memoryvariable + 3, newstring, 0, (size_t)length);
			memoryvariable[3 + length - 1] = specialchar[0];//close the dataset with the star
			_memccpy(memoryvariable + 3 + length, readbuffer1 + frontoffset + 2, 0, (size_t)fileszie - frontoffset);
			int datachange = (frontoffset - firstoffset) - (length + 1);
			int amounttowrite = fileszie - firstoffset - datachange;
			SafewriteFile(hFile, memoryvariable, (DWORD)amounttowrite, NULL, &overlapstruct);
			if (datachange > 0)
			{//shrinkfile
				SafeSetFilePointer(hFile, -datachange, 0, FILE_END);
				SafeSetEndOfFile(hFile);
			}
			//update the macrolist
			if (macrolist[selection][0] == 0)//macro not present, add it
			{
				WCHAR* charsstring = (WCHAR*)SafeCalloc(5, sizeof(char));
				charsstring[0] = '%';
				charsstring[1] = (WCHAR)selection;
				(void)SendMessageA(listbox, LB_ADDSTRING, 0, (LPARAM)charsstring);
				free(charsstring);
			}
			memset(macrolist[selection], 0, 101);//empty the macro
			_memccpy(macrolist[selection], newstring, 0, (size_t)length + 2);
			assert(hFile != INVALID_HANDLE_VALUE);
			SafeCloseHandle(hFile);
			free(tempstring);
			free(bullshitcancer);
			free(newstring);
			return TRUE;
		}
		case TXTSCRIPTINPUT2://DEFINITION MACROBOX
			return TRUE;
		case TXTSCRIPTINPUT3://ONE-LETTER MACROBOX
			if (HIWORD(wParam) == EN_UPDATE)
			{
				LPWSTR Remaints = (LPWSTR)SafeCalloc(  100,    sizeof(char));
				Edit_GetText(SafeGetDlgItem(ParenthwndDlg, TXTSCRIPTINPUT3), Remaints, 2);
				selection = (int)Remaints[0];
				free(Remaints);
				SafeSetWindowTextA(textstring2, macrolist[selection]);
			}
			return TRUE;
		case TXTSCRIPTSHOW:
			return TRUE;
		case IDC_LIST1:
			switch(HIWORD(wParam))
			{
			case LBN_DBLCLK:
				return TRUE;
			case LBN_KILLFOCUS:
				return TRUE;
			case LBN_SELCANCEL:
				return TRUE;
			case LBN_SELCHANGE:
			{
				selection = (int)SendDlgItemMessageA(ParenthwndDlg, IDC_LIST1, LB_GETCARETINDEX, 0, 0);
				char* indexname = (char*)SafeCalloc(4, sizeof(char));
				(void)SendDlgItemMessageA(ParenthwndDlg, IDC_LIST1, LB_GETTEXT, (WPARAM)selection, (LPARAM)indexname);
				selection = (int)indexname[1];
				SafeSetWindowTextA(textstring2, macrolist[selection]);
				SafeSetWindowTextA(SafeGetDlgItem(hwndDlg, TXTSCRIPTINPUT3), indexname + 1);
				free(indexname);
				return TRUE;
			}
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
		SafeSetDlgItemInt(hwndDlgMain, IDMONTHF, (unsigned int)monthf, FALSE);
		SafeSetDlgItemInt(hwndDlgMain, IDMONTHL, (unsigned int)monthl, FALSE);
		SafeSetDlgItemInt(hwndDlgMain, IDYEARF, (unsigned int)yearf, FALSE);
		SafeSetDlgItemInt(hwndDlgMain, IDYEARL, (unsigned int)yearl, FALSE);
		SafeSetDlgItemInt(hwndDlgMain, IDDAYF, (unsigned int)dayf, FALSE);
		SafeSetDlgItemInt(hwndDlgMain, IDDAYL, (unsigned int)dayl, FALSE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			SafeEndDialog(hwndDlg, 0);
			return TRUE;
		case IDOK:
			monthf = SafeGetDlgItemInt(hwndDlgMain, IDMONTHF, &failcheck, FALSE);
			monthl = SafeGetDlgItemInt(hwndDlgMain, IDMONTHL, &failcheck, FALSE);
			yearf = SafeGetDlgItemInt(hwndDlgMain, IDYEARF, &failcheck, FALSE);
			yearl = SafeGetDlgItemInt(hwndDlgMain, IDYEARL, &failcheck, FALSE);
			dayf = SafeGetDlgItemInt(hwndDlgMain, IDDAYF, &failcheck, FALSE);
			dayl = SafeGetDlgItemInt(hwndDlgMain, IDDAYL, &failcheck, FALSE);
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
			assert(dayl >= 0 || yearl >= 0 || monthl >= 0 || dayf>=0 || yearf>= 0|| monthf>= 0);

			SafeEndDialog(hwndDlg, 0);
			return TRUE;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return FALSE;
}
//Extracts the macros
char* ScriptFormat(char* Inputbuffer)
{
	assert(Inputbuffer != NULL);
	int stringlength = 0;
	stringlength = (int)strlen(Inputbuffer);
	for (int i = 0; i < stringlength && i<MAXINT16; i++)
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
					_memccpy(Inputbuffer + i, Inputbuffer + i +2, 0, (size_t)stringlength - 2);//removes '% and following character"
					int mstringlength = (int)strlen(macrolist[macro]);
					char* macroformatted = (char*)SafeCalloc(  (size_t)mstringlength+4,    sizeof(char));
					_memccpy(macroformatted, macrolist[macro], 0, (size_t)mstringlength);
					macroformatted = MacroFormating(macroformatted, TRUE);
					if(macroformatted == NULL)
					{
						CrashDumpFunction(41971, 0);
						return Inputbuffer;
					}
					mstringlength = (int)strlen(macroformatted);
					if (i + mstringlength >= 1000)//larger then scriptstirngbuffer
					{
						CrashDumpFunction(8989, 0);
						free(macroformatted);
						return Inputbuffer;
					}
					_memccpy(Inputbuffer + i + mstringlength, Inputbuffer + 2, 0, (size_t)stringlength - i);//moves the string forward to make space for macrostringformatted, +2 excludes the % and symbol that follows it
					_memccpy(Inputbuffer + i, macroformatted, 0, (size_t)mstringlength);//pastes macro into the string
					free(macroformatted);
					stringlength = stringlength + mstringlength;
					assert(stringlength >= 0);
					memset(Inputbuffer + stringlength, 0, 8);//set delimiting nulls
					charspassed += strlen(Inputbuffer);
				}
			}
		}
	}
	return Inputbuffer;
}

//expands the macro to its desired form
char* MacroFormating(char* macroformated, BOOL firstrun)
{
	int length = 0, templength = 0, oldselecteddate = 0, daysempty = 0;
	int lastbyte = 0;
	assert(macroformated != NULL);
	length = (int)strlen(macroformated);
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
	int numberindex = 0, day = (selecteddate & 0x7F), month = (selecteddate & 0x780) >> 7, year = selecteddate >> 11, amountread = 0, appendindexlocation = 0, ifnumber = 1, ifsign = 0, macrolength = 0, difference = 0;
	int dummyint = 0;
	char* temppointer6 = 0;
	BOOL myswitch = FALSE;
	double dummyintfloat = 0, ifstatement1 = 0, ifstatement2 = 0;
	int static symbolindex = 0;
	static int stateflag[100] = { 0 };//used for arithemtic flags and if statements, capable of nesting them.
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
	OVERLAPPED overlapstruct = { 0 };
	BOOL datepresent = { 0 }, ifflag = FALSE, foundstring = FALSE;
	char* tempvariable = { 0 };
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(hFile != INVALID_HANDLE_VALUE);
	firstrun = FALSE;
	for (int i = 0; i < length && i < MAXINT16 && macroformated != NULL && macroformated+i != NULL; i++)
	{
		foundstring = FALSE;
#pragma warning(push)
#pragma warning(disable:6385)
		if (macroformated[i] == '!')
		{
#pragma warning(pop)
			for(int n = 0; n < 38 && foundstring == FALSE; n++)
				if (0 == comparestrings(macroformated+(i+1), scripts[n]))
				{
					foundstring = TRUE;
					macrolength = (int)strlen(scripts[n]+1);//length of the macro including ! sign 
					switch (n)
					{
					case 0://!Date ie. day
						tempvariable = (char*)SafeCalloc(  10,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 10,"%i", day))
						{
							CrashDumpFunction(50, 0);
						}
						assert(day >= 0);
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)abs(difference));//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 1://month
						tempvariable = (char*)SafeCalloc(  10,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 10,"%i", month))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						assert(macrolength >= 0);
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)abs(difference));//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 2://year
						tempvariable = (char*)SafeCalloc(  10,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 10,"%i", year))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							assert(templength>=0);
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)abs(difference));//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 3://!TotalTextAmount, Sum of the data within the data source
						dummyint = (int)GetFileSize(hFile, NULL);
						if (dummyint == INVALID_FILE_SIZE)
						{
							CrashDumpFunction(12194, 0);
						}
						assert(dummyint >= 0);
						tempvariable = (char*)SafeCalloc(  20,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 10,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 4://!TotalLines, Sum of lines of the data within the data source
						dummyint = (int)GetFileSize(hFile, NULL);
						if (dummyint == INVALID_FILE_SIZE)
						{
							CrashDumpFunction(12194, 0);
						}
						assert(dummyint >= 0);
						tempvariable = (char*)SafeCalloc( (size_t)dummyint, sizeof(char));
						if (FALSE == ReadFile(hFile, tempvariable, (DWORD)dummyint, (LPDWORD)&dummyint, NULL))
						{
							CrashDumpFunction(12194, 0);
							return NULL;
						}
						for (i = 0; i < dummyint && i < MAXINT16; i++)
						{
							if (tempvariable[i] == '\n')
								numberindex++;
						}
						memset(tempvariable, 0, 50);
						if(-1==sprintf_s(tempvariable, 50,"%i", numberindex))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 5://!TotalCharacters, Sum of characters of data within the data source
						dummyint = (int)GetFileSize(hFile, NULL);
						if (dummyint == INVALID_FILE_SIZE)
						{
							CrashDumpFunction(12194, 0);
						}
						assert(dummyint >= 0);
						tempvariable = (char*)SafeCalloc(  100,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 100,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 6://!TotalDateTextAmount, Sum of data within a Date
						dummyint = (int)GetFileSize(hFile, NULL);
						if (dummyint == INVALID_FILE_SIZE)
						{
							CrashDumpFunction(12194, 0);
						}
						assert(dummyint >= 0);
						tempvariable = (char*)SafeCalloc(  100,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 100,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)- difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 7://!TotalDateLines, Sum of lines within a Date
						DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
						assert(hFile == NULL);
						tempvariable = (char*)SafeCalloc(  (size_t)amountread+10,    sizeof(char));
						overlapstruct.Offset = (DWORD)appendindexlocation;
						if (FALSE == ReadFile(hFile, tempvariable, (DWORD)amountread, NULL, &overlapstruct))
						{
							CrashDumpFunction(12284, 0);
							return NULL;
						}
						for (i = 0; i < amountread && i <MAXINT; i++)
						{
							if (tempvariable[i] == '\n')
								dummyint++;
						}
						memset(tempvariable, 0, (size_t)amountread);
						if(-1==sprintf_s(tempvariable, (size_t)amountread+10,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 8://!TotalDateCharacters, Sum of characters within a Date
						DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
						tempvariable = (char*)SafeCalloc(  50,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 50,"%i", amountread))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						assert(templength >= 0);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 9://!TotalMemoryRemaining, memory remaining in bytes 
						dummyint = GetWindowTextLengthA(TextBoxHwnd);
						dummyint = (int)SendMessageA(TextBoxHwnd, EM_GETLIMITTEXT, 0, 0) - dummyint;
						assert(dummyint >= 0);
						tempvariable = (char*)SafeCalloc(  50,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 50,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 10://!PassedDates, amount of dates passed within the runtime of the script
						dummyint = (int)dayspassed;
						assert(dummyint >= 0);
						tempvariable = (char*)SafeCalloc(  50,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 50,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 11://!PassedMonths, amount of months passed within the runtime of the script 
						dummyint = (int)monthspassed;
						assert(dummyint >= 0);
						tempvariable = (char*)SafeCalloc(  50,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 50,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 12://!PassedYears, amount of years passed within the runtime of the script
						dummyint = (int)yearspassed;
						assert(dummyint >= 0);
						tempvariable = (char*)SafeCalloc(  50,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 50,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 13://!EmptyFlag, Yes if there is no data in date, No otherwise
						DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
						tempvariable = (char*)SafeCalloc(  4,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 4,"%i", datepresent))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 14://!EmptyDatesWithinAMonth, How many dates within the month are empty
						oldselecteddate = selecteddate;
						assert(oldselecteddate >= 0);
						//leap year mechanism
						monthsinaday[2] = 28;
						if (year % 4 == 0)
						{
							if (year % 100)
							{
								if (year % 400)
									monthsinaday[2] = 29;
							}
							else
								monthsinaday[2] = 29;
						}
						//end of leapyear mechanism
						daysempty = 0;
						for (i = 1; i <= monthsinaday[month] && i < MAXINT; i++)
						{
							selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
							selecteddate += i + 1;//set the day bits
							DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
							if (datepresent == FALSE)
								daysempty++;
						}
						tempvariable = (char*)SafeCalloc(  10,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 10,"%i", daysempty))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 15://!EmptyDatesWithinAYear, How many dates within the year are empty
						oldselecteddate = selecteddate;
						assert(oldselecteddate >= 0);
						//leap year mechanism
						monthsinaday[2] = 28;
						if (year % 4 == 0)
						{
							if (year % 100)
							{
								if (year % 400)
									monthsinaday[2] = 29;
							}
							else
								monthsinaday[2] = 29;
						}
						//end of leapyear mechanism
						daysempty = 0;
						for(int k = 1; k<=12;k++)
						{
							selecteddate = selecteddate & 0xfff87f;//zeroout the month
							selecteddate += k << 7;//implant the month
							for (i = 1; i <= monthsinaday[k] && i < 13; i++)
							{
								selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
								selecteddate += i + 1;//set the day bits
								DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
								if (datepresent == FALSE)
									daysempty++;
							}
						}
						tempvariable = (char*)SafeCalloc(  10,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 10,"%i", daysempty))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 16://!TotalDateSelectedTextAmount, Amount of total data within the text selected for the script input
						dummyint = 0;
						oldselecteddate = selecteddate;
						daysempty = 0;
						for (int l = (int)yearf; l <= yearl && l < MAXINT16; l++)
						{
							selecteddate = selecteddate & 0x7ff;//zerout the year
							selecteddate += i << 11;//put the year inside selecteddate;
							//leap year mechanism
							monthsinaday[2] = 28;
							if (l % 4 == 0)
							{
								if (l % 100)
								{
									if (l % 400)
										monthsinaday[2] = 29;
								}
								else
									monthsinaday[2] = 29;
							}
							//end of leapyear mechanism
							for (int k = (int)monthf; k <= monthl && k < MAXINT16; k++)
							{
								selecteddate = selecteddate & 0xfff87f;//zeroout the month
								selecteddate += k << 7;//implant the month
								for (i = 1; i <= monthsinaday[k] && k<1000; i++)
								{
									selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
									selecteddate += i + 1;//set the day bits
									DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
									dummyint += amountread;
								}
							}
						}
						tempvariable = (char*)SafeCalloc(  10,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 10,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 17://!TotalDateSelectedLines, Amount of total lines within the text selected for script input
						dummyint = 0;
						oldselecteddate = selecteddate;
						daysempty = 0;
						assert(oldselecteddate >= 0);
						for (int l = (int)yearf; l <= yearl && l < MAXINT; l++)
						{
							selecteddate = selecteddate & 0x7ff;//zerout the year
							selecteddate += i << 11;//put the year inside selecteddate;
							//leap year mechanism
							monthsinaday[2] = 28;
							if (l % 4 == 0)
							{
								if (l % 100)
								{
									if (l % 400)
										monthsinaday[2] = 29;
								}
								else
									monthsinaday[2] = 29;
							}
							//end of leapyear mechanism
							for (int k = (int)monthf; k <= monthl && k<MAXINT16; k++)
							{
								selecteddate = selecteddate & 0xfff87f;//zeroout the month
								selecteddate += k << 7;//implant the month
								for (i = 1; i <= monthsinaday[k] && i < MAXINT16; i++)
								{
									selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
									selecteddate += i + 1;//set the day bits
									DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
									int sizeoffile = (int)GetFileSize(hFile, NULL);
									if (sizeoffile == INVALID_FILE_SIZE)
									{
										CrashDumpFunction(2683, 0);
									}
									char* tempbuffer = (char*)SafeCalloc((size_t)sizeoffile,    sizeof(char));
									for (int z = (int)overlapstruct.Offset; z < amountread && z<MAXINT; z++)
									{
										if (tempbuffer[z] == '\n')
											dummyint++;
									}
								}
							}
						}
						tempvariable = (char*)SafeCalloc(  10,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 10,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 18://!TotalDateSelectedCharacters, Amount of total characters within the text selected for script input, tbd with compatibility update for wide characters, right now same as the data 
						dummyint = 0;
						oldselecteddate = selecteddate;
						assert(oldselecteddate >= 0);
						daysempty = 0;
						for (int l = (int)yearf; l <= yearl && l < MAXINT16; l++)
						{
							selecteddate = selecteddate & 0x7ff;//zerout the year
							selecteddate += i << 11;//put the year inside selecteddate;
							//leap year mechanism
							monthsinaday[2] = 28;
							if (l % 4 == 0)
							{
								if (l % 100)
								{
									if (l % 400)
										monthsinaday[2] = 29;
								}
								else
									monthsinaday[2] = 29;
							}
							//end of leapyear mechanism
							for (int k = (int)monthf; k <= monthl && k < MAXINT16; k++)
							{
								selecteddate = selecteddate & 0xfff87f;//zeroout the month
								selecteddate += k << 7;//implant the month
								for (i = 1; i <= monthsinaday[k] && i < MAXINT16; i++)
								{
									selecteddate = selecteddate & 0xFFFF80;//zeroout the day;//wipe the day
									selecteddate += i + 1;//set the day bits
									DateTestBufferLoad(&amountread, &overlapstruct, &appendindexlocation, &datepresent);
									dummyint += amountread;
								}
							}
						}
						tempvariable = (char*)SafeCalloc(  10,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 10,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						selecteddate = oldselecteddate;//reset the selecteddate back
						break;
					case 19://!PassedChars, Amount of chars typed in the runtime of the script
						dummyint = 0;
						tempvariable = (char*)SafeCalloc(  30,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 28,"%lli", charspassed))
						{
							CrashDumpFunction(50, 0);
						}
						assert(charspassed >= 0);
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 20://!PassedLines, Amount of lines typed in the runtime of the script
						dummyint = 0;
						tempvariable = (char*)SafeCalloc(  30,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 30,"%lli", newlinespassed))
						{
							CrashDumpFunction(50, 0);
						}
						assert(newlinespassed >= 0);
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
						}
						free(tempvariable);
						break;
					case 21://!TotalPassedTextData, Total amount of data inputted within the runtime of the script, same as passed chars, to be updated when we make compatibility with widechars
						dummyint = 0;
						tempvariable = (char*)SafeCalloc(  30,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 30,"%lli", charspassed))
						{
							CrashDumpFunction(50, 0);
						}
						assert(charspassed >= 0);
						templength = (int)strlen(tempvariable);
						difference = templength - macrolength;
						if (difference > 0)//add additional memory and push forward
						{
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference < 0)//zerout the exccess memory and shrink data
						{
							_memccpy(macroformated + (i + templength), macroformated + i, 0, (size_t)templength);
							memset(macroformated + length + difference, 0, (size_t)-difference);//zerout excess bytes
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
						for (int p = 0; p < symbolindex && p < 10000; p++)
						{
							dummyint = NumbersFunction(stateflag, symbolindex, macroformated, i, FALSE, &lastbyte);
							//uses nesting system ie. !sqrt!sum!Number50!Number80!Plus!Logarithm!Number20!Number50, you nest these to do multiple operations in an order of desire, once !Number appears, the nest stops and operation happen
						}
						assert(macroformated != NULL);
						tempvariable = (char*)SafeCalloc(  30,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 30,"%i", dummyint))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						//replace the ! macro members with results, move index i to the first position of the result
						for (int l = symbolindex-1; l >= 0 && l > -10000; l--)
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
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							memset(macroformated + length + difference, 0, (size_t)-difference);//push data forwards to make space
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if (difference > 0)
						{
							//shrinkg the string now
							_memccpy(macroformated + i + templength, macroformated + lastbyte, 0, (size_t)length - lastbyte);
							//zerout the excess bytes if necessary
							memset(macroformated + difference + i, 0, (size_t)length - difference);//zerout excess bytes
						}
						free(tempvariable);
						if (ifflag == TRUE)
						{
							IFComparison(macroformated, dummyint, ifstatement2, &ifnumber, &ifstatement1, &i, &myswitch, length, ifsign, &n);
						}
						break;
					case 28://!FloatNumber, Will translate number as a float ie if you type !Number100.200 will only give you number 100, !FloatNumber100.200 however will give you 100.200 as a number.
						for (int p = 0; p <= symbolindex && p < 10000; p++)
						{
							dummyintfloat = NumbersFunction(stateflag, symbolindex, macroformated, i, TRUE, &lastbyte);
							//uses nesting system ie. !sqrt!sum!Number50!Number80!Plus!Logarithm!Number20!Number50, you nest these to do multiple operations in an order of desire, once !Number appears, the nest stops and operation happen
						}
						tempvariable = (char*)SafeCalloc(  30,    sizeof(char));
						if(-1==sprintf_s(tempvariable, 30,"%lf", dummyintfloat))
						{
							CrashDumpFunction(50, 0);
						}
						templength = (int)strlen(tempvariable);
						for (int l = symbolindex-1; l >= 0 && l > -10000; l--)
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
							temppointer6 = (char*)realloc(macroformated, sizeof(char) * difference + 1);
							if (temppointer6 == NULL)
							{
								CrashDumpFunction(12093, 1);
								return NULL;
							}
							macroformated = temppointer6;
							temppointer6 = NULL;
							memset(macroformated + length + difference, 0, (size_t)-difference);//push data forwards to make space
						}
						_memccpy(macroformated + i, tempvariable, 0, (size_t)templength);
						if(difference>0)
						{
							//shrinkg the string now
							_memccpy(macroformated + i + templength, macroformated + lastbyte, 0, (size_t)length - lastbyte);
							//zerout the excess bytes if necessary
							memset(macroformated + difference + i, 0, (size_t)length - difference);//zerout excess bytes
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
								assert(macroformated != NULL);
								//below is skipping of endif in case if returned false
								if (ifresult == 0)//skip everything till next endif
								{
									for (; i < length && myswitch == FALSE; i++)
									{
										if (macroformated[i] == '!')
										{
											{
												for (; n < 38 && myswitch == FALSE; n++)
													if (0 == comparestrings(macroformated+(i + 1), (char*)"!EndIf"))
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
										if (0 == comparestrings(macroformated + (i + 1), (char*)"!StringApply"))
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
			length = (int)strlen(macroformated);
		}
	}
	SafeCloseHandle(hFile);
	return macroformated;
}

INT_PTR CALLBACK MarkBoxInputSrc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CHAR:
		if (wParam == (unsigned char)specialchar[0])
		{
			return FALSE;//dont type in start
		}
	}
	return DefSubclassProc(hwnd, message, wParam, lParam);
}

INT_PTR CALLBACK ListBoxSrc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	assert(hwnd != NULL);
	switch (message)
	{
	case LB_ADDSTRING:
		InvalidateRectSafe(hwnd, NULL, TRUE);
		UpdateWindowSafe(hwnd);
		return DefSubclassProc(hwnd, message, wParam, lParam);
	case LB_SETITEMDATA:
		return DefSubclassProc(hwnd, message, wParam, lParam);
	case LBN_DBLCLK:
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
	assert(macroformated != NULL);
	int number = 0, index = 0, length = 0, number2 = 0;
	long double numberf1 = 0, numberf2 = 0;
	number = StrToIntA(macroformated+(numbersbegginingindex + 7));
		for (int i = maxsymbols-1; i >= 0 && i > -10000; i--)
		{
			if(FloatFlag == FALSE)
			{
				switch (symbolsarray[i])
				{
				case 1://exponent
					number = (int)exp(number);
					break;
				case 0://sqrt
					number = (int)sqrt(number);
					break;
				case 5://Logarithm, number2 determines base, number determines the argument
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
					{
						number2 = StrToIntA(macroformated+(index + 7));
						number = (int)(log10(number) / log10(number2));
						numbersbegginingindex += index + 7;
					}
					else
					{
						number2 = -1;
						return number;
					}
					break;
				case 6://Division, number is dividend, number2 divisor
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
					{
						number2 = StrToIntA(macroformated + (index + 7));
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
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
					{
						number2 = StrToIntA(macroformated + (index + 7));
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
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + index, (char*)"!Number")))
					{
						number2 = StrToIntA(macroformated + (index + 7));
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
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
					{
						number2 = StrToIntA(macroformated + (index + 7));
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
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16; index++)
					{
						if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
						{
							number2 = StrToIntA(macroformated + (index + 7));
							number = number * number2;
							numbersbegginingindex += index + 7;
						}
						else if (isdigit(macroformated[index]) == FALSE)
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
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
					{
						numberf1 = strtod(macroformated + (index + 7), NULL);
						if (errno != 0)
						{
							CrashDumpFunction(43453, 1);
						}
						numberf1 = log10l(numberf1) / log10l(numberf2);
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return (int)numberf1;
					}
					break;
				case 6://Division, number is dividend, number2 divisor
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
					{
						numberf2 = strtod(macroformated + (index + 7), NULL);
						if (errno != 0)
						{
							CrashDumpFunction(43453, 1);
						}
						numberf1 = numberf1 / numberf2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return (int)numberf1;
					}
					break;
				case 2://Multiplication
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
					{
						numberf2 = strtod(macroformated+(index + 7), NULL);
						if (errno != 0)
						{
							CrashDumpFunction(43453, 1);
						}
						numberf1 = numberf1 * numberf2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return (int)numberf1;
					}
					break;
				case 3://Plus
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
					{
						numberf2 = strtod(macroformated + (index + 7), NULL);
						if (errno != 0)
						{
							CrashDumpFunction(43453, 1);
						}
						numberf1 = numberf1 + numberf2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return (int)numberf1;
					}
					break;
				case 4://Minus
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16 && isdigit(macroformated[index]); index++)
						;
					if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
					{
						numberf2 = strtod(macroformated + (index + 7), NULL);
						if (errno != 0)
						{
							CrashDumpFunction(43453, 1);
						}
						numberf1 = numberf1 - numberf2;
						numbersbegginingindex += index + 7;
					}
					else
					{
						numberf2 = -1;
						return (int)numberf1;
					}
					break;
				case 7://Sum
					length = (int)strlen(macroformated);
					for (index = numbersbegginingindex + 7; index <= length && index < MAXINT16; index++)
					{
						if ((0 == comparestrings(macroformated + (index), (char*)"!Number")))
						{
							numberf2 = strtod(macroformated + (index + 7), NULL);
							if (errno != 0)
							{
								CrashDumpFunction(43453, 1);
							}
							numberf1 = numberf1 * numberf2;
							numbersbegginingindex += index + 7;
						}
						else if (isdigit(macroformated[numbersbegginingindex + index]) == FALSE)
						{
							numberf2 = -1;
							return (int)numberf1;
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
			for (kl = 0; isdigit(macroformated[7 + index + kl]) && kl < MAXINT16; kl++)
				;
			*lastbyteindex = index + 7+kl;
			return number;
		}
		else
		{
			for (kl = 0; kl < MAXINT16 && isdigit(macroformated[7 + index + kl]); kl++)
				;
			*lastbyteindex = index + 7+kl;
			return (int)numberf1;
		}
}

;//beggining to the end linear simple string comparing
BOOL comparestrings(char* string1, char* string2)
{
	assert(string1 != NULL && string2 != NULL);
	int string1length = (int)strlen(string1);
	int string2length = (int)strlen(string2);
	for (int i = 0; i < MAXINT16 && i < string1length && i < string2length; i++)
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
	static HWND oghwnd = { 0 };
	static int tripval = 0;
	static DWORD filesize = 0;
	static HANDLE hFile = { 0 };
	static char* tempstring = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		oghwnd = hwndDlg;
		colorbutton = SafeGetDlgItem(hwndDlg, IDC_BUTTON8);
		break;
	case WM_DRAWITEM:
		if (wParam == IDC_BUTTON8)
		{
			DRAWITEMSTRUCT * drawstruck = { 0 };
			drawstruck = (DRAWITEMSTRUCT*)lParam;
			HBRUSH tempbrush = { 0 };
			if (button8color != NULL)
				tempbrush = SafetCreateSolidBrush(button8color[0]);
			if (tempbrush == 0)
			{
				//CrashDumpFunction(13531, 0);
				return FALSE;
			}
			assert(tempbrush != INVALID_HANDLE_VALUE);
			SafeFillRect(drawstruck->hDC, &drawstruck->rcItem, tempbrush);
			SafeDeleteObject(tempbrush);
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
				choosecolor.lpCustColors = customcolors;
				if (FALSE == ChooseColorA(&choosecolor))
				{
					CrashDumpFunction(43643,0);
				}
				HWND updatelist[10] = { 0 };
				updatelist[1] = SafeGetParent(buttonarray[0]);;
				updatelist[3] = DatesHwnd;
				updatelist[4] = SafeGetParent(buttonarray[0]);
				updatelist[5] = DatesHwnd;
				updatelist[6] = TextBoxHwnd;
				updatelist[7] = SafeGetParent(buttonmarks[0]);
				*button8color = choosecolor.rgbResult;
				if (tripval == 6)
				{
					(void)SendMessage(TextBoxHwnd, TEXTBOXCOLORCHANGE, 0, 0);
				}
				assert(updatelist != NULL);
				InvalidateRectSafe(updatelist[tripval], NULL, TRUE);
				UpdateWindowSafe(updatelist[tripval]);
				SaveSettings();
			}
			break;
		case IDC_BUTTON1://month button colors
			button8color = &monthsbuttoncolor;
			InvalidateRectSafe(colorbutton, NULL, FALSE);
			UpdateWindowSafe(colorbutton);
			tripval = 1;
			break;
		case IDC_BUTTON3://day button color
			button8color = &datesbuttoncolor;
			InvalidateRectSafe(colorbutton, NULL, TRUE);
			UpdateWindowSafe(colorbutton);
			tripval = 3;
			break;
		case IDC_BUTTON4://months background colors
			button8color = &monthsbackground;
			InvalidateRectSafe(colorbutton, NULL, TRUE);
			UpdateWindowSafe(colorbutton);
			tripval = 4;
			break;
		case IDC_BUTTON5://days background colors
			button8color = &datesbackground;
			InvalidateRectSafe(colorbutton, NULL, TRUE);
			UpdateWindowSafe(colorbutton);
			tripval = 5;
			break;
		case IDC_BUTTON6://textbox background color
			button8color = &textbackground;
			InvalidateRectSafe(colorbutton, NULL, TRUE);
			UpdateWindowSafe(colorbutton);
			tripval = 6;
			break;
		case IDC_BUTTON7://input signal color
			button8color = &inputsignalcolor;
			InvalidateRectSafe(colorbutton, NULL, TRUE);
			UpdateWindowSafe(colorbutton);
			tripval = 7;
			break;
		case IDOK:
		case IDCANCEL:
			SafeEndDialog(hwndDlg, 0);
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
	int rangeyearf = 0, rangeyearl = 0;
	static char* tempstring = { 0 };
	HANDLE hFile = { 0 };
	static OVERLAPPED overlapped = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		hwndDlgMain = hwndDlg;
		SafeSetDlgItemInt(hwndDlgMain, IDYEARF, (UINT)yearzero, FALSE);
		SafeSetDlgItemInt(hwndDlgMain, IDYEARL, (UINT)(yearrange/12+yearzero), FALSE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			SafeEndDialog(hwndDlg, 0);
			return TRUE;
		case IDOK://TODO EMPTY AND CREATE NEW LOCALDATA.tXT With new range.
		{
			rangeyearf = (int)SafeGetDlgItemInt(hwndDlgMain, IDYEARF, &failcheck, FALSE);
			rangeyearl = (int)SafeGetDlgItemInt(hwndDlgMain, IDYEARL, &failcheck, FALSE) * 12 - (rangeyearf * 12);
			assert(rangeyearf >= 0 && rangeyearl >= 0);
			//code below ensures that 0 when inputted turns to one
			if (rangeyearl == 0)
				rangeyearl = 1;
			if (rangeyearf == 0)
				rangeyearf = 1;
			CalendarCreate(1, rangeyearf, rangeyearl);
			SaveSettings();
			int zerothwindowpos = (yearrange / 2) + 1;
			for (int lk = 0; lk < buttonfactor; lk++, zerothwindowpos++)
			{
				SafeSetWindowLongPtr(buttonarray[lk], GWLP_USERDATA, zerothwindowpos);
			}
			(void)SendMessageA(buttonarray[0], WM_LBUTTONDOWN, 0, 0);//update everything
			InvalidateRectSafe(SafeGetParent(buttonarray[0]), NULL, TRUE);
			UpdateWindowSafe(SafeGetParent(buttonarray[0]));
			SafeEndDialog(hwndDlg, 0);
			return TRUE;
		}
		default:
			return FALSE;
		}
	case WM_DESTROY:
		free(tempstring);
		return FALSE;
	default:
		return FALSE;
	}
	return FALSE;
}



//reorders data save into ordered state
char* DataSaveReordering(char* readbuffer)
{
	assert(readbuffer != NULL);
	lpdateanddata mydataset = { 0 };
	mydataset = (lpdateanddata)SafeCalloc(  1,    sizeof(fagdata));
	int datesamount = 0;
	int stringlength = (int)strlen(readbuffer);
	//1st get all dates and data
	for (int i = 0; i < stringlength && i < MAXINT16; i++)
	{
		if (readbuffer[i] == specialchar[0])
		{//allocate the struct
			//allocate the pointers to the array of struct pointers
			char* temppointer = { 0 };
			temppointer = (char*)realloc(mydataset, sizeof(fagdata)*((size_t)datesamount+2));
			if (temppointer == NULL)
			{
				CrashDumpFunction(13745, 0);
				return NULL;
			}
			mydataset = (lpdateanddata)temppointer;
			//initialize the pointer necessary
			mydataset[datesamount] = (dateanddata*)SafeCalloc( 1,   sizeof(dateanddata));
			//set the date of the struct
			mydataset[datesamount]->date[0] = specialchar[0];//
			i++;//to skip the first star
			_memccpy(mydataset[datesamount]->date+1, readbuffer + i, specialchar[0], 20);
			//get the datalength
			int tempdatalength = 0;
			for (int m = i + (int)strlen(mydataset[datesamount]->date)-1; m<MAXINT16 && m < stringlength && readbuffer[m] != specialchar[0]; m++)//-1 cause we plused it before
				tempdatalength++;
			mydataset[datesamount]->data = (char*)SafeCalloc(  (size_t)tempdatalength + 10,    sizeof(char));
			//paste the data
			_memccpy(mydataset[datesamount]->data, readbuffer + i + strlen(mydataset[datesamount]->date)-1, specialchar[0], (size_t)tempdatalength);
			i += tempdatalength + (int)strlen(mydataset[datesamount]->date)-1;//to liquidite the +1 we had manually added
			datesamount++;
			i--;//to avoid skipping the '*' upon i++ at the end of the loop
			temppointer = 0;
		}
	}

	//now memset and apply new data
	memset(readbuffer, 0, (size_t)stringlength);
	pdate datesarray = (pdate)SafeCalloc((size_t)datesamount,    sizeof(date));
	char* tempdate = (char*)SafeCalloc(  20,    sizeof(char));

	ArrangeDates(tempdate, datesamount, datesarray, mydataset);
	assert(tempdate != NULL);
	memset(readbuffer, 0, (size_t)stringlength);
	char* date = (char*)SafeCalloc(  20,    sizeof(char));
	for (int i = 0; i < datesamount && i < MAXINT16; i++)
	{
		FindFirstDate(datesarray, i, date, readbuffer, datesamount, stringlength, mydataset);
	}
	free(date);
	free(datesarray);
	free(tempdate);
#pragma warning(push)
#pragma warning(disable: 6001)
	for (int i = 0; i < datesamount && i < MAXINT16; i++)
	{
		free(mydataset[i]->data);
		free(mydataset[i]);
	}
#pragma warning(pop)
	free(mydataset);
	return readbuffer;
}

int NearestDate(int mflag, int filesize, char* readbuffer, int* appendindexlocation, char* dateset, int i)
{
	assert(readbuffer != NULL);
	assert(specialchar[0] != '\0');
	int yearchars = (int)strlen(dateset + 7) - 1;//-1 removes the star at the end 
	int switchflag = 0;//when it switches means like 
	char tempstring[12] = { 0 };
	char tempstring2[12] = { 0 };
	if (mflag == 0)
	{
		//no date alike is present, input to the closest year range
		for (int k = 0; k < filesize && i < MAXINT16; k++)
		{
			if (readbuffer[k] == specialchar[0])//load the year and check if its larger
			{
				long long inputyear = 0, presentyear = 0;
				for (int l = 7, m = 0; readbuffer[k + l] && l < 11; l++, m++)
				{
					tempstring[m] = readbuffer[k + l];
					tempstring2[m] = dateset[l];
				}
				SafeStrToInt64ExA(tempstring, 0, &presentyear);
				SafeStrToInt64ExA(tempstring2, 0, &inputyear);
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
		for (int k = 0; k < filesize && k < MAXINT16; k++)
		{
			if (readbuffer[k] == specialchar[0])//load the month and check if its larger
			{
				long long inputmonth = 0, presentmonth = 0;
				for (int l = 4, m = 0; readbuffer[k + l] && l < 6 && m < MAXINT16; l++,m++)
				{
					tempstring[m] = readbuffer[k + l];
					tempstring2[m] = dateset[l];
				}
				SafeStrToInt64ExA(tempstring, 0, &presentmonth);
				SafeStrToInt64ExA(tempstring2, 0, &inputmonth);
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
		for (int k = 0; k < filesize && k < MAXINT16; k++)
		{
			if (readbuffer[k] == specialchar[0])//load the day and check if its larger
			{
				memset(tempstring, 0, 12);
				memset(tempstring2, 0, 12);
				long long inputday = 0, presentday = 0;
				for (int l = 1, p = 0; readbuffer[k + l] && l < 3; l++, p++)
				{
					tempstring[p] = readbuffer[k + l];
					tempstring2[p] = dateset[l];
				}
				SafeStrToInt64ExA(tempstring, 0, &presentday);
				SafeStrToInt64ExA(tempstring2, 0, &inputday);
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
		return *appendindexlocation;
	}
	return *appendindexlocation;
}

//LoadConfigData loads the configuration data from Calendar.dat
int LoadConfigData(size_t filesize, char * tempstring, BOOL readflag, HANDLE hFile)
{
	assert(tempstring != NULL);
	assert(hFile != INVALID_HANDLE_VALUE);
	/*1 - monthbutton
	3-daybutton
	4-monthsbackground
	5-daysbackground
	6-textbox color
	7- input signal color*/
	size_t templength = 0;
	char* totaltemp = NULL;
	if (tempstring == 0)
		CrashDumpFunction(3, 0);
	totaltemp = (char*)realloc(tempstring, (size_t)filesize + 10);
	if (totaltemp == 0)
		CrashDumpFunction(3, 0);
	tempstring = totaltemp;
	if (tempstring == 0)
	{
		CrashDumpFunction(3, 0);
		return 0;
	}
	//memset(tempstring, 0, (size_t)filesize + 5);
	DWORD outval = 0;
	readflag = ReadFile(hFile, tempstring, (DWORD)filesize, &outval, NULL);
	if (readflag == FALSE)
		CrashDumpFunction(2, 0);
	//the windowscolorsdat separates datasets by newlines
	char* tempstring2 = (char*)SafeCalloc((size_t)(filesize + 100), sizeof(char));
	if (tempstring == 0)
	{
		CrashDumpFunction(3, 0);
		return 0;
	}
	_memccpy(tempstring2, tempstring, '\n', filesize);
	if (tempstring2 == 0)
	{
		CrashDumpFunction(3, 0);
		return 0;
	}
	monthsbuttoncolor = (COLORREF)StrToIntA(tempstring2);
	templength = strlen(tempstring2);
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	datesbuttoncolor = (COLORREF)StrToIntA(tempstring2);
	templength += strlen(tempstring2);
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	monthsbackground = (COLORREF)StrToIntA(tempstring2);
	templength += strlen(tempstring2);
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	datesbackground = (COLORREF)StrToIntA(tempstring2);
	templength += strlen(tempstring2);
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	textbackground = (COLORREF)StrToIntA(tempstring2);
	templength += strlen(tempstring2);
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	inputsignalcolor = (COLORREF)StrToIntA(tempstring2);
	templength += strlen(tempstring2) + 1;//+1 to include the 2nd delimiting newline as we are moving towards yearzero and yearrange
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	yearzero = StrToIntA(tempstring2);
	templength += strlen(tempstring2);
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	yearrange = StrToIntA(tempstring2) * 12 - yearzero * 12;
	templength += strlen(tempstring2) + 1;//+1 due to double newlines
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	startday = StrToIntA(tempstring2 + 4);
	templength += strlen(tempstring2);
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	startmonth = StrToIntA(tempstring2 + 6);
	templength += strlen(tempstring2);
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	startyear = StrToIntA(tempstring2 + 5);
	templength += strlen(tempstring2) + 1;//+1 due to double newlines
	memset(tempstring2, 0, filesize);
	int maxcount = min((int)filesize, 2000);
	_memccpy(datasource, tempstring + templength, '\n', (size_t)maxcount);
	size_t stringlength20 = strlen(datasource);
	datasource[stringlength20 - 1] = 0;
	templength += stringlength20;
	_memccpy(specialchar, tempstring + templength, '\n', filesize - templength);
	specialchar[1] = 0;
	templength += strlen(specialchar) + 1;//zbog kurcine
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	ordereddatasave = StrToIntA(tempstring2);
	templength += strlen(tempstring2);
	memset(tempstring2, 0, filesize);
	_memccpy(tempstring2, tempstring + templength, '\n', filesize - templength);
	RTForTXT = StrToIntA(tempstring2);

	free(tempstring2);

	if (startyear < yearzero)
	{
		startyear = yearzero;
	}
	return 1;
}

//Arranges, years months and days
int ArrangeDates(char * tempdate, int datesamount, pdate datesarray, lpdateanddata mydataset)
{
	assert(tempdate != NULL);
	//arrange the years, all data arrange from smallest to largest
	for (int i = 0; i < datesamount && i < MAXINT16; i++)
	{
		memset(tempdate, 0, 20);
		_memccpy(tempdate, mydataset[i]->date + 7, specialchar[0], 20);
		datesarray[i].year = StrToIntA(tempdate);
		for (int k = i; k > 0 && datesarray[k - 1].year > datesarray[k].year && k < MAXINT16; k--)
		{//data in back is larger
			int temp = datesarray[k].year;//save the i
			datesarray[k].year = datesarray[k - 1].year;//put the larger data forward
			datesarray[k - 1].year = temp;//put the smaller data back
		}
	}
	//arrange the months
	for (int i = 0; i < datesamount && i < MAXINT16; i++)
	{//year are already arranged!!
		memset(tempdate, 0, 20);
		_memccpy(tempdate, mydataset[i]->date + 7, ':', 20);
		int loadingyear = StrToIntA(tempdate);
		memset(tempdate, 0, 20);//get the month
		_memccpy(tempdate, mydataset[i]->date + 4, ':', 20);
		int loadingmonth = StrToIntA(tempdate);
		assert(loadingmonth >= 0);
		int yearset = 0;
		for (int m = datesamount - 1; m >= 0 && i < MAXINT16; m--)
		{
			if (datesarray[m].year == loadingyear)
			{
				yearset = m;//set the yearset to m
				//check if there is more same years, and insert it in m-1 pos if there is, this is to avoid overwriting and ensuring 
				for (int z = m; datesarray[z].year == loadingyear && z >= 0 && i > -MAXINT16; z--)
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
		for (int n = yearset; n > 0 && datesarray[n - 1].year == datesarray[n].year && n > -MAXINT16; n--)//make sure its month of a given year range
		{
			for (int k = n; datesarray[k - 1].month > datesarray[k].month && datesarray[k - 1].year == datesarray[k].year && k < MAXINT16; k++)
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
	for (int i = 0; i < datesamount && i < MAXINT16; i++)
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
		assert(loadingday >= 0);
		int monthset = 0;
		for (int m = datesamount - 1; m >= 0; m--)
		{
			if (datesarray[m].year == loadingyear)
			{
				for (int j = m; j >= 0; j--)//get the monthset
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
		for (int n = monthset; n > 0 && datesarray[n - 1].year == datesarray[n].year && datesarray[n - 1].month == datesarray[n].month && n > -MAXINT16; n--)//make sure its month of a given year range
		{
			for (int k = n; datesarray[k - 1].day > datesarray[k].day && datesarray[k - 1].year == datesarray[k].year && datesarray[k - 1].month == datesarray[k].month && k < MAXINT16; k++)
			{//data in back is larger
				int temp = datesarray[k].day;//save the i
				datesarray[k].day = datesarray[k - 1].day;//put the larger data forward
				datesarray[k - 1].day = temp;//put the smaller data back
			}
		}
	}
	return 0;
}
//finds first date in the datesarray
void FindFirstDate(pdate datesarray, int i, char * date, char * readbuffer, int datesamount, int stringlength, lpdateanddata mydataset)
{
	assert(readbuffer != NULL);
	assert(date != NULL);
	//get the first date

	if (datesarray[i].day < 10 && datesarray[i].month < 10)
	{
		if (-1 == sprintf_s(date, 20, "%d :%d :%d", datesarray[i].day, datesarray[i].month, datesarray[i].year))
		{
			CrashDumpFunction(50, 0);
		}
	}
	else if (datesarray[i].day < 10)
	{
		if (-1 == sprintf_s(date, 20, "%d :%d:%d", datesarray[i].day, datesarray[i].month, datesarray[i].year))
		{
			CrashDumpFunction(50, 0);
		}
	}
	else if (datesarray[i].month < 10)
	{
		if (-1 == sprintf_s(date, 20, "%d:%d :%d", datesarray[i].day, datesarray[i].month, datesarray[i].year))
		{
			CrashDumpFunction(50, 0);
		}
	}
	else
	{
		if (-1 == sprintf_s(date, 20, "%d:%d:%d", datesarray[i].day, datesarray[i].month, datesarray[i].year))
		{
			CrashDumpFunction(50, 0);
		}
	}

	int datavalue = 0;
	for (int k = 0; k < datesamount && k < MAXINT16; k++)
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
					_memccpy(readbuffer + strlen(readbuffer), mydataset[k]->date, 0, (size_t)stringlength);
					_memccpy(readbuffer + strlen(readbuffer), mydataset[k]->data, 0, stringlength - strlen(readbuffer));
					k = datesamount;//date found break the loop
				}
			}
		}
	}
	memset(date, 0, 20);

}
//deals with if-comparison of numbers
void IFComparison(char * macroformated, int dummyint, double ifstatement2, int * ifnumber, double * ifstatement1, int * i, BOOL * myswitch, int length, int ifsign, int * n)
{
	assert(macroformated != NULL);
	if (*ifnumber == 1)
	{
		*ifstatement1 = dummyint;
	}
	else if (*ifnumber == 2)
	{
		int ifresult = 0;
		ifstatement2 = dummyint;
		*ifnumber = 1;
		//determine if the if statement is true, then keep going properly within the bound of ifstatement till end if, else stop and skip everything till after !endif
		switch (ifsign)
		{
		case 1://!Equivalent
			if (*ifstatement1 == ifstatement2)
				ifresult = 1;
			break;
		case 2://!LargerThen
			if (*ifstatement1 > ifstatement2)
				ifresult = 1;
			break;
		case 3://!SmallerThen
			if (*ifstatement1 < ifstatement2)
				ifresult = 1;
			break;
		case 4://LargerorEqual
			if (*ifstatement1 >= ifstatement2)
				ifresult = 1;
			break;
		case 5://SmallerorEqual
			if (*ifstatement1 <= ifstatement2)
				ifresult = 1;
			break;
		case 6://NotEquivalent
			if (*ifstatement1 != ifstatement2)
				ifresult = 1;
			break;
		default:
			break;
		}
		//below is skipping of endif in case if returned false
		myswitch = FALSE;
		assert(length >= 0);
		if (ifresult == 0)//skip everything till next endif
		{
			for (;myswitch != NULL && *i < length && *myswitch == FALSE && *i < 100000; (*i)++)
			{
				if (macroformated[*i] == '!')
				{
					{
						for (; *n < 38 && myswitch == FALSE; (*n)++)
							if (0 == comparestrings(macroformated + ((*i) + 1), (char*)"!EndIf"))
								*myswitch = TRUE;//kill loop sitting at EndIf
					}
				}
			}
		}
	}
}

INT_PTR CALLBACK DlgMacroInfoProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return 1;
}

void SaveSettings()
{
	OVERLAPPED overlapped = { 0 };
	HANDLE hFile = { 0 };
	char * tempstring = (char*)SafeCalloc(1000, sizeof(char));
	if (-1 == sprintf_s(tempstring, 1000, "%sCalendar.dat", theworkingdirectory))
	{
		CrashDumpFunction(50, 0);
	} 
	hFile = CreateFileA(tempstring, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(hFile != INVALID_HANDLE_VALUE);
	SafeSetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	SafeSetEndOfFile(hFile);
	char* buffer = (char*)SafeCalloc(3000, sizeof(char));
	if (-1 == sprintf_s(buffer, 1000, "%lu\n%lu\n%lu\n%lu\n%lu\n%lu\n\n%d\n%d\n\nDay:%d\nMonth:%d\nYear:%d\n\n%s\n%s\n%d\n%d\n", monthsbuttoncolor, datesbuttoncolor, monthsbackground, datesbackground, textbackground, inputsignalcolor, yearzero, yearzero + yearrange / 12, startday, startmonth, startyear, datasource, specialchar, ordereddatasave,RTForTXT))
	{
		CrashDumpFunction(50, 0);
	}
	SafewriteFile(hFile, buffer, (DWORD)strlen(buffer), NULL, &overlapped);
	SafeCloseHandle(hFile);
	free(buffer);
	free(tempstring);
}