#pragma once

#include "resource.h"
#include <Windowsx.h>
#include <stdbool.h>
#include <strsafe.h>
#include <Richedit.h>
#include <CommCtrl.h>
#include <assert.h>
#include <cstddef>

typedef struct dateanddata
{
	char date[24];
	char* data;
}dateanddata, ** lpdateanddata, * fagdata;

typedef struct date
{
	int year;
	int month;
	int day;
}date, ** ppdate, * pdate;

#define buttonfactor 24
extern size_t TextHeapRemaining;
extern int yearrange;
extern BOOL RTForTXT;

typedef struct RGBcolorsData
{
	char colorname[52];
	COLORREF RGBcolor;
}RGBData;

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
BOOL buttonMCreationRoutine(HWND* buttonarray, HWND hwnd, LPWSTR* Months);
BOOL buttonDCreationRoutine(HWND* buttonarray, HWND hwnd, LPWSTR* Dates);
BOOL buttonMkCreationRoutine(HWND* markarray, HWND hwnd, LPWSTR* Marks);
int PopChildMonth(HWND hwnd, int lastwindow);
int PushChildMonth(HWND hwnd, int firstwindow);

BOOL ChildCreationFunction(void);
BOOL DestroyButton(HWND hwnd);

//Debugging function
BOOL ShowMessage(HWND hwnd, int XClient, int YClient, UINT message);

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

COLORREF ColorSelection[100];

extern int shoddyvar;

HBITMAP * MyBitmapsArray[100];

//dataformating.c structure

#pragma pack(1)

int mystringtoint(LPCSTR string);
BOOL RangedDataWipe(int monthup, int monthdown, int yearup, int yeardown, int dayup, int daydown);
int MarkPresenceCheck(HANDLE hFile, int filelength, int dateticks, BOOL lswitch, int* shapevalue, int* colorvalue, BOOL* mydflags);

extern HWND TextBoxHwnd;
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

void txtColor(HWND hWindow, COLORREF clr); //set the text color
void txtBackColor(HWND hWindow, COLORREF clr); //Set the textbackgroundcolor

char* ScriptFormat(char* Inputbuffer);
INT_PTR CALLBACK DlgSettingsProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
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
char* monthtypegen(char* wchararray, int dateyear, int datemonth, int* monthtype, int* thirty, int* leap, int* thirtyone);
char* MacroFormating(char* macroformated, BOOL firstrun);
int NumbersFunction(int* symbolsarray, int maxsymbols, char* macroformated, int numbersbegginingindex, BOOL FloatFlag, int* lastbyteindex);
BOOL comparestrings(char* string1, char* string2);

char* DataSaveReordering(char* readbuffer);
int NearestDate(int mflag, int filesize, char* readbuffer, int* appendindexlocation, char* dateset, int i);

LRESULT TextBoxInputSbc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

int DateTestBufferLoad(int* amountread, OVERLAPPED* overlapstruct, int* appendlocationindex, BOOL* datepresent);
BOOL savingFunction(int* appendlocationindex, char* pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, BOOL datepresent);
BOOL LargeDataWrite(char* pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile, size_t* oldstringlength);
BOOL SmallDataWrite(char* pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile, size_t* oldstringlength);
BOOL DataWriteOver(char* pchInputBuf, OVERLAPPED* overlapstruct, int* amountread, size_t strLength, HANDLE hFile);

BOOL DateWrite(int* appendindexlocation, char* dateset);
char* DateTextShow(char* selecteddate1);

void txtColor(HWND hWindow, COLORREF clr); //set the text color
void txtBackColor(HWND hWindow, COLORREF clr); //Set the textbackgroundcolor

DWORD CALLBACK EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb);
DWORD CALLBACK EditStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb);

LRESULT CALLBACK ButtonProcMk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TempBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void Timerproc(HWND hwnd, UINT message, UINT_PTR wParam, DWORD lParam);
void marksfilepresent(int bithsize, HANDLE hFile);
RECT buttonswindow(HWND hwnd, POINT mypt);
void DatesChangeWindow(HWND hwnd, POINT mypt, RECT TempRect2);
void ShapesChangeButton(int indexselected, HWND hwnd);
void RGBDatatxtUpdate(char* filebuffer, int realindex, int selectedcolor, int filesize, HANDLE hFile, OVERLAPPED overlapstruct);
RECT ButtonsBarFunction(RECT LocalRect, HDC hdc, HWND hwnd, int* step, RECT DataUpdateRect);
RECT ButtonsDataWipeandBelow(char* mytimedata, RECT LocalRect, RECT TempRect, HDC hdc, HBRUSH frameBrush, int* step, RECT DataUpdateRect);

LRESULT CALLBACK ButtonProcD(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FontBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
int CALLBACK EnumFontFamilyProc(const LOGFONT* lpelfe, const void* lpntme, DWORD      FontType, LPARAM     lParam);

HWND datetickfunction(int datetick, char* filebuffer, int* i);
BOOL DateColoring(int colorid, HWND refferentdate, HDC hdc, char* filebuffer, int* i);
BOOL DateShaping(int shapeid, char* filebuffer, int* i, int datetick);
HWND ColorRemovalRoutine(int offsetpresent, int shiuze, OVERLAPPED fuckshit, HANDLE hFile, HWND refferentdate, int datetick);
HWND MarkRemoveRoutine(int offsetpresent, int shiuze, OVERLAPPED fuckshit, HANDLE hFile, int datetick, HWND refferentdate);
char* marksbuffermodifier(int shapeid, int colorid, char* filebuffer, int datetick, int* i, int presentshapeval, HWND refferentdate, HDC hdc, int presentcolorval);
void markspaint(BOOL* dateflags);
bool marksmonthcheck(char* marksbuffer, int filesize);
char* tempbuffermarkchecker(BOOL* flag69, int* checker, char* tempbuffer2, char* TempBuffer, BOOL lswitch, int* day, BOOL* mydflags, int* i, int month, int* indexplace, int* colorvalue, int* shapevalue);
HWND colorshapepresent(HWND refferentbutton, BOOL* mydflags, int shape, int color, int day);
char* dataovverridingsmarks(char* tempbuffer2, char* TempBuffer, BOOL* flag69, int day, int* indexplace, int* colorvalue, int* shapevalue, int* checker, int* i);
void ColorsMaker(RGBData mycolors[]);

int LoadConfigData(size_t filesize, char* tempstring, BOOL readflag, HANDLE hFile);
int ArrangeDates(char* tempdate, int datesamount, pdate datesarray, lpdateanddata mydataset);
void FindFirstDate(pdate datesarray, int i, char* date, char* readbuffer, int datesamount, int stringlength, lpdateanddata mydataset);
void IFComparison(char* macroformated, int dummyint, double ifstatement2, int* ifnumber, double* ifstatement1, int* i, BOOL* myswitch, int length, int ifsign, int* n);
void ButtonsFunction(LPARAM lParam, HWND hwnd, int step);

void* SafeCalloc(size_t size, int bytes);
void GetWindowRectSafe(HWND hwnd, RECT * rectval);
void MapWindowPointsSafe(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints);
void UpdateWindowSafe(HWND hwnd);
void InvalidateRectSafe(HWND hwnd, RECT* lpRect, BOOL bErase);
void MoveWindowSafe(HWND hwnd, int x, int y, int width, int height, BOOL repaint);
HDC safeGetDC(HWND hwnd);
void SafewriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumbrofBytesToWrite, LPDWORD lpNumberofbytestobewritten, LPOVERLAPPED lpoverlapped);
void SafeCloseHandle(HDC hdc);
BOOL SafetGetClientRect(HWND hwnd, LPRECT lpRect);
HBRUSH SafetCreateSolidBrush(COLORREF rgb);
void SafeFillRect(HDC hdc, RECT* lprc, HANDLE hbr);
void SafeDeleteObject(HGDIOBJ ho);
void SafeReleaseDC(HWND hwnd, HDC hdc);
void SafeSetBkColor(HDC hdc, COLORREF color);
void SafeSetTextColor(HDC hdc, COLORREF color);
void SafeSelectObject(HDC hdc, HGDIOBJ h);
void SafeTextOutA(HDC hdc, int x, int y, LPCSTR lpString, int c);
void SafeFrameRect(HDC hdc, RECT* myrect, HBRUSH hBrush);
void SafeSetFocus(HWND hwnd);
void safe_itoa_s(int num, char* buff, size_t buffercount, int radix);
HWND SafeGetParent(HWND hwnd);