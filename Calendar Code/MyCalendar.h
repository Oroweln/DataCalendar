#pragma once

#include "Resource.h"
#include <Windowsx.h>
#include <stdbool.h>
#include <strsafe.h>
#include <Richedit.h>
#include <CommCtrl.h>

#define buttonfactor 24
extern size_t TextHeapRemaining;
extern HANDLE myHeap;
extern int yearrange;
extern BOOL RTForTXT;

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

typedef struct RGBcolorsData
{
	char colorname[50];
	COLORREF RGBcolor;
}RGBData;

typedef struct Streamcallbackdat
{
	HANDLE hFile;//hFile
	size_t offset;//offset
	size_t writeamount;//amount to write
} Streamdata;

int startyear, startmonth, startday;//used for saving the last date checked before closing the program

HWND CreateButtonM(LPWSTR szString, RECT Size, HWND hwnd, int index);
HWND CreateButtonD(LPWSTR szString, HWND hwnd, int index);
HWND CreateButtonMk(LPWSTR szString, RECT Size, HWND hwnd, int index);

extern HWND TextBoxInput;

#define TEXTBOXINPUTSBC 50010
#define TEXTBOXCOLORCHANGE 50011

extern char theworkingdirectory[1000];
extern char datasource[2000];
extern char specialchar[2];
extern HWND FontBoxHwnd;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK TextBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MarkProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DatesProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MonthsProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ButtonProcM(HWND, UINT, WPARAM, LPARAM);

int DynamicScroll(int scrollamount, HWND hwnd);
BOOL buttonMCreationRoutine(int cyChar, HWND* buttonarray, HWND hwnd, LPWSTR* Months);
BOOL buttonDCreationRoutine(int cyChar, HWND* buttonarray, HWND hwnd, LPWSTR* Dates);
BOOL buttonMkCreationRoutine(HWND* markarray, HWND hwnd, LPWSTR* Marks);
int PopChildMonth(HWND hwnd, int lastwindow);
int PushChildMonth(HWND hwnd, int firstwindow);

int ScrollingFunction(HWND hwnd, WPARAM wParam, int type);
BOOL ChildCreationFunction(void);
BOOL DestroyButton(HWND hwnd);

//Debugging functions
void cleaner4100(LPWSTR something, HINSTANCE something2);
BOOL ShowMessage(HWND hwnd, int XClient, int YClient, UINT message);

//cause of autism we put this shit here

extern RECT buttonrect;
extern RECT buttonrectd;

//Sizes of the 4 main child windows
extern RECT MonthRect;
extern RECT DatesRect;
extern RECT StringRect;
extern RECT MarksRect;
extern HWND buttondates[32];
extern HWND buttonarray[24];

extern int TrustedIndex;//mittigating "Speculative Execution Side Channel hardware vulnerabilities" for some i loops.

typedef struct tCharBox {
	short int Index; //buffer index
	short int xPos;
	short int yPos;
}Ichar;

COLORREF ColorSelection[100];

extern int pusenjekurca;

HBITMAP * MyBitmapsArray[100];

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
BOOL RangedDataWipe(int monthup, int monthdown, int yearup, int yeardown, int dayup, int daydown);
int MarkPresenceCheck(HANDLE hFile, int filelength, int dateticks, BOOL lswitch, int* shapevalue, int* colorvalue, BOOL* mydflags);

extern HWND TextBoxHwnd;
extern BOOL datechangecheck;
extern HWND DatesHwnd;
extern int MonthYearIndex;
extern int yearzero;
extern int selecteddate;
extern HWND buttonmarks[3];
extern BOOL markflagmonth;
extern BOOL TextBoxFlag;
extern BOOL TextBoxCalFlag;
extern BOOL GlobalDebugFlag;
extern long long pchinputbuffermemory;
extern long long xAllocamount;
extern long long yAllocamount;
extern COLORREF monthsbackground;
extern COLORREF datesbackground;
extern COLORREF textbackground;
extern COLORREF datesbuttoncolor;
extern COLORREF monthsbuttoncolor;
extern COLORREF inputsignalcolor;
extern int ordereddatasave;

#define TRIANGLEIDSHAPE 6000
#define SQUAREIDSHAPE 6100
#define CIRCLEIDSHAPE 6200
#define STARIDSHAPE 6300

#define IDCOLORS 100
#define IDCOLORREMOVE 201
#define IDMARKSREMOVE 202

#define MARK_SIGNAL 30005
#define MARK_CHECK_SIGNAL 30004
#define WORDWRAP 30003
#define TIMEUPDATE 30002
#define DATEUPDATETICK 30001
#define MARK_DATESIGNAL 30000
#define IDDATECHANGE 59000

BOOL EmptyDataWrite(OVERLAPPED* overlapstruct, int* amountread, HANDLE hFile, size_t* oldstringlength, int* appendlocationindex);
char* findthenearestdate(int filesize, char* readbuffer, char* dateset, int* appendindexlocation, int flag);
void CrashDumpFunction(int return_value, int fatality);
void Timerproc(HWND hwnd, UINT message, UINT_PTR wParam, DWORD lParam);
LRESULT CALLBACK ButtonProcMk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL savingFunction(int* appendlocationindex, char* pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, BOOL datepresent);
int DateTestBufferLoad(int* amountread, OVERLAPPED* overlapstruct, int* appendlocationindex, BOOL* datepresent);
char* DataSaveReordering(char* readbuffer);

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