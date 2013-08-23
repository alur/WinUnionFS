/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Group.hpp
 *  The WinUnionFS Project
 *  
 *  Represents one of 
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

#include <vector>

class Group
{
public:
    // Static methods
    static void AddUser();
    static void RemoveUser();

    static Group* Create(LPCWSTR name);
    static void Delete(LPCWSTR name);
    static Group* Find(LPCWSTR name);
    static Group* Find(int index);

    // Instance methods
    void GetShellFoldersFor(LPCWSTR path, std::vector<IShellFolder*> *out);

    // The name of the group
    LPCWSTR name;

private:
    //
    static HRESULT Load();

    // The number of live objects which use this class
    static ULONG userCount;

    // The currently loaded groups
    static std::vector<Group*> groups;


    // Constructor/Destructor
    explicit Group(LPCWSTR name);
    virtual ~Group();
    
    // Instance methods
    HRESULT AddPath(LPCWSTR path);

    // The IShellFolders which make up this group
    std::vector<IShellFolder*> folders;
};
