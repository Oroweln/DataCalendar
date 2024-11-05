#include "framework.h"
//fix the problem with \r and \n is the reason behind, i think, all failures and pitfals of the textbox
/*
1. Implement scrolling of textbox, separate h into "graphical h" and "logical h". Graphical ought not to use scrollvalue, but logical should.[X]
2. On top of this make it put '\n' automatically when you reach xmaxchar[X]
3. After all of this, get the whole code as much as possible accomodate to TenRules
3a. Fix retard copy-pasting multi-line, single line, etc. Thhere is problem with it copy past "after" first line thus making whole thing inept. Then it goes on to fail copying totally.
4. Make month boxes auto-center on current month upon starting
5. Make a input box, to input desired date,month and year, or just to quick search a month.
6. To the TopTen thing again
7. Begin the UI phase
8. end the tyranny of PchInputBuff, stop using it as a two-dimensional array. Rework the program in such a way it can be used as one dimensional.
*/
#define BUFSIZE 64000
#define SHIFTED 0x8000 
#define yMAXLINES 30
#define xMAXCHARS 500
size_t TextHeapRemaining = 0;
long long xAllocamount = 0;
long long yAllocamount = 0;
static size_t TotalAmountAllocatedXY = 0;
COLORREF textbackground = RGB(255,255,255);
char* rtfbuffer = { 0 };
int rtfindex = 0;
typedef struct mcallbackinfo
{
	HFILE hFile;
	int amountoread;
	int offset;
}mcallbackinfo;
POINT GlobalPolygonPoints[100] = { 0 };
int pointsvarglobal = 0;
int TrustedIndex2 = 0;//stupid idiotic ugly index, TrustedIndex is a ghost of TrustedIndex that is homs at Mycalendar.c i think
BOOL pastingflag = FALSE;//used for calls to space function from the paste
BOOL pasteunderselection = FALSE;//global flag for TextBox, turned on when program enters "Dragging loop" and off whenever you click left-click the textbox regardless if you dragged or not.
BOOL SpaceAllocFlag = TRUE;//Used for communicating to space to allocate or not when using it recursively
LRESULT TextBoxInputSbc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

int DateTestBufferLoad(int* amountread, OVERLAPPED* overlapstruct, int* appendlocationindex, BOOL* datepresent);
BOOL savingFunction(int* appendlocationindex, char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, BOOL datepresent);
BOOL LargeDataWrite(char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile, size_t* oldstringlength);
BOOL SmallDataWrite(char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile, size_t* oldstringlength);
BOOL DataWriteOver(char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile);

BOOL alphnumBufCheck(BOOL* EmptyDataFlag, char * pchInputBuf[]);
int newlinecounter(char* cStringP, int xlength);

void CreateAHorizontalScrollBar(HWND hwndParent, int sbHeight);

int WINAPI OnContextMenu(HWND hwnd, int x, int y);
int APIENTRY DisplayContextMenu(HWND hwnd, POINT pt);

int* GetLastChar(char* pchInputBuf[], int lastchar[]);
int DeAllocPOINTx(size_t amount, POINT* buffer);
BOOL DateWrite(int* appendindexlocation, char* dateset);
char* DateTextShow(char* selecteddate1);

void txtBold(HWND hWindow); //for bold-text
void txtUnderlined(HWND hWindow); //for underlined-text
void txtItalic(HWND hWindow); //for itaic-text
void txtStrikeout(HWND hWindow); //for strikeout-text
void Subscript(HWND hWindow); //for Sub-text
void Superscript(HWND hWindow); //for Super-text
void SetFont(HWND hWindow, const char* Font); //define the fontname in the ""
void FontSize(HWND hWindow, int size); //set the fontsize 
void txtColor(HWND hWindow, COLORREF clr); //set the text color
void txtBackColor(HWND hWindow, COLORREF clr); //Set the textbackgroundcolor

DWORD CALLBACK EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb);
void SaveRichTextToFile(HWND hWnd, LPCWSTR filename);
DWORD CALLBACK EditStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb);
void ReadRichTextFromFile(HWND hWnd, LPCWSTR filename);

HWND TextBoxHwnd = NULL;
BOOL datechangecheck = FALSE; //True if date has been changed, hence textfile needs to be read again! 
BOOL ArrowLineScroll = FALSE;



/*
TODO: Selection, so one can cut/copy/paste replace etc.. aka mouse selection system + Copy-Pasting with it, and make it compatible with scrolling, and fix it to work.
	- in standard non scroll value reality, it still failed to copy paste, first and/or last letter[X]
	- implement scrollvalue and stress test, ensure it works.[X]
	- Fix inline pasting[X]
	- Fix Up and down, left and right arrows, not accounting for rules around '\n' and '\r', besides that, up arrow doesnt work at all lol[X]
TODO: Reorganize the whole code so it doesnt look like shit, before adding mouse selection and stuff.
	- use the tenrule as  the guide.
TODO: Create MouseTextSelection(HWND hwnd, int * CaretY, int * CaretX, int cyChar) to implement input based drop-box for textbox
TODO: reffer to the beginning of the code for further objectives
1. create a custom brush(by creating a custom bitmap that is transparent)
2. paste the bitmap to the polygon that represent according text selection
3. REPRESENT POLYGON PROPERLY TO MAKE THING WORK.
*/

INT_PTR CALLBACK TextBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL CtrlPressed = FALSE;
	BOOL nigga = TRUE;
	int fagsdgadg = 0;
	int id = 0;
	static HBRUSH hbrbackground = { 0 }, textcolor = { 0 };
	HDC myHdc = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
		fagsdgadg = GetLastError();
		TextBoxHwnd = (GetDlgItem(hwnd, TEXTBOXINPUT));//old one was named TEXTBOXINPUT in case something breaks bad
		fagsdgadg = GetLastError();
		nigga = SetWindowSubclass(TextBoxHwnd, TextBoxInputSbc, 0, 0);
		txtBackColor(TextBoxHwnd, textbackground);
		COLORREF nigger2 = RGB(GetRValue(~textbackground), GetGValue(~textbackground), GetBValue(~textbackground));
		txtColor(TextBoxHwnd, nigger2);
		break;
		/*

All you need is to set the required color in control's device context and pass an HBRUSH with same color in WM_CTLCOLOREDIT message. If you want to change both foreground & background colors, use SetTextColor t0 change the text color. But you must pass the background color HBRUSH. But if you want to change the text color only, then you must pass a DC_BRUSH with GetStockObject function.
*/
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case TEXTBOXINPUT:
			switch(HIWORD(wParam))
			{
			case EN_UPDATE:
				TextHeapRemaining = pchinputbuffermemory - Edit_GetTextLength(TextBoxInput);
				SendMessageA(buttonmarks[2], DATEUPDATETICK, 0, 0);
			default:
				return TRUE;
			}
			return TRUE;
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
	BOOL stupidflag = FALSE, datepresentflag = 0;
	int appendlocationindex = 0, amountread = 0;
	OVERLAPPED overlapstruct = { 0 };
	HANDLE hFile = { 0 };
	CHARFORMAT static cf = { 0 };
	LOGFONT lgf = { 0 };
	HMENU hPopMenu = { 0 };
	HDC temphdc = { 0 };
	CHARRANGE crc = { 0 };
	CHARFORMAT Chrfm = { 0 };
	CHOOSEFONT thefont = { 0 };
	int zoomden = 1, zoomnom = 1;
	switch (uMsg)
	{
	case IDDATECHANGE:
		DateTestBufferLoad(&amountread, &overlapstruct, &appendlocationindex, &datepresentflag);
		if (datepresentflag == TRUE)
		{
			hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(RTForTXT == 1)
			{
				char* readbuffer = calloc(amountread + 1, sizeof(char));
				overlapstruct.Offset = appendlocationindex;
				ReadFile(hFile, readbuffer, amountread, NULL, &overlapstruct);
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
				SendMessage(hWnd, EM_STREAMIN, SF_RTF, (LPARAM)&es);
			}
			CloseHandle(hFile);
		}
		else
			SetWindowTextA(hWnd, "");
		break;	
	case TEXTBOXCOLORCHANGE:
		txtBackColor(TextBoxHwnd, textbackground);
		COLORREF nigger2 = RGB(GetRValue(~textbackground), GetGValue(~textbackground), GetBValue(~textbackground));
		txtColor(TextBoxHwnd, nigger2);
		break;
	case WM_RBUTTONDOWN:
		hPopMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCEA(TXTPOPMENU));
		HMENU hTrackPopMenu = GetSubMenu(hPopMenu, 0);
		POINT pt = { 0 };
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		ClientToScreen(hWnd, (LPPOINT)&pt);
		//hPopMenu = CreatePopupMenu();
		int status = TrackPopupMenuEx(hTrackPopMenu, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_NOANIMATION | TPM_HORIZONTAL, pt.x, pt.y, hWnd, NULL);
		switch (status)
		{
		case ID_COPY:
			SendMessage(hWnd, WM_COPY, 0, 0);
			break;
		case ID_PASTE:
			SendMessage(hWnd, WM_PASTE, 0, 0);
			break;
		case ID_CUT:
			SendMessage(hWnd, WM_CUT, 0, 0);
			break;
		case ID_DELETE:
			SendMessage(hWnd, WM_CLEAR, 0, 0);
			break;
		case ID_SELECTALL:
			SendMessage(hWnd, EM_SETSEL, 0, -1);
			break;
		case ID_UNDO:
			SendMessage(hWnd, EM_UNDO, 0, 0);
			break;
		case ID_REDO:
			SendMessage(hWnd, EM_REDO, 0, 0);
			break;
		case ID_GETFONT:
			cf.cbSize = sizeof(CHARFORMAT);
			SendMessage(hWnd, EM_GETCHARFORMAT, SCF_SELECTION, &cf);
			break;
		case ID_APPLYFONT:
			cf.cbSize = sizeof(CHARFORMAT);
			SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, &cf);
			break;
		case ID_REMOVEFONT:
			break;
		case ID_CHANGEFONT:
			thefont.hwndOwner = GetParent(hWnd);
			thefont.lpLogFont = &lgf;
			thefont.Flags = CF_EFFECTS;
			thefont.lStructSize = sizeof(CHOOSEFONT);
			LPCHOOSEFONT lpthefont = &thefont;
			ChooseFont(lpthefont);
			int nigger = GetLastError();
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
			SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, &cf);
			break;
		case ID_ZOOMIN:
			SendMessage(hWnd, EM_GETZOOM, (WPARAM)(&zoomnom), (LPARAM)(&zoomden));
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
	//case WM_LBUTTONUP:
		//SendMessage(TextBoxInput, EM_EXGETSEL, 0, &crc);
		//Chrfm.cbSize = sizeof(CHARFORMAT);
		//Chrfm.dwMask = CFM_ALL;
		//SendMessage(TextBoxInput, EM_GETCHARFORMAT, SCF_SELECTION, &Chrfm);
		//SetDlgItemInt(FontBoxHwnd, IDC_EDITFONT, Chrfm.yHeight/20, FALSE);
		//break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 0x53://"S" key
			if (CtrlPressed)//save the text into textdata
			{
				int textlength = 0;
				char* pchInputBuf = { 0 };
				char** PPch = { 0 };
					if(RTForTXT == 1)
					{
						textlength = GetWindowTextLengthA(TextBoxInput);
						pchInputBuf = calloc(textlength + 1, sizeof(char));
						if (pchInputBuf == NULL)
							break;
						GetWindowTextA((TextBoxInput), (pchInputBuf), (textlength + 1));
					}
					else if(RTForTXT == 0)
					{
						EDITSTREAM es = { 0 };
						PPch = calloc(2, sizeof(char*));
						es.dwCookie = PPch;
						es.pfnCallback = EditStreamOutCallback;
						SendMessage(hWnd, EM_STREAMOUT, SF_RTF, (LPARAM)&es);
						pchInputBuf = PPch[0];
						rtfindex = 0;
					}
					DateTestBufferLoad(&amountread, &overlapstruct, &appendlocationindex, &datepresentflag);
					savingFunction(&appendlocationindex, pchInputBuf, &overlapstruct, &amountread, datepresentflag);
					free(pchInputBuf);
					free(PPch);
			}
			break;
		case 0x55://"U" key
			if(CtrlPressed)//undo
				SendMessage(hWnd, EM_REDO, 0, 0);
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
	int xtoalloc = 0, ytoalloc = 0;
	HANDLE hFile = { 0 };
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL, NULL);
	int amounttowrite = 0, oldlocindex = 0;
	BOOL randomflag = FALSE;
	char* selecteddate1 = calloc(25, sizeof(char));
	selecteddate1 = DateTextShow(selecteddate1);
	*amountread = GetFileSize(hFile, NULL);
	char * readbuffer = calloc((size_t)(*amountread) + 1, sizeof(char));//MALLOCING HERE IS TEMP WORKAROUND TILL WE MAKE PROGRAM FULLY FUNCTIONAL, to run from stack.
	randomflag = ReadFile(hFile, readbuffer, *amountread, &((DWORD)amounttowrite), overlapstruct);//check if the data for a date is present
	if (randomflag == 0 || readbuffer == NULL)
	{
		CloseHandle(hFile);
		free(readbuffer);
		readbuffer = NULL;
		CrashDumpFunction(270,0);
		return 0;
	}
	oldlocindex = *appendlocationindex;
	for (int i = 0; i < *amountread && readbuffer[i] != '\0'; i++)//this loop will start reading when it reaches the appropriate data, and stop when it finishes reading the appropriate data from textdata
	{
		*appendlocationindex = i;//this value is the beggining of the text following the date within dataread
		if (readbuffer[i] == specialchar[0] && selecteddate1 != NULL)//specialchar[0] symbolizes posibility of beggining of the date, hence start checking for whole string afterwards below if it matches the selecteddate
		{
			for (int z = 1; (z + i <= *amountread) && (selecteddate1[z] == readbuffer[i + z]); z++)//date checking
			{
				if (readbuffer[i + z] == specialchar[0] || readbuffer[i + z] == '\0')//we have reached second start without quiting the loop, ie. date is matching
				{//here we will allocate all memory necessary. Loadingdataintobuffer will input actual data
					*datepresent = TRUE;
					*appendlocationindex = z + i + 1;//to the beginning of text
					for (int k = *appendlocationindex; k<*amountread && readbuffer[k] != specialchar[0] && readbuffer[k] != '\0'; k++, xtoalloc++)
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
	CloseHandle(hFile);
	*amountread = xtoalloc;//+ytoalloc cause we used window /r/n
	return 0;
}

BOOL savingFunction(int* appendlocationindex, char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, BOOL datepresent)
{//we need to push all data forwards or backwards depending if we are ammending smaller sized new dataset or larger sized.
	HANDLE hFile = { 0 };
	BOOL EmptyDataFlag = TRUE;
	size_t oldstringlength = *amountread;//amountread here is amount of data within the date selected
	//figure out if the data to be applied is less or more then last one that was, or perhaps same size.
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		CrashDumpFunction(227,0);
		return 0;
	}
	int faggotcancer = GetLastError();
	*amountread = GetFileSize(hFile, NULL);//amountread will be here used as total length
	faggotcancer = GetLastError();
	//to do this get already existing data's size
	char * tempcharbuf = calloc(*amountread+5,sizeof(char));
	if (tempcharbuf == NULL)
	{
		CrashDumpFunction(247,0);
		return FALSE;
	}
	//test read
	if (FALSE == ReadFile(hFile, tempcharbuf, *amountread, NULL, overlapstruct))
	{
		int errorniggas = GetLastError();
		if ((*amountread != 0) && (errorniggas != 38))//if amountread is 0, that means file is empty, hence why error was thrown.
		{
			CloseHandle(hFile);
			CrashDumpFunction(246,0);
			return FALSE;
		}
		else if (errorniggas == 38 && *amountread > 0)//the amountread is not empty but the date itself is empty and standing at the border of EOF, effectively offset is at EOF, we fix this by doing nothing
			;
	}
	free(tempcharbuf);
	OVERLAPPED noverlapstruct = { 0 };
	noverlapstruct.Offset = *appendlocationindex;//point at the beggining of data
	faggotcancer = GetLastError();
	CloseHandle(hFile);
	OVERLAPPED ewreoverlapstruct = { 0 };
	ewreoverlapstruct.Offset = *appendlocationindex;
	faggotcancer = GetLastError();
	size_t strLength = strnlen_s(pchInputBuf, pchinputbuffermemory);
	if (datepresent == FALSE && strLength>0)
	{//append date mark
		char * selecteddate1 = calloc(sizeof(char), 25);
		selecteddate1 = DateTextShow(selecteddate1);
		if(ordereddatasave == 1)
			DateWrite(appendlocationindex, selecteddate1);
		else 
		{
			hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			faggotcancer = WriteFile(hFile, selecteddate1, 12, NULL, &ewreoverlapstruct);
			faggotcancer = GetLastError();
			CloseHandle(hFile);
		}
		hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		*amountread = GetFileSize(hFile, NULL);//update amount read
		noverlapstruct.Offset = strlen(selecteddate1) + *appendlocationindex;//point at the beggining of data instead of mark data
		free(selecteddate1);
		CloseHandle(hFile);
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
	CloseHandle(hFile);
	hFile = NULL;
	return TRUE;
}

BOOL LargeDataWrite(char* pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile, size_t* oldstringlength)
{
	int shit = 0;
	char* readbuffer = calloc(*amountread, sizeof(char));
	char* readbuffer2 = calloc(*amountread + strLength + 1, sizeof(char));
	if (readbuffer == NULL || readbuffer2 == NULL || pchInputBuf == NULL)
	{
		CrashDumpFunction(250,1);
		return FALSE;
	}
	if (FALSE == ReadFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct))
	{
		int faggotnigger = GetLastError();
		free(readbuffer);
		free(readbuffer2);
		readbuffer2 = readbuffer = NULL;
		CrashDumpFunction(251,0);
		return -1;
	}
	if ((*amountread - overlapstruct->Offset) == 0)//we need to append data at EOF
	{
		WriteFile(hFile, pchInputBuf, strLength, &((DWORD)shit), overlapstruct);
	}
	else//we need to push data forward and then append data
	{
		
		_memccpy(readbuffer2 + strLength, readbuffer + *oldstringlength, 0, *amountread - overlapstruct->Offset);//push daata forward with help of offset, readbuffer+*oldstringlength ensure that the already present "oldstring" isnt pushed forwards but overwritten
		_memccpy(readbuffer2, pchInputBuf, 0, strLength);//append data at the newly created space
		WriteFile(hFile, readbuffer2, (DWORD)(((unsigned long long)(*amountread) - overlapstruct->Offset) + ((unsigned long long)strLength - *oldstringlength)), &((DWORD)shit), overlapstruct);
	}
	free(readbuffer);
	free(readbuffer2);
	readbuffer2 = readbuffer = NULL;
	return TRUE;
}

BOOL SmallDataWrite(char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile, size_t* oldstringlength)
{
	int shit = 0;
	char* readbuffer = calloc(*amountread+strLength, sizeof(char));
	char* readbuffer2 = calloc(*amountread+strLength, sizeof(char));
	int offset = overlapstruct->Offset + *oldstringlength;//offset will skip reading the data of the given date date as it will be wholly wiped out and replaced
	if (readbuffer == NULL || readbuffer2 == NULL || pchInputBuf == NULL)
	{
		CrashDumpFunction(253,0);
		return FALSE;
	}
	if (FALSE == ReadFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct))
	{
		CloseHandle(hFile);
		free(readbuffer);
		free(readbuffer2);
		readbuffer2 = readbuffer = NULL;
		CrashDumpFunction(253,0);
		return -1;
	}
	if ((*amountread - offset) == 0)//we need to append data at EOF
	{
		WriteFile(hFile, pchInputBuf, strLength, &((DWORD)shit), overlapstruct);
	}
	else//we are pasting in middle of data
	{
		_memccpy(readbuffer2 + strLength, readbuffer+*oldstringlength, 0, *amountread-*oldstringlength);//copy only date in front of oldstringlength
		_memccpy(readbuffer2, pchInputBuf, 0, strLength);//paste into remaining space for the date
		WriteFile(hFile, readbuffer2, (*amountread - overlapstruct->Offset) - (DWORD)(*oldstringlength - strLength), &((DWORD)shit), overlapstruct);
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
	int shit = 0;
	overlapstruct->Offset = *appendlocationindex-12; //points to the beggining of the date-mark, so we can delete it as well, since all data is being deleted.
	char* readbuffer = calloc(*amountread, sizeof(char));//for reading the txt file
	char* readbuffer2 = calloc(*amountread, sizeof(char));//for new data to be written
	if ((readbuffer == NULL) || (readbuffer2 == NULL))
	{
		if (readbuffer != NULL)
		{
			free(readbuffer);
			readbuffer = NULL;
		}
		else if (readbuffer2 != NULL)
		{
			free(readbuffer2);
			readbuffer2 = NULL;
		}
		CrashDumpFunction(254,0);
		return 0;
	}
	if (FALSE == ReadFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct))
	{
		CloseHandle(hFile);
		free(readbuffer);
		free(readbuffer2);
		readbuffer2 = readbuffer = NULL;
		CrashDumpFunction(255,0);
		return 0;
	}
	_memccpy(readbuffer2, readbuffer+*oldstringlength+12, 0, *amountread-*oldstringlength-12);
	
	WriteFile(hFile, readbuffer2, (*amountread - overlapstruct->Offset) - (DWORD)(*oldstringlength + 12), &((DWORD)shit), overlapstruct);
	SetFilePointer(hFile, -12-*oldstringlength, NULL, FILE_END);
	SetEndOfFile(hFile);
	free(readbuffer);
	free(readbuffer2);
	readbuffer2 = readbuffer = NULL;
	return TRUE;
}

BOOL DataWriteOver(char * pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile)
{
	char* readbuffer = calloc(*amountread, sizeof(char));
	if (readbuffer == NULL)
	{
		CrashDumpFunction(260,1);
		return 0;
	}
	if (FALSE == ReadFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct))
	{
		CloseHandle(hFile);
		hFile =NULL;
		free(readbuffer);
		readbuffer = NULL;
		CrashDumpFunction(260,0);
		return 0;
	}
	_memccpy(readbuffer, pchInputBuf, 0, strLength);
	WriteFile(hFile, readbuffer, *amountread - overlapstruct->Offset, NULL, overlapstruct);
	free(readbuffer);
	readbuffer = NULL;
	return TRUE;
}

BOOL alphnumBufCheck(BOOL * EmptyDataFlag, char * pchInputBuf[])
{
	size_t maxY = HeapSize(myHeap, 0, pchInputBuf);
	if (maxY >= sizeof(char*))
		maxY = maxY / sizeof(char*);
	else maxY = 0;
	if(pchInputBuf == NULL|| pchInputBuf[0] == NULL|| pchInputBuf[0][0] == 0)
	{
		if ((pchInputBuf != NULL && pchInputBuf[0] != NULL && pchInputBuf[0][0] !=  0) && pchInputBuf[0][0] >= 0 && isalnum(pchInputBuf[0][0]))
		{
			*EmptyDataFlag = FALSE;
		}
		*EmptyDataFlag = TRUE;
	}
	else
	{
		for (int z = 0, n = 0; z< (long long)maxY && (z < yAllocamount-1) && pchInputBuf[z] != NULL && pchInputBuf[z][n]>=0 && (FALSE == isalnum(pchInputBuf[z][n])); n++)
		{
			size_t maxX = 0;
			maxX = HeapSize(myHeap, 0, pchInputBuf[z]);
			BOOL zchangeflag = FALSE;
			if((long long)n+1< (long long)maxX)
			{
				if (!(pchInputBuf[z]))
				{
					z++;
					n = 0;
					zchangeflag = TRUE;
				}
				if (pchInputBuf[z][n + 1] >= 0 && isalnum(pchInputBuf[z][n + 1]) && (z < yAllocamount-1))
				{
					*EmptyDataFlag = FALSE;
					if (zchangeflag == TRUE)
					{
						if (pchInputBuf[z][n + 1] >= 0 && isalnum(pchInputBuf[z][n]))
							*EmptyDataFlag = FALSE;
						zchangeflag = FALSE;
					}
				}
			}
		}
	}
	return TRUE;
}

//counts amount of newline linux version and/or windows newlines
//if a newline is at the end of data
//xlength must represent, amount of data in bytes, excluding the terminating null.
int newlinecounter(char * cStringP,int xlength)
{
	int a = 0;//this variable will count up as it finds newlines
	if (cStringP[xlength - 1] == '\n')//if the last character before terminating null is the newline, remove it, and replace it with a null. This is for sake of making the algorithm less complicated.
	{
		a--;//this will deduce that null so that in loop below, it will exclude it from the sum.
		cStringP[xlength - 1] = '\0';
		if (xlength > 1 && cStringP[xlength - 2] == '\r')//in case its a windows newline
		{
			cStringP[xlength - 2] = '\0';
		}
	}
	//loop for calculating newlines
	for (int z = 0; z <= xlength; z++)
	{
		if (cStringP[z] == '\n')
		{
			a++;
		}
		else if (cStringP[z] == '\r')
		{
			a++;
			if ((z + 1) <= xlength && cStringP[z + 1] == '\n')
				z++;//skip the /r/n windows combo when it appears
		}
	}
	return a;
}

int WINAPI OnContextMenu(HWND hwnd, int x, int y)
{
	RECT rc;                    // client area of window 
	POINT pt = { x, y };        // location of mouse click 

	// Get the bounding rectangle of the client area. 

	GetClientRect(hwnd, &rc);

	// Convert the mouse position to client coordinates. 

	ScreenToClient(hwnd, &pt);

	// If the position is in the client area, display a  
	// shortcut menu. 

	if (PtInRect(&rc, pt))
	{
		ClientToScreen(hwnd, &pt);
		return DisplayContextMenu(hwnd, pt);
	}

	// Return FALSE if no menu is displayed. 
	CrashDumpFunction(371,0);
	return FALSE;
}

int APIENTRY DisplayContextMenu(HWND hwnd, POINT pt)
{
	HMENU hmenu;            // top-level menu 
	HMENU hmenuTrackPopup;  // shortcut menu 

	// Load the menu resource. 

	if ((hmenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(TXTPOPMENU))) == NULL)
	{
		CrashDumpFunction(354,0);
		return 0;
	}

	// TrackPopupMenu cannot display the menu bar so get 
	// a handle to the first shortcut menu. 

	hmenuTrackPopup = GetSubMenu(hmenu, 0);

	// Display the shortcut menu. Track the right mouse 
	// button. 
	int returnval = 0;
	returnval = TrackPopupMenu(hmenuTrackPopup,
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
		pt.x, pt.y, 0, hwnd, NULL);

	// Destroy the menu. 

	DestroyMenu(hmenu);
	return returnval;
}

//lastchar indexes will be allocated relative to the y position, the number itself will point to the exact array position of last valid non-null character
int * GetLastChar(char* pchInputBuf[], int lastchar[])
{
	size_t maxX = 0;
	if (pchInputBuf == NULL || lastchar == NULL)
	{
		CrashDumpFunction(401,1);
		return lastchar;
	}
	size_t yMax = 0;
	yMax = HeapSize(myHeap, 0, pchInputBuf);
	if (yMax >= sizeof(char*))
		yMax /= sizeof(char*);
	else yMax = 0;
	if(yAllocamount && pchInputBuf && lastchar)
	{
		for (int f = 0; f < yAllocamount-1 && f< (long long)yMax && (pchInputBuf[f]!=NULL) && (pchInputBuf[f][0] != '\0'); f++)
		{
			maxX = HeapSize(myHeap, 0, pchInputBuf[f]);
			for (int r = 1; r< (long long)maxX && pchInputBuf[f][r-1] != '\0'; r++)
			{
				lastchar[f] = r;//its inclusive
			}
		}
	}

	return lastchar;
}

int DeAllocPOINTx(size_t amount, POINT * buffer)
{
	buffer = calloc(amount, sizeof(POINT));
	if (!buffer)
	{
		CrashDumpFunction(356,0);
		return -1;
	}
	buffer->x = 0;
	buffer->y = 0;
	return 1;
}

//When there is no date uploaded to datafile, this positions it properly and uploads it accordingly
//If the function returns false, simply input the date at the end.
BOOL DateWrite(int * appendindexlocation, char * dateset)
{
	HANDLE hFile = { 0 };
	int filesize = 0;
	DWORD byteswritten = 0;
	char* readbuffer = { 0 };
	OVERLAPPED Overlapped = { 0 };
	
	hFile = CreateFileA(datasource, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	filesize = GetFileSize(hFile, NULL);
	readbuffer = calloc((long long)filesize+20, sizeof(char));//20 for dateset
	if (readbuffer == NULL)
	{
		CrashDumpFunction(387,0);
		return FALSE;
	}

	if (FALSE == ReadFile(hFile, readbuffer, filesize, NULL, &Overlapped))
	{
		CrashDumpFunction(385,0);
		return FALSE;
	}
	
	//scan for the year dataset range
	findthenearestdate(filesize, readbuffer, dateset, appendindexlocation, 1);
	//walk back to the beginning of date set
	int dateplace = 0;
	for (*appendindexlocation = *appendindexlocation-1; *appendindexlocation >= 0 && readbuffer[*appendindexlocation] != specialchar[0]; (*appendindexlocation)--)
	{
		;
	}
	//now push all data forward and paste the dateset
	int datelength = strlen(dateset);
	char* readbuffer2 = calloc(filesize, sizeof(char));
	_memccpy(readbuffer2, readbuffer, 0, filesize);
	_memccpy(readbuffer + datelength + *appendindexlocation, readbuffer2+ *appendindexlocation, 0, filesize- *appendindexlocation +datelength);
	_memccpy(readbuffer + *appendindexlocation, dateset, 0, datelength);
	Overlapped.Offset = (DWORD)*appendindexlocation;
	WriteFile(hFile, readbuffer+*appendindexlocation, strlen(readbuffer)-*appendindexlocation, &byteswritten, &Overlapped);
	free(readbuffer);
	free(readbuffer2);
	CloseHandle(hFile);
	return TRUE;
}

// Description:
//   Creates a horizontal scroll bar along the bottom of the parent 
//   window's area.
// Parameters:
//   hwndParent - handle to the parent window.
//   sbHeight - height, in pixels, of the scroll bar.
// Returns:
//   The handle to the scroll bar.
void CreateAHorizontalScrollBar(HWND hwndParent, int sbHeight)
{
	RECT rect;

	// Get the dimensions of the parent window's client area;
	if (!GetClientRect(hwndParent, &rect))
		return;

	// Create the scroll bar.
	(CreateWindowEx(
		0,                      // no extended styles 
		L"SCROLLBAR",           // scroll bar control class 
		(PTSTR)NULL,           // no window text 
		WS_CHILD | WS_VISIBLE   // window styles  
		| SBS_HORZ,         // horizontal scroll bar style 
		rect.left,              // horizontal position 
		rect.bottom - sbHeight, // vertical position 
		rect.right,             // width of the scroll bar 
		sbHeight,               // height of the scroll bar
		hwndParent,             // handle to main window 
		(HMENU)NULL,           // no menu 
		GetModuleHandle(NULL),                // instance owning this window 
		(PVOID)NULL            // pointer not needed 
	));
}
char * DateTextShow(char * selecteddate1)
{
	//everything below happens when you click a date, 12120 is a flag for this mechanism: You click a date and SETFOCUS is sent from the dates.c under the context of Datesproc during message "LM_BUTTONDOWN", in that message switch, there is a section where it sends WM_SETOFUCS to here with LOWORD(lParam) se to 12120. All this is done so text could be shown in textbox upon clicking a date.
					//In this section particulary you have "selecteddate1" buffer being loaded up for sake of later comparison with the read data from datatext, when exact match string of selectedate, according to its dating format, after this if statemant, the data is read.
					//Also note wparam and lparam also both used to store and the decompile year/day/month values. when flag is 12120(?)
	int dateyear = selecteddate >> 11;
	int datemonthindex = (selecteddate & 0x780) >> 7;
	int datedayindex = (selecteddate & 0x7F);
	//sprintf_s(selecteddate1, 20, "*00:00:%d*", );
	if (datedayindex < 10 && datemonthindex < 10)
		sprintf_s(selecteddate1, 20, "*%d :%d :%d*", datedayindex, datemonthindex, dateyear);
	else if (datedayindex < 10)
		sprintf_s(selecteddate1, 20, "*%d :%d:%d*", datedayindex, datemonthindex, dateyear);
	else if (datemonthindex < 10)
		sprintf_s(selecteddate1, 20, "*%d:%d :%d*", datedayindex, datemonthindex, dateyear);
	else
		sprintf_s(selecteddate1, 20, "*%d:%d:%d*", datedayindex, datemonthindex, dateyear);
	/*StringCbCatExA((LPSTR)selecteddate1, 20, "*00:00:0000*", NULL, NULL, 0);
	_itoa_s(datedayindex, selecteddate1 + 1, 3, 10);
	if (datedayindex < 10)
		selecteddate1[2] = ' ';
	_itoa_s(datemonthindex, selecteddate1 + 4, 3, 10);
	if (datemonthindex < 10)
		selecteddate1[5] = ' ';
	_itoa_s(dateyear, selecteddate1 + 7, 5, 10);
	selecteddate1[3] = ':';
	selecteddate1[6] = ':';
	selecteddate1[11] = specialchar[0];
	selecteddate1[12] = 0;*/
	return selecteddate1;

}
//richedit quick functions 

void txtBold(HWND hWindow) {
	CHARFORMAT boldfont;
	boldfont.cbSize = sizeof(CHARFORMAT);
	boldfont.dwMask = CFM_BOLD;
	boldfont.dwEffects = CFE_BOLD;
	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&boldfont);
}

void txtUnderlined(HWND hWindow) {
	CHARFORMAT2 underlinedfont;
	underlinedfont.cbSize = sizeof(CHARFORMAT);
	underlinedfont.dwMask = CFM_UNDERLINE;
	underlinedfont.dwEffects = CFM_UNDERLINE;

	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&underlinedfont);
}
void txtItalic(HWND hWindow) {
	CHARFORMAT Kursivfont;
	Kursivfont.cbSize = sizeof(CHARFORMAT);
	Kursivfont.dwMask = CFM_ITALIC;
	Kursivfont.dwEffects = CFM_ITALIC;
	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&Kursivfont);
}
void txtStrikeout(HWND hWindow) {
	CHARFORMAT underlinedfont;
	underlinedfont.cbSize = sizeof(CHARFORMAT);
	underlinedfont.dwMask = CFM_STRIKEOUT;
	underlinedfont.dwEffects = CFM_STRIKEOUT;
	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&underlinedfont);
}
void Subscript(HWND hWindow) {
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFE_SUBSCRIPT;
	cf.dwEffects = CFE_SUBSCRIPT;
	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}

void Superscript(HWND hWindow) {
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_SUPERSCRIPT;
	cf.dwEffects = CFM_SUPERSCRIPT;
	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}
void SetFont(HWND hWindow, const char* Font) {
	CHARFORMAT2 cf;
	memset(&cf, 0, sizeof cf);
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_FACE;
	wsprintf(cf.szFaceName, Font);
	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}

void FontSize(HWND hWindow, int size) {
	CHARFORMAT2 cf;
	memset(&cf, 0, sizeof cf);
	cf.cbSize = sizeof cf;
	cf.dwMask = CFM_SIZE;
	cf.yHeight = size * 20;
	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}
void txtColor(HWND hWindow, COLORREF clr) {
	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = clr;
	cf.dwEffects = 0;
	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}

void txtBackColor(HWND hWindow, COLORREF clr) {
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BACKCOLOR;
	cf.crBackColor = clr;
	cf.dwEffects = 0;
	SendMessage(hWindow, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
}

DWORD CALLBACK EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
	char** localbuffer = dwCookie;
	if (rtfindex == 0)
		localbuffer[0] = calloc(cb + 10, sizeof(char));
	else
		localbuffer[0] = realloc(localbuffer, rtfindex + cb + 10);
	int i = 0;
	for (i = 0; i < cb; i++)
	{
		localbuffer[0][rtfindex + i] = pbBuff[i];
	}
	*pcb = i;
	rtfindex += cb;
	return 0;
}

void SaveRichTextToFile(HWND hWnd, LPCWSTR filename)
{
	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	EDITSTREAM es = { 0 };
	es.dwCookie = (DWORD_PTR)hFile;
	es.pfnCallback = EditStreamOutCallback;
	SendMessage(hWnd, EM_STREAMOUT, SF_RTF, (LPARAM)&es);
	CloseHandle(hFile);
	if (es.dwError != 0)
	{
		//handle errors
	}
}

DWORD CALLBACK EditStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
	mcallbackinfo * mydata = dwCookie;
	HFILE hFile = mydata->hFile;
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

void ReadRichTextFromFile(HWND hWnd, LPCWSTR filename)
{
	HANDLE hFile = CreateFile(filename, GENERIC_READ, 0, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//handle errors
	}
	EDITSTREAM es = { 0 };
	es.dwCookie = (DWORD_PTR)hFile;
	es.pfnCallback = EditStreamInCallback;
	SendMessage(hWnd, EM_STREAMIN, SF_RTF, (LPARAM)&es);
	CloseHandle(hFile);
	if (es.dwError != 0)
	{
		//handle errors
	}
}

