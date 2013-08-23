/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  ClassFactory.cpp
 *  The WinUnionFS Project
 *
 *  Implementation of IClassFactory.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <Windows.h>
#include <Shobjidl.h>

#include "ClassFactory.hpp"
#include "ShellFolder.hpp"


// The number of in-use objects.
extern long objectCounter;


/// <summary>
/// Constructor.
/// </summary>
ClassFactory::ClassFactory() {
    this->refCount = 1;
    InterlockedIncrement(&::objectCounter);
}


/// <summary>
/// Destructor.
/// </summary>
ClassFactory::~ClassFactory() {
    InterlockedDecrement(&::objectCounter);
}


/// <summary>
/// IUnknown::AddRef
/// Increments the reference count for an interface on an object.
/// </summary>
ULONG ClassFactory::AddRef() {
    return InterlockedIncrement(&this->refCount);
}


/// <summary>
/// IUnknown::Release
/// Decrements the reference count for an interface on an object.
/// </summary>
ULONG ClassFactory::Release() {
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
HRESULT ClassFactory::QueryInterface(REFIID riid, void **ppvObject) {
    if (ppvObject == NULL) {
        return E_POINTER;
    }

    if (riid == IID_IUnknown) {
        *ppvObject = (IUnknown*)this;
    }
    else if (riid == IID_IClassFactory) {
        *ppvObject = (IClassFactory*)this;
    }
    else {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    ++this->refCount;
    return S_OK;
}


/// <summary>
/// IClassFactory::CreateInstance
/// 
/// </summary>
HRESULT ClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject) {
    if (ppvObject == NULL) {
        return E_POINTER;
    }

    // TODO::Aggregate?
    if (pUnkOuter != NULL) {
        return CLASS_E_NOAGGREGATION;
    }

    if (riid == IID_IShellFolder) {
        *ppvObject = (IShellFolder*)(new ShellFolder(NULL));
    }
    else if (riid == IID_IPersistFolder) {
        *ppvObject = (IPersistFolder*)(new ShellFolder(NULL));
    }
    else {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    // We failed to create the object.
    if (*ppvObject == NULL) {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


/// <summary>
/// IClassFactory::LockServer
/// 
/// </summary>
HRESULT ClassFactory::LockServer(BOOL fLock) {
    return S_OK;
}


/// <summary>
/// IClassFactory::IsLocked
/// 
/// </summary>
bool ClassFactory::IsLocked() {
    return false;
}
