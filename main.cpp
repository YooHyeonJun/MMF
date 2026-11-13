#define BLOCK_LEN 65536
#include <tchar.h>
#include <windows.h>
#include <stdio.h>

void ShowErrorMessage(HRESULT hr)
{
    LPTSTR pszMsg = NULL;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, HRESULT_CODE(hr), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&pszMsg, 0, NULL);

    if (pszMsg) {
        _tprintf(_T("Error 0x%08X: %s\n"), hr, pszMsg);
        LocalFree(pszMsg);
    } else {
        _tprintf(_T("Unknown error 0x%08X\n"), hr);
    }
}



int _tmain(int argc, _TCHAR* argv[]){
    LPCTSTR szSource = argv[1];
    LPCTSTR szTarget = argv[2];

    HANDLE hSource = INVALID_HANDLE_VALUE;
    HANDLE hMapSource = NULL;
    PVOID pSource = NULL;

    HANDLE hTarget = INVALID_HANDLE_VALUE;
    HANDLE hMapTarget = NULL;
    PVOID pTarget = NULL;


    try{
        hSource = CreateFile(szSource, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        if(hSource == INVALID_HANDLE_VALUE)
            throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());
        
        hTarget = CreateFile(szTarget, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
        if(hTarget == INVALID_HANDLE_VALUE)
            throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());

        DWORD dwFileSize = GetFileSize(hSource, NULL);
        hMapSource = CreateFileMapping(hSource, NULL, PAGE_READONLY, 0, dwFileSize, NULL);
        if(!hMapSource)
            throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());
        
        //Make Target MMF
        hMapTarget = CreateFileMapping(hTarget, NULL, PAGE_READWRITE, 0, dwFileSize, NULL);
        if(!hMapTarget)
            throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());


        DWORD dwCopyLenth = BLOCK_LEN;
        DWORD dwTransferred = 0;
        while(dwTransferred < dwFileSize){
            dwCopyLenth = dwFileSize - dwTransferred;
            if(dwCopyLenth >= BLOCK_LEN)
                dwCopyLenth = BLOCK_LEN;
            
            pSource = MapViewOfFile(hMapSource, FILE_MAP_READ, 0, dwTransferred, dwCopyLenth);
            if(!pSource)
                throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());
            
            pTarget = MapViewOfFile(hMapTarget, FILE_MAP_WRITE, 0, dwTransferred, dwCopyLenth);

            if(!pTarget)
                throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());

            CopyMemory(pTarget, pSource, dwCopyLenth);
            _tprintf(_T("offset=%6lu len=%5lu src=%p dst=%p\n"),
         dwTransferred, dwCopyLenth, pSource, pTarget);

            dwTransferred += dwCopyLenth;

            UnmapViewOfFile(pSource);
            pSource = NULL;
            UnmapViewOfFile(pTarget);
            pTarget = NULL;
        }
        SetFilePointer(hTarget, dwFileSize, NULL, FILE_BEGIN);
        SetEndOfFile(hTarget);
    }
    catch(HRESULT hr){
        ShowErrorMessage(hr);
    }
    if(pSource)
        UnmapViewOfFile(pSource);
    if(hMapSource)
        CloseHandle(hMapSource);
    if(hSource != INVALID_HANDLE_VALUE)
        CloseHandle(hSource);
    
    if(pTarget)
        UnmapViewOfFile(pTarget);
    if(hMapTarget)
        CloseHandle(hMapTarget);
    if(hTarget != INVALID_HANDLE_VALUE)
        CloseHandle(hTarget);

    return 0;
}