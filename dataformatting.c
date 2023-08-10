#include "framework.h"

#define RETRIEVE 1
#define APPEND 2
#define CREATE 3

/*
TODO: add means of adding RGB and shape data
TODO: Create a sort of paging system to make a compromise between memory and speed, in order to be able to efficiently save and append text data for the dates.
*/

void WINAPI CompletionRoutine(DWORD u32_ErrorCode, DWORD u32_BytesTransfered, LPOVERLAPPED pk_Overlapped);
BOOL datacreate(void);
BOOL dataretrieve(void);
BOOL datappend(void);
//this source file is used for retrieving, appending and creating data file for this program.

BOOL datactionswitch(short int message)
{
	switch (message)
	{
	case RETRIEVE:

		break;

	case APPEND:

		break;

	case CREATE:
		datacreate();
		break;
	}
	return TRUE;
}

BOOL dataretrieve(void)
{
	return TRUE;
}

BOOL datappend(void)
{
	return TRUE;
}


BOOL datacreate(void)
{
	int i = 0;
	int thirtyone[] = { 1,3,5,7,8,10,12 };
	int thirty[] = { 4,6,9,11 };
	int leap[] = { 2 };
	OVERLAPPED dummyoverlap = { 0 };
	HANDLE datafile = NULL;
	SYSTEMTIME temptime = { 0 };
	char wchararray[150] = {" "};
	for (i = 0; i < 150; i++)
		wchararray[i] = ' ';
	static LPSTR CreationBuffer = { 0 };
	dummyoverlap.Offset = 0xFFFFFFFF;
	dummyoverlap.OffsetHigh = 0xFFFFFFFF;//these two thing set the "write" to be done at the end of file.
	datafile = CreateFile(TEXT("Localdata.txt"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	int dateprint = 1, monthtype = 0, datemonth = 0, dateyear = 0;
	SetEndOfFile(datafile);
	GetSystemTime(&temptime);
	dateyear = temptime.wYear - 50;
	for(int z=0; z<1200; z++)
	{
		datemonth++;
		if (z >= 12 && z%12==0)
		{
			datemonth = 1;
			dateyear++;
		}
		for (int y = 0; y < 7; y++)
		{
			if (y == 0)
			{
				_itoa_s(dateyear, wchararray, 5, 10);
				wchararray[4] = ',';
				wchararray[5] = ' ';
				_itoa_s(datemonth, wchararray + 6, 3, 10);
				wchararray[8] = ' ';
				for (i = 0; i < 7; i++)
				{
					if (i < 1)
					{
						if (datemonth == leap[i])
							monthtype = 1;
					}
					if (i < 4)
					{
						if (datemonth == thirty[i])
							monthtype = 2;
					}
					if (i < 20)
					{
						if (datemonth == thirtyone[i])
							monthtype = 3;
					}
				}
					
				if (datemonth < 10)
					wchararray[7] = ' ';
				wchararray[24] = '\r';
			}
			else if (y == 6) //append RGB and shape value at the end
			{
				CreationBuffer = "RGB: TBD, SHAPE: TBD";
				StringCbCatExA(&wchararray[5 * 24], 24, CreationBuffer, NULL, NULL, 0);
			}
			else
			{
				for (i = 0; i < 19; i += 3)//+3 cause two from decimal value(even non decimals have it reserved for simplicity sake, and 1 for empty space.
				{
					if (y == 1)
					{
						_itoa_s(dateprint++, wchararray + (i + y * 24), 3, 10);//puts digit in
						wchararray[(i + y * 24) + 2] = ' ';
						wchararray[(i + y * 24) + 1] = ' ';
					}

					else if(y<5)
					{
						_itoa_s(((7 * y)-7) + dateprint++, wchararray + (i + y * 24), 3, 10);//puts digit in
						wchararray[(i + y * 24) + 2] = ' ';
						if(((7 * y) + dateprint-7)<11)
							wchararray[(i + y * 24) + 1] = ' ';
					}

					else if (y == 5)
					{
						BOOL shitflag = FALSE;
						_itoa_s((7 * y)-7 + dateprint++, wchararray + (i + y * 24), 3, 10);//puts digit i;
						wchararray[(i + y * 24) + 2] = ' ';
						if ((monthtype == 1) && ((7 * y) + dateprint - 7) > 28)
						{
							shitflag = TRUE;
						}
						if ((monthtype == 2) && ((7 * y) + dateprint - 7) > 30)
						{
							shitflag = TRUE;
						}
						if ((monthtype == 3) && ((7 * y) + dateprint-7) > 31)
						{
							shitflag = TRUE;
						}
						if (shitflag == TRUE)
							i = 19;//breakthestupidloop
					}

				}
				wchararray[24*y-1] = '\r';
				dateprint = 1;
			}

		}
		wchararray[149] = '\r';
		WriteFile(datafile, wchararray, 150, NULL, &dummyoverlap);//wtf shit about asynchronous I/O no idea why botherin me with that stuf??
		for (i = 0; i < 149; i++)//empty the buffer/s
		{
			wchararray[i] = ' ';
		}

	}
	CloseHandle(datafile);

	


	return TRUE;
}

void WINAPI CompletionRoutine(DWORD u32_ErrorCode, DWORD u32_BytesTransfered, LPOVERLAPPED pk_Overlapped)
{
	;
}

/*
BOOL stacktostring(LPWSTR string, wchar_t stack, int bufferbeginning, int widthtocopy)
{
	for (int i = bufferbeginning; i < widthtocopy; i++);
	return TRUE;
}
*/