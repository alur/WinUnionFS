/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Debug.h
 *  The WinUnionFS Project
 *
 *  Debugging macros and functions. Taken from the LiteStep core.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#if defined(_DEBUG)
    #define TRACE  DbgTraceMessage
    void DbgTraceMessage(LPCWSTR format, ...);
#else
    #define TRACE
#endif
