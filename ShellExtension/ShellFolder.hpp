/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  ShellFolder.hpp
 *  The WinUnionFS Project
 *
 *  Implementation of IShellFolder (and related interfaces).
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#include <vector>

class ShellFolder :
    public IShellFolder2,
    public IPersistIDList,
    public IPersistFolder3
{
public:
    // Constructor
    explicit ShellFolder(LPCITEMIDLIST path);

    // IUnknown
    ULONG STDMETHODCALLTYPE AddRef();
    STDMETHOD(QueryInterface) (REFIID, void**);
    ULONG STDMETHODCALLTYPE Release();

    // IShellFolder
    STDMETHOD(BindToObject) (PCUIDLIST_RELATIVE, IBindCtx*, REFIID, void**);
    STDMETHOD(BindToStorage) (PCUIDLIST_RELATIVE, IBindCtx*, REFIID, void**);
    STDMETHOD(CompareIDs) (LPARAM, PCUIDLIST_RELATIVE, PCUIDLIST_RELATIVE);
    STDMETHOD(CreateViewObject) (HWND, REFIID, void**);
    STDMETHOD(EnumObjects) (HWND, SHCONTF, IEnumIDList**);
    STDMETHOD(GetAttributesOf) (UINT, PCUITEMID_CHILD_ARRAY, SFGAOF*);
    STDMETHOD(GetDisplayNameOf) (PCUITEMID_CHILD, SHGDNF, STRRET*);
    STDMETHOD(GetUIObjectOf) (HWND, UINT, PCUITEMID_CHILD_ARRAY, REFIID, UINT*, void**);
    STDMETHOD(ParseDisplayName) (HWND, IBindCtx*, LPWSTR, ULONG*, PIDLIST_RELATIVE*, ULONG*);
    STDMETHOD(SetNameOf) (HWND, PCUITEMID_CHILD, LPCWSTR, SHGDNF, PITEMID_CHILD*);

    // IShellFolder2
    STDMETHOD(EnumSearches) (IEnumExtraSearch**);
    STDMETHOD(GetDefaultColumn) (DWORD, ULONG*, ULONG*);
    STDMETHOD(GetDefaultColumnState) (UINT, SHCOLSTATEF*);
    STDMETHOD(GetDefaultSearchGUID) (GUID*);
    STDMETHOD(GetDetailsEx) (PCUITEMID_CHILD, const SHCOLUMNID*, VARIANT*);
    STDMETHOD(GetDetailsOf) (PCUITEMID_CHILD, UINT, SHELLDETAILS*);
    STDMETHOD(MapColumnToSCID) (UINT, SHCOLUMNID*);

    // IPersist
    STDMETHOD(GetClassID) (CLSID*);

    // IPersistIDList
    STDMETHOD(GetIDList) (LPITEMIDLIST*);
    STDMETHOD(SetIDList) (LPCITEMIDLIST);

    // IPersistFolder
    STDMETHOD(Initialize) (LPCITEMIDLIST);
    
    // IPersistFolder2
    STDMETHOD(GetCurFolder) (LPITEMIDLIST*);
    
    // IPersistFolder3
    STDMETHOD(GetFolderTargetInfo) (PERSIST_FOLDER_TARGET_INFO*);
    STDMETHOD(InitializeEx) (IBindCtx*, LPCITEMIDLIST, const PERSIST_FOLDER_TARGET_INFO*);

private:
    // Destructor
    virtual ~ShellFolder();

    ULONG refCount;

    LPITEMIDLIST folder;

    std::vector<IShellFolder*> folders;

    PERSIST_FOLDER_TARGET_INFO folderTargetInfo;
};
