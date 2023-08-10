#pragma once

#include "resource.h"
#include <Windowsx.h>
#include <strsafe.h>

#define buttonfactor 24

typedef struct windowlocdata
{
	HWND chwnd;
	RECT wcrect;
} winloc;

typedef struct carrierreturnspotter
{
	POINT carretpos;//position of carrier return
	int characterindex; //index of carrier return in the buffer
} CrgS;

HWND CreateButtonM(LPWSTR szString, RECT Size, HWND hwnd, int index);
HWND CreateButtonD(LPWSTR szString, RECT Size, HWND hwnd, int index);
HWND CreateButtonMk(LPWSTR szString, RECT Size, HWND hwnd, int index);



LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TextBoxProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MarkProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DatesProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MonthsProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ButtonProcM(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK EnumMonthsLocation(HWND, LPARAM);

int DynamicScroll(int lastpos, HWND hwnd, SCROLLINFO si, int offset, HWND* buttonarray);
BOOL buttonMCreationRoutine(int cyChar, HWND* buttonarray, HWND hwnd, LPWSTR* Months);
BOOL buttonDCreationRoutine(int cyChar, HWND* buttonarray, HWND hwnd, LPWSTR* Dates);
BOOL buttonMkCreationRoutine(HWND* markarray, HWND hwnd, LPWSTR* Marks);
RECT PopChildMonth(HWND hwnd[], int offset);
RECT PushChildMonth(HWND hwnd[], int offset);

int ScrollingFunction(HWND hwnd, WPARAM wParam, int type);
BOOL ChildCreationFunction(void);
BOOL DestroyButton(HWND hwnd);

//Debugging functions
void cleaner4100(LPWSTR something, HINSTANCE something2);
BOOL ShowMessage(HWND hwnd, int XClient, int YClient, UINT message);

//cause of autism we put this shit here

extern RECT buttonrect;

//Sizes of the 4 main child windows
extern RECT MonthRect;
extern RECT DatesRect;
extern RECT StringRect;
extern RECT MarksRect;
extern winloc windowdataorg[buttonfactor];
extern HWND buttondates[32];

extern int TrustedIndex;//mittigating "Speculative Execution Side Channel hardware vulnerabilities" for some i loops.
extern int tiShift;

typedef struct tCharBox {
	short int Index; //buffer index
	short int xPos;
	short int yPos;
}Ichar;

//dataformating.c structure

#pragma pack(1)
typedef struct markflags
{
	short int shape;
	COLORREF color;
} markflags;

typedef struct datedataunit
{
	short int date;
	TCHAR stringcontext[15000];
	markflags localflag;
	
}datedataunit;

typedef struct monthdataunit 
{
	short int month;
	datedataunit monthdata[32];
}monthdataunit;

BOOL datactionswitch(short int message);
int mystringtoint(LPCSTR string);

extern HWND TextBoxHwnd;
extern BOOL datechangecheck;