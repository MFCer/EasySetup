
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FINDPROCDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FINDPROCDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ANYSHAREDLL_EXPORTS
#define ANYSHAREDLL_API __declspec(dllexport)
#else
#define ANYSHAREDLL_API __declspec(dllimport)
#endif

#ifdef _DEBUG
static void TRACE(LPCTSTR lpszFmt, ...);
#else
inline static void TRACE(...) {}
#endif
