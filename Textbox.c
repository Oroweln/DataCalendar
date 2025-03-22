#include "framework.h"
#define BUFSIZE 64000
#define SHIFTED 0x8000 
#define yMAXLINES 30
#define xMAXCHARS 500
size_t TextHeapRemaining = 0;
COLORREF textbackground = RGB(255,255,255);
int rtfindex = 0;
typedef struct mcallbackinfo
{
	HANDLE hFile;
	int amountoread;
	int offset;
}mcallbackinfo;
HWND TextBoxHwnd = NULL;
WCHAR** fontlist = { 0 };
int fontamount = 0;
//INT_PTR CALLBACK FontBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK TextBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	COLORREF stupidtextcolor = 0;
	static HBRUSH textcolor = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		TextBoxHwnd = (SafeGetDlgItem(hwnd, TEXTBOXINPUT));//old one was named TEXTBOXINPUT in case something breaks bad
		assert(TextBoxHwnd != NULL);
		SetWindowSubclass(TextBoxHwnd, TextBoxInputSbc, 0, 0);
		txtBackColor(TextBoxHwnd, textbackground);
		stupidtextcolor = RGB((~textbackground) & 0xff, ((~textbackground) & 0xff) >> 8, ((~textbackground) & 0xff) >> 16);
		txtColor(TextBoxHwnd, stupidtextcolor);
		break;
		/*
All you need is to set the required color in control's device context and pass an HBRUSH with same color in WM_CTLCOLOREDIT message. If you want to change both foreground & background colors, use SafeSetTextColor t0 change the text color. But you must pass the background color HBRUSH. But if you want to change the text color only, then you must pass a DC_BRUSH with GetStockObject function.
*/
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case TEXTBOXINPUT:
			switch(HIWORD(wParam))
			{
			case EN_UPDATE:
				TextHeapRemaining = (size_t)(pchinputbuffermemory - Edit_GetTextLength(TextBoxInput));
				assert(buttonmarks != NULL);
				(void)SendMessageA(buttonmarks[2], DATEUPDATETICK, 0, 0);
				return TRUE;
			default:
				return TRUE;
			}
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return FALSE;
}

LRESULT TextBoxInputSbc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static BOOL CtrlPressed = FALSE;
	BOOL datepresentflag = 0;
	int appendlocationindex = 0, amountread = 0;
	OVERLAPPED overlapstruct = { 0 };
	HANDLE hFile = { 0 };
	CHARFORMATA static cf = { 0 };
	LOGFONT lgf = { 0 };
	HMENU hPopMenu = { 0 };
	CHOOSEFONTA thefont = { 0 };
	HFONT defaultFont = { 0 };
	int zoomden = 1, zoomnom = 1;
	switch (uMsg)
	{
	case WORDWRAP:
		MessageBoxA(hWnd, "Wordwrapping is not introduced yet.", "Wordwrap", MB_OK);
		break;
	case IDDATECHANGE:
		assert(amountread >= 0);
		DateTestBufferLoad(&amountread, &overlapstruct, &appendlocationindex, &datepresentflag);
		if (amountread > pchinputbuffermemory)
		{
			CrashDumpFunction(10000, 1);
		}
		EnableWindow(hWnd, TRUE);
		ShowWindow(hWnd, TRUE);
		if (datepresentflag == TRUE)
		{
			hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (INVALID_HANDLE_VALUE == hFile)
			{
				CrashDumpFunction(578, 0);
				return FALSE;
			}
			if(RTForTXT == 1)
			{
				char* readbuffer = (char*)SafeCalloc(  (size_t)amountread + 1,    sizeof(char));
				overlapstruct.Offset = (DWORD)appendlocationindex;
				if (FALSE == ReadFile(hFile, readbuffer, (DWORD)amountread, NULL, &overlapstruct))
				{
					CrashDumpFunction(3168, 0);
					return FALSE;
				}
				assert(amountread >= 0);
				SafeSetWindowTextA(hWnd, readbuffer);
			}
			else if (RTForTXT == 0)
			{
				mcallbackinfo mydata = { 0 };
				mydata.hFile = hFile;
				mydata.amountoread = amountread;
				mydata.offset = appendlocationindex;
				mcallbackinfo* pmd = &mydata;
				EDITSTREAM es = { 0 };
				es.dwCookie = (DWORD_PTR)pmd;
				es.pfnCallback = EditStreamInCallback;
				int charspassed = SendMessage(hWnd, EM_STREAMIN, SF_RTF, (LPARAM)&es);
				if (amountread > 0 && charspassed == 0)
				{
					MessageBoxA(hWnd, "Can't Read PlainText with RichEdit", "Error", MB_OK);
				}
			}
			SafeCloseHandle(hFile);
		}
		else
			SafeSetWindowTextA(hWnd, "");
		break;	
	case TEXTBOXCOLORCHANGE:
	{
		assert(TextBoxHwnd);
		txtBackColor(TextBoxHwnd, textbackground);
		COLORREF textboxcolor = RGB((~textbackground) & 0xff, ((~textbackground) & 0xff) >> 8, ((~textbackground) & 0xff) >> 16);
		txtColor(TextBoxHwnd, textboxcolor);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		hPopMenu = LoadMenuA(GetModuleHandle(NULL), MAKEINTRESOURCEA(TXTPOPMENU));
		assert(hPopMenu != NULL);
		HMENU hTrackPopMenu = GetSubMenu(hPopMenu, 0);
		assert(hTrackPopMenu != NULL);
		POINT pt = { 0 };
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		if (FALSE == ClientToScreen(hWnd, (LPPOINT)&pt))
		{
			CrashDumpFunction(5126, 0);
			return FALSE;
		}
		int status = TrackPopupMenuEx(hTrackPopMenu, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_NOANIMATION | TPM_HORIZONTAL, pt.x, pt.y, hWnd, NULL);
		switch (status)
		{
		case ID_COPY:
			(void)SendMessage(hWnd, WM_COPY, 0, 0);
			break;
		case ID_PASTE:
			(void)SendMessage(hWnd, WM_PASTE, 0, 0);
			break;
		case ID_CUT:
			(void)SendMessage(hWnd, WM_CUT, 0, 0);
			break;
		case ID_DELETE:
			(void)SendMessage(hWnd, WM_CLEAR, 0, 0);
			break;
		case ID_SELECTALL:
			(void)SendMessage(hWnd, EM_SETSEL, 0, -1);
			break;
		case ID_UNDO:
			(void)SendMessage(hWnd, EM_UNDO, 0, 0);
			break;
		case ID_REDO:
			(void)SendMessage(hWnd, EM_REDO, 0, 0);
			break;
		case ID_GETFONT:
			cf.cbSize = sizeof(CHARFORMAT);
			(void)SendMessage(hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			break;
		case ID_APPLYFONT:
			cf.cbSize = sizeof(CHARFORMAT);
			(void)SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			break;
		case ID_REMOVEFONT:
			defaultFont = (HFONT)GetStockObject(SYSTEM_FONT);
			SendMessage(hWnd, WM_SETFONT, WPARAM(defaultFont), TRUE); // Send this to each control
			SafeDeleteObject(defaultFont);
			break;
		case ID_ZOOMIN:
			(void)SendMessage(hWnd, EM_GETZOOM, (WPARAM)(&zoomnom), (LPARAM)(&zoomden));
			if (zoomnom == 0)
			{
				zoomnom = 100;
				zoomden = 100;
			}
			Edit_SetZoom(hWnd, zoomnom + 10, zoomden);
			break;
		case ID_ZOOMOUT:
			Edit_GetZoom(hWnd, &zoomnom, &zoomden);
			if (zoomnom == 0)
			{
				zoomnom = 100;
				zoomden = 100;
			}
			Edit_SetZoom(hWnd, zoomnom - 10, zoomden);
			break;
		}
		status = (int)GetLastError();
		if (FALSE == DestroyMenu(hPopMenu))
		{
			CrashDumpFunction(5122, 0);
			return FALSE;
		}
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 0x53://"S" key
		{
			if (CtrlPressed)//save the text into textdata
			{
				assert(TextBoxInput);
				int textlength = 0;
				char* pchInputBuf = { 0 };
				char** PPch = { 0 };
				textlength = GetWindowTextLengthA(TextBoxInput);
				if (GetLastError() != 0)
					CrashDumpFunction(5230, 0);
				if (0 == textlength)
				{//empty, remove date data.
					pchInputBuf = (char*)SafeCalloc(1, sizeof(char));
				}
				else if (RTForTXT == 1)
				{
					pchInputBuf = (char*)SafeCalloc((size_t)textlength + 1, sizeof(char));
					if (pchInputBuf == NULL)
						break;
					GetWindowTextA((TextBoxInput), (pchInputBuf), (textlength + 1));
					if (GetLastError() != 0)
						CrashDumpFunction(5236, 0);
				}
				else if (RTForTXT == 0)
				{
					EDITSTREAM es = { 0 };
					PPch = (char**)SafeCalloc(2, sizeof(char*));
					es.dwCookie = (DWORD_PTR)PPch;
					es.pfnCallback = EditStreamOutCallback;
					(void)SendMessage(hWnd, EM_STREAMOUT, SF_RTF, (LPARAM)&es);
					pchInputBuf = PPch[0];
					rtfindex = 0;
				}
				assert(amountread >= 0);
				DateTestBufferLoad(&amountread, &overlapstruct, &appendlocationindex, &datepresentflag);
				if (FALSE == savingFunction(&appendlocationindex, pchInputBuf, &overlapstruct, &amountread, datepresentflag))
				{
					CrashDumpFunction(5252, 0);
				}
				free(pchInputBuf);
				free(PPch);
			}
			break;
		}
		case 0x55://"U" key
			if (CtrlPressed)//undo
				(void)SendMessage(hWnd, EM_REDO, 0, 0);
			break;
		case 0x44://"D" key
			if (CtrlPressed)//delete
				(void)SendMessage(hWnd, WM_CLEAR, 0, 0);
			break;
		case VK_CONTROL:
			CtrlPressed = TRUE;
			break;
		}
		break;
	}
	case WM_KEYUP:
	{
		if (wParam == VK_CONTROL)
			CtrlPressed = FALSE;
		break;
	}
	default:
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void DateTestBufferLoad(int* amountread, OVERLAPPED* overlapstruct, int* appendlocationindex, BOOL* datepresent)
{
	assert(amountread != NULL);
	assert(overlapstruct != NULL);
	int xtoalloc = 0;
	HANDLE hFile = { 0 };
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		CrashDumpFunction(5286, 0);
	}
	int amounttowrite = 0, oldlocindex = 0;
	BOOL randomflag = FALSE;
	char* selecteddate1 = (char*)SafeCalloc(  25,    sizeof(char));
	selecteddate1 = DateTextShow(selecteddate1);
	*amountread = (int)GetFileSize(hFile, NULL);
	if (*amountread == INVALID_FILE_SIZE)
	{
		CrashDumpFunction(272, 0);
	}
	char * readbuffer = (char*)SafeCalloc(  (size_t)(*amountread) + 1,    sizeof(char));//MALLOCING HERE IS TEMP WORKAROUND TILL WE MAKE PROGRAM FULLY FUNCTIONAL, to run from stack.
	randomflag = ReadFile(hFile, readbuffer, (DWORD)*amountread, (DWORD*)amounttowrite, overlapstruct);//check if the data for a date is present
	if (randomflag == 0|| readbuffer == NULL)
	{
		SafeCloseHandle(hFile);
		free(readbuffer);
		readbuffer = NULL;
		CrashDumpFunction(270,0);
		return;
	}
	oldlocindex = *appendlocationindex;
	for (int i = 0; i < *amountread && readbuffer[i] != '\0' && i < MAXINT; i++)//this loop will start reading when it reaches the appropriate data, and stop when it finishes reading the appropriate data from textdata
	{
		*appendlocationindex = i;//this value is the beggining of the text following the date within dataread
		if (readbuffer[i] == specialchar[0] && selecteddate1 != NULL)//specialchar[0] symbolizes posibility of beggining of the date, hence start checking for whole string afterwards below if it matches the selecteddate
		{
			for (int z = 1; (z + i <= *amountread) && (selecteddate1[z] == readbuffer[i + z]) && z < MAXINT; z++)//date checking
			{
				if (readbuffer[i + z] == specialchar[0] || readbuffer[i + z] == '\0')//we have reached second start without quiting the loop, ie. date is matching
				{//here we will allocate all memory necessary. Loadingdataintobuffer will input actual data
					*datepresent = TRUE;
					*appendlocationindex = z + i + 1;//to the beginning of text
					for (int k = *appendlocationindex; k<*amountread && readbuffer[k] != specialchar[0] && readbuffer[k] != '\0' && k < MAXINT; k++, xtoalloc++)
					{
						;
					}
					i = *amountread + 1;//to kill the loop
				}
			}
		}
#pragma warning(disable: 6386)
		if (readbuffer[i + 1] == '\0' && i < *amountread)
		{
			readbuffer[i + 1] = '\r';
			(*appendlocationindex)++;
		}
#pragma warning(default: 6386)
	}
	free(readbuffer);
	free(selecteddate1);
	readbuffer = NULL;
	SafeCloseHandle(hFile);
	*amountread = xtoalloc;//+ytoalloc cause we used window /r/n
	return;
}

BOOL savingFunction(int* appendlocationindex, char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, BOOL datepresent)
{//we need to push all data forwards or backwards depending if we are ammending smaller sized new dataset or larger sized.
	assert(pchInputBuf != NULL);
	assert(appendlocationindex != NULL);
	HANDLE hFile = { 0 };//errorint
	BOOL EmptyDataFlag = TRUE;
	size_t oldstringlength = (size_t)*amountread;//amountread here is amount of data within the date selected
	//figure out if the data to be applied is less or more then last one that was, or perhaps same size.
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		CrashDumpFunction(227,0);
		return 0;
	}
	*amountread = (int)GetFileSize(hFile, NULL);//amountread will be here used as total length
	if (*amountread == INVALID_FILE_SIZE)
	{
		CrashDumpFunction(231, 0);
		return FALSE;
	}
	//to do this get already existing data's size
	char * tempcharbuf = (char*)SafeCalloc(  (size_t)*amountread+5,   sizeof(char));
	//test read
	if (FALSE == ReadFile(hFile, tempcharbuf, (DWORD)*amountread, NULL, overlapstruct))
	{
		int errorval = (int)GetLastError();
		if ((*amountread != 0) && (errorval != 38))//if amountread is 0, that means file is empty, hence why error was thrown.
		{
			SafeCloseHandle(hFile);
			CrashDumpFunction(246,0);
			return FALSE;
		}
	}
	free(tempcharbuf);
	OVERLAPPED noverlapstruct = { 0 };
	noverlapstruct.Offset = (DWORD)*appendlocationindex;//point at the beggining of data
	SafeCloseHandle(hFile);
	OVERLAPPED ewreoverlapstruct = { 0 };
	ewreoverlapstruct.Offset = (DWORD)*appendlocationindex;
	size_t strLength = strnlen_s(pchInputBuf, (size_t)pchinputbuffermemory);
	if (datepresent == FALSE && strLength>0)
	{//append date mark
		char * selecteddate1 = (char*)SafeCalloc(  sizeof(char),    25);
		selecteddate1 = DateTextShow(selecteddate1);
		if(ordereddatasave == 1)
		{
			if (FALSE == DateWrite(appendlocationindex, selecteddate1))
			{
				CrashDumpFunction(3392, 0);
				return FALSE;
			}
		}
		else 
		{
			hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			assert(hFile != INVALID_HANDLE_VALUE);
			SafewriteFile(hFile, selecteddate1, 12, NULL, &ewreoverlapstruct);
			SafeCloseHandle(hFile);
		}
		hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		*amountread = (int)GetFileSize(hFile, NULL);//update amount read
		if (*amountread == INVALID_FILE_SIZE)
		{
			CrashDumpFunction(247, 0);
			return FALSE;
		}
		noverlapstruct.Offset = (DWORD)(*appendlocationindex);//point at the beggining of data instead of mark data
		if (datepresent == FALSE)
		{
			noverlapstruct.Offset += (DWORD)strlen(selecteddate1);
		}
		free(selecteddate1);
		SafeCloseHandle(hFile);
	}
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		CrashDumpFunction(3416, 0);
		return FALSE;
	}
	if (strLength < 1)//if input is empty, delete the date mark as well as the data. Assuming data mark exist(develop check to see if it does!
	{
		if (EmptyDataFlag == TRUE && datepresent == TRUE)
		EmptyDataWrite(&noverlapstruct, amountread, hFile, &oldstringlength, appendlocationindex);
	}
	//Now finally determine the relation and apply according process
	else if (oldstringlength > strLength)//new data is smaller, pull back all data in front of it and shrink.
	{
		SmallDataWrite(pchInputBuf, &noverlapstruct, amountread, strLength, hFile, &oldstringlength);
	}
	else if (oldstringlength < strLength)//new data is larger, push forward all data in front of it and add size.
	{
		LargeDataWrite(pchInputBuf, &noverlapstruct, amountread, strLength, hFile, &oldstringlength);
	}
	else	//in case they are same we do nothing except write over it
	{
		if (FALSE == DataWriteOver(pchInputBuf, &noverlapstruct, amountread, strLength, hFile))
		{
			CrashDumpFunction(248,0);
			return FALSE;
		}
	}
	if (INVALID_HANDLE_VALUE == hFile)
	{
		CrashDumpFunction(249,0);
		return FALSE;
	}
	SafeCloseHandle(hFile);
	return TRUE;
}

BOOL LargeDataWrite(char* pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile, size_t* oldstringlength)
{
	assert(pchInputBuf != NULL);
	assert(overlapstruct != NULL);
	int charlength = 0;
	char* readbuffer = { 0 }, * readbuffer2 = { 0 };
	readbuffer = (char*)SafeCalloc((size_t)*amountread,    sizeof(char));
	readbuffer2 = (char*)SafeCalloc((size_t)(*amountread + strLength + 1), sizeof(char));
	assert(pchInputBuf != NULL);
	if (FALSE == ReadFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct))
	{
		DWORD nigga = GetLastError();
		free(readbuffer);
		free(readbuffer2);
		readbuffer2 = readbuffer = NULL;
		CrashDumpFunction(251,0);
		return -1;
	}
	if ((*amountread - overlapstruct->Offset) == 0)//we need to append data at EOF
	{
		SafewriteFile(hFile, pchInputBuf, (DWORD)strLength, (LPDWORD)(&charlength), overlapstruct);
	}
	else//we need to push data forward and then append data
	{
		
		_memccpy(readbuffer2 + strLength, readbuffer + *oldstringlength, 0, (size_t)*amountread - overlapstruct->Offset);//push daata forward with help of offset, readbuffer+*oldstringlength ensure that the already present "oldstring" isnt pushed forwards but overwritten
		_memccpy(readbuffer2, pchInputBuf, 0, strLength);//append data at the newly created space
		SafewriteFile(hFile, readbuffer2, (DWORD)(((unsigned long long)(*amountread) - overlapstruct->Offset) + ((unsigned long long)strLength - *oldstringlength)), (LPDWORD)(&charlength), overlapstruct);
	}
	free(readbuffer);
	free(readbuffer2);
	readbuffer2 = readbuffer = NULL;
	return TRUE;
}

BOOL SmallDataWrite(char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile, size_t* oldstringlength)
{
	assert(pchInputBuf != NULL);
	assert(hFile != INVALID_HANDLE_VALUE);
	int charlength = 0;
	char* readbuffer = (char*)SafeCalloc((size_t)(*amountread + strLength), sizeof(char));
	char* readbuffer2 = (char*)SafeCalloc(  (size_t)(*amountread+strLength),    sizeof(char));
	int offset = (int)(overlapstruct->Offset + *oldstringlength);//offset will skip reading the data of the given date date as it will be wholly wiped out and replaced
	assert(pchInputBuf != NULL);
	if (FALSE == ReadFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct))
	{
		SafeCloseHandle(hFile);
		free(readbuffer);
		free(readbuffer2);
		readbuffer2 = readbuffer = NULL;
		CrashDumpFunction(253,0);
		return -1;
	}
	if ((*amountread - offset) == 0)//we need to append data at EOF
	{
		SafewriteFile(hFile, pchInputBuf, (DWORD)strLength, (LPDWORD)(&charlength), overlapstruct);
	}
	else//we are pasting in middle of data
	{
		_memccpy(readbuffer2 + strLength, readbuffer+*oldstringlength, 0, *amountread-*oldstringlength);//copy only date in front of oldstringlength
		_memccpy(readbuffer2, pchInputBuf, 0, strLength);//paste into remaining space for the date
		SafewriteFile(hFile, readbuffer2, (*amountread - overlapstruct->Offset) - (DWORD)(*oldstringlength - strLength), (LPDWORD)(&charlength), overlapstruct);
	}
	//squeeze out extra date down here now
	SafeSetFilePointer(hFile, (LONG)(strLength - *oldstringlength), NULL, FILE_END);
	SafeSetEndOfFile(hFile);
	free(readbuffer);
	free(readbuffer2);
	readbuffer2 = readbuffer = NULL;
	return TRUE;
}

BOOL EmptyDataWrite(OVERLAPPED* overlapstruct, int* amountread, HANDLE hFile, size_t* oldstringlength, int * appendlocationindex)
{
	assert(hFile != INVALID_HANDLE_VALUE);
	assert(oldstringlength != NULL);
	int charlength = 0;
	overlapstruct->Offset = (DWORD)(*appendlocationindex - 12); //points to the beggining of the date-mark, so we can delete it as well, since all data is being deleted.
	char* readbuffer = (char*)SafeCalloc((size_t)*amountread,    sizeof(char));//for reading the txt file
	char* readbuffer2 = (char*)SafeCalloc((size_t)*amountread,    sizeof(char));//for new data to be written
	if (FALSE == ReadFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct))
	{
		SafeCloseHandle(hFile);
		free(readbuffer);
		free(readbuffer2);
		readbuffer2 = readbuffer = NULL;
		CrashDumpFunction(255,0);
		return 0;
	}
	_memccpy(readbuffer2, readbuffer+*oldstringlength+12, 0, *amountread-*oldstringlength-12);
	
	SafewriteFile(hFile, readbuffer2, (*amountread - overlapstruct->Offset) - (DWORD)(*oldstringlength + 12), (LPDWORD)(&charlength), overlapstruct);
	SafeSetFilePointer(hFile, (LONG)(-12 - *oldstringlength), NULL, FILE_END);
	SafeSetEndOfFile(hFile);
	free(readbuffer);
	free(readbuffer2);
	readbuffer2 = readbuffer = NULL;
	return TRUE;
}

BOOL DataWriteOver(char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile)
{
	assert(pchInputBuf);
	assert(hFile != INVALID_HANDLE_VALUE);
	char* readbuffer = (char*)SafeCalloc((size_t)*amountread,    sizeof(char));
	if (FALSE == ReadFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct))
	{
		SafeCloseHandle(hFile);
		hFile =NULL;
		free(readbuffer);
		readbuffer = NULL;
		CrashDumpFunction(260,0);
		return 0;
	}
	_memccpy(readbuffer, pchInputBuf, 0, strLength);
	SafewriteFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct);
	free(readbuffer);
	readbuffer = NULL;
	return TRUE;
}

//When there is no date uploaded to datafile, this positions it properly and uploads it accordingly
//If the function returns false, simply input the date at the end.
BOOL DateWrite(int * appendindexlocation, char * dateset)
{
	assert(dateset != NULL);
	assert(appendindexlocation != NULL);
	HANDLE hFile = { 0 };
	int filesize = 0;
	DWORD byteswritten = 0;
	char* readbuffer = { 0 };
	OVERLAPPED Overlapped = { 0 };
	
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		CrashDumpFunction(5584, 0);
		return FALSE;
	}
	filesize = (int)GetFileSize(hFile, NULL);
	if (filesize == INVALID_FILE_SIZE)
	{
		CrashDumpFunction(383, 0);
		return FALSE;
	}
	readbuffer = (char*)SafeCalloc((size_t)(filesize + 20), sizeof(char));//20 for dateset

	if (FALSE == ReadFile(hFile, readbuffer, (DWORD)filesize, NULL, &Overlapped))
	{
		CrashDumpFunction(385,0);
		return FALSE;
	}
	
	//scan for the year dataset range
	findthenearestdate(filesize, readbuffer, dateset, appendindexlocation, 1);
	//walk back to the beginning of date set
	for (*appendindexlocation = *appendindexlocation-1; *appendindexlocation >= 0 && readbuffer[*appendindexlocation] != specialchar[0]; (*appendindexlocation)--)
	{
		;
	}
	//now push all data forward and paste the dateset
	int datelength = (int)strlen(dateset);
	char* readbuffer2 = (char*)SafeCalloc((size_t)filesize,    sizeof(char));
	_memccpy(readbuffer2, readbuffer, 0, (size_t)filesize);
	if ((readbuffer2 + *appendindexlocation) == 0)
	{
		CrashDumpFunction(5851, 0);
		return FALSE;
	}
	_memccpy(readbuffer + datelength + *appendindexlocation, readbuffer2+ *appendindexlocation, 0, (size_t)filesize- *appendindexlocation +datelength);
	_memccpy(readbuffer + *appendindexlocation, dateset, 0, (size_t)datelength);
	Overlapped.Offset = (DWORD)*appendindexlocation;
	SafewriteFile(hFile, readbuffer + *appendindexlocation, (DWORD)(strlen(readbuffer) - *appendindexlocation), &byteswritten, &Overlapped);
	free(readbuffer);
	free(readbuffer2);
	SafeCloseHandle(hFile);
	return TRUE;
}

char * DateTextShow(char * selecteddate1)
{
	assert(selecteddate1 != NULL);
	//everything below happens when you click a date, 12120 is a flag for this mechanism: You click a date and SETFOCUS is sent from the dates.c under the context of Datesproc during message "LM_BUTTONDOWN", in that message switch, there is a section where it sends WM_SETOFUCS to here with LOWORD(lParam) se to 12120. All this is done so text could be shown in textbox upon clicking a date.
	//In this section particulary you have "selecteddate1" buffer being loaded up for sake of later comparison with the read data from datatext, when exact match string of selectedate, according to its dating format, after this if statemant, the data is read.
	//Also note wparam and lparam also both used to store and the decompile year/day/month values. when flag is 12120(?)
	assert(selecteddate >= 0);
	int dateyear = selecteddate >> 11;
	int datemonthindex = (selecteddate & 0x780) >> 7;
	int datedayindex = (selecteddate & 0x7F);
	if (datedayindex < 10 && datemonthindex < 10)
	{
		if (-1 == sprintf_s(selecteddate1, 20, "%s%d :%d :%d%s", specialchar, datedayindex, datemonthindex, dateyear, specialchar))
		{
			CrashDumpFunction(50, 0);
		}
	}
	else if (datedayindex < 10)
	{
		if (-1 == sprintf_s(selecteddate1, 20, "%s%d :%d :%d%s", specialchar, datedayindex, datemonthindex, dateyear, specialchar))
		{
			CrashDumpFunction(50, 0);
		}
	}
	else if (datemonthindex < 10)
	{
		if (-1 == sprintf_s(selecteddate1, 20, "%s%d :%d :%d%s", specialchar, datedayindex, datemonthindex, dateyear, specialchar))
		{
			CrashDumpFunction(50, 0);
		}
	}
	else
	{
		if (-1 == sprintf_s(selecteddate1, 20, "%s%d :%d :%d%s", specialchar, datedayindex, datemonthindex, dateyear, specialchar))
		{
			CrashDumpFunction(50, 0);
		}
	}
	return selecteddate1;

}
//richedit quick functions 


void txtColor(HWND hWindow, COLORREF clr) {
	assert(hWindow != NULL);
	CHARFORMAT cf = { 0 };
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = clr;
	cf.dwEffects = 0;
	(void)SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}

void txtBackColor(HWND hWindow, COLORREF clr) {
	assert(hWindow != NULL);
	CHARFORMAT2 cf{};
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BACKCOLOR;
	cf.crBackColor = clr;
	cf.dwEffects = 0;
	(void)SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}

DWORD CALLBACK EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
	char** localbuffer = (char**)dwCookie;
	assert(localbuffer != NULL);
	char* temppointer = 0;
	if (rtfindex == 0)
	{
		localbuffer[0] = (char*)SafeCalloc( (size_t)cb + 10,   sizeof(char));
	}
	else
	{
		temppointer = (char *)realloc(localbuffer, (size_t)rtfindex + cb + 10);
		if (temppointer == NULL)
		{
			CrashDumpFunction(61021, 1);
			return FALSE;
		}
		localbuffer[0] = temppointer;
	}
	int i = 0;
	assert(rtfindex >= 0);
	for (i = 0; i < cb && i < MAXINT; i++)
	{
		localbuffer[0][rtfindex + i] = (char)pbBuff[i];
	}
	*pcb = i;
	rtfindex += cb;
	return 0;
}

DWORD CALLBACK EditStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
	mcallbackinfo * mydata = (mcallbackinfo*)dwCookie;
	assert(mydata != NULL);
	HANDLE hFile = mydata->hFile;
	assert(hFile != INVALID_HANDLE_VALUE);
	OVERLAPPED overlapstruct = { 0 };
	overlapstruct.Offset = (DWORD)mydata->offset;

	DWORD NumberOfBytesRead = 0;
	if (!ReadFile(hFile, pbBuff, (DWORD)mydata->amountoread, &NumberOfBytesRead, &overlapstruct))
	{
		CrashDumpFunction(5733, 0);
		return 1;
	}
	*pcb = (LONG)NumberOfBytesRead;
	return 0;
}

//this is fontbox
INT_PTR CALLBACK FontBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	assert(hwnd != NULL);
	HWND hwndtemp = 0, hwndFont = 0, controlhwnd = 0;
	int value = 0, failcheck = 0, returnval = 0;
	if (lParam == 0)
		value = 1;
	int static fontvalue = 0;
	CHARFORMAT2 cfrm{};
	char tempstring[12] = { 0 };
	HDC hdc = { 0 };
	LOGFONT logfont = { 0 };
	char tempstring22[200] = "Cf1,Underline,Invert,Dash,DashDot,DashDotDot,Dotted,Double,DoubleWave,Hairline,HeavyWave,LongDash,None,Thick,ThickDash,ThickDashDot,ThickDashDotDot,ThickDotted,ThickLongDash,Wave,Word";
	switch (message)
	{
	case WM_INITDIALOG:
		FontBoxHwnd = hwnd;
		//fill the font type list
		hdc = safeGetDC(hwnd);
		assert(hdc != NULL);
		EnumFontFamiliesExW(hdc, &logfont, (FONTENUMPROCW)EnumFontFamilyProc, 0, 0);
		controlhwnd = SafeGetDlgItem(hwnd, IDC_FONTTYPE);
		for (int i = 0; i < fontamount - 1 && i < 1000; i++)
		{
			returnval = ComboBox_InsertString(controlhwnd, i, fontlist[i]);
			if (returnval == CB_ERR || returnval == CB_ERRSPACE)
				CrashDumpFunction(1187, 0);
		}
		hwndFont = SafeGetDlgItem(hwnd, IDC_EDITFONT);
		fontvalue = 8;
		safe_itoa_s(8, tempstring, 12, 10);
		SafeSetWindowTextA(hwndFont, tempstring);
		controlhwnd = SafeGetDlgItem(hwnd, IDC_FONTCOLOR);
		for (int i = 0; i < 138; i++)
		{
			returnval = ((int)(DWORD)SendMessageA((controlhwnd), 0x014A, (WPARAM)(int)(i), (LPARAM)(LPCTSTR)(colorsdata[i].colorname)));
			if (returnval == CB_ERR || returnval == CB_ERRSPACE)
				CrashDumpFunction(1187, 0);
		}
		controlhwnd = SafeGetDlgItem(hwnd, IDC_FONTBKG);
		for (int i = 0; i < 138; i++)
		{
			returnval = ((int)(DWORD)SendMessageA((controlhwnd), 0x014A, (WPARAM)(int)(i), (LPARAM)(LPCTSTR)(colorsdata[i].colorname)));
			if (returnval == CB_ERR || returnval == CB_ERRSPACE)
				CrashDumpFunction(1187, 0);
		}
		controlhwnd = SafeGetDlgItem(hwnd, IDC_FUNDERLINE);
		assert(controlhwnd != NULL);
		for (int i = 0, m = 0; i < 21; i++)
		{
			char stringtosend[30] = { 0 };
			_memccpy(stringtosend, tempstring22 + m, ',', 30);
			m += (int)strlen(stringtosend);
			returnval = ((int)(DWORD)SendMessageA((controlhwnd), 0x014A, (WPARAM)(int)(i), (LPARAM)(LPCTSTR)(stringtosend)));
			if (returnval == CB_ERR || returnval == CB_ERRSPACE)
				CrashDumpFunction(1187, 0);
		}
		SafeReleaseDC(hwnd, hdc);
		break;
	case WM_COMMAND:
		hwndtemp = SafeGetDlgItem(hwnd, LOWORD(wParam));
		assert(hwndtemp != NULL);
		switch (LOWORD(wParam))
		{//RADIOS
		case IDC_RBOLD:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				value = Button_GetCheck(hwndtemp);
				if (value == BST_UNCHECKED)
				{
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);							
					cfrm.dwMask = CFM_BOLD;
					cfrm.dwEffects = CFE_BOLD;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_BOLD;
					cfrm.dwEffects = 0;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				break;
			}
			break;
		case IDC_RITALIC:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				value = Button_GetCheck(hwndtemp);
				if (value == BST_UNCHECKED)
				{
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_ITALIC;
					cfrm.dwEffects = CFM_ITALIC;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_ITALIC;
					cfrm.dwEffects = 0;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
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
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_UNDERLINE;
					cfrm.dwEffects = CFM_UNDERLINE;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_UNDERLINE;
					cfrm.dwEffects = 0;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
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
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_STRIKEOUT;
					cfrm.dwEffects = CFM_STRIKEOUT;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_STRIKEOUT;
					cfrm.dwEffects = 0;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
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
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_OFFSET | CFM_SIZE;
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.yHeight /= 2;
					cfrm.yOffset = 100;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_OFFSET | CFM_SIZE;
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.yHeight *= 2;
					cfrm.yOffset = 0;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
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
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0001), 0L));
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_OFFSET | CFM_SIZE;
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.yHeight /= 2;
					cfrm.yOffset = -100;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
				else if (value == BST_CHECKED)
				{
					assert(TextBoxInput != NULL);
					((void)SendMessageW((hwndtemp), 0x00F1, (WPARAM)(int)(0x0000), 0L));
					cfrm.cbSize = sizeof(CHARFORMAT2);
					cfrm.dwMask = CFM_OFFSET | CFM_SIZE;
					(void)SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					cfrm.yHeight *= 2;
					cfrm.yOffset = 0;
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
					(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				}
			}
			break;
		case IDC_EDITFONT:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				assert(TextBoxInput != NULL);
				value = (int)SafeGetDlgItemInt(FontBoxHwnd, IDC_EDITFONT, &failcheck, FALSE);
				if (value == 0)
				{
					CrashDumpFunction(1423, 0);
				}
				(void)SendMessage(TextBoxHwnd, EM_SETFONTSIZE, (WPARAM)(-(fontvalue - value)), 0);
				fontvalue = value;
			}
			break;
		case IDC_FONTTYPE:
			switch (HIWORD(wParam))
			{
			case CBN_SELENDOK:
				assert(TextBoxInput != NULL);
				value = (int)SendMessage(hwndtemp, CB_GETCURSEL, 0, 0);
				cfrm.cbSize = sizeof(CHARFORMAT2);
				cfrm.dwMask = CFM_FACE;
				if (0 != wmemcpy_s(cfrm.szFaceName, 32, fontlist[value], 32))
				{
					CrashDumpFunction(1423, 0);
				}
				(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
				(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
			}
			break;
		case IDC_FONTCOLOR:
			switch (HIWORD(wParam))
			{
			case CBN_SELENDOK:
				assert(TextBoxInput != NULL);
				value = (int)SendMessage(hwndtemp, CB_GETCURSEL, 0, 0);
				cfrm.cbSize = sizeof(CHARFORMAT2);
				cfrm.dwMask = CFM_COLOR;
				cfrm.crTextColor = colorsdata[value].RGBcolor;
				cfrm.dwEffects = 0;
				(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
				(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
				break;
			}
			break;
		case IDC_FONTBKG:
			switch (HIWORD(wParam))
			{
			case CBN_SELENDOK:
				assert(TextBoxInput != NULL);
				value = (int)SendMessage(hwndtemp, CB_GETCURSEL, 0, 0);
				cfrm.cbSize = sizeof(CHARFORMAT2);
				cfrm.dwMask = CFM_BACKCOLOR;
				cfrm.crBackColor = colorsdata[value].RGBcolor;
				cfrm.dwEffects = 0;
				(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
				(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
			}
			break;
		case IDC_FUNDERLINE:
			switch (HIWORD(wParam))
			{
			case CBN_SELENDOK:
				value = (int)SendMessage(hwndtemp, CB_GETCURSEL, 0, 0);//enumerate underline types
				cfrm.cbSize = sizeof(CHARFORMAT2);
				cfrm.dwMask = CFM_UNDERLINETYPE;
				switch (value)
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
				assert(TextBoxInput != NULL);
				(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfrm);
				(void)SendMessage(TextBoxInput, EM_SETCHARFORMAT, 0, (LPARAM)&cfrm);
			}
			break;
		default:
			return FALSE;
		}
	return 0;
	}
	return 0;
}

int CALLBACK EnumFontFamilyProc(const LOGFONT* lpelfe, const void* lpntme, DWORD      FontType, LPARAM     lParam)
{
	assert(lpelfe != NULL);
	static int amount = 3;
	static int index = 0;
	char* tempppointer = 0;
	WCHAR fontname[100] = { 0 };
	if (0 != wmemcpy_s(fontname, 100, lpelfe->lfFaceName, 32))
	{
		CrashDumpFunction(1587, 0);
		return FALSE;
	}
	if (fontname[0] == '\0')
		return 0;
#pragma warning(disable:4047)
	if (fontlist == NULL)
	{
		tempppointer = (char*)SafeCalloc(sizeof(WCHAR*), 2);
		fontlist = (WCHAR**)tempppointer;
	}
	else
	{
		tempppointer = (char*)realloc(fontlist, sizeof(WCHAR*) * amount);
		if (tempppointer == NULL)
		{
			CrashDumpFunction(1603, 1);
			return FALSE;
		}
		fontlist = (WCHAR**)tempppointer;
		fontlist[index] = NULL;
	}
#pragma warning(default:4047)
	assert(fontlist[index] == NULL);
	fontlist[index] = (WCHAR*)SafeCalloc(32, sizeof(WCHAR));
	if (0 != wmemcpy_s(fontlist[index], 100, fontname, 32))
	{
		CrashDumpFunction(1614, 0);
		return FALSE;
	}
	index++;
	amount++;
	fontamount = index + 1;
	return 1;
}