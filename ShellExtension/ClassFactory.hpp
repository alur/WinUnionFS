/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  ClassFactory.hpp
 *  The WinUnionFS Project
 *
 *  Implementation of IClassFactory.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

class ClassFactory : public IClassFactory
{
public:
    // Constructor/Destructor
    explicit ClassFactory();

    // IUnknown
    ULONG STDMETHODCALLTYPE AddRef();
    STDMETHOD(QueryInterface) (REFIID, void**);
    ULONG STDMETHODCALLTYPE Release();

    // IClassFactory
    STDMETHOD(CreateInstance) (IUnknown*, REFIID, void**);
    STDMETHOD(LockServer) (BOOL);

    // Utility
    bool IsLocked();

private:
    virtual ~ClassFactory();

    ULONG refCount; //
};
