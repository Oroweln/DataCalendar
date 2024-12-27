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

INT_PTR CALLBACK TextBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH textcolor = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		TextBoxHwnd = (GetDlgItem(hwnd, TEXTBOXINPUT));//old one was named TEXTBOXINPUT in case something breaks bad
		assert(TextBoxHwnd != NULL);
		SetWindowSubclass(TextBoxHwnd, TextBoxInputSbc, 0, 0);
		txtBackColor(TextBoxHwnd, textbackground);
		COLORREF stupidtextcolor = RGB(GetRValue(~textbackground), GetGValue(~textbackground), GetBValue(~textbackground));
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
				TextHeapRemaining = pchinputbuffermemory - Edit_GetTextLength(TextBoxInput);
				assert(buttonmarks != NULL);
				(void)SendMessageA(buttonmarks[2], DATEUPDATETICK, 0, 0);
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
	int zoomden = 1, zoomnom = 1;
	switch (uMsg)
	{
	case IDDATECHANGE:
		assert(amountread >= 0);
		DateTestBufferLoad(&amountread, &overlapstruct, &appendlocationindex, &datepresentflag);
		if (datepresentflag == TRUE)
		{
			hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(RTForTXT == 1)
			{
				char* readbuffer = SafeCalloc(  (size_t)amountread + 1,    sizeof(char));
				overlapstruct.Offset = appendlocationindex;
				if (FALSE == ReadFile(hFile, readbuffer, amountread, NULL, &overlapstruct))
				{
					CrashDumpFunction(3168, 0);
					return FALSE;
				}
				assert(amountread >= 0);
				SetWindowTextA(hWnd, readbuffer);
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
				(void)SendMessage(hWnd, EM_STREAMIN, SF_RTF, (LPARAM)&es);
			}
			SafeCloseHandle(hFile);
		}
		else
			SetWindowTextA(hWnd, "");
		break;	
	case TEXTBOXCOLORCHANGE:
		assert(TextBoxHwnd);
		txtBackColor(TextBoxHwnd, textbackground);
		COLORREF textboxcolor = RGB(GetRValue(~textbackground), GetGValue(~textbackground), GetBValue(~textbackground));
		txtColor(TextBoxHwnd, textboxcolor);
		break;
	case WM_RBUTTONDOWN:
		hPopMenu = LoadMenuA(GetModuleHandle(NULL), MAKEINTRESOURCEA(TXTPOPMENU));
		assert(hPopMenu != NULL);
		HMENU hTrackPopMenu = GetSubMenu(hPopMenu, 0);
		assert(hTrackPopMenu != NULL);
		POINT pt = { 0 };
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		ClientToScreen(hWnd, (LPPOINT)&pt);
		//hPopMenu = CreatePopupMenu();
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
			break;
		case ID_CHANGEFONT:
			thefont.hwndOwner = SafeGetParent(hWnd);
			assert(thefont.hwndOwner != NULL);
			thefont.lpLogFont = (LPLOGFONTA)&lgf;
			thefont.Flags = CF_EFFECTS;
			thefont.lStructSize = sizeof(CHOOSEFONT);
			LPCHOOSEFONT lpthefont = (LPCHOOSEFONT)&thefont;
			ChooseFont(lpthefont);
			cf.bCharSet = thefont.lpLogFont->lfCharSet;
			cf.bPitchAndFamily = thefont.lpLogFont->lfPitchAndFamily;
			cf.cbSize = sizeof(CHARFORMAT);
			cf.crTextColor = thefont.rgbColors;
			DWORD flagset = 0;
			if (thefont.lpLogFont->lfUnderline > 0)
				flagset += CFE_UNDERLINE;
			if (thefont.lpLogFont->lfItalic > 0)
				flagset += CFE_ITALIC;
			if (thefont.lpLogFont->lfStrikeOut > 0)
				flagset += CFE_STRIKEOUT;
			if (thefont.lpLogFont->lfWeight == 700)
				flagset += CFE_BOLD;
			cf.dwMask = CFM_ALL;
			strcpy_s(cf.szFaceName, 32,thefont.lpLogFont->lfFaceName);
			cf.yHeight = thefont.lpLogFont->lfHeight;
			cf.yOffset = 0;
			(void)SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			break;
		case ID_ZOOMIN:
			(void)SendMessage(hWnd, EM_GETZOOM, (WPARAM)(&zoomnom), (LPARAM)(&zoomden));
			if (zoomnom == 0)
			{
				zoomnom = 100;
				zoomden = 100;
			}
			Edit_SetZoom(hWnd, zoomnom+10, zoomden);
			break;
		case ID_ZOOMOUT:
			Edit_GetZoom(hWnd, &zoomnom, &zoomden);
			if (zoomnom == 0)
			{
				zoomnom = 100;
				zoomden = 100;
			}
			Edit_SetZoom(hWnd, zoomnom-10, zoomden);
			break;
		}
		status = GetLastError();
		DestroyMenu(hPopMenu);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 0x53://"S" key
			if (CtrlPressed)//save the text into textdata
			{
				assert(TextBoxInput);
				int textlength = 0;
				char* pchInputBuf = { 0 };
				char** PPch = { 0 };
					if(RTForTXT == 1)
					{
						textlength = GetWindowTextLengthA(TextBoxInput);
						pchInputBuf = SafeCalloc(  (size_t)textlength + 1,    sizeof(char));
						if (pchInputBuf == NULL)
							break;
						GetWindowTextA((TextBoxInput), (pchInputBuf), (textlength + 1));
					}
					else if(RTForTXT == 0)
					{
						EDITSTREAM es = { 0 };
						PPch = SafeCalloc(  2,    sizeof(char*));
						es.dwCookie = (DWORD_PTR)PPch;
						es.pfnCallback = EditStreamOutCallback;
						(void)SendMessage(hWnd, EM_STREAMOUT, SF_RTF, (LPARAM)&es);
						pchInputBuf = PPch[0];
						rtfindex = 0;
					}
					assert(amountread >= 0);
					DateTestBufferLoad(&amountread, &overlapstruct, &appendlocationindex, &datepresentflag);
					savingFunction(&appendlocationindex, pchInputBuf, &overlapstruct, &amountread, datepresentflag);
					free(pchInputBuf);
					free(PPch);
			}
			break;
		case 0x55://"U" key
			if(CtrlPressed)//undo
				(void)SendMessage(hWnd, EM_REDO, 0, 0);
			break;
		case VK_CONTROL:
			CtrlPressed = TRUE;
			break;
		}
		break;
	case WM_KEYUP:
		if (wParam == VK_CONTROL)
			CtrlPressed = FALSE;
		break;
	default:
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

int DateTestBufferLoad(int* amountread, OVERLAPPED* overlapstruct, int* appendlocationindex, BOOL* datepresent)
{
	assert(amountread != NULL);
	assert(overlapstruct != NULL);
	int xtoalloc = 0;
	HANDLE hFile = { 0 };
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL, NULL);
	int amounttowrite = 0, oldlocindex = 0;
	BOOL randomflag = FALSE;
	char* selecteddate1 = SafeCalloc(  25,    sizeof(char));
	selecteddate1 = DateTextShow(selecteddate1);
	*amountread = GetFileSize(hFile, NULL);
	if (*amountread == INVALID_FILE_SIZE)
	{
		CrashDumpFunction(272, 0);
	}
	char * readbuffer = SafeCalloc(  (size_t)(*amountread) + 1,    sizeof(char));//MALLOCING HERE IS TEMP WORKAROUND TILL WE MAKE PROGRAM FULLY FUNCTIONAL, to run from stack.
	randomflag = ReadFile(hFile, readbuffer, *amountread, &((DWORD)amounttowrite), overlapstruct);//check if the data for a date is present
	if (randomflag == 0 || readbuffer == NULL)
	{
		SafeCloseHandle(hFile);
		free(readbuffer);
		readbuffer = NULL;
		CrashDumpFunction(270,0);
		return 0;
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
	return 0;
}

BOOL savingFunction(int* appendlocationindex, char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, BOOL datepresent)
{//we need to push all data forwards or backwards depending if we are ammending smaller sized new dataset or larger sized.
	assert(pchInputBuf != NULL);
	assert(appendlocationindex != NULL);
	HANDLE hFile = { 0 };//errorint
	BOOL EmptyDataFlag = TRUE;
	size_t oldstringlength = *amountread;//amountread here is amount of data within the date selected
	//figure out if the data to be applied is less or more then last one that was, or perhaps same size.
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		CrashDumpFunction(227,0);
		return 0;
	}
	*amountread = GetFileSize(hFile, NULL);//amountread will be here used as total length
	if (*amountread == INVALID_FILE_SIZE)
	{
		CrashDumpFunction(231, 0);
		return FALSE;
	}
	//to do this get already existing data's size
	char * tempcharbuf = SafeCalloc(  (size_t)*amountread+5,   sizeof(char));
	//test read
	if (FALSE == ReadFile(hFile, tempcharbuf, *amountread, NULL, overlapstruct))
	{
		int errorval = GetLastError();
		if ((*amountread != 0) && (errorval != 38))//if amountread is 0, that means file is empty, hence why error was thrown.
		{
			SafeCloseHandle(hFile);
			CrashDumpFunction(246,0);
			return FALSE;
		}
		else if (errorval == 38 && *amountread > 0)//the amountread is not empty but the date itself is empty and standing at the border of EOF, effectively offset is at EOF, we fix this by doing nothing
			;
	}
	free(tempcharbuf);
	OVERLAPPED noverlapstruct = { 0 };
	noverlapstruct.Offset = *appendlocationindex;//point at the beggining of data
	SafeCloseHandle(hFile);
	OVERLAPPED ewreoverlapstruct = { 0 };
	ewreoverlapstruct.Offset = *appendlocationindex;
	size_t strLength = strnlen_s(pchInputBuf, pchinputbuffermemory);
	if (datepresent == FALSE && strLength>0)
	{//append date mark
		char * selecteddate1 = SafeCalloc(  sizeof(char),    25);
		selecteddate1 = DateTextShow(selecteddate1);
		if(ordereddatasave == 1)
			DateWrite(appendlocationindex, selecteddate1);
		else 
		{
			hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			assert(hFile != INVALID_HANDLE_VALUE);
			SafewriteFile(hFile, selecteddate1, 12, NULL, &ewreoverlapstruct);
			SafeCloseHandle(hFile);
		}
		hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		*amountread = GetFileSize(hFile, NULL);//update amount read
		if (*amountread == INVALID_FILE_SIZE)
		{
			CrashDumpFunction(247, 0);
			return FALSE;
		}
		noverlapstruct.Offset = (DWORD)(strlen(selecteddate1) + *appendlocationindex);//point at the beggining of data instead of mark data
		free(selecteddate1);
		SafeCloseHandle(hFile);
	}
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
	char* readbuffer = SafeCalloc(  *amountread,    sizeof(char));
	char* readbuffer2 = SafeCalloc((size_t)(*amountread + strLength + 1), sizeof(char));
	assert(pchInputBuf != NULL);
	if (FALSE == ReadFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct))
	{
		free(readbuffer);
		free(readbuffer2);
		readbuffer2 = readbuffer = NULL;
		CrashDumpFunction(251,0);
		return -1;
	}
	if ((*amountread - overlapstruct->Offset) == 0)//we need to append data at EOF
	{
		SafewriteFile(hFile, pchInputBuf, (DWORD)strLength, &((DWORD)charlength), overlapstruct);
	}
	else//we need to push data forward and then append data
	{
		
		_memccpy(readbuffer2 + strLength, readbuffer + *oldstringlength, 0, (size_t)*amountread - overlapstruct->Offset);//push daata forward with help of offset, readbuffer+*oldstringlength ensure that the already present "oldstring" isnt pushed forwards but overwritten
		_memccpy(readbuffer2, pchInputBuf, 0, strLength);//append data at the newly created space
		SafewriteFile(hFile, readbuffer2, (DWORD)(((unsigned long long)(*amountread) - overlapstruct->Offset) + ((unsigned long long)strLength - *oldstringlength)), &((DWORD)charlength), overlapstruct);
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
	char* readbuffer = SafeCalloc((size_t)(*amountread + strLength), sizeof(char));
	char* readbuffer2 = SafeCalloc(  (size_t)(*amountread+strLength),    sizeof(char));
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
		SafewriteFile(hFile, pchInputBuf, (DWORD)strLength, &((DWORD)charlength), overlapstruct);
	}
	else//we are pasting in middle of data
	{
		_memccpy(readbuffer2 + strLength, readbuffer+*oldstringlength, 0, *amountread-*oldstringlength);//copy only date in front of oldstringlength
		_memccpy(readbuffer2, pchInputBuf, 0, strLength);//paste into remaining space for the date
		SafewriteFile(hFile, readbuffer2, (*amountread - overlapstruct->Offset) - (DWORD)(*oldstringlength - strLength), &((DWORD)charlength), overlapstruct);
	}
	//squeeze out extra date down here now
	SetFilePointer(hFile, (LONG)(strLength - *oldstringlength), NULL, FILE_END);
	SetEndOfFile(hFile);
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
	overlapstruct->Offset = *appendlocationindex-12; //points to the beggining of the date-mark, so we can delete it as well, since all data is being deleted.
	char* readbuffer = SafeCalloc(  *amountread,    sizeof(char));//for reading the txt file
	char* readbuffer2 = SafeCalloc(  *amountread,    sizeof(char));//for new data to be written
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
	
	SafewriteFile(hFile, readbuffer2, (*amountread - overlapstruct->Offset) - (DWORD)(*oldstringlength + 12), &((DWORD)charlength), overlapstruct);
	SetFilePointer(hFile, (LONG)(-12 - *oldstringlength), NULL, FILE_END);
	SetEndOfFile(hFile);
	free(readbuffer);
	free(readbuffer2);
	readbuffer2 = readbuffer = NULL;
	return TRUE;
}

BOOL DataWriteOver(char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile)
{
	assert(pchInputBuf);
	assert(hFile != INVALID_HANDLE_VALUE);
	char* readbuffer = SafeCalloc(  *amountread,    sizeof(char));
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
	filesize = GetFileSize(hFile, NULL);
	if (filesize == INVALID_FILE_SIZE)
	{
		CrashDumpFunction(383, 0);
		return FALSE;
	}
	readbuffer = SafeCalloc(  (long long)filesize+20,    sizeof(char));//20 for dateset

	if (FALSE == ReadFile(hFile, readbuffer, filesize, NULL, &Overlapped))
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
	char* readbuffer2 = SafeCalloc(  filesize,    sizeof(char));
	_memccpy(readbuffer2, readbuffer, 0, filesize);
	if ((readbuffer2 + *appendindexlocation) == 0)
	{
		CrashDumpFunction(5851, 0);
		return FALSE;
	}
	_memccpy(readbuffer + datelength + *appendindexlocation, readbuffer2+ *appendindexlocation, 0, (size_t)filesize- *appendindexlocation +datelength);
	_memccpy(readbuffer + *appendindexlocation, dateset, 0, datelength);
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
		if (-1 == sprintf_s(selecteddate1, 20, "*%d :%d :%d*", datedayindex, datemonthindex, dateyear))
		{
			CrashDumpFunction(50, 0);
		}
	}
	else if (datedayindex < 10)
	{
		if (-1 == sprintf_s(selecteddate1, 20, "*%d :%d:%d*", datedayindex, datemonthindex, dateyear))
		{
			CrashDumpFunction(50, 0);
		}
	}
	else if (datemonthindex < 10)
	{
		if (-1 == sprintf_s(selecteddate1, 20, "*%d:%d :%d*", datedayindex, datemonthindex, dateyear))
		{
			CrashDumpFunction(50, 0);
		}
	}
	else
	{
		if (-1 == sprintf_s(selecteddate1, 20, "*%d:%d:%d*", datedayindex, datemonthindex, dateyear))
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
	CHARFORMAT2 cf = { 0 };
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
		localbuffer[0] = SafeCalloc( (size_t)cb + 10,   sizeof(char));
	}
	else
	{
		temppointer = realloc(localbuffer, (size_t)rtfindex + cb + 10);
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
		localbuffer[0][rtfindex + i] = pbBuff[i];
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
	overlapstruct.Offset = mydata->offset;

	DWORD NumberOfBytesRead = 0;
	if (!ReadFile(hFile, pbBuff, mydata->amountoread, &NumberOfBytesRead, &overlapstruct))
	{
		//handle errors
		return 1;
		// or perhaps return GetLastError();
	}
	*pcb = NumberOfBytesRead;
	return 0;
}

