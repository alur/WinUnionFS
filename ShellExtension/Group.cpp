/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Group.cpp
 *  The WinUnionFS Project
 *
 *  
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <Windows.h>
#include <Shlobj.h>
#include <Shlwapi.h>

#include "Group.hpp"


// The number of live objects which use this class
ULONG Group::userCount = 0;

// The currently loaded groups
std::vector<Group*> Group::groups;


/// <summary>
/// Should be called when a new object which uses groups is created.
/// </summary>
void Group::AddUser() {
    if (InterlockedIncrement(&Group::userCount) == 1) {
        Load();
    }
}


/// <summary>
/// Should be called when a object which uses groups is deleted.
/// </summary>
void Group::RemoveUser() {
    if (InterlockedDecrement(&Group::userCount) == 0) {
        // Erase all groups
        for (std::vector<Group*>::const_iterator group = Group::groups.begin(); group != Group::groups.end(); ++group) {
            delete *group;
        }
        Group::groups.clear();
    }
}


/// <summary>
/// Loads the groups from the registry.
/// </summary>
HRESULT Group::Load() {
    // HKCU/Software/WinUnionFS/Groups
    HKEY groupsKey, groupKey, foldersKey;

    if (RegCreateKeyW(HKEY_CURRENT_USER, L"SOFTWARE\\WinUnionFS\\Groups", &groupsKey) != ERROR_SUCCESS) {
        return E_UNEXPECTED;
    }

    DWORD groupIndex = 0, cchName = MAX_PATH;
    WCHAR name[MAX_PATH];
    while (RegEnumKeyExW(groupsKey, groupIndex++, name, &cchName, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS) {
        Group* group = new Group(name);

        RegCreateKeyW(groupsKey, name, &groupKey);

        RegCreateKeyW(groupKey, L"Folders", &foldersKey);

        // Enumerate the folders
        DWORD folderIndex = 0, cchFolderKeyName = MAX_PATH;
        WCHAR folderKeyName[MAX_PATH];
        while (RegEnumKeyExW(foldersKey, folderIndex++, folderKeyName, &cchFolderKeyName, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS) {
            DWORD cbFolderPath = MAX_PATH*sizeof(WCHAR);
            WCHAR folderPath[MAX_PATH];

            RegGetValueW(foldersKey, folderKeyName, L"Path", RRF_RT_REG_SZ, NULL, &folderPath, &cbFolderPath);

            group->AddPath(folderPath);

            cchFolderKeyName = MAX_PATH;
        }

        RegCloseKey(foldersKey);

        RegCloseKey(groupKey);

        Group::groups.push_back(group);
        cchName = MAX_PATH;
    }

    RegCloseKey(groupsKey);

    return S_OK;
}


/// <summary>
/// Constructor.
/// </summary>
Group::Group(LPCWSTR name) {
    this->name = _wcsdup(name);
}


/// <summary>
/// Destructor.
/// </summary>
Group::~Group() {
    for (std::vector<IShellFolder*>::const_iterator folder = this->folders.begin(); folder != this->folders.end(); ++folder) {
        (*folder)->Release();
    }
    free((LPVOID)this->name);
}


/// <summary>
/// Adds a folder to this group.
/// </summary>
HRESULT Group::AddPath(LPCWSTR path) {
    IShellFolder *desktopFolder = NULL, *folder;
    PIDLIST_ABSOLUTE idList = NULL;
    HRESULT hr;

    hr = SHGetDesktopFolder(&desktopFolder);

    if (SUCCEEDED(hr)) {
        hr = desktopFolder->ParseDisplayName(NULL, NULL, (LPWSTR)path, NULL, &idList, NULL);
    }

    if (SUCCEEDED(hr)) {
        hr = desktopFolder->BindToObject(idList, NULL, IID_IShellFolder, reinterpret_cast<LPVOID*>(&folder));
    }

    if (SUCCEEDED(hr)) {
        this->folders.push_back(folder);
    }

    if (idList != NULL) {
        CoTaskMemFree(idList);
    }

    if (desktopFolder != NULL) {
        desktopFolder->Release();
    }

    return hr;
}


/// <summary>
/// Retrives IShellFolder pointers for all folders in this group.
/// </summary>
void Group::GetShellFoldersFor(LPCWSTR path, std::vector<IShellFolder*> *out) {
    PIDLIST_ABSOLUTE idList = NULL;
    IShellFolder *targetFolder;

    for (std::vector<IShellFolder*>::const_iterator folder = this->folders.begin(); folder != this->folders.end(); ++folder) {
        if (path[0] == '\0') {
            out->push_back(*folder);
            (*folder)->AddRef();
        }
        else {
            if (SUCCEEDED((*folder)->ParseDisplayName(NULL, NULL, (LPWSTR)path, NULL,  &idList, NULL))) {
                if (SUCCEEDED((*folder)->BindToObject(idList, NULL, IID_IShellFolder, reinterpret_cast<LPVOID*>(&targetFolder)))) {
                    out->push_back(targetFolder);
                }
                CoTaskMemFree(idList);
            }
        }
    }
}


/// <summary>
/// Finds an existing group with the specified name.
/// </summary>
Group* Group::Find(LPCWSTR name) {
    int index = 0;
    Group* group;
    while ((group = Find(index++)) != NULL) {
        if (wcscmp(name, group->name) == 0) {
            break;
        }
    }

    return group;
}


/// <summary>
/// Returns group # index if it exists, otherwise NULL.
/// </summary>
Group* Group::Find(int index) {
    return index < Group::groups.size() ? Group::groups[index] : NULL;
}
