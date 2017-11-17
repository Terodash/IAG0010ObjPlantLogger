#pragma once
//#include "afxwin.h"
//#include "afxmt.h"
#include "ClientSocket.h"

#if defined(UNICODE) || defined(_UNICODE)
#define _tcout std::wcout
#else
#define _tcout std::cout
#endif

using namespace std;