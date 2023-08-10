/*
************

MyCalendar.c, This is the main WinMain located.
- REORGANIZE WHOLE THINK INTO MULTIPLE FILES, THIS THING IS CHAOS RN
************
*/

#include "framework.h" //All header includes stuff is mashed up over there

RECT buttonrect = { 0,0,0,0 };

//Sizes of the 4 main child windows
RECT MonthRect = { 0 };
RECT DatesRect = { 0 };
RECT StringRect = { 0 };
RECT MarksRect = { 0 };
winloc windowdataorg[buttonfactor] = { 0, 0 };

int TrustedIndex = 0;//mittigating "Speculative Execution Side Channel hardware vulnerabilities" for some i loops.
int tiShift = 0;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstances, _In_ LPWSTR pCmdLine, _In_ int nShowCmd)
{
	HWND hwnd;
	MSG msg;
	cleaner4100(pCmdLine, hPrevInstances);

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
	wndclass.hbrBackground = (HBRUSH)GetStockObject(TRANSPARENT),
	wndclass.lpszMenuName = NULL,
	wndclass.lpszClassName = TEXT("Calendar"),
	wndclass.hIconSm = NULL,
	};

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(0, TEXT("Calendar"), TEXT("Calendar"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DestroyWindow(hwnd);
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int cxClient, cyClient, xMeasure, yMeasure, cxReminder, cyReminder;
	static HWND hWMonths = NULL, hWTextBox = NULL, hWMarks = NULL, hWDates = NULL;
	static BOOL CreationFlag = FALSE; //False if childwindows have not been created, true if they where.

	ShowMessage(hwnd, cxClient, cyClient, message);
	switch (message)
	{
	case WM_CREATE:
		datactionswitch(3);
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
		return 0;

	case WM_PAINT:

		if (CreationFlag == FALSE)
		{
			if (ChildCreationFunction())
			{
				hWMonths = CreateWindowEx(0, TEXT("Months class"), TEXT("Months"), WS_CHILD | WS_VSCROLL, 0, 0, 2 * xMeasure, cyClient, hwnd, NULL, NULL, NULL);
				hWTextBox = CreateWindowEx(0, TEXT("Text Box Class"), TEXT("TextBox"), WS_CHILD | ES_LEFT | ES_MULTILINE | ES_WANTRETURN, 2 * xMeasure, 8 * yMeasure, cxClient - 4 * xMeasure, cyClient-8*yMeasure, hwnd, NULL, NULL, NULL);
				hWMarks = CreateWindowEx(0, TEXT("MarkClass"), TEXT("Marks"), WS_CHILD, cxClient - 4 * xMeasure, 8 * yMeasure, cxClient-8*xMeasure, cyClient-8*yMeasure, hwnd, NULL, NULL, NULL);
				hWDates = CreateWindowEx(0, TEXT("Dates Class"), TEXT("Dates"), WS_CHILD, 2 * xMeasure, 0, cxClient - 2 * xMeasure, 8 * yMeasure, hwnd, NULL, NULL, NULL);

				ShowWindow(hWMonths, SW_SHOW);
				UpdateWindow(hWMonths);
				ShowWindow(hWTextBox, SW_SHOW);
				UpdateWindow(hWTextBox);
				ShowWindow(hWMarks, SW_SHOW);
				UpdateWindow(hWMarks);
				ShowWindow(hWDates, SW_SHOW);
				UpdateWindow(hWDates);
				CreationFlag = TRUE;
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
				MoveWindow(hWTextBox, 2 * xMeasure, 8 * yMeasure, cxClient - 4 * xMeasure, cyClient-8*yMeasure, TRUE);
				UpdateWindow(hWTextBox);
			}

			if (hWMarks != NULL)
			{
				MoveWindow(hWMarks, cxClient - 2 * xMeasure, 8 * yMeasure, cxClient - 8 * xMeasure, cyClient - 8 * yMeasure, TRUE);
				UpdateWindow(hWMarks);
			}

			if (hWDates != NULL)
			{
				MoveWindow(hWDates, 2 * xMeasure, 0, cxClient-2*xMeasure, 8 * yMeasure, TRUE);
				UpdateWindow(hWDates);
			}
		}

		return 0;


	case WM_DESTROY:
		DestroyWindow(hWTextBox);
		DestroyWindow(hWMarks);
		DestroyWindow(hWDates);
		DestroyWindow(hWMonths);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void cleaner4100(LPWSTR something, HINSTANCE something2)
{
	int a = 1;
	if (something2 || something)
	{
		do {
			a -= 1;
		} while (a == 1);
	}

}

BOOL ChildCreationFunction(void)
{
	WNDCLASSEX TextBoxClass =
	{
	TextBoxClass.cbSize = sizeof(WNDCLASSEX),
	TextBoxClass.style = CS_VREDRAW | CS_DBLCLKS | CS_HREDRAW,
	TextBoxClass.lpfnWndProc = TextBoxProc,
	TextBoxClass.cbClsExtra = 0,
	TextBoxClass.cbWndExtra = 0,
	TextBoxClass.hInstance = NULL,
	TextBoxClass.hIcon = LoadIcon(NULL, IDI_APPLICATION),
	TextBoxClass.hCursor = LoadCursor(NULL, IDC_ARROW),
	TextBoxClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH),
	TextBoxClass.lpszMenuName = NULL,
	TextBoxClass.lpszClassName = TEXT("Text Box Class"),
	TextBoxClass.hIconSm = NULL,
	};

	RegisterClassEx(&TextBoxClass);

	WNDCLASSEX MarkClass =
	{
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
	MarkClass.hIconSm = NULL,
	};

	RegisterClassEx(&MarkClass);

	WNDCLASSEX DatesClass =
	{
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
	DatesClass.hIconSm = NULL,
	};

	RegisterClassEx(&DatesClass);

	WNDCLASSEX MonthsClass =
	{
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
	MonthsClass.hIconSm = NULL,
	};

	RegisterClassEx(&MonthsClass);

	if (&MonthsClass != FALSE && &DatesClass != FALSE && &MarkClass != FALSE && &TextBoxClass != FALSE)
		return TRUE;

	else
		return FALSE;
}

BOOL ShowMessage(HWND hwnd, int XClient, int YClient, UINT message)
{
	TCHAR szString[100] = { L"Hi!" };
	HDC ihdc = GetDC(hwnd);
	TextOut(ihdc, XClient / 2, YClient / 2, szString, wsprintf(szString, TEXT("%u"), message));
	ReleaseDC(hwnd, ihdc);

	return FALSE;
}


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


