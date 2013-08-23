/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Main.h
 *  The WinUnionFS Project
 *
 *  Exported functions.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID);
STDAPI DllCanUnloadNow();
STDAPI DllGetClassObject(REFCLSID, REFIID, LPVOID*);
