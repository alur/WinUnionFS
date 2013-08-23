/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  ShellView.cpp
 *  The WinUnionFS Project
 *
 *  Implementation of IShellView.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include <Windows.h>
#include <Shobjidl.h>
#include <strsafe.h>
#include <Shlobj.h>

#include "Debug.h"
#include "Macros.h"
#include "ShellFolder.hpp"
#include "ShellView.hpp"


// The number of in-use objects.
extern long objectCounter;

// The CLSID of WinUnionFS.
extern const CLSID CLSID_WinUnionFS;


/// <summary>
/// Constructor.
/// </summary>
ShellView::ShellView(IShellFolder* folder) {
    this->refCount = 1;
    InterlockedIncrement(&::objectCounter);
    this->folder = folder;
}


/// <summary>
/// Destructor.
/// </summary>
ShellView::~ShellView() {
    this->folder->Release();
    InterlockedDecrement(&::objectCounter);
}


/// <summary>
/// IUnknown::AddRef
/// Increments the reference count for an interface on an object.
/// </summary>
ULONG ShellView::AddRef() {
    return InterlockedIncrement(&this->refCount);
}


/// <summary>
/// IUnknown::Release
/// Decrements the reference count for an interface on an object.
/// </summary>
ULONG ShellView::Release() {
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
HRESULT ShellView::QueryInterface(REFIID riid, void **ppvObject) {
    if (ppvObject == NULL) {
        return E_POINTER;
    }

    if (riid == IID_IShellView) {
        *ppvObject = (IShellView*)this;
    }
    /*else if (riid == IID_IShellView2) {
        *ppvObject = (IShellView2*)this;
    }
    else if (riid == IID_IShellView3) {
        *ppvObject = (IShellView3*)this;
    }*/
    else {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


/// <summary>
/// IShellView::AddPropertySheetPages
/// Allows the view to add pages to the Options property sheet from the View menu.
/// </summary>
HRESULT ShellView::AddPropertySheetPages(DWORD, LPFNADDPROPSHEETPAGE lpfn, LPARAM lparam) {

    return S_OK;
}


/// <summary>
/// IShellView::CreateViewWindow
/// Creates a view window. This can be either the right pane of Windows Explorer or the client window of a folder window.
/// </summary>
HRESULT ShellView::CreateViewWindow(IShellView *pszPrevious, LPCFOLDERSETTINGS pfs, IShellBrowser *psb, RECT *prcView, HWND *phWnd) {
    if (this->hwnd != NULL) {
        return E_FAIL;
        *phWnd = NULL;
    }

    this->folderSettings = *pfs;
    this->previousView = pszPrevious;
    this->browser = psb;

    return E_NOTIMPL;
}


/// <summary>
/// IShellView::DestroyViewWindow
/// Destroys the view window.
/// </summary>
HRESULT ShellView::DestroyViewWindow() {
    if (this->hwnd == NULL) {
        return E_FAIL;
    }

    //DestroyWindow(this->hwnd);
    this->hwnd = NULL;
    return S_OK;
}


/// <summary>
/// IShellView::EnableModeless
/// Enables or disables modeless dialog boxes.
/// </summary>
HRESULT ShellView::EnableModeless(BOOL) {
    return S_OK;
}


/// <summary>
/// IShellView::EnableModelessSV
/// This method is not implemented.
/// </summary>
HRESULT ShellView::EnableModelessSV() {
    return E_NOTIMPL;
}


/// <summary>
/// IShellView::GetCurrentInfo
/// Gets the current folder settings.
/// </summary>
HRESULT ShellView::GetCurrentInfo(LPFOLDERSETTINGS lpfs) {
    *lpfs = this->folderSettings;
    return S_OK;
}


/// <summary>
/// IShellView::GetItemObject
/// Gets an interface that refers to data presented in the view.
/// </summary>
HRESULT ShellView::GetItemObject(UINT uItem, REFIID riid, LPVOID *ppv) {
    HRESULT hr = S_OK;
    *ppv = NULL;
    bool useViewOrder = FLAGSET(uItem, SVGIO_FLAG_VIEWORDER);
    UINT type = uItem & SVGIO_TYPE_MASK;


    if (riid == IID_IContextMenu) {
        switch (type) {
        case SVGIO_BACKGROUND:
            {
            }
            break;

        default:
            {
                hr = E_FAIL;
            }
            break;
        }
    }
    else if (riid == IID_IDispatch) {
        switch (type) {
        case SVGIO_SELECTION:
            {
            }
            break;

        case SVGIO_ALLVIEW:
            {
            }
            break;

        case SVGIO_CHECKED:
            {
            }
            break;

        default:
            {
                hr = E_FAIL;
            }
            break;
        }
    }
    else if (riid == IID_IDataObject) {
        switch (type) {
        case SVGIO_SELECTION:
            {
            }
            break;

        case SVGIO_ALLVIEW:
            {
            }
            break;

        case SVGIO_CHECKED:
            {
            }
            break;

        default:
            {
                hr = E_FAIL;
            }
            break;
        }
    }
    else {
        hr = E_NOINTERFACE;
    }

    hr = E_NOINTERFACE;

    return hr;
}


/// <summary>
/// IShellView::Refresh
/// Refreshes the view's contents in response to user input.
/// </summary>
HRESULT ShellView::Refresh() {
    return S_OK;
}


/// <summary>
/// IShellView::SaveViewState
/// Saves the Shell's view settings so the current state can be restored during a subsequent browsing session.
/// </summary>
HRESULT ShellView::SaveViewState() {
    return S_OK;
}


/// <summary>
/// IShellView::SelectItem
/// Changes the selection state of one or more items within the Shell view window.
/// </summary>
HRESULT ShellView::SelectItem(PCUITEMID_CHILD pidlItem, UINT uFlags) {
    return S_OK;
}


/// <summary>
/// IShellView::TranslateAccelerator
/// Translates keyboard shortcut (accelerator) key strokes when a namespace extension's view has the focus.
/// </summary>
HRESULT ShellView::TranslateAccelerator(LPMSG lpmsg) {
    return S_FALSE;
}


/// <summary>
/// IShellView::UIActivate
/// Called when the activation state of the view window is changed by an event that is not caused by the Shell view itself.
/// </summary>
HRESULT ShellView::UIActivate(UINT uState) {
    return S_OK;
}


/// <summary>
/// IShellView2::CreateViewWindow2
/// Used to request the creation of a new Shell view window.
/// </summary>
HRESULT ShellView::CreateViewWindow2(LPSV2CVW2_PARAMS) {
    return E_NOTIMPL;
}


/// <summary>
/// IShellView2::GetView
/// Requests the current or default Shell view, together with all other valid view identifiers (VIDs) supported by this implementation of IShellView2.
/// </summary>
HRESULT ShellView::GetView(SHELLVIEWID*, ULONG) {
    return E_NOTIMPL;
}


/// <summary>
/// IShellView2::HandleRename
/// Used to change an item's identifier.
/// </summary>
HRESULT ShellView::HandleRename(PCUITEMID_CHILD) {
    return E_NOTIMPL;
}


/// <summary>
/// IShellView2::SelectAndPositionItem
/// Selects and positions an item in a Shell View.
/// </summary>
HRESULT ShellView::SelectAndPositionItem(PCUITEMID_CHILD pidlItem, UINT uFlags, POINT *point) {
    return E_NOTIMPL;
}


/// <summary>
/// IShellView3::CreateViewWindow3
/// Requests the creation of a new Shell view window.
/// </summary>
HRESULT ShellView::CreateViewWindow3(IShellBrowser*, IShellView*, SV3CVW3_FLAGS, FOLDERFLAGS, FOLDERFLAGS, FOLDERVIEWMODE, const SHELLVIEWID*, const RECT*, HWND*) {
    return E_NOTIMPL;
}