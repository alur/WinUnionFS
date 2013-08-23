/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  PIDL.cpp
 *  The WinUnionFS Project
 *
 *  Functions for managing WinUnionFS PIDLs.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <Windows.h>
#include <Shobjidl.h>
#include <strsafe.h>
#include <ShlObj.h>

#include "Group.hpp"
#include "PIDL.h"


/// <summary>
/// 
/// </summary>
LPITEMIDLIST PIDL::Concatenate(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) {
    if (pidl1 == NULL) {
        return Copy(pidl2);
    }
    if (pidl2 == NULL) {
        return Copy(pidl1);
    }

    ULONG size1 = Size(pidl1);
    ULONG size2 = Size(pidl2);

    LPBYTE copy = (LPBYTE)CoTaskMemAlloc(size1 + size2 - sizeof(ITEMIDLIST));
    memcpy(copy, pidl1, size1);
    memcpy(copy + size1 - sizeof(ITEMIDLIST), pidl2, size2);

    return LPITEMIDLIST(copy);
}


/// <summary>
/// 
/// </summary>
LPITEMIDLIST PIDL::Create(LPCITEMIDLIST parent, LPWSTR path, SFGAOF attributes, USHORT folder) {
    // Size of the path.
    USHORT cbName = USHORT(sizeof(WCHAR)*wcslen(path));
    ULONG parentSize = 0;

    if (parent != NULL) {
        parentSize = Size(parent);
    }

    //
    LPITEMIDLIST ret = (LPITEMIDLIST)CoTaskMemAlloc(parentSize + cbName + sizeof(PIDLItem) + sizeof(ITEMIDLIST));
    PIDLItem* item = Item(ret);

    if (parent != NULL) {
        memcpy(ret, parent, parentSize);
        item = Item(End(ret));
    }

    cbName += sizeof(WCHAR); // The terminating NULL.

    item->cb = USHORT(cbName + sizeof(PIDLItem));
    item->attributes = attributes;
    item->folder = folder;
    item->cbName = cbName;
    memcpy(item->name, path, cbName);

    Next(ret)->mkid.cb = 0;

    return ret;
}


/// <summary>
/// 
/// </summary>
LPITEMIDLIST PIDL::Copy(LPCITEMIDLIST source) {
    if (source == NULL) {
        return NULL;
    }

    ULONG size = Size(source);
    LPITEMIDLIST copy = (LPITEMIDLIST)CoTaskMemAlloc(size);
    memcpy(copy, source, size);

    return copy;
}


/// <summary>
/// Returns an empty PIDL.
/// </summary>
LPITEMIDLIST PIDL::Empty() {
    LPITEMIDLIST ret = (LPITEMIDLIST)CoTaskMemAlloc(sizeof(ITEMIDLIST));
    ZeroMemory(ret, sizeof(ITEMIDLIST));

    return ret;
}


/// <summary>
/// 
/// </summary>
LPITEMIDLIST PIDL::End(LPCITEMIDLIST pidl){
    LPCITEMIDLIST last = pidl;
    while (last->mkid.cb != 0) {
        last = Next(last);
    }
    return (LPITEMIDLIST)last;
}


/// <summary>
/// 
/// </summary>
void PIDL::Free(LPITEMIDLIST pidl) {
    if (pidl != NULL) {
        CoTaskMemFree(pidl);
    }
}


/// <summary>
/// 
/// </summary>
SFGAOF PIDL::GetAttributes(PCITEMID_CHILD pidl) {
    return Item(pidl)->attributes;
}


/// <summary>
/// 
/// </summary>
LPWSTR PIDL::GetDisplayName(PCITEMID_CHILD pidl) {
    PIDLItem* item = Item(pidl);
    LPWSTR ret = (LPWSTR)CoTaskMemAlloc(Item(pidl)->cbName);
    memcpy(ret, Item(pidl)->name, Item(pidl)->cbName);

    return ret;
}


/// <summary>
/// Returns the full parse path of the item. Folder1\Folder2\File
/// </summary>
void PIDL::GetFullPath(LPCITEMIDLIST parent, PCITEMID_CHILD pidl, LPWSTR path, UINT cchPath) {
    LPCITEMIDLIST iter = Next(parent);
    path[0] = '\0';

    while (iter->mkid.cb != 0) {
        StringCchCatW(path, cchPath, Item(iter)->name);
        StringCchCatW(path, cchPath, L"\\");
        iter = Next(iter);
    }
    if (pidl != NULL) {
        StringCchCatW(path, cchPath, Item(pidl)->name);
    }
    else if(Next(parent)->mkid.cb != 0) {
        // Should drop that last backslash
        path[wcslen(path)-1] = '\0';
    }
}


/// <summary>
/// Returns the full parse path of the item. Folder1\Folder2\File
/// </summary>
LPWSTR PIDL::GetFullPath(LPCITEMIDLIST parent, PCITEMID_CHILD pidl) {
    WCHAR path[MAX_PATH] = L"";

    GetFullPath(parent, pidl, path, MAX_PATH);

    size_t size = (wcslen(path)+1)*sizeof(WCHAR);
    LPWSTR ret = (LPWSTR)CoTaskMemAlloc(size);
    memcpy(ret, path, size);

    return ret;
}


/// <summary>
/// Returns the size, in bytes, of the entire ITEMIDLIST.
/// </summary>
HRESULT PIDL::GetShellFoldersFor(LPCITEMIDLIST pidl, std::vector<IShellFolder*> *out) {
    //  ShellFolder will have to deal with the top-level folder.
    if (Next(pidl)->mkid.cb != 0) {
        // This is the group level, we should get IShellFolder interfaces for all folders included in the group.

        // The first PIDL is the group.
        Group* group = Group::Find(Item(Next(pidl))->name);

        if (group != NULL) {
            WCHAR path[MAX_PATH];
            LPCITEMIDLIST pathStart = Next(pidl);

            GetFullPath(pathStart, NULL, path, MAX_PATH);
            group->GetShellFoldersFor(path, out);
        }
    }

    return S_OK;
}


/// <summary>
/// Returns the "item" of the PIDL.
/// </summary>
PIDL::PIDLItem* PIDL::Item(LPCITEMIDLIST pidl) {
    return (PIDLItem*)&pidl->mkid;
}


/// <summary>
/// Returns the "item" of the PIDL.
/// </summary>
ULONG PIDL::ItemCount(LPCITEMIDLIST pidl) {
    ULONG count = 0;
    for (LPCITEMIDLIST iter = pidl; iter->mkid.cb != 0; iter = Next(iter)) {
        ++count;
    }

    return count;
}


/// <summary>
/// Returns last id in the PIDL.
/// </summary>
LPITEMIDLIST PIDL::Last(LPCITEMIDLIST pidl) {
    while (Next(pidl)->mkid.cb != 0) {
        pidl = Next(pidl);
    }

    return LPITEMIDLIST(pidl);
}


/// <summary>
/// Returns the next item in the ITEMIDLIST.
/// </summary>
LPITEMIDLIST PIDL::Next(LPCITEMIDLIST pidl) {
    return LPITEMIDLIST(((LPBYTE)pidl)+pidl->mkid.cb);
}


/// <summary>
/// Returns the size, in bytes, of the entire ITEMIDLIST.
/// </summary>
ULONG PIDL::Size(LPCITEMIDLIST pidl) {
    ULONG size = sizeof(ITEMIDLIST); // Terminating item ID
    for (LPCITEMIDLIST iter = pidl; iter->mkid.cb != 0; iter = Next(iter)) {
        size += iter->mkid.cb;
    }

    return size;
}
