// MyWindowsNew.h

#ifndef __MY_WINDOWS_NEW_H
#define __MY_WINDOWS_NEW_H

#include <commctrl.h>

#ifdef _MSC_VER

    MIDL_INTERFACE("56FDF342-FD6D-11d0-958A-006097C9A090")
    ITaskbarList : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE HrInit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddTab( 
            /* [in] */ HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteTab( 
            /* [in] */ HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ActivateTab( 
            /* [in] */ HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetActiveAlt( 
            /* [in] */ HWND hwnd) = 0;
        
    };

EXTERN_C const IID IID_ITaskbarList2;

   
    MIDL_INTERFACE("602D4995-B13A-429b-A66E-1935E44F4317")
    ITaskbarList2 : public ITaskbarList
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MarkFullscreenWindow( 
            /* [in] */ HWND hwnd,
            /* [in] */ BOOL fFullscreen) = 0;
        
    };

#ifndef __ITaskbarList3_INTERFACE_DEFINED__
#define __ITaskbarList3_INTERFACE_DEFINED__

typedef enum THUMBBUTTONFLAGS
{
  THBF_ENABLED = 0,
  THBF_DISABLED = 0x1,
  THBF_DISMISSONCLICK = 0x2,
  THBF_NOBACKGROUND = 0x4,
  THBF_HIDDEN = 0x8,
  THBF_NONINTERACTIVE = 0x10
} THUMBBUTTONFLAGS;

typedef enum THUMBBUTTONMASK
{
  THB_BITMAP = 0x1,
  THB_ICON = 0x2,
  THB_TOOLTIP = 0x4,
  THB_FLAGS = 0x8
} THUMBBUTTONMASK;

// #include <pshpack8.h>

typedef struct THUMBBUTTON
{
  THUMBBUTTONMASK dwMask;
  UINT iId;
  UINT iBitmap;
  HICON hIcon;
  WCHAR szTip[260];
  THUMBBUTTONFLAGS dwFlags;
} THUMBBUTTON;

typedef struct THUMBBUTTON *LPTHUMBBUTTON;

typedef enum TBPFLAG
{
  TBPF_NOPROGRESS = 0,
  TBPF_INDETERMINATE = 0x1,
  TBPF_NORMAL = 0x2,
  TBPF_ERROR = 0x4,
  TBPF_PAUSED = 0x8
} TBPFLAG;

DEFINE_GUID(IID_ITaskbarList3, 0xEA1AFB91, 0x9E28, 0x4B86, 0x90, 0xE9, 0x9E, 0x9F, 0x8A, 0x5E, 0xEF, 0xAF);

struct ITaskbarList3: public ITaskbarList2
{
  STDMETHOD(SetProgressValue)(HWND hwnd, ULONGLONG ullCompleted, ULONGLONG ullTotal) = 0;
  STDMETHOD(SetProgressState)(HWND hwnd, TBPFLAG tbpFlags) = 0;
  STDMETHOD(RegisterTab)(HWND hwndTab, HWND hwndMDI) = 0;
  STDMETHOD(UnregisterTab)(HWND hwndTab) = 0;
  STDMETHOD(SetTabOrder)(HWND hwndTab, HWND hwndInsertBefore) = 0;
  STDMETHOD(SetTabActive)(HWND hwndTab, HWND hwndMDI, DWORD dwReserved) = 0;
  STDMETHOD(ThumbBarAddButtons)(HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) = 0;
  STDMETHOD(ThumbBarUpdateButtons)(HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) = 0;
  STDMETHOD(ThumbBarSetImageList)(HWND hwnd, HIMAGELIST himl) = 0;
  STDMETHOD(SetOverlayIcon)(HWND hwnd, HICON hIcon, LPCWSTR pszDescription) = 0;
  STDMETHOD(SetThumbnailTooltip)(HWND hwnd, LPCWSTR pszTip) = 0;
  STDMETHOD(SetThumbnailClip)(HWND hwnd, RECT *prcClip) = 0;
};

#endif

#endif

#endif
