/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  EnumIDList.cpp
 *  The WinUnionFS Project
 *
 *  Enumerates over a collection of PIDLs.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <Windows.h>
#include <ShObjIdl.h>

#include "EnumIDList.hpp"
#include "Macros.h"
#include "PIDL.h"


// The number of in-use objects.
extern long objectCounter;


/// <summary>
/// Constructor.
/// </summary>
EnumIDList::EnumIDList() {
    this->refCount = 1;
    this->position = 0;
    
    InterlockedIncrement(&::objectCounter);
}


/// <summary>
/// Destructor.
/// </summary>
EnumIDList::~EnumIDList() {
    for (std::vector<PITEMID_CHILD>::iterator iter = this->items.begin(); iter != this->items.end(); ++iter) {
        CoTaskMemFree(*iter);
    }
    this->items.clear();

    InterlockedDecrement(&::objectCounter);
}


/// <summary>
/// IUnknown::AddRef
/// Increments the reference count for an interface on an object.
/// </summary>
ULONG EnumIDList::AddRef() {
    return InterlockedIncrement(&this->refCount);
}


/// <summary>
/// IUnknown::Release
/// Decrements the reference count for an interface on an object.
/// </summary>
ULONG EnumIDList::Release() {
    if (InterlockedDecrement(&this->refCount) == 0) {
        delete this;
        return 0;
    }

    return this->refCount;
}


/// <summary>
/// IUnknown::QueryInterface
/// Retrieves pointers to the supported interfaces on an object.
/// </summary>
HRESULT EnumIDList::QueryInterface(REFIID riid, void **ppvObject) {
    if (ppvObject == NULL) {
        return E_POINTER;
    }

    if (riid == IID_IEnumIDList) {
        *ppvObject = (IEnumIDList*)this;
    }
    else {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


/// <summary>
/// IEnumIDList::Clone
/// Creates a new item enumeration object with the same contents and state as the current one.
/// </summary>
HRESULT EnumIDList::Clone(IEnumIDList **ppenum) {
    EnumIDList* clone = new EnumIDList();

    for (std::vector<LPITEMIDLIST>::iterator iter = this->items.begin(); iter != this->items.end(); ++iter) {
        clone->AddItem(PIDL::Copy(*iter));
    }
    clone->position = this->position;

    *ppenum = clone;

    return S_OK;
}


/// <summary>
/// IEnumIDList::Next
/// Retrieves the specified number of item identifiers in the enumeration sequence and advances
/// the current position by the number of items retrieved.
/// </summary>
HRESULT EnumIDList::Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched) {
    ULONG fetched = 0;

    for (ULONG i = 0; this->position != this->items.size() && fetched < celt; ++i) {
        rgelt[i] = PIDL::Copy(this->items[this->position++]);
        ++fetched;
    }

    if (pceltFetched != NULL) {
        *pceltFetched = fetched;
    }

    return fetched == celt ? S_OK : S_FALSE;
}


/// <summary>
/// IEnumIDList::Reset
/// Returns to the beginning of the enumeration sequence.
/// </summary>
HRESULT EnumIDList::Reset() {
    this->position = 0;
    return S_OK;
}


/// <summary>
/// IEnumIDList::Skip
/// Skips the specified number of elements in the enumeration sequence.
/// </summary>
HRESULT EnumIDList::Skip(ULONG celt) {
    this->position = max((ULONG)this->items.size(), this->position + celt);
    return S_OK;
}


/// <summary>
/// EnumIDList::AddItem
/// Skips the specified number of elements in the enumeration sequence.
/// </summary>
void EnumIDList::AddItem(LPITEMIDLIST item) {
    // Remove duplicate folders.
    if (FLAGSET(PIDL::Item(item)->attributes, SFGAO_FOLDER)) {
        LPCWSTR name = PIDL::Item(item)->name;
        for (std::vector<LPITEMIDLIST>::const_iterator iter = this->items.begin(); iter != this->items.end(); ++iter) {
            PIDL::PIDLItem* p = PIDL::Item(*iter);
            if (FLAGSET(p->attributes, SFGAO_FOLDER) && wcscmp(name, p->name) == 0) {
                return;
            }
        }
    }
    this->items.push_back(item);
}
