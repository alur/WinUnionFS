/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Debug.cpp
 *  The WinUnionFS Project
 *
 *  Debugging functions.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#if defined(_DEBUG)
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <strsafe.h>
#include "Debug.h"

#include <fstream>
#include <string>


//
std::ofstream outStream;


/// <summary>
/// Sends a formatted (printf-style) message to the debug output window.
/// Automatically inserts \n at the end of the string.
/// </summary>
void DbgTraceMessage(LPCWSTR format, ...) {
    if (!outStream.is_open()) {
        outStream.open("c:\\debug.txt", std::ios::out | std::ios::app);
    }

    va_list args;
    WCHAR buffer[512];
    char wcBuffer[512];

    va_start(args, format);
    StringCchVPrintfExW(buffer, 512, NULL, NULL, STRSAFE_NULL_ON_FAILURE, format, args);
    va_end(args);
    
    OutputDebugStringW(buffer);
    OutputDebugStringW(L"\n");

    wcstombs(wcBuffer, buffer, 512);

    outStream << std::string(wcBuffer) << std::endl;

    outStream.flush();
}
#endif
