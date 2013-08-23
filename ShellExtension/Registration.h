/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Registration.h
 *  The WinUnionFS Project
 *
 *  Handles registration and unregistration of this DLL.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

// Helper functions
LSTATUS RegSetString(HKEY key, LPCWSTR value, LPCWSTR data);
LSTATUS RegPrintf(HKEY key, LPCWSTR value, LPCWSTR format, ...);

// Elevation stuff
bool IsElevated();
HRESULT RunElevated(LPCWSTR func);
void CALLBACK DllElevatedEntry(HWND hwnd, HINSTANCE instance, LPSTR cmdLine, int cmdShow);

// The various registry keys used
typedef enum {
    KEY_CLSID,
    KEY_NAMESPACE
} KEY_TYPE;

// Registration/Unregistration
HRESULT GetKeyPath(KEY_TYPE type);
HRESULT DeleteRegKeys();
HRESULT RegisterDLL();
HRESULT RegisterNamespace();

// Exports
STDAPI DllRegisterServer();
STDAPI DllUnregisterServer();
