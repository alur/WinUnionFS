/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Registration.cpp
 *  The WinUnionFS Project
 *
 *  Handles registration and unregistration of this DLL.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <Windows.h>
#include <ShlObj.h>
#include <strsafe.h>

#include "Debug.h"
#include "Registration.h"


// The handle to this DLL.
extern HMODULE module;

// The CLSID of WinUnionFS.
extern const CLSID CLSID_WinUnionFS;

// The keys used for registration.
#define CLSIDKEY L"CLSID\\%s"
#define NAMESPACEKEY L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\%s"


/// <summary>
/// Sets a registry string value.
/// </summary>
inline LSTATUS RegSetString(HKEY key, LPCWSTR value, LPCWSTR data) {
    return RegSetValueEx(key, value, NULL, REG_SZ, (const BYTE*)data, DWORD(sizeof(WCHAR)*(wcslen(data)+1)));
}


/// <summary>
/// Sets a formatted registry value.
/// </summary>
LSTATUS RegPrintf(HKEY key, LPCWSTR value, LPCWSTR format, ...) {
    WCHAR data[4096];
    va_list args;

    va_start(args, format);
    StringCchVPrintf(data, 4096, format, args);
    va_end(args);

    return RegSetString(key, value, data);
}


/// <summary>
/// Checks if the current process is elevated.
/// </summary>
bool IsElevated() {
	BOOL fIsRunAsAdmin = FALSE;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
		return false;
	}

	// Determine whether the SID of administrators group is enabled in the primary access token of the process.
	CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin);

	FreeSid(pAdministratorsGroup);

	return fIsRunAsAdmin != FALSE;
}


/// <summary>
/// Attempts to run the specified function in elevated mode.
/// We do that by calling rundll32 with runas.
/// </summary>
HRESULT RunElevated(LPCWSTR func) {
    WCHAR dllPath[MAX_PATH], commandLine[1024], workingDirectory[MAX_PATH];
    HRESULT hr = E_UNEXPECTED;
    HRESULT* pResult = (HRESULT*)malloc(sizeof(pResult));

    // 
    GetModuleFileNameW(::module, dllPath, MAX_PATH);
    StringCchPrintfW(commandLine, 1024, L"%s,DllElevatedEntry %s %x %x", dllPath, func, pResult, GetCurrentProcessId());
    GetCurrentDirectoryW(MAX_PATH, workingDirectory);

    SHELLEXECUTEINFOW info;
    info.cbSize = sizeof(SHELLEXECUTEINFOW);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.lpDirectory = workingDirectory;
    info.lpFile = L"rundll32";
    info.lpParameters = commandLine;
    info.lpVerb = L"runas";
    info.nShow = SW_SHOW;

    ShellExecuteExW(&info);

    if (info.hProcess != NULL) {
        WaitForSingleObject(info.hProcess, INFINITE);
        CloseHandle(info.hProcess);

        hr = *pResult;
    }

    free(pResult);

    return hr;
}


/// <summary>
/// Called by rundll32 when we are "elevating" the process.
/// </summary>
void CALLBACK DllElevatedEntry(HWND hwnd, HINSTANCE instance, LPSTR cmdLine, int cmdShow) {
    char *context, *token, *endPtr;
    char func[64];
    DWORD processID;
    HRESULT* pResult;

    HANDLE hProcess;
    HRESULT hr;

    // Get the function to execute
    token = strtok_s(cmdLine, " ", &context);
    StringCchCopyA(func, 64, token);

    // Get the address to store the result in
    token = strtok_s(NULL, " ", &context);
    pResult = (HRESULT*)strtoul(token, &endPtr, 16);

    // Get the process ID
    token = strtok_s(NULL, " ", &context);
    processID = (DWORD)strtoul(token, &endPtr, 16);

    // Execute the function
    if (strcmp(func, "DllRegisterServer") == 0) {
        hr = DllRegisterServer();
    }
    else if (strcmp(func, "DllUnregisterServer") == 0) {
        hr = DllUnregisterServer();
    }
    else {
        hr = E_UNEXPECTED;
    }

    // Store the result in the callers memory
    if ((hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, false, processID)) == NULL) {
        WriteProcessMemory(hProcess, pResult, &hr, sizeof(HRESULT), NULL);
        CloseHandle(hProcess);
    }
}


/// <summary>
/// Retrives the path to the registry keys used.
/// </summary>
HRESULT GetKeyPath(KEY_TYPE type, LPWSTR path, UINT cchPath) {
    HRESULT hr = S_OK;
    LPWSTR clsidString;

    hr = StringFromCLSID(CLSID_WinUnionFS, &clsidString);

    if (SUCCEEDED(hr)) {
        switch (type) {
        case KEY_CLSID:
            {
                hr = StringCchPrintfW(path, cchPath, CLSIDKEY, clsidString);
            }
            break;

        case KEY_NAMESPACE:
            {
                hr = StringCchPrintfW(path, cchPath, NAMESPACEKEY, clsidString);
            }
            break;

        default:
            {
                hr = E_UNEXPECTED;
            }
            break;
        }

        CoTaskMemFree(clsidString);
    }

    return hr;
}


/// <summary>
/// Deletes all registration-related registry keys.
/// </summary>
HRESULT DeleteRegKeys() {
    WCHAR key[MAX_PATH];
    HRESULT hr;
    LSTATUS ls;

    // Drop the CLSID key
    hr = GetKeyPath(KEY_CLSID, key, MAX_PATH);
    if (SUCCEEDED(hr)) {
        ls = RegDeleteTreeW(HKEY_CLASSES_ROOT, key);
        hr = ls == ERROR_SUCCESS || ls == ERROR_FILE_NOT_FOUND ? S_OK : HRESULT_FROM_WIN32(ls);
    }

    // Drop the namespace key
    if (SUCCEEDED(hr)) {
        hr = GetKeyPath(KEY_NAMESPACE, key, MAX_PATH);
    }
    if (SUCCEEDED(hr)) {
        ls = RegDeleteTreeW(HKEY_LOCAL_MACHINE, key);
        hr = ls == ERROR_SUCCESS || ls == ERROR_FILE_NOT_FOUND ? S_OK : HRESULT_FROM_WIN32(ls);
    }

    return hr;
}


/// <summary>
/// Registers this DLL.
/// </summary>
HRESULT RegisterDLL() {
    HKEY baseKey, subKey;
    WCHAR dllPath[MAX_PATH], key[MAX_PATH];

    // Get the path to this DLL.
    GetModuleFileName(::module, dllPath, MAX_PATH);

    //
    GetKeyPath(KEY_CLSID, key, MAX_PATH);

    // Configure the top-level key.
    if (RegCreateKey(HKEY_CLASSES_ROOT, key, &baseKey) != ERROR_SUCCESS) {
        return E_UNEXPECTED;
    }
    RegSetValue(baseKey, NULL, REG_SZ, L"WinUnionFS", NULL);
    RegSetString(baseKey, L"InfoTip", L"WinUnionFS InfoTip");

    // Create a DefaultIcon subkey.
    RegSetValue(baseKey, L"DefaultIcon", REG_SZ, L"%SystemRoot%\\system32\\imageres.dll,-1023", NULL);

    // Configure the InprocServer32 subkey.
    if (RegCreateKey(baseKey, L"InprocServer32", &subKey) != ERROR_SUCCESS) {
        RegCloseKey(baseKey);
        return E_UNEXPECTED;
    }
    RegSetValue(subKey, NULL, REG_SZ, dllPath, NULL);
    RegSetString(subKey, L"ThreadingModel", L"Apartment");
    RegCloseKey(subKey);

    // Configure the ShellFolder subkey.
    if (RegCreateKey(baseKey, L"ShellFolder", &subKey) != ERROR_SUCCESS) {
        RegCloseKey(baseKey);
        return E_UNEXPECTED;
    }
    DWORD attributes = 0x20000000 | 0x80000000 | 0x00000010;
    RegSetValueEx(subKey, L"Attributes", NULL, REG_DWORD, (const BYTE*)&attributes, sizeof(DWORD));
    RegPrintf(subKey, L"DefaultIcon", L"%s,1", dllPath);
    RegCloseKey(subKey);

    // Close the top-level key.
    RegCloseKey(baseKey);

    return S_OK;
}


/// <summary>
/// Register a system-wide namespace below the desktop.
/// </summary>
HRESULT RegisterNamespace() {
    HKEY baseKey;
    WCHAR key[MAX_PATH];
    
    GetKeyPath(KEY_NAMESPACE, key, MAX_PATH);
    if (RegCreateKey(HKEY_LOCAL_MACHINE, key, &baseKey) != ERROR_SUCCESS) {
        return E_UNEXPECTED;
    }
    RegSetValue(baseKey, NULL, REG_SZ, L"WinUnionFS", NULL);
    RegCloseKey(baseKey);

    return S_OK;
}


/// <summary>
/// Registers this DLL.
/// </summary>
STDAPI DllRegisterServer() {
    HRESULT hr;

    // Check if we are elevated.
    if (!IsElevated()) {
        return RunElevated(L"DllRegisterServer");
    }

    // Delete old registry keys
    hr = DeleteRegKeys();

    // Register this DLL
    if (SUCCEEDED(hr)) {
        RegisterDLL();
    }
    
    // Register a system-wide namespace below the desktop
    if (SUCCEEDED(hr)) {
        RegisterNamespace();
    }

    // Refresh Desktop
    if (SUCCEEDED(hr)) {
        //SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    }

    return hr;
}


/// <summary>
/// Unregisters this DLL.
/// </summary>
STDAPI DllUnregisterServer() {
    HRESULT hr;

    // Check if we are elevated.
    if (!IsElevated()) {
        return RunElevated(L"DllUnregisterServer");
    }

    // Delete registry keys
    hr = DeleteRegKeys();

    // Refresh Desktop
    if (SUCCEEDED(hr)) {
        //SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    }

    return hr;
}
