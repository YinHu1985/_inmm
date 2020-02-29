// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the _INMM_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// _INMM_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef _INMM_EXPORTS
#define _INMM_API __declspec(dllexport)
#else
#define _INMM_API __declspec(dllimport)
#endif

// This class is exported from the _inmm.dll
//class _INMM_API C_inmm {
//public:
//	C_inmm(void);
//	// TODO: add your methods here.
//};
//
//extern _INMM_API int n_inmm;
#include   <ddraw.h>
extern "C"
{

//_INMM_API int fn_inmm(void);

_INMM_API DWORD mciSendCommandA(
    UINT IDDevice,
    UINT uMsg,
    DWORD fdwCommand,
    DWORD_PTR dwParam
);

_INMM_API DWORD timeBeginPeriod(
  UINT uPeriod  
);

_INMM_API DWORD timeGetDevCaps(
  UINT ptc,  
  UINT cbtc        
);

_INMM_API DWORD timeGetTime(void);

_INMM_API DWORD timeKillEvent(UINT uTimerID );

_INMM_API DWORD timeSetEvent(
UINT           uDelay,      
  UINT           uResolution, 
  UINT lpTimeProc,  
  UINT      dwUser,      
  UINT           fuEvent      
);

//_INMM_API int TextOutDC0(int,int,char*,int,int,int,int);
//int WINAPI GetTextWidth(LPCBYTE lpString, int nMagicCode, DWORD dwFlags);
_INMM_API void  TextOutDC0(int x, int y, LPBYTE lpString, LPDIRECTDRAWSURFACE lpDDS, int nMagicCode, DWORD dwColor, DWORD dwFlags);
_INMM_API void  TextOutDC1(LPRECT lpRect, int x, int y, LPBYTE lpString, LPDIRECTDRAWSURFACE lpDDS, int nMagicCode, DWORD dwColor, DWORD dwFlags);
_INMM_API void  TextOutDC2(LPRECT lpRect, int *px, int *py, LPBYTE lpString, LPDIRECTDRAWSURFACE lpDDS, int nMagicCode, DWORD dwColor, DWORD dwFlags);
_INMM_API int CalcLineBreak(LPBYTE lpBuffer, const BYTE* lpString);
_INMM_API int GetTextWidth(char*,int,int);

}