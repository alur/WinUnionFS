/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  ShellFolder.cpp
 *  The WinUnionFS Project
 *
 *  Implementation of IShellFolder (and related interfaces).
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <Windows.h>
#include <NTQuery.h>
#include <Shobjidl.h>
#include <strsafe.h>
#include <Shlobj.h>
#include <Shlwapi.h>

#include "Debug.h"
#include "EnumIDList.hpp"
#include "Group.hpp"
#include "Macros.h"
#include "PIDL.h"
#include "ShellFolder.hpp"
#include "ShellView.hpp"


// The number of in-use objects.
extern long objectCounter;

// The CLSID of WinUnionFS.
extern const CLSID CLSID_WinUnionFS;


/// <summary>
/// Constructor.
/// </summary>
ShellFolder::ShellFolder(LPCITEMIDLIST path) {
    this->refCount = 1;
    InterlockedIncrement(&::objectCounter);
    Group::AddUser();
    this->folder = PIDL::Copy(path);

    if (path != NULL) {
        PIDL::GetShellFoldersFor(path, &this->folders);
    }
}


/// <summary>
/// Destructor.
/// </summary>
ShellFolder::~ShellFolder() {
    Group::RemoveUser();
    InterlockedDecrement(&::objectCounter);
    PIDL::Free(this->folder);

    for (std::vector<IShellFolder*>::const_iterator folder = this->folders.begin(); folder != this->folders.end(); ++folder) {
        (*folder)->Release();
    }
}


/// <summary>
/// IUnknown::AddRef
/// Increments the reference count for an interface on an object.
/// </summary>
ULONG ShellFolder::AddRef() {
    return InterlockedIncrement(&this->refCount);
}


/// <summary>
/// IUnknown::Release
/// Decrements the reference count for an interface on an object.
/// </summary>
ULONG ShellFolder::Release() {
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
HRESULT ShellFolder::QueryInterface(REFIID riid, void **ppvObject) {
    if (ppvObject == NULL) {
        return E_POINTER;
    }

    if (riid == IID_IUnknown) {
        *ppvObject = (IUnknown*)(IShellFolder*)this;
    }
    else if (riid == IID_IPersist) {
        *ppvObject = (IPersist*)(IPersistFolder*)this;
    }
    else if (riid == IID_IPersistIDList) {
        *ppvObject = (IPersistIDList*)this;
    }
    else if (riid == IID_IShellFolder) {
        *ppvObject = (IShellFolder*)this;
    }
    else if (riid == IID_IShellFolder2) {
        *ppvObject = (IShellFolder2*)this;
    }
    else if (riid == IID_IPersistFolder) {
        *ppvObject = (IPersistFolder*)this;
    }
    else if (riid == IID_IPersistFolder2) {
        *ppvObject = (IPersistFolder2*)this;
    }
    /*else if (riid == IID_IPersistFolder3) {
        *ppvObject = (IPersistFolder3*)this;
    }*/
    else {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


/// <summary>
/// IShellFolder::BindToObject
/// Retrieves a handler, typically the Shell folder object that implements IShellFolder for a particular item.
/// </summary>
HRESULT ShellFolder::BindToObject(PCUIDLIST_RELATIVE pidl, IBindCtx *pbc, REFIID riid, void **ppvOut) {
    if (ppvOut == NULL) {
        return E_POINTER;
    }

    if (riid == IID_IShellFolder) {
        LPITEMIDLIST newPidl = PIDL::Concatenate(this->folder, pidl);
        *ppvOut = (IShellFolder*)(new ShellFolder(newPidl));
        PIDL::Free(newPidl);
        return S_OK;
    }

    return E_NOINTERFACE;
}


/// <summary>
/// IShellFolder::BindToStorage
/// Requests a pointer to an object's storage interface.
/// </summary>
HRESULT ShellFolder::BindToStorage(PCUIDLIST_RELATIVE pidl, IBindCtx *pbc, REFIID riid, void **ppvOut) {
    return E_NOTIMPL;
}


/// <summary>
/// IShellFolder::CompareIDs
/// Determines the relative order of two file objects or folders, given their item identifier lists.
/// </summary>
HRESULT ShellFolder::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2) {
    return MAKE_HRESULT(0, 0, (USHORT)wcscmp(PIDL::Item(pidl1)->name, PIDL::Item(pidl2)->name));
}


/// <summary>
/// IShellFolder::CreateViewObject
/// Requests an object that can be used to obtain information from or interact with a folder object.
/// </summary>
HRESULT ShellFolder::CreateViewObject(HWND hwndOwner, REFIID riid, void **ppv) {
    HRESULT hr;

    if (ppv == NULL) {
        return E_POINTER;
    }

    if (riid == IID_IShellView) {
        *ppv = NULL;
        hr = E_NOINTERFACE;

        SFV_CREATE c;
        c.cbSize = sizeof(SFV_CREATE);
        c.psvOuter = NULL;
        c.psfvcb = NULL;
        QueryInterface(IID_IShellFolder, reinterpret_cast<LPVOID*>(&c.pshf));

        hr = SHCreateShellFolderView(&c, reinterpret_cast<IShellView**>(ppv));
    }
    else {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


/// <summary>
/// IShellFolder::EnumObjects
/// Enables a client to determine the contents of a folder by creating an item identifier enumeration
/// object and returning its IEnumIDList interface. The methods supported by that interface can then be
/// used to enumerate the folder's contents.
/// </summary>
HRESULT ShellFolder::EnumObjects(HWND hwndOwner, SHCONTF grfFlags, IEnumIDList **ppenumIDList) {
    if (ppenumIDList == NULL) {
        return E_POINTER;
    }

    EnumIDList* list = new EnumIDList();

    if (PIDL::ItemCount(this->folder) == 1) {
        // This is the root folder, we should list the groups
        if (FLAGSET(grfFlags, SHCONTF_CHECKING_FOR_CHILDREN) || FLAGSET(grfFlags, SHCONTF_FOLDERS)) {
            int groupIndex = 0;
            Group* group;
            while ((group = Group::Find(groupIndex++)) != NULL) {
                list->AddItem(PIDL::Create(NULL, (LPWSTR)group->name, SFGAO_FOLDER | SFGAO_BROWSABLE | SFGAO_HASSUBFOLDER, 0));
            }
        }
    }
    else {
        // Enumerate the contents of all the shell folders
        int f = 0;
        for (std::vector<IShellFolder*>::const_iterator folder = this->folders.begin(); folder != this->folders.end(); ++folder) {
            IEnumIDList* enumIDList;
            PIDLIST_RELATIVE idNext = NULL;

            (*folder)->EnumObjects(hwndOwner, grfFlags, &enumIDList);

            while (enumIDList->Next(1, &idNext, NULL) != S_FALSE) {
                WCHAR fileName[MAX_PATH];
                STRRET name;
                SFGAOF attributes = SFGAOF(-1);

                (*folder)->GetAttributesOf(1, (LPCITEMIDLIST *)&idNext, &attributes);
                (*folder)->GetDisplayNameOf(idNext, SHGDN_NORMAL, &name);
                StrRetToBufW(&name, idNext, fileName, MAX_PATH);
                list->AddItem(PIDL::Create(NULL, fileName, SFGAO_FOLDER | SFGAO_BROWSABLE | SFGAO_HASSUBFOLDER, f));

                ILFree(idNext);
            }
            ++f;
            enumIDList->Release();
        }
    }
    
    list->QueryInterface(IID_IEnumIDList, reinterpret_cast<LPVOID*>(ppenumIDList));
    list->Release();

    return S_OK;
}


/// <summary>
/// IShellFolder::GetAttributesOf
/// Gets the attributes of one or more file or folder objects contained in the object represented by IShellFolder.
/// </summary>
HRESULT ShellFolder::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF *rgfInOut) {
    SFGAOF attributes = SFGAOF(-1);

    if (cidl == 0 || apidl[0]->mkid.cb == 0) {
        // Top level folder.
        attributes = SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_HASSUBFOLDER;
    }
    else {
        for (UINT i = 0; i < cidl; ++i) {
            attributes &= PIDL::GetAttributes(apidl[i]);
        }
    }

    *rgfInOut &= attributes;

    return S_OK;
}


/// <summary>
/// IShellFolder::GetDisplayNameOf
/// Retrieves the display name for the specified file object or subfolder.
/// </summary>
HRESULT ShellFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF uFlags, STRRET *pName) {
    pName->uType = STRRET_WSTR;

    if ((uFlags & SHGDN_INFOLDER) == SHGDN_INFOLDER) {
        pName->pOleStr = PIDL::GetDisplayName(pidl);
    }
    else {
        pName->pOleStr = PIDL::GetFullPath(this->folder, pidl);
    }

    return S_OK;
}


/// <summary>
/// IShellFolder::GetUIObjectOf
/// Gets an object that can be used to carry out actions on the specified file objects or folders.
/// </summary>
HRESULT ShellFolder::GetUIObjectOf(HWND hwndOwner, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, REFIID riid, UINT *rgfReserved, void **ppv) {
    HRESULT hr;
    *ppv = NULL;

    if (ppv == NULL) {
        return E_POINTER;
    }

    // TODO::We need to override some things to make navigation work properly...
    if (PIDL::ItemCount(this->folder) > 1) {
        PIDLIST_ABSOLUTE idList = NULL;
        PIDL::PIDLItem* item = PIDL::Item(apidl[0]);

        if (item->folder >= this->folders.size()) {
            return E_FAIL;
        }

        hr = this->folders[item->folder]->ParseDisplayName(hwndOwner, NULL, item->name, NULL, &idList, NULL);
        hr = this->folders[item->folder]->GetUIObjectOf(hwndOwner, 1, (LPCITEMIDLIST *)&idList, riid, rgfReserved, ppv);
        ILFree(idList);
    }
    else {
        // These are pure virtual folders...
        if (riid == IID_IExtractIconA || riid == IID_IExtractIconW) {
            hr = SHCreateFileExtractIconW(L"C:\\WinUnionFSTest", FILE_ATTRIBUTE_DIRECTORY, riid, ppv);
        }
        else {
            *ppv = NULL;
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}


/// <summary>
/// IShellFolder::ParseDisplayName
/// Translates the display name of a file object or a folder into an item identifier list.
/// </summary>
HRESULT ShellFolder::ParseDisplayName(HWND hwnd, IBindCtx *pbc, LPWSTR pszDisplayName, ULONG *pchEaten, PIDLIST_RELATIVE *ppidl, ULONG *pdwAttributes) {
    HRESULT hr = E_FAIL;

    ULONG attributes = ULONG(-1);
    
    PIDLIST_ABSOLUTE idList = NULL;
    int i;
    for (i = 0; i < this->folders.size(); ++i) {
        if (SUCCEEDED(hr = this->folders[i]->ParseDisplayName(hwnd, NULL, pszDisplayName, pchEaten, &idList, &attributes))) {
            ILFree(idList);
            break;
        }
    }

    if (SUCCEEDED(hr)) {
        WCHAR path[MAX_PATH];
        WCHAR *context, *token;
        LPITEMIDLIST pidl, temp;

        StringCchCopyW(path, MAX_PATH, pszDisplayName);

        token = wcstok_s(path, L"/", &context);
        pidl = PIDL::Create(NULL, token, SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_HASSUBFOLDER, i);

        while ((token = wcstok_s(NULL, L"/", &context)) != NULL) {
            temp = PIDL::Create(pidl, token, SFGAO_BROWSABLE | SFGAO_FOLDER | SFGAO_HASSUBFOLDER, i);
            PIDL::Free(pidl);
            pidl = temp;
        }

        PIDL::Item(PIDL::Last(pidl))->attributes = attributes;
        *ppidl = pidl;

        if (pdwAttributes != NULL) {
            *pdwAttributes &= attributes;
        }
    }

    return hr;
}


/// <summary>
/// IShellFolder::SetNameOf
/// Sets the display name of a file object or subfolder, changing the item identifier in the process.
/// </summary>
HRESULT ShellFolder::SetNameOf(HWND hwndOwner, PCUITEMID_CHILD pidl, LPCWSTR pszName, SHGDNF uFlags, PITEMID_CHILD *ppidlOut) {
    return E_NOTIMPL;
}


/// <summary>
/// IShellFolder2::EnumSearches
/// Requests a pointer to an interface that allows a client to enumerate the available search objects.
/// </summary>
/// <param name="ppEnum">The address of a pointer to an enumerator object's IEnumExtraSearch interface.</param>
HRESULT ShellFolder::EnumSearches(IEnumExtraSearch **ppEnum) {
    *ppEnum = NULL;
    return E_NOTIMPL;
}


/// <summary>
/// IShellFolder2::GetDefaultColumn
/// Gets the default sorting and display columns.
/// </summary>
HRESULT ShellFolder::GetDefaultColumn(DWORD, ULONG *pSort, ULONG *pDisplay) {
    *pSort = 0;
    *pDisplay = 0;
    return S_OK;
}


/// <summary>
/// IShellFolder2::GetDefaultColumnState
/// Gets the default state for a specified column.
/// </summary>
HRESULT ShellFolder::GetDefaultColumnState(UINT iColumn, SHCOLSTATEF *pcsFlags) {
    switch (iColumn) {
    case 0:
        *pcsFlags = SHCOLSTATE_TYPE_STR;
        break;
    case 1:
        *pcsFlags = SHCOLSTATE_TYPE_STR;
        break;
    case 2:
        *pcsFlags = SHCOLSTATE_TYPE_STR;
        break;
    case 3:
        *pcsFlags = SHCOLSTATE_TYPE_DATE;
        break;
    default:
        return E_INVALIDARG;
    }

    return S_OK;
}


/// <summary>
/// IShellFolder2::GetDefaultSearchGUID
/// Returns the globally unique identifier (GUID) of the default search object for the folder.
/// </summary>
HRESULT ShellFolder::GetDefaultSearchGUID(GUID *pGuid) {
    *pGuid = CLSID_WinUnionFS;
    return E_NOTIMPL;
}


/// <summary>
/// IShellFolder2::GetDetailsEx
/// Gets detailed information, identified by a property set identifier (FMTID) and a property
/// identifier (PID), on an item in a Shell folder.
/// </summary>
HRESULT ShellFolder::GetDetailsEx(PCUITEMID_CHILD pidl, const SHCOLUMNID *pscid, VARIANT *pv) {
    if (pscid->fmtid == FMTID_Storage) {
        switch (pscid->pid) {
        case PID_STG_NAME:
            {
                LPWSTR name = PIDL::GetDisplayName(pidl);
                pv->bstrVal = SysAllocString(name);
                CoTaskMemFree(name);
            }
            break;

        case PID_STG_SIZE:
            {
                pv->bstrVal = SysAllocString(L"Size");
            }
            break;

        case PID_STG_WRITETIME:
            {
                pv->bstrVal = SysAllocString(L"Type");
            }
            break;

        case PID_STG_STORAGETYPE:
            {
                pv->bstrVal = SysAllocString(L"Date Modified");
            }
            break;

        default:
            return E_INVALIDARG;
        }
    }
    else {
        return E_FAIL;
    }

    return S_OK;
}


/// <summary>
/// IShellFolder2::GetDetailsOf
/// Gets detailed information, identified by a column index, on an item in a Shell folder.
/// </summary>
HRESULT ShellFolder::GetDetailsOf(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS *psd) {
    switch (iColumn) {
    case 0:
        {
            psd->fmt = LVCFMT_LEFT;
            psd->cxChar = 20;
            psd->str.uType = STRRET_WSTR;
            if (pidl == NULL) {
                SHStrDupW(L"Name", &psd->str.pOleStr);
            }
            else {
                psd->str.pOleStr = PIDL::GetDisplayName(pidl);
            }
        }
        break;

    case 1:
        {
            psd->fmt = LVCFMT_RIGHT;
            psd->cxChar = 20;
            psd->str.uType = STRRET_WSTR;
            SHStrDupW(L"Size", &psd->str.pOleStr);
        }
        break;

    case 2:
        {
            psd->fmt = LVCFMT_LEFT;
            psd->cxChar = 20;
            psd->str.uType = STRRET_WSTR;
            SHStrDupW(L"Type", &psd->str.pOleStr);
        }
        break;

    case 3:
        {
            psd->fmt = LVCFMT_LEFT;
            psd->cxChar = 20;
            psd->str.uType = STRRET_WSTR;
            SHStrDupW(L"Date Modified", &psd->str.pOleStr);
        }
        break;

    default:
        return E_INVALIDARG;
    }
    return S_OK;
}


/// <summary>
/// IShellFolder2::MapColumnToSCID
/// Converts a column to the appropriate property set ID (FMTID) and property ID (PID).
/// </summary>
HRESULT ShellFolder::MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid) {
    switch (iColumn) {
    case 0:
        {
            pscid->fmtid = FMTID_Storage;
            pscid->pid = PID_STG_NAME;
        }
        break;

    case 1:
        {
            pscid->fmtid = FMTID_Storage;
            pscid->pid = PID_STG_SIZE;
        }
        break;

    case 2:
        {
            pscid->fmtid = FMTID_Storage;
            pscid->pid = PID_STG_STORAGETYPE;
        }
        break;

    case 3:
        {
            pscid->fmtid = FMTID_Storage;
            pscid->pid = PID_STG_WRITETIME;
        }
        break;

    default:
        return E_INVALIDARG;
    }
    return S_OK;
}


/// <summary>
/// IPersist::GetClassID
/// Retrieves the class identifier (CLSID) of the object.
/// </summary>
HRESULT ShellFolder::GetClassID(CLSID *pClassID) {
    *pClassID = CLSID_WinUnionFS;

    return S_OK;
}


/// <summary>
/// IPersistIDList::GetIDList
/// Gets an item identifier list.
/// </summary>
HRESULT ShellFolder::GetIDList(LPITEMIDLIST *ppidl) {
    return GetCurFolder(ppidl);
}


/// <summary>
/// IPersistIDList::SetIDList
/// Sets a persisted item identifier list.
/// </summary>
HRESULT ShellFolder::SetIDList(LPCITEMIDLIST pidl) {
    return Initialize(pidl);
}


/// <summary>
/// IPersistFolder::Initialize
/// Instructs a Shell folder object to initialize itself based on the information passed.
/// </summary>
HRESULT ShellFolder::Initialize(LPCITEMIDLIST pidl) {
    PIDL::Free(this->folder);
    this->folder = PIDL::Copy(pidl);
    
    for (std::vector<IShellFolder*>::const_iterator folder = this->folders.begin(); folder != this->folders.end(); ++folder) {
        (*folder)->Release();
    }

    PIDL::GetShellFoldersFor(this->folder, &this->folders);

    return S_OK;
}
    

/// <summary>
/// IPersistFolder2::GetCurFolder
/// Gets the ITEMIDLIST for the folder object.
/// </summary>
HRESULT ShellFolder::GetCurFolder(LPITEMIDLIST *ppidl) {
    *ppidl = PIDL::Copy(this->folder);
    return S_OK;
}


/// <summary>
/// IPersistFolder3::GetFolderTargetInfo
/// Provides the location and attributes of a folder shortcut's target folder.
/// </summary>
HRESULT ShellFolder::GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO *ppfti) {
    ppfti->pidlTargetFolder = PIDL::Copy(this->folder);
    //PIDL::GetFullPath(this->folder, ,

    return E_NOTIMPL;
}


/// <summary>
/// IPersistFolder3::InitializeEx
/// Initializes a folder and specifies its location in the namespace. If the folder is a shortcut,
/// this method also specifies the location of the target folder.
/// </summary>
HRESULT ShellFolder::InitializeEx(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *ppfti) {
    Initialize(pidlRoot);

    return E_NOTIMPL;
}
