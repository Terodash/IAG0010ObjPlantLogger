#pragma once
#include <string>
#include <iostream>
#include "afxwin.h"
#include "afxmt.h"

#if defined(UNICODE) || defined(_UNICODE)
#define _tcout std::wcout
#define _tcin std::wcin
#else
#define _tcout std::cout
#define _tcin std::cin
#endif

typedef std::basic_string<TCHAR> _tstring;

using namespace std;

class ReadingKeyboardThread :
	public CWinThread
{
public:
	ReadingKeyboardThread(CEvent * ptrStopEvent, CEvent * ptrCommandGot, CEvent * ptrCommandProcessed);
	virtual ~ReadingKeyboardThread(void);
	virtual int Run(void);
	virtual BOOL InitInstance() { return TRUE; }

private:
	CEvent * ptrStopEvent; 
	CEvent * ptrCommandGot;
	CEvent * ptrCommandProcessed;
};

