#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <Shlobj.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")


#include "./include/detours.h"
#if defined _M_X64
constexpr auto SYSTEMBIT = "x64";
#pragma comment(lib, "./lib.X64/detours.lib")
#elif defined _M_IX86
constexpr auto SYSTEMBIT = "x32";
#pragma comment(lib, "./lib.X86/detours.lib")
#endif

WNDPROC WndProc;
void HookApi();
extern "C" __declspec(dllexport) void Parsing()
{    
    HookApi();
}
#pragma comment(linker, "/export:DllCanUnloadNow=C:\\Windows\\System32\\explorerframe.DllCanUnloadNow")
#pragma comment(linker, "/export:DllGetClassObject=C:\\Windows\\System32\\explorerframe.DllGetClassObject")

typedef BOOL(__stdcall* DGetWindowBand)(HWND hWnd, DWORD dwBand);
DGetWindowBand pGetWindowBand = (DGetWindowBand)GetProcAddress(LoadLibraryW(L"user32.dll"), "GetWindowBand");

typedef BOOL(__stdcall* DSetWindowBand)(HWND hWnd, HWND hWndInsertAfter, DWORD dwBand);
DSetWindowBand pSetWindowBand = (DSetWindowBand)GetProcAddress(LoadLibraryW(L"user32.dll"), "SetWindowBand");

typedef HWND(WINAPI* DCreateWindowInBand)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam, DWORD dwBand);
DCreateWindowInBand pCreateWindowInBand = (DCreateWindowInBand)GetProcAddress(LoadLibraryW(L"user32.dll"), "CreateWindowInBand");

typedef int(WINAPI* DBrandingLoadString)(const wchar_t* lpBasebrd, unsigned int uid, wchar_t* lpBuffer, int size);
DBrandingLoadString pBrandingLoadString = (DBrandingLoadString)GetProcAddress(LoadLibraryW(L"winbrand.dll"), "BrandingLoadString");

typedef wchar_t* (WINAPI* DBrandingFormatStringForEdition)(const wchar_t* lpBuffer, int EditionId, unsigned int size);
DBrandingFormatStringForEdition pBrandingFormatStringForEdition = (DBrandingFormatStringForEdition)GetProcAddress(LoadLibraryW(L"WINBRAND.DLL"), "BrandingFormatStringForEdition");
//typedef int(WINAPI* DLoadStringW)(HINSTANCE hInt, UINT uid, LPWSTR lpBuffer, int sizeBuffer);
//DLoadStringW pLoadStringW;
static int(WINAPI* DLoadStringW)(HINSTANCE hInt, UINT uid, LPWSTR lpBuffer, int sizeBuffer) = LoadStringW;

static int(WINAPI* DGetTimeFormatW)(LCID Locale, DWORD dwFlags, const SYSTEMTIME* lpTime, PCWSTR lpFormat, LPWSTR lpTimeStr, int cchTime) = GetTimeFormatW;
static int(WINAPI* DGetDateFormatW)(LCID Locale, DWORD dwFlags, const SYSTEMTIME* lpDate, LPCWSTR lpFormat, LPWSTR lpDateStr, int cchDate) = GetDateFormatW;
static BOOL(WINAPI* DExtTextOutW)(HDC hdc, int x, int y, UINT options, const RECT* lprect, LPCWSTR lpString, UINT c, const INT* lpDx) = ExtTextOutW;

std::wstring string_replace(std::wstring src, std::wstring const& target, std::wstring const& repl)
{
    if ((target.length() == 0) || (src.length() == 0))
        return src;
    size_t idx = 0;
    for (;;)
    {
        idx = src.find(target, idx);
        if (idx == std::wstring::npos)
            break;

        src.replace(idx, target.length(), repl);
        idx += repl.length();
    }
    return src;
}

int WINAPI myGetDateFormatW(LCID Locale, DWORD dwFlags, const SYSTEMTIME* lpDate, LPCWSTR lpFormat, LPWSTR lpDateStr, int cchDate)
{
    int res = DGetDateFormatW(Locale, dwFlags, lpDate, lpFormat, lpDateStr, cchDate);
    memset(lpDateStr, '\0', cchDate);
    return res;
}
int WINAPI myGetTimeFormatW(LCID Locale, DWORD dwFlags, const SYSTEMTIME* lpTime, PCWSTR lpFormat, LPWSTR lpTimeStr, int cchTime)
{
    int res = DGetTimeFormatW(Locale, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
    memset(lpTimeStr, '\0', cchTime);
    return res;
}

int WINAPI myLoadStringW(HINSTANCE hInt, UINT uid, LPWSTR lpBuffer, int sizeBuffer)
{
    int res= DLoadStringW(hInt, uid, lpBuffer, sizeBuffer);
    std::wstringstream ss;
    ss << "Function: " << __FUNCTION__ << " : " << uid << " : " << lpBuffer << "\n";
    OutputDebugStringW(ss.str().c_str());
    //if (uid == 102 || uid == 33088 || uid == 33089 || uid == 33108 || uid == 33109 || uid == 33111 || uid == 3311)
    if (uid == 102)
    {
        memset(lpBuffer, '\0', sizeBuffer);
    }
    return res;
}
int WINAPI myBrandingLoadString(const wchar_t* lpBasebrd, unsigned int uid, wchar_t* lpBuffer, int size)
{
    int res = pBrandingLoadString(lpBasebrd,uid,lpBuffer, size);
    std::wstringstream ss;
    ss << "Function: " << __FUNCTION__ << " : " << uid << " : " << lpBuffer << "\n";
    OutputDebugStringW(ss.str().c_str());
    if (wcsstr(lpBuffer, L" Insider Preview") != 0)
    {
        std::wstring wsoriginal = lpBuffer;
        memset(lpBuffer, '\0', size);
        std::wstring wspatterntofind = L" Insider Preview";
        std::wstring wsreplacement = L"";
        std::wstring wsOut = string_replace(wsoriginal, wspatterntofind, wsreplacement);
        memcpy(lpBuffer, wsOut.c_str(), wsOut.length() * 2);
    }
    return res;
}
wchar_t* WINAPI myBrandingFormatStringForEdition(const wchar_t* lpBuffer, int EditionId, unsigned int size)
{
    wchar_t* res = pBrandingFormatStringForEdition(lpBuffer, EditionId, size);
    std::wstringstream ss;
    ss << "Function: " << __FUNCTION__ << " : " << EditionId << " : " << res << "\n";
    OutputDebugStringW(ss.str().c_str());
    if (wcsstr(res, L" Insider Preview") != 0)
    {
        std::wstring wsoriginal = res;
        memset(res, '\0', wsoriginal.length());
        std::wstring wspatterntofind = L" Insider Preview";
        std::wstring wsreplacement = L"";
        std::wstring wsOut = string_replace(wsoriginal, wspatterntofind, wsreplacement);
        memcpy(res, wsOut.c_str(), wsOut.length() * 2);
    }
    return res;
}

BOOL WINAPI myExtTextOutW(HDC hdc, int x, int y, UINT options, const RECT* lprect, LPCWSTR lpString, UINT c, const INT* lpDx)
{
    if (c > 0)
    {
        if (wcsstr(lpString, L"Windows ") != 0 || wcsstr(lpString, L" Build ") != 0)
        {
            return DExtTextOutW(hdc, x, y, options, lprect, L"", 0, lpDx);
        }
    }
    BOOL res = DExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);
    return res;
}

BOOL __stdcall myGetWindowBand(HWND hWnd, PDWORD pdwBand)
{
    DWORD dwBand = 0;
    BOOL res = pGetWindowBand(hWnd, &dwBand);
    if (dwBand == 14)
    {
        //wchar_t wszWindowText[4096];
        //GetWindowTextW(hWnd, wszWindowText, 4096);
        //std::wstringstream ss;
        //ss << "Function: " << __FUNCTION__ << " dwBand:" << dwBand << " hWnd:" << hWnd << " wszWindowText:" << wszWindowText << "\n";
        //OutputDebugStringW(ss.str().c_str());
        while (TRUE) {
            LONG_PTR status = GetWindowLongPtrW(hWnd, GWL_STYLE);
            if (status & WS_VISIBLE) {
                pSetWindowBand(hWnd,0, 1);
                ShowWindow(hWnd, SW_HIDE);
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
    }
    return res;
}

void HookApi()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pBrandingFormatStringForEdition, myBrandingFormatStringForEdition);
    DetourAttach(&(PVOID&)pBrandingLoadString, myBrandingLoadString);
    DetourAttach(&(PVOID&)DLoadStringW, myLoadStringW);
    DetourAttach(&(PVOID&)DGetDateFormatW, myGetDateFormatW);
    DetourAttach(&(PVOID&)DGetTimeFormatW, myGetTimeFormatW);
    DetourAttach(&(PVOID&)DExtTextOutW, myExtTextOutW);
    //DetourAttach(&(PVOID&)pGetWindowBand, myGetWindowBand);
    DetourTransactionCommit();
}

//void UnHookApi()
//{
//    DetourTransactionBegin();
//    DetourUpdateThread(GetCurrentThread());
//    DetourDetach(&(PVOID&)pBrandingFormatStringForEdition, myBrandingFormatStringForEdition);
//    DetourDetach(&(PVOID&)pBrandingLoadString, myBrandingLoadString);
//    DetourDetach(&(PVOID&)DLoadStringW, myLoadStringW);
//    DetourDetach(&(PVOID&)DGetDateFormatW, myGetDateFormatW);
//    DetourDetach(&(PVOID&)DGetTimeFormatW, myGetTimeFormatW);
//    DetourTransactionCommit();
//}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hInstance);
        //MessageBoxW(NULL, L"Hooked", L"Hook", MB_OK);
        //HMODULE handles = GetModuleHandleW(L"shell32.dll");
        //if (handles)
        //{
        //    HMODULE handle = GetModuleHandleW(L"api-ms-win-core-libraryloader-l1-2-0.dll");
        //    if (GetProcAddress(handle, "LoadStringW"))
        //    {
        //        pLoadStringW = (DLoadStringW)GetProcAddress(handle, "LoadStringW");
        //    }
        //    else
        //    {
        //        handle = GetModuleHandleW(L"api-ms-win-core-libraryloader-l1-1-1.dll");
        //        if (GetProcAddress(handle, "LoadStringW"))
        //        {
        //            pLoadStringW = (DLoadStringW)GetProcAddress(handle, "LoadStringW");
        //        }
        //        else
        //        {
        //            pLoadStringW = LoadStringW;
        //        }
        //    }
        //}
        //CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HookApi, 0, 0, 0);      
        HookApi();
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

