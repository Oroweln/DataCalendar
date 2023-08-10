#include "framework.h"

#define BUFSIZE 64000
#define SHIFTED 0x8000 
#define yMAXLINES 30
#define xMAXCHARS 500


int TrustedIndex2 = 0;

BOOL spacingfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int * XC, int * YC, int * caretY, int * caretX, LPARAM lParam, int cyChar);
BOOL Carriagereturnfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, int cyChar);
BOOL updownfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int * XC, int * YC, int * caretY, int * caretX, WPARAM wParam, int cyChar);
BOOL leftandrightfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int * XC, int * YC, int * caretY, int * caretX, WPARAM wParam, int cyChar);
BOOL homendbackfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int * XC, int * YC, int * caretY, int * caretX, WPARAM wParam);
BOOL deletefunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, int cyChar);
BOOL characterouputfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, WPARAM wParam, int cyChar);
BOOL charnkeydownmegafunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, UINT message, int cyChar, WPARAM wParam, LPARAM lParam);
HWND TextBoxHwnd = NULL;
BOOL datechangecheck = FALSE; //True if date has been changed, hence textfile needs to be read again! 

/*
TODO: Selection, so one can cut/copy/paste replace etc.. aka mouse selection system + Copy-Pasting with it
TODO: Reorganize the whole code so it doesnt look like shit, before adding mouse selection and stuff.
TODO: Make everything use wide char instead of multybyte ones for now, later we will allow for multibyte[x]
TODO: Make system for moving pul push work[x]
TODO: Fix the problem with the dates failing to be found. When i try to type on the same date once more, it both copies part of the date itself, and wipes out everything right of the inserted text, whilist also copying(instead of overriding) the oldstring. Something to do with appendlocation index line 192 artifact. And perhaps appendindex point at the end of the curier instead of at the beginning of the date, try and put it at the beginning of the date, and see how it works.
*/

LRESULT CALLBACK TextBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TextBoxHwnd = hwnd;
	HDC hdc;
	static int cxClient = 0, cyClient = 0, cxChar = 0, cyChar = 0, cxCaps = 0, Linemax = 0, maxLines = 0, nCaretPosX = 0, nCaretPosY = 0, nCharWidth = 0, dateyear=0;
	static int Xcch = 0, Ycch = 0; //bufferpos
	RECT rc/*output rectangle for drawtext*/;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	OFSTRUCT filestructr = { 0 };
	static BOOL CTRLflag = FALSE, stupidasfuckflag = FALSE;
	static CHAR pchInputBuf[yMAXLINES][xMAXCHARS];
	static CHAR selecteddate[30];
	static int datemonthindex = 0, datedayindex = 0, amountread = 0, amounttowrite = 0, appendlocationindex = 0;
	static BOOL datepresent = FALSE;
	static CHAR* readbuffer = NULL;
	static HFILE hFile = 0;
	OVERLAPPED overlapstruct = { 0 };
	switch (message)
	{
	case WM_CREATE:
		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		return 0;
	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		CreateCaret(hwnd, NULL, 2, cyChar);
		SetCaretPos(nCaretPosX, nCaretPosY * cyChar);
		ShowCaret(hwnd);
		return 0;
	case WM_SETFOCUS:
		nCaretPosX = 0;
		nCaretPosY = 0;
		Xcch = 0;
		Ycch = 0;
		if (datechangecheck == TRUE)
		{
			int h = 0, p = 0;
			for(h=0; h<yMAXLINES; h++)
				for (p = 0; p < xMAXCHARS; p++)
				{
					pchInputBuf[h][p] = '\0';
				}
			RECT temprect2 = { 0 };
			GetClientRect(hwnd, &temprect2);
			InvalidateRect(hwnd, &temprect2, TRUE);
			datepresent = FALSE;
			if (LOWORD(lParam) == 12120) //flag for intrediction through clicking dates, meaning wParam is meaningfull
			{
				datemonthindex = LOWORD(wParam);
				datedayindex = HIWORD(wParam);//lParam;
				dateyear = HIWORD(lParam);
				StringCbCatExA(&selecteddate, 24, "*00:00:0000*", NULL, NULL, 0);
				_itoa_s(datedayindex, selecteddate + 1, 3, 10);
				if (datedayindex < 10)
					selecteddate[2] = ' ';
				_itoa_s(datemonthindex, selecteddate + 4, 3, 10);
				if (datemonthindex < 10)
					selecteddate[5] = ' ';
				_itoa_s(dateyear, selecteddate + 7, 5, 10);
				selecteddate[3] = ':';
				selecteddate[6] = ':';
				selecteddate[11] = '*';
			}
			hFile = OpenFile("Textdata.txt", &filestructr, OF_READWRITE);
			//system to read the file, or way that txt will be located/allocated.
			if (hFile != HFILE_ERROR && (amountread = (GetFileSize(hFile, NULL))) > 0)//file exists thus reading it makes sense and presting it in textbox
			{
				BOOL randomflag = FALSE;
				readbuffer = calloc(amountread+1, sizeof(char));//MALLOCING HERE IS TEMP WORKAROUND TILL WE MAKE PROGRAM FULLY FUNCTIONAL, to run from stack.
				randomflag = ReadFile(hFile, readbuffer, amountread, &amounttowrite, &overlapstruct);//check if the data for a date is present
				for (int i = 0; readbuffer[i] != '\0' && i < amountread; i++)
				{
					appendlocationindex = i;
					if (readbuffer[i] == '*')
					{
						for (int z = 1; selecteddate[z] == readbuffer[i + z] && z < 30; z++)
						{
							if (readbuffer[i + z] == '*' || readbuffer[i + z] == '\0')//we have reached second start without quiting the loop, ie. date is matching
							{
								datepresent = TRUE;
								appendlocationindex = z + i + 1;//to the beginning of text
								i = amountread;//to kill the loop
								z = 30;//to kill the loop
							}
						}
					}
					if (readbuffer[i + 1] == '\0' && i != amountread)
					{
						readbuffer[i + 1] = '\r';
						appendlocationindex++;
					}
				}
				if (readbuffer[appendlocationindex + 1] == '\r')
					appendlocationindex += 1;
				//load readbuffer dataset into pchInputBuffer
				if (datepresent == TRUE)
				{
					for (int i = 0, g = appendlocationindex; i < yMAXLINES; i++)
					{
						for (int z = 0; z < xMAXCHARS && readbuffer[g] != '\0' && readbuffer[g] != '\r' && readbuffer[g] != '\n'; z++, g++)
						{
							pchInputBuf[i][z] = readbuffer[g];
							if (readbuffer[g + 1] == '\0' || readbuffer[g + 1] == '*' || readbuffer[g+1] < 0)
							{
								z = xMAXCHARS; // to break the loop
								i = yMAXLINES; //to break the loop
								RECT localrect = { 0 };
								GetClientRect(hwnd, &localrect);
								InvalidateRect(hwnd, &localrect, TRUE);
							}
						}
					}
				}

			}
			else if (hFile == HFILE_ERROR)//file doesnt exist, create a new one, we will put data into it as we type.
			{
				hFile = CreateFileA("Textdata.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
			}
			CloseHandle(hFile);
		}
		datechangecheck = FALSE;
		return 0;
	case WM_KILLFOCUS:
		HideCaret(hwnd);
		DestroyCaret();
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_CONTROL)//make ctrl+s save text
		{
			CTRLflag = TRUE;
			return 0;
		}
		else if (wParam == 0x53 && CTRLflag == TRUE)
		{//we need to push all data forwards or backwards depending if we are ammending smaller sized new dataset or larger sized.
			stupidasfuckflag = TRUE;
			size_t strLength = 0;
			//get total buffer size
			for (int i = 0; i < yMAXLINES; i++)
			{
				size_t tempval = 0;
				StringCchLengthA(pchInputBuf[i], xMAXCHARS, &tempval);
				strLength += tempval;
			}

			//figure out if the data to be applied is less or more then last one that was, or perhaps same size.
			overlapstruct.Offset = appendlocationindex+12; //does it always point at the beggining of writing place?
			hFile = OpenFile("Textdata.txt", &filestructr, OF_READWRITE);
			amountread = GetFileSize(hFile, NULL);

			//to do this get already existing data's size
			char tempcharbuf[xMAXCHARS * yMAXLINES] = { 0 };
			ReadFile(hFile, tempcharbuf, xMAXCHARS * yMAXLINES, NULL, &overlapstruct);
			size_t oldstringlength = 0;
			for (oldstringlength = 0; tempcharbuf[oldstringlength] != '*' && tempcharbuf[oldstringlength] != '\0' && oldstringlength < xMAXCHARS * yMAXLINES; oldstringlength++)//calculating length of the string.
			{
				;
			}
			//Now finally determine the relation and apply according process
			if (oldstringlength > strLength)//new data is smaller, pull back all data in front of it and shrink.
			{
				readbuffer = calloc(amountread, sizeof(char));
				char* readbuffer2 = calloc(amountread, sizeof(char));
				ReadFile(hFile, readbuffer, overlapstruct.Offset, NULL, &overlapstruct);
				for (int i = (oldstringlength - strLength), k=0; i<(amountread - overlapstruct.Offset) && readbuffer[i] != '\0'; i++, k++)
				{
					readbuffer2[k] = readbuffer[i];
				}//we squeeze space by copying only bits later for amount of memory needs to be squeezed out. the whole file in front needs to be copied so we can move everything later on properly. unlike in function below, here i = length difference cause we want to ignore overlapping bits and thus annule them. Whilst in else statement below we simply move whole txt forwards and annule overlapping bits here we begin from the beginning of the overlap.
				for (int j = 0, k = 0; j < yMAXLINES; j++)
				{
					for (int i = 0; pchInputBuf[j][i] != '\0' && pchInputBuf[j][i] != '\r' && k < strLength; i++, k++)
						readbuffer2[k] = pchInputBuf[j][i];//printout new string into the empty space.
				}
				WriteFile(hFile, readbuffer2, (amountread - overlapstruct.Offset) - (oldstringlength - strLength), NULL, &overlapstruct);
			}
			else if (oldstringlength < strLength)//new data is larger, push forward all data in front of it and add size.
			{
				readbuffer = calloc(amountread, sizeof(char));
				char* readbuffer2 = calloc(amountread + strLength + 1, sizeof(char));
				ReadFile(hFile, readbuffer, amountread-overlapstruct.Offset, NULL, &overlapstruct);
				for (int i = 0; i < (amountread - overlapstruct.Offset) && readbuffer[i] != '\0'; i++)
					readbuffer2[i] = readbuffer[(strLength - oldstringlength) + i];//put the old string in front to get extra space and overwrite everything that is left of old string
				for(int j=0, k = 0;j<yMAXLINES;j++)
				{
					for (int i = 0; k < strLength && pchInputBuf[j][i] != '\0' && pchInputBuf[j][i] != '\r'; i++, k++)//put in the left space the new string
						readbuffer2[k] = pchInputBuf[j][i];
				}
				WriteFile(hFile, readbuffer2, (amountread-overlapstruct.Offset) + (strLength - oldstringlength), NULL, &overlapstruct);
			}
			//in case they are same we do nothing except write over it
			else
			{
				readbuffer = calloc(amountread, sizeof(char));
				ReadFile(hFile, readbuffer, overlapstruct.Offset, NULL, &overlapstruct);
				for (int j = 0, k = 0; j < yMAXLINES; j++)
				{
					for (int i = 0; pchInputBuf[j][i] != '\0' && pchInputBuf[j][i] != '\r' && k < strLength; i++, k++)
						readbuffer[k] = pchInputBuf[j][i];
				}
				WriteFile(hFile, readbuffer, amountread-overlapstruct.Offset, NULL, &overlapstruct);;
			}
			CloseHandle(hFile);
		}
		else//there is no existing date save, so we are inputting new data thus we must add the new date mark as well.
		{
			if(datepresent == FALSE)
			{
				hFile = OpenFile("Textdata.txt", &filestructr, OF_READWRITE);
				overlapstruct.Offset = appendlocationindex;
				WriteFile(hFile, selecteddate, 12, &amounttowrite, &overlapstruct);
				datepresent = TRUE;
			}
			charnkeydownmegafunction(hwnd, pchInputBuf, &Xcch, &Ycch, &nCaretPosY, &nCaretPosX, message, cyChar, wParam, lParam);
		}
		CTRLflag = FALSE;
		break;
	case WM_CHAR:
		if (stupidasfuckflag == FALSE)
		{
			charnkeydownmegafunction(hwnd, pchInputBuf, &Xcch, &Ycch, &nCaretPosY, &nCaretPosX, message, cyChar, wParam, lParam);
		}
		if (stupidasfuckflag == TRUE)
		{
			stupidasfuckflag = FALSE;
		}
		break;
	case WM_SHOWWINDOW:
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		HideCaret(hwnd);
		HBRUSH tempbrush = CreateSolidBrush(RGB(255, 255, 255));
		FillRect(hdc, &ps.rcPaint, tempbrush);
		DeleteObject(tempbrush);
		for (TrustedIndex2 = 0; TrustedIndex2 >= 0 && TrustedIndex2 < 30; TrustedIndex2++)
		{
			SetRect(&rc, 0, cyChar* TrustedIndex2, cxClient, cyChar* TrustedIndex2 +cyChar);
			LPSTR tempstring2 = pchInputBuf[TrustedIndex2];
			DrawTextExA(hdc, tempstring2, -1, &rc, DT_LEFT, NULL);
		}//keeps deleting everything else but current y-line?
		ShowCaret(hwnd);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY: 
		DestroyCaret();
		CloseHandle(hFile);
		UnregisterHotKey(hwnd, 0xAAAA); 		// Free the input buffer.
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


BOOL spacingfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, LPARAM lParam, int cyChar)
{
	HDC hdc;
	int nCharWidth = 0, i = 0;
	char ch = 0, tempstring[500] = { 0 };

	hdc = GetDC(hwnd);

	HideCaret(hwnd);
	//blank out the 0x20 space char space
	GetCharWidth32A(hdc, 0x20, 0x20, &nCharWidth);
	ReleaseDC(hwnd, hdc);

	for (i = 0; Inputbuffer[*YC][*XC+i] != 0 && i < xMAXCHARS; i++)
	{
		tempstring[i] = Inputbuffer[*YC][*XC+i];//find the end of the line/its length
	}
	//Then move all character for space width forward then call WM_PAINT to put the string.

	for (int k = 0; k < i; k++)
	{
		ch = tempstring[k];
		Inputbuffer[*YC][*XC + k + 1] = ch; //+1 is to avoid space character
	}
	Inputbuffer[*YC][(*XC)++] = 0x20;
	if (lParam != 1)//for if space is called by char being typed into the already occupied place, aka auto-space, to avoid these activities
	{
		*caretX += nCharWidth;
		if (*XC > xMAXCHARS - 1)
		{
			(*YC)++;
			*XC = 0;
		}
	}
	SetCaretPos(*caretX, *caretY * cyChar);
	ShowCaret(hwnd);
	return TRUE;
}

BOOL Carriagereturnfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, int cyChar)
{
	HDC hdc; 
	int i = 0, j=0;
	char tempstring[500] = { 0 };

	hdc = GetDC(hwnd);
	if (Inputbuffer[*YC][*XC] != 0x00 && Inputbuffer[*YC][*XC] != 0x0D) //for when you click enter when you are not at the end of the string
	{
		char tempstring2[500][29] = { 0 };
		int z = 0;
		/*1. save the string past xcch for the current line 2. move all line below for one place, and empty line below 3. input into empty line the foremntioned string*/
		for (i = *XC, j = 0; Inputbuffer[*YC][i] != 0x00; i++, j++)
		{
			tempstring[j] = Inputbuffer[*YC][i];//saves og string that is subject of movement
			Inputbuffer[*YC][i] = 0x00;//removing the saved chars
		}
		Inputbuffer[*YC][*XC] = 0x0D;//put carriage return at very we clicked enter;
		for (i = *YC + 1, z = 0; Inputbuffer[i][0] != 0x00; i++, z++)//+1 in order to get to strings below
		{
			for (j = 0; Inputbuffer[i][j]; j++)
			{
				tempstring2[z][j] = Inputbuffer[i][j]; //saves everything below
				Inputbuffer[i][j] = 0x00; //annulated the places
			}
		}
		for (i = 0; i < z; i++)//+2 cause we moving the +1s to +2s, so 0 could be at +1
		{
			for (j = 0; tempstring2[i][j] != 0x00; j++)
				Inputbuffer[*YC + i + 2][j] = tempstring2[i][j];//doing the pasting of the saved shit, step 2 complete
		}
		(*YC)++;//now we can move onto the next line and finish third step
		*caretX = 0;
		for (i = 0; tempstring[i] != 0x00; i++)
		{
			Inputbuffer[*YC][i] = tempstring[i];
		}
		RECT temprect = { 0 };
		GetClientRect(hwnd, &temprect);
		InvalidateRect(hwnd, &temprect, TRUE);
		*XC = 0;
	}
	else
	{
		Inputbuffer[(*YC)++][*XC] = 0x0D;
		*XC = 0;
		*caretX = 0;
	}
	*caretY += 1;
	ReleaseDC(hwnd, hdc);
	SetCaretPos(*caretX, *caretY * cyChar);
	return TRUE;
}

BOOL updownfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, WPARAM wParam, int cyChar)
{
	HDC hdc;
	char ch = 0;
	int nCharWidth = 0, i = 0;
	switch(wParam)
	{

	case VK_UP:     // UP ARROW 
		HideCaret(hwnd);
		hdc = GetDC(hwnd);
		--(*caretY);
		if (*caretY < 0)
			*caretY = 0;
		else
			--(*YC);
		*caretX = 0;
		for (i = 0; Inputbuffer[*YC][i] != '\r'; i++)
		{
			ch = Inputbuffer[*YC][i];
			GetCharWidth32A(hdc, ch, ch, &nCharWidth);
			*caretX += nCharWidth;
			*XC = i + 1;//due to overlap
		}
		ReleaseDC(hwnd, hdc);
		SetCaretPos(*caretX, *caretY * cyChar);
		ShowCaret(hwnd);
		break;
	case VK_DOWN:   // DOWN ARROW 
		HideCaret(hwnd);
		hdc = GetDC(hwnd);
		if (*caretY > 29 || Inputbuffer[*YC + 1][0] == 0x00)
		{
			ShowCaret(hwnd);
			return 0;
		}
		else
			++(*YC);
		++(*caretY);
		*caretX = 0;
		for (i = 0; Inputbuffer[*YC][i] != '\r' && Inputbuffer[*YC][i] != '\0'; i++)
		{
			ch = Inputbuffer[*YC][i];
			GetCharWidth32A(hdc, ch, ch, &nCharWidth);
			*caretX += nCharWidth;
			*XC = i + 1;//due to overlap
		}
		SetCaretPos(*caretX, *caretY * cyChar);
		ShowCaret(hwnd);
		ReleaseDC(hwnd, hdc);
		break;
	}
	return TRUE;
}

BOOL leftandrightfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, WPARAM wParam, int cyChar)
{
	HDC hdc;
	char ch;
	int nCharWidth = 0;
	switch(wParam)
	{
	case VK_LEFT:   // LEFT ARROW 

		// The caret can move only to the beginning of 
		// the current line. 

		if (*caretX > 0)
		{
			HideCaret(hwnd);

			// Retrieve the character to the left of 
			// the caret, calculate the character's 
			// width, then subtract the width from the 
			// current horizontal position of the caret 
			// to obtain the new position. 
			if ((*XC)-- < 0)
			{
				(*YC)--;
				(*XC) = 0;
			}
			ch = Inputbuffer[*YC][*XC];
			hdc = GetDC(hwnd);
			GetCharWidth32A(hdc, ch, ch, &nCharWidth);
			ReleaseDC(hwnd, hdc);
			*caretX = max(*caretX - nCharWidth,
				0);
			SetCaretPos(*caretX, *caretY * cyChar);
			ShowCaret(hwnd);
		}
		break;

	case VK_RIGHT:  // RIGHT ARROW

		if (*XC < xMAXCHARS - 1 && Inputbuffer[*YC][*XC] != 0x00 && Inputbuffer[*YC][*XC] != '\r')//+1 to avoid carriage return
		{
			HideCaret(hwnd);
			ch = Inputbuffer[*YC][*XC];
			hdc = GetDC(hwnd);
			GetCharWidth32A(hdc, ch, ch, &nCharWidth);
			ReleaseDC(hwnd, hdc);
			*caretX += nCharWidth;
			if (++(*XC) > xMAXCHARS)
			{
				(*YC)++;
				*XC = 0;
				(*caretY)++;
			}
			SetCaretPos(*caretX, *caretY * cyChar);

			ShowCaret(hwnd);
		}
		break;
	}
	return TRUE;
}

BOOL homendbackfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, WPARAM wParam)
{
	int cCR = 0, nCRIndex = 0, i = 0;
	HDC hdc;
	HRESULT hResult; 
	size_t asff = 200000;
	size_t* pcch = &asff;
	SIZE sz;
	char szBuf[128] = { 0 };
	switch(wParam)
	{
	case VK_HOME:   // HOME 

		// Set the caret's position to the upper left 
		// corner of the client area. 

		*caretX = *caretY = *caretX = *YC = 0;
		break;

	case VK_END:    // END  

		// Move the caret to the end of the text. 

		for (i = 0; i < *caretX; i++)
		{
			// Count the carriage returns and save the 
			// index of the last one. 

			if (Inputbuffer[*YC][i] == 0x0D)
			{
				cCR++;
				nCRIndex = i + 1;
			}
		}
		*caretY = cCR;
		// Copy all text between the last carriage 
		// return and the end of the keyboard input 
		// buffer to a temporary buffer. 

		for (i = nCRIndex, TrustedIndex2 = 0; i < *XC; i++, TrustedIndex2++)
			szBuf[TrustedIndex2] = Inputbuffer[*YC][i];
		szBuf[TrustedIndex2] = '\0';

		// Retrieve the text extent and use it 
		// to set the horizontal position of the 
		// caret. 

		hdc = GetDC(hwnd);
		hResult = StringCchLengthA(szBuf, 128, pcch);
		if (FAILED(hResult))
		{
			// TODO: write error handler
		}
		GetTextExtentPoint32A(hdc, szBuf, (int)*pcch,
			&sz);
		*caretX = sz.cx;
		ReleaseDC(hwnd, hdc);
		break;
	}
	return TRUE;
}

BOOL deletefunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, int cyChar)
{
	// Calculate the new horizontal position of the 
	// caret. If the position exceeds the maximum, 
	// insert a carriage return and move the caret 
	// to the beginning of the next line. 
	char ch = 0;
	char tempstring[500] = { 0 };
	int nCharWidth = 0, i = 0;
	HDC hdc; 

	if (*XC <= 0)
		return 0;
	HideCaret(hwnd);
	ch = Inputbuffer[*YC][--(*XC)];
	hdc = GetDC(hwnd);
	GetCharWidth32A(hdc, ch, ch, &nCharWidth);
	if (*XC < 0)
	{
		(*YC)--;
		*XC = 0;
		--(*caretY);
		if (*caretY < 0)
			*caretY = 0;
		*caretX = 0;//stupid ass band aid solution tldr
	}
	for (i = 0; Inputbuffer[*YC][*XC + i] != 0x00 && i < xMAXCHARS; i++)
	{
		tempstring[i] = Inputbuffer[*YC][*XC + i + 1];//find the end of the line/its length, +1 cause we using characters in front
	}
	//Then move all character for space width forward then call WM_PAINT to put the string.
	i--; //so it does not "copy over"
	int k = 0;
	for (k = 0; k < i && k <= xMAXCHARS - 1; k++)
	{
		Inputbuffer[*YC][*XC + k] = tempstring[k];
	}
	Inputbuffer[*YC][*XC + i] = 0x00;
	*caretX -= nCharWidth;
	ReleaseDC(hwnd, hdc);
	SetCaretPos(*caretX, *caretY * cyChar);
	ShowCaret(hwnd);
	return TRUE;
}

BOOL characterouputfunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, WPARAM wParam, int cyChar)
{
	char ch = 0;
	int nCharWidth = 0;
	HDC hdc;

	ch = (char)wParam;
	HideCaret(hwnd);
	// Retrieve the character's width and output 
	// the character. 
	if (Inputbuffer[*YC][*XC] != 0x00)//problem here
	{
		SendMessage(hwnd, WM_CHAR, 0x20, 1);
		--(*XC);
	}
	Inputbuffer[*YC][*XC] = ch;
	// Store the character in the buffer.
	if ((*XC)++ > xMAXCHARS - 1)
	{
		(*YC)++;
		*XC = 0;
	}
	// somewhere here implement word wrapping and caret wrapping, buffer wrapping etc.
	hdc = GetDC(hwnd);
	GetCharWidth32A(hdc, (UINT)wParam, (UINT)wParam,
		&nCharWidth);
	TextOutA(hdc, *caretX, *caretY * cyChar,
		&ch, 1);
	ReleaseDC(hwnd, hdc);
	*caretX += nCharWidth;
	ShowCaret(hwnd);

	return TRUE;
}

BOOL charnkeydownmegafunction(HWND hwnd, char Inputbuffer[][xMAXCHARS], int* XC, int* YC, int* caretY, int* caretX, UINT message, int cyChar, WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	RECT FUCKSHITRECT = { 0 };

	switch(message)
	{
	case WM_CHAR:
		// check if current location is close enough to the
		// end of the buffer that a buffer overflow may
		// occur. If so, add null and display contents. 
		if (*XC * *YC > yMAXLINES * xMAXCHARS)
		{
			Inputbuffer[*YC][*XC] = 0x00;
			SendMessage(hwnd, WM_PAINT, 0, 0);
		}
		switch (wParam)
		{
		case 0x0A:  // linefeed 
		case 0x1B:  // escape 
		case 0x13: //CTRL+S to not be printer xddd
			return 0;
		case 0x08: //backspace
			deletefunction(hwnd, Inputbuffer, XC, YC, caretY, caretX, cyChar);
			GetClientRect(hwnd, &FUCKSHITRECT);
			InvalidateRect(hwnd, &FUCKSHITRECT, TRUE);
			break;
		case 0x09:  // tab 
			// Convert tabs to four consecutive spaces. 
			for (i = 0; i < 4; i++)
				SendMessage(hwnd, WM_CHAR, 0x20, 0);
			return 0;
		case 0x20: //SPACE
			spacingfunction(hwnd, Inputbuffer, XC, YC, caretY, caretX, lParam, cyChar);
			GetClientRect(hwnd, &FUCKSHITRECT);
			InvalidateRect(hwnd, &FUCKSHITRECT, TRUE);
			break;
		case 0x0D:  // carriage return, we must make it put all text in front of the caret, down one line, and move all other lines further down

			// Record the carriage return and position the 			// caret at the beginning of the new line.
			Carriagereturnfunction(hwnd, Inputbuffer, XC, YC, caretY, caretX, cyChar);
			break;
		default:    // displayable character 
			characterouputfunction(hwnd, Inputbuffer, XC, YC, caretY, caretX, wParam, cyChar);
			break;
		}
		SetCaretPos(*caretX, *caretY * cyChar);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
		case VK_RIGHT:
			leftandrightfunction(hwnd, Inputbuffer, XC, YC, caretY, caretX, wParam, cyChar);
			break;
		case VK_UP:     // UP ARROW 
		case VK_DOWN:   // DOWN ARROW 
			updownfunction(hwnd, Inputbuffer, XC, YC, caretY, caretX, wParam, cyChar);
			break;
		case VK_HOME:   // HOME
		case VK_END:    // END 
			homendbackfunction(hwnd, Inputbuffer, XC, YC, caretY, caretX, wParam);//fucki'n broken mate
			break;
		default:
			break;
		}
	}

	return TRUE;
}
