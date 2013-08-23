/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Main.cpp
 *  The WinUnionFS Project
 *
 *  Exported functions.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <Windows.h>
#include <ShlObj.h>
#include <strsafe.h>

#include "ClassFactory.hpp"
#include "Debug.h"
#include "Main.h"
#include "Registration.h"


// The handle to this DLL.
HMODULE module;

// The number of in-use objects.
long objectCounter;

// The CLSID of WinUnionFS, {313f9e44-11ba-45de-8ecf-4a8ba0424d26}.
extern const CLSID CLSID_WinUnionFS = {0x313f9e44, 0x11ba, 0x45de, {0x8e, 0xcf, 0x4a, 0x8b, 0xa0, 0x42, 0x4d, 0x26}};


/// <summary>
/// The main entry point for this DLL.
/// </summary>
/// <param name="module">A handle to the DLL module.</param>
/// <param name="reasonForCall">The reason code that indicates why the DLL entry-point function is being called.</param>
/// <returns>TRUE on success, FALSE on failure.</returns>
BOOL APIENTRY DllMain(HMODULE module, DWORD reasonForCall, LPVOID /* reserved */) {
    switch (reasonForCall) {
    case DLL_PROCESS_DETACH:
        {
        }
        break;

    case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(module);
            ::module = module;
            ::objectCounter = 0;
        }
        break;
    }

    return TRUE;
}


/// <summary>
/// Determines whether the DLL that implements this function is in use.
/// </summary>
STDAPI DllCanUnloadNow() {
    return ::objectCounter == 0 ? S_OK : S_FALSE;
}


/// <summary>
/// Retrieves the class object from a DLL object handler or object application.
/// </summary>
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
    // Wait for the debugger to attach...
    //while (!IsDebuggerPresent()) {
    //  Sleep(100);
    //}

    if (riid == IID_IClassFactory) {
        *ppv = (IClassFactory*)(new ClassFactory());
    }
    else {
        *ppv = NULL;
        return CLASS_E_CLASSNOTAVAILABLE;
    }
    return S_OK;
}
