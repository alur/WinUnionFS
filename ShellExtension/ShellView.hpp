/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  ShellView.hpp
 *  The WinUnionFS Project
 *
 *  Implementation of IShellView.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once

class ShellView : public IShellView3
{
public:
    // Constructor
    explicit ShellView(IShellFolder* folder);

    // IUnknown
    ULONG STDMETHODCALLTYPE AddRef();
    STDMETHOD(QueryInterface) (REFIID, void**);
    ULONG STDMETHODCALLTYPE Release();

    // IShellView
    STDMETHOD(AddPropertySheetPages) (DWORD, LPFNADDPROPSHEETPAGE, LPARAM);
    STDMETHOD(CreateViewWindow) (IShellView*, LPCFOLDERSETTINGS, IShellBrowser*, RECT*, HWND*);
    STDMETHOD(DestroyViewWindow) ();
    STDMETHOD(EnableModeless) (BOOL);
    STDMETHOD(EnableModelessSV) ();
    STDMETHOD(GetCurrentInfo) (LPFOLDERSETTINGS);
    STDMETHOD(GetItemObject) (UINT, REFIID, LPVOID*);
    STDMETHOD(Refresh) ();
    STDMETHOD(SaveViewState) ();
    STDMETHOD(SelectItem) (PCUITEMID_CHILD, UINT);
    STDMETHOD(TranslateAccelerator) (LPMSG);
    STDMETHOD(UIActivate) (UINT);

    // IShellView2
    STDMETHOD(CreateViewWindow2) (LPSV2CVW2_PARAMS);
    STDMETHOD(GetView) (SHELLVIEWID*, ULONG);
    STDMETHOD(HandleRename) (PCUITEMID_CHILD);
    STDMETHOD(SelectAndPositionItem) (PCUITEMID_CHILD, UINT, POINT*);

    // IShellView3
    STDMETHOD(CreateViewWindow3) (IShellBrowser*, IShellView*, SV3CVW3_FLAGS, FOLDERFLAGS, FOLDERFLAGS, FOLDERVIEWMODE, const SHELLVIEWID*, const RECT*, HWND*);

    // Creates the class for our view window
    bool CreateWindowClass();

private:
    // Destructor
    virtual ~ShellView();

    ULONG refCount; //

    IShellFolder* folder; // The IShellFolder 
    IShellView* previousView; //
    IShellBrowser* browser; //

    FOLDERSETTINGS folderSettings; //

    HWND hwnd; //
};


