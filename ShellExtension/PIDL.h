/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  PIDL.hpp
 *  The WinUnionFS Project
 *
 *  Functions for managing WinUnionFS PIDLs.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#include <vector>

namespace PIDL {
    typedef struct {
        USHORT cb;
        USHORT folder;
        SFGAOF attributes;
        USHORT cbName;
        WCHAR name[1];
    } PIDLItem;

    LPITEMIDLIST Concatenate(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    LPITEMIDLIST Create(LPCITEMIDLIST parent, LPWSTR path, SFGAOF attributes, USHORT folder);
    LPITEMIDLIST CreateFromPath(LPCWSTR path);
    LPITEMIDLIST Copy(LPCITEMIDLIST source);
    LPITEMIDLIST Empty();
    LPITEMIDLIST End(LPCITEMIDLIST pidl);
    void Free(LPITEMIDLIST pidl);
    SFGAOF GetAttributes(PCITEMID_CHILD pidl);
    LPWSTR GetDisplayName(PCITEMID_CHILD pidl);
    void GetFullPath(LPCITEMIDLIST parent, PCITEMID_CHILD pidl, LPWSTR path, UINT cchPath);
    LPWSTR GetFullPath(LPCITEMIDLIST parent, PCITEMID_CHILD pidl);
    HRESULT GetShellFoldersFor(LPCITEMIDLIST pidl, std::vector<IShellFolder*> *out);
    PIDLItem* Item(LPCITEMIDLIST pidl);
    ULONG ItemCount(LPCITEMIDLIST pidl);
    LPITEMIDLIST Last(LPCITEMIDLIST pidl);
    LPITEMIDLIST Next(LPCITEMIDLIST pidl);
    ULONG Size(LPCITEMIDLIST pidl);
}
