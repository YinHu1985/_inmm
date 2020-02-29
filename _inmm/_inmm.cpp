// _inmm.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "_inmm.h"
#include   <ddraw.h>
#include "gdiplus.h"
using namespace Gdiplus;

HMODULE hWinmm = NULL;

bool init = false;
HFONT fontlist[30]; 
int heights[30];
int colortable[255];

Image* bmplist[256][256][256];
int shadow = 100;
int DH = 0;
FILE* tplog;
int debug = 0;
int logcount = 0;

typedef DWORD (__stdcall *pmciSendCommandA)(UINT,
                                            UINT,
                                            DWORD,
                                            DWORD_PTR); 
typedef DWORD (__stdcall *ptimeBeginPeriod)(UINT); 
typedef DWORD (__stdcall *ptimeGetDevCaps)( UINT,  
                                           UINT); 
typedef DWORD (__stdcall *ptimeGetTime)(void); 
typedef DWORD (__stdcall *ptimeKillEvent)(UINT); 
typedef DWORD (__stdcall *ptimeSetEvent)(UINT, 
                                         UINT  , 
                                         UINT ,  
                                         UINT   ,      
                                         UINT     ); 

//LPDIRECTDRAW m_pDD;
unsigned int CreateRGB( int r, int g, int b )
{
    int g_iBpp = 16;
    switch (g_iBpp)
    {
    case 8:
        // Here you should do a palette lookup to find the closes match.
        // I'm not going to bother with that. Many modern games no
        // longer support 256-color modes, and neither should you :)
        return 0;
    case 16:
        // Break down r,g,b into 5-6-5 format.
        return ((r/8)<<11) | ((g/4)<<5) | (b/8);
    case 24:
    case 32:
        return (r<<16) | (g<<8) | (b);
    }
    return 0;
}

Image* LoadBMP(WCHAR* filename,HDC hdc)
{
    //Bitmap* BackBMP;
    Image* BackBMP;
    BackBMP = Image::FromFile(filename);
    return BackBMP;
};

bool Initialize(void)
{
    FILE* fp;
    errno_t aaa;

    for(int i = 0; i < 256; i ++)
    {
        for(int j = 0; j < 256; j ++)
        {
            for(int k = 0; k < 256; k ++)
            {
                bmplist[i][j][k] = NULL;
            }
        }
    }
    for(int i = 0; i < 30; i ++)
    {	
        fontlist[i] = NULL;
        heights[i] = 0;
    }
    for(int i = 0; i < 256; i ++)
    {	
        colortable[i] = -1;
        switch((unsigned char)i)
        {
        case 'W':colortable[i] = 0x00FFFFFF;break;
        case 'Y':colortable[i] = 0x00F8E8D0;break;
        case 'R':colortable[i] = 0x000000ff;break;
        case 'G':colortable[i] = 0x0000AA00;break;
        case 'B':colortable[i] = 0x00FF0000;break;
        case 'M':colortable[i] = 0x00660088;break;
        case 'N':colortable[i] = 0x00FF00FF;break;
        case 'Z':colortable[i] = 0x00000000;break;
        default:break;
        }
    }

    //default
    fontlist[0] = CreateFont(12,0,0,0, FW_REGULAR ,FALSE,FALSE,FALSE,
        GB2312_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH ,L"ËÎÌו");

    char tp[50];

    aaa = fopen_s(&fp,"_INMM.ini","r");
    while(!feof(fp))
    {
        fscanf_s(fp,"%s",tp,50);
        int i = 0;
        while(tp[i] == 0x20)i ++;

        if(tp[i] == '[')continue;
        if(tp[i] == '#')continue;
        if(tp[i] == 0)continue;
        if(tp[i] == 'S')
        {
            sscanf_s(tp,"Shadow=%d",&shadow);
            continue;
        }
        if(tp[i] == 'D')
        {
            sscanf_s(tp,"DarkestHour=%d",&DH);
            continue;
        }
        if(tp[i] == 'd')
        {
            sscanf_s(tp,"debug=%d",&debug);
            continue;
        }
        if(tp[i] == 'C')
        {
            int color;
            char colorindex;
            sscanf_s(tp,"Color%c=$%x",&colorindex,1,&color);
            colortable[colorindex]=color;
            continue;
        }
        if(tp[i] == 'M')
        {
            int code;
            sscanf_s(tp,"MagicCode=%d",&code);
            while(true)
            {
                fscanf_s(fp,"%s",tp,50);

                i = 0;
                while(tp[i] == 0x20)i ++;

                if(tp[i] == '[')continue;
                if(tp[i] == '#')continue;
                if(tp[i] == 0)continue;

                if(tp[i] == 'F')
                {
                    char font[50];

                    sscanf_s(tp,"Font=%s",font,50);
                    while(true)
                    {
                        fscanf_s(fp,"%s",tp,50);

                        i = 0;
                        while(tp[i] == 0x20)i ++;

                        if(tp[i] == '[')continue;
                        if(tp[i] == '#')continue;
                        if(tp[i] == 0)continue;

                        if(tp[i] == 'H')
                        {
                            int height;
                            WCHAR fontw[50];
                            ::OemToChar(font,fontw);
                            sscanf_s(tp,"Height=%d",&height);
                            fontlist[code] = CreateFont(0-height,0,0,0, FW_REGULAR ,FALSE,FALSE,FALSE,
                                GB2312_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH ,fontw);
                            heights[code] = height;
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    fclose(fp);
    if(debug==1)
    {
        fopen_s(&tplog,"_inmmdebug.txt","w");
    }
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    init = true;
    return true;
}

extern "C" _INMM_API DWORD mciSendCommandA(
    UINT IDDevice,
    UINT uMsg,
    DWORD fdwCommand,
    DWORD_PTR dwParam
    )
{
    if(hWinmm == NULL)
    {
        hWinmm = ::LoadLibrary(L"winmm.dll");
    }
    pmciSendCommandA faddr;
    faddr = (pmciSendCommandA)GetProcAddress(hWinmm,"mciSendCommandA");
    return (faddr)( IDDevice,
        uMsg,
        fdwCommand,
        dwParam
        );
}

extern "C" _INMM_API DWORD timeBeginPeriod(
    UINT uPeriod  
    )
{
    if(hWinmm == NULL)
    {
        hWinmm = ::LoadLibrary(L"winmm.dll");
    }
    ptimeBeginPeriod faddr;
    faddr = (ptimeBeginPeriod)GetProcAddress(hWinmm,"timeBeginPeriod");
    return (faddr)(uPeriod );
}

extern "C" _INMM_API DWORD timeGetDevCaps(
    UINT ptc,  
    UINT cbtc        
    )
{
    if(hWinmm == NULL)
    {
        hWinmm = ::LoadLibrary(L"winmm.dll");
    }
    ptimeGetDevCaps faddr;
    faddr = (ptimeGetDevCaps)GetProcAddress(hWinmm,"timeGetDevCaps");
    return (faddr)(ptc,cbtc);
}

extern "C" _INMM_API DWORD timeGetTime(void)
{
    if(hWinmm == NULL)
    {
        hWinmm = ::LoadLibrary(L"winmm.dll");
    }
    ptimeGetTime faddr;
    faddr = (ptimeGetTime)GetProcAddress(hWinmm,"timeGetTime");
    return (faddr)();
}

extern "C" _INMM_API DWORD timeKillEvent(UINT uTimerID )
{
    if(hWinmm == NULL)
    {
        hWinmm = ::LoadLibrary(L"winmm.dll");
    }
    ptimeKillEvent faddr;
    faddr = (ptimeKillEvent)GetProcAddress(hWinmm,"timeKillEvent");
    return (faddr)(uTimerID);
}

extern "C" _INMM_API DWORD timeSetEvent(
    UINT           uDelay,      
    UINT           uResolution, 
    UINT lpTimeProc,  
    UINT      dwUser,      
    UINT           fuEvent      
    )
{
    if(hWinmm == NULL)
    {
        hWinmm = ::LoadLibrary(L"winmm.dll");
    }
    ptimeSetEvent faddr;
    faddr = (ptimeSetEvent)GetProcAddress(hWinmm,"timeSetEvent");
    return (faddr)(uDelay,uResolution,lpTimeProc,dwUser, fuEvent     );
}

extern "C" _INMM_API void TextOutDC2(LPRECT lpRect, int *px, int *py, LPBYTE lpString, LPDIRECTDRAWSURFACE lpDDS, int nMagicCode, DWORD dwColor, DWORD dwFlags)
    //extern "C" _INMM_API void TextOutDC0(int x,int y,char* text,int d,int magicnum,int color,int gg)
{

    int x;
    int y;
    char* text;
    int d;
    int magicnum;
    int color;
    int gg;
    x = *px;
    y = *py;
    text = (char*)lpString;
    d = (int)lpDDS;
    magicnum=nMagicCode;
    color = dwColor;
    gg = dwFlags;

    int tpx;

    if(!init)Initialize();

    if(debug == 1)
    {
        logcount ++;
        if(logcount > 10000)
        {
            logcount = 0;
            fclose(tplog);
            fopen_s(&tplog,"_inmmdebug.txt","w");
        }
        //FILE* tplog;
        fprintf_s(tplog,"TextOut:\n%s\n",text);
    }

    HDC hdc;
    LPDIRECTDRAWSURFACE   lpDDSPrimary;
    lpDDSPrimary = (LPDIRECTDRAWSURFACE)d;
    if(lpDDSPrimary->GetDC(&hdc) == DD_OK)
    {
        SetBkMode(hdc, TRANSPARENT);
        if(fontlist[magicnum] != NULL)
        {
            SelectObject(hdc, fontlist[magicnum]); 
        }
        else
        {
            SelectObject(hdc, fontlist[0]); 
        }

        char tp[5000];
        int currentcolor;
        unsigned char r,g,b;
        r = color>>11;
        r = r*8;
        g = (color - ((color>>11)<<11))>>5;
        g = g*4;
        b = (color<<27)>>27;
        b = b*8;

        int origincolor;
        currentcolor = RGB(r,g,b);
        origincolor = currentcolor;
        int nextcolor;
        nextcolor = currentcolor;
        tpx = x;

        int tpindex = 0;
        for(int i = 0; i < 5000; i ++)
        {
            if(text[i] == 0)
            {
                tp[tpindex] = 0;
                break;
            }
            if(text[i] == '@')
            {
                tp[tpindex] = 0;
                if(0.299 *(unsigned char)GetRValue(currentcolor)+0.587 *(unsigned char)GetGValue(currentcolor) +0.114*(unsigned char)GetBValue(currentcolor) > shadow)
                {
                    SetTextColor(hdc, 0x000000);
                    if (lpRect != NULL)
                    {
                        ExtTextOutA(hdc, tpx+1,y+1, ETO_CLIPPED, lpRect, tp, tpindex, NULL);
                    }
                    else TextOutA(hdc,tpx+1,y+1,tp,tpindex);
                }

                SetTextColor(hdc, currentcolor);

                if (lpRect != NULL)
                    ExtTextOutA(hdc, tpx,y, ETO_CLIPPED, lpRect, tp, tpindex, NULL);
                else 
                    TextOutA(hdc,tpx,y,tp,tpindex);

                tpx += GetTextWidth(tp,magicnum,0);
                tpindex = 0;
                WCHAR tpfilename[50];
                WCHAR tpfilename1[50];

                tpfilename1[0] = text[i+1];
                tpfilename1[1] = text[i+2];
                tpfilename1[2] = text[i+3];
                tpfilename1[3] = '.';
                tpfilename1[4] = 'p';
                tpfilename1[5] = 'n';
                tpfilename1[6] = 'g';
                tpfilename1[7] = 0;

                swprintf_s(tpfilename,50,L"extragfx\\%s",tpfilename1);

                if(bmplist[(unsigned char)text[i+1]][(unsigned char)text[i+2]][(unsigned char)text[i+3]]==NULL)
                {
                    bmplist[(unsigned char)text[i+1]][(unsigned char)text[i+2]][(unsigned char)text[i+3]] = LoadBMP(tpfilename,hdc);
                }

                Image* tpbitmap;
                tpbitmap = bmplist[(unsigned char)text[i+1]][(unsigned char)text[i+2]][(unsigned char)text[i+3]];
                if(tpbitmap!= NULL)
                {
                    Graphics graphics(hdc);
                    graphics.DrawImage(tpbitmap, tpx,y,heights[magicnum],heights[magicnum]);
                    //StretchBlt(hdc,tpx,y,heights[magicnum],heights[magicnum],BitmapDC,0,0,100,100,SRCCOPY);
                    //	DeleteDC(BitmapDC);
                    tpx+=heights[magicnum];
                }
                i+=3;
            }
            else if((unsigned char)text[i] == 0xa7)
            {
                bool draw = true;
                if((unsigned char)text[i+1] == 0xa7&&((unsigned char)text[i+2]==0x20))//a special case,maybe a bug
                {
                    nextcolor = origincolor;
                    //draw = false;
                }
                else if((unsigned char)text[i+1] == 0xa7&&DH==1)//a special case for Darkest Hour
                {
                    nextcolor = origincolor;
                    //draw = false;
                }
                else if(text[i+1] >= '0'&&text[i+1] <= '9'&&text[i+2] >= '0'&&text[i+2] <= '9'&&text[i+3] >= '0'&&text[i+3] <= '9')
                {

                    b = ((int)(text[i+1]-'0'))*32;
                    g = ((int)(text[i+2]-'0'))*32;
                    r = ((int)(text[i+3]-'0'))*32;
                    nextcolor = RGB(r,g,b);
                    i+=2;
                }
                /*
                else if(i == 0&&text[i+1] == '0'&&text[i+2]=='6'&&text[i+3]=='0')
                {
                nextcolor = origincolor;
                i+=2;
                }
                else if(i == 0&&text[i+1] == '0'&&text[i+2]=='0'&&text[i+3]=='7')//another two stupid bug
                {
                nextcolor = origincolor;
                i+=2;
                }
                */
                else
                {
                    if(colortable[(unsigned char)text[i+1]] == -1)
                    {
                        draw = false;
                    }
                    else
                    {
                        nextcolor = colortable[(unsigned char)text[i+1]];
                    }
                }
                if(draw)
                {
                    tp[tpindex] = 0;
                    if(0.299 *(unsigned char)GetRValue(currentcolor)+0.587 *(unsigned char)GetGValue(currentcolor) +0.114*(unsigned char)GetBValue(currentcolor) > shadow)
                    {
                        SetTextColor(hdc, 0x000000);
                        if (lpRect != NULL)
                        {
                            ExtTextOutA(hdc, tpx+1,y+1, ETO_CLIPPED, lpRect, tp, tpindex, NULL);
                        }
                        else TextOutA(hdc,tpx+1,y+1,tp,tpindex);
                    }
                    SetTextColor(hdc, currentcolor);
                    if (lpRect != NULL)
                    {
                        ExtTextOutA(hdc, tpx,y, ETO_CLIPPED, lpRect, tp, tpindex, NULL);
                    }
                    else TextOutA(hdc,tpx,y,tp,tpindex);
                    tpx += GetTextWidth(tp,magicnum,0);
                    tpindex = 0;
                    i ++;
                    currentcolor = nextcolor;
                }
                else
                {
                    tp[tpindex] = text[i];
                    tpindex ++;
                }
            }
            else if((unsigned char)text[i] == '\\')
            {
                if((unsigned char)text[i+1] == '%')
                {
                    tp[tpindex] = '%';
                    tpindex ++;
                    i++;
                }
                else
                {
                    tp[tpindex] = text[i];
                    tpindex ++;
                }
            }
            else if(text[i] == '%')
            {
                if(text[i+1] >= '0'&&text[i+1] <= '9')
                {
                    if(text[i+2] >= '0'&&text[i+2] <= '9')
                    {	
                        if(text[i+3] >= '0'&&text[i+3] <= '9')
                        {
                            b = ((int)(text[i+1]-'0'))*32;
                            g = ((int)(text[i+2]-'0'))*32;
                            r = ((int)(text[i+3]-'0'))*32;
                            currentcolor = RGB(r,g,b);
                            i+=3;
                        }
                        else
                        {
                            tp[tpindex] = text[i];
                            tpindex ++;
                        }
                    }
                    else
                    {
                        tp[tpindex] = text[i];
                        tpindex ++;
                    }
                }
                else
                {
                    tp[tpindex] = text[i];
                    tpindex ++;
                }
            }
            else if(text[i] < 0 && text[i + 1] != 0)
            {
                tp[tpindex] = text[i];
                tpindex ++;
                i++;
                tp[tpindex] = text[i];
                tpindex ++;
            }
            else
            {
                tp[tpindex] = text[i];
                tpindex ++;
            }
        }

        if(0.299 *(unsigned char)GetRValue(currentcolor)+0.587 *(unsigned char)GetGValue(currentcolor) +0.114*(unsigned char)GetBValue(currentcolor) > shadow)
        {
            SetTextColor(hdc, 0x000000);
            if (lpRect != NULL)
            {
                ExtTextOutA(hdc, tpx+1,y+1, ETO_CLIPPED, lpRect, tp, tpindex, NULL);
            }
            else TextOutA(hdc,tpx+1,y+1,tp,tpindex);
        }
        SetTextColor(hdc, currentcolor);

        if (lpRect != NULL)
        {
            ExtTextOutA(hdc, tpx,y, ETO_CLIPPED, lpRect, tp, tpindex, NULL);
        }
        else TextOutA(hdc,tpx,y,tp,tpindex);

        lpDDSPrimary->ReleaseDC(hdc);
        //	lpDDSPrimary->Flip(lpDDSPrimary,0);
    }
    *px = tpx;
    *py = y;
    return ;
};

extern "C" _INMM_API int GetTextWidth(char* text,int magicnum,int c)
{
    if(!init)Initialize();

    if(debug == 1)
    {
        //	FILE* tplog;
        logcount ++;
        if(logcount > 10000)
        {
            logcount = 0;
            fclose(tplog);
            fopen_s(&tplog,"_inmmdebug.txt","w");
        }
        fprintf_s(tplog,"GetWidth:\n%s\n",text);	
    }

    int result = 0;
    for(int i = 0; i < 1000;i ++)
    {
        if(text[i] == 0)
        {
            break;
        }
        if(text[i] == '@')
        {
            i+=3;
            result += 2;
        }
        else if((unsigned char)text[i] == 0xa7)
        {
            if((unsigned char)text[i+1] == 0xa7)
            {
                i++;
                //result ++;
            }
            else if(text[i+1] >= '0'&&text[i+1] <= '9'&&text[i+2] >= '0'&&text[i+2] <= '9'&&text[i+3] >= '0'&&text[i+3] <= '9')
            {					
                i+=3;
            }
            /*
            else if(i == 0&&text[i+1] == '0'&&text[i+2]=='6'&&text[i+3]=='0')
            {
            i+=3;
            }
            else if(i == 0&&text[i+1] == '0'&&text[i+2]=='0'&&text[i+3]=='7')//another two stupid bug
            {
            i+=3;
            }
            */
            else
            {
                if(colortable[(unsigned char)text[i+1]] == -1)
                {
                    result ++;
                }
                else
                {
                    i++;
                }
            }
        }
        else if((unsigned char)text[i] == '\\')
        {
            if((unsigned char)text[i+1] == '%')
            {
                result ++;
                i++;
            }
            else
            {
                result ++;
            }
        }
        else if((unsigned char)text[i] == '%')
        {
            i+=3;
        }
        else if(text[i] < 0)
        { 
            i++;  
            if(text[i]==0)
            {
                break;
            }
            result +=2;
        }
        else
        {
            result ++;
        }
    }
    //if(result > 2)result --;

    if(fontlist[magicnum]!=NULL)
    {
        result *= (heights[magicnum]/2);
    }
    else
    {
        result *= 6;//default
    }
    if (result > 0)
        result += (c%2);
    return result;
    //return 1000;
};

extern "C" _INMM_API void TextOutDC0(int x, int y, LPBYTE lpString, LPDIRECTDRAWSURFACE lpDDS, int nMagicCode, DWORD dwColor, DWORD dwFlags)
{
    TextOutDC2(NULL, &x, &y, lpString, lpDDS, nMagicCode, dwColor, dwFlags);
}

extern "C" _INMM_API void TextOutDC1(LPRECT lpRect, int x, int y, LPBYTE lpString, LPDIRECTDRAWSURFACE lpDDS, int nMagicCode, DWORD dwColor, DWORD dwFlags)
{
    TextOutDC2(lpRect, &x, &y, lpString, lpDDS, nMagicCode, dwColor, dwFlags);
}

extern "C" _INMM_API size_t strnlen0(char* text, size_t maxlen)
{
    for (int i = 0; i < maxlen; i++)
    {
        if (text[i] == 0)
        {
            return i;
        }
    }
    return maxlen;
}

extern "C" _INMM_API int CalcLineBreak(LPBYTE lpBuffer, const BYTE* lpString)
{
    LPBYTE target = lpBuffer;
    const char* text = (const char*)lpString;

    for (int i = 0; i < 100; ++ i)
    {
        if(text[i] == 0)
        {
            break;
        }
        if(text[i] == '@')
        {
            *target++ = text[i++]; 
            *target++ = text[i++]; 
            *target++ = text[i++]; 
            *target++ = text[i]; 
        }
        else if((unsigned char)text[i] == 0xa7)
        {
            if((unsigned char)text[i+1] == 0xa7)
            {
                *target++ = text[i++]; 
                *target++ = text[i]; 
            }
            else if(text[i+1] >= '0'&&text[i+1] <= '9'&&text[i+2] >= '0'&&text[i+2] <= '9'&&text[i+3] >= '0'&&text[i+3] <= '9')
            {					
                *target++ = text[i++]; 
                *target++ = text[i++]; 
                *target++ = text[i++]; 
                *target++ = text[i]; 
            }
            /*
            else if(i == 0&&text[i+1] == '0'&&text[i+2]=='6'&&text[i+3]=='0')
            {

            }
            else if(i == 0&&text[i+1] == '0'&&text[i+2]=='0'&&text[i+3]=='7')//another two stupid bug
            {

            }
            */
            else
            {
                if(colortable[(unsigned char)text[i+1]] == -1)
                {
                    *target++ = text[i]; 
                }
                else
                {
                    *target++ = text[i++]; 
                    *target++ = text[i]; 
                }
            }
        }
        else if((unsigned char)text[i] == '\\')
        {
            if((unsigned char)text[i+1] == '%')
            {
                *target++ = text[i++]; 
                *target++ = text[i]; 
                break;
            }
            else
            {
                *target++ = text[i]; 
            }
        }
        else if((unsigned char)text[i] == '%')
        {
            if((unsigned char)text[i+1] >= '0' && (unsigned char)text[i+1] <= '9' &&
                (unsigned char)text[i+2] >= '0' && (unsigned char)text[i+2] <= '9' &&
                (unsigned char)text[i+3] >= '0' && (unsigned char)text[i+3] <= '9')
            {
                *target++ = text[i++]; 
                *target++ = text[i++]; 
                *target++ = text[i++]; 
                *target++ = text[i]; 
            }
            else
            {
                *target++ = text[i]; 
            }
        }
        else if(text[i] < 0)
        {
            if(text[i+1]!=0)
            {
                *target++ = text[i++]; 
                *target++ = text[i];
            }
            else 
            {
                *target++ = text[i];
                break;
            }
            while (text[i] != 0 &&  i < 100)
            {
                 if (text[i] == ' ')
                    break;
                if ((unsigned char)text[i+1] == 0xA3 || (unsigned char)text[i+1] == 0xA1)
                {
                    if (text[i+2] != 0 && 
                        !((unsigned char)text[i+1] == 0xA1 && (unsigned char)text[i+2] == 0xB0) &&
                        !((unsigned char)text[i+1] == 0xA1 && (unsigned char)text[i+2] == 0xAE))
                    {
                        ++ i;
                        *target++ = text[i++]; 
                        *target++ = text[i];
                        continue;
                    }
                }
                else if (text[i+1] == ' ')
                {
                    ++ i;
                    *target++ = text[i];
                    continue;
                }
                break;
            }
            break;
        }
        else
        {
            while (text[i] > 0 &&  i < 100)
            {
                *target++ = text[i]; 
                if (text[i] <= ' ' || text[i + 1] < 0 || text[i] == '-')
                    break;
                ++ i;
            }
            if (text[i] == '\n' || text[i] == '\r')
                -- target;
            if (text[i] <= ' ' )
                break;
            while (text[i] != 0 &&  i < 100)
            {
                if ((unsigned char)text[i+1] == 0xA3 || (unsigned char)text[i+1] == 0xA1)
                {
                    if (text[i+2] != 0 && 
                        !((unsigned char)text[i+1] == 0xA1 && (unsigned char)text[i+2] == 0xB0) &&
                        !((unsigned char)text[i+1] == 0xA1 && (unsigned char)text[i+2] == 0xAE))
                    {
                        ++ i;
                        *target++ = text[i++]; 
                        *target++ = text[i];
                        continue;
                    }
                }
                break;
            }
            break;
        }
    }
    *target = 0;
    return target - lpBuffer;

}
