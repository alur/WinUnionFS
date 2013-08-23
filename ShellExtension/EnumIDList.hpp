/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  EnumIDList.hpp
 *  The WinUnionFS Project
 *
 *  Enumerates over a collection of PIDLs.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#include <vector>

class EnumIDList : public IEnumIDList  {
public:
    explicit EnumIDList();
    virtual ~EnumIDList();

    // IUnknown
    ULONG STDMETHODCALLTYPE AddRef();
    STDMETHOD(QueryInterface) (REFIID, void**);
    ULONG STDMETHODCALLTYPE Release();

    // IEnumIDList
    STDMETHOD(Clone) (IEnumIDList**);
    STDMETHOD(Next) (ULONG, LPITEMIDLIST*, ULONG*);
    STDMETHOD(Reset) ();
    STDMETHOD(Skip) (ULONG);

    //
    void AddItem(LPITEMIDLIST item);

private:
    std::vector<LPITEMIDLIST> items;
    ULONG position;
    ULONG refCount;
};
