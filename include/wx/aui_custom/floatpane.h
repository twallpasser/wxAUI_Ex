///////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/floatpane.h
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by:
// Created:     2005-05-17
// Copyright:   (C) Copyright 2005, Kirix Corporation, All Rights Reserved.
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FLOATPANE_H_
#define _WX_FLOATPANE_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/defs.h"
#include "wx/weakref.h"

#if wxUSE_AUI

#if wxUSE_MINIFRAME
    #include "wx/minifram.h"
    #define wxAuiFloatingFrameBaseClass wxMiniFrame
#else
    #include "wx/frame.h"
    #define wxAuiFloatingFrameBaseClass wxFrame
#endif

WX_DECLARE_OBJARRAY(wxWindow*,wxWindowPtrArray);

class WXDLLIMPEXP_AUI wxAuiFloatingFrame : public wxAuiFloatingFrameBaseClass
{
public:
    wxAuiFloatingFrame(wxWindow* parent,
                   wxAuiManager* ownerMgr,
                   const wxAuiPaneInfo& pane,
                   wxWindowID id = wxID_ANY,
                   long style = wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION |
                                wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT |
                                wxCLIP_CHILDREN
                   );
    virtual ~wxAuiFloatingFrame();
//    void SetPaneWindow(const wxAuiPaneInfo& pane);
	void ResetPaneWindow(wxWindow* win)
	{
		m_mgr.GetPane(m_paneWindow).window = win;
		m_paneWindow = win;
		m_mgr.Update();
	};
	void SetPaneWindow(wxAuiPaneInfo& pane,wxAuiManagerDock dir=wxAUI_DOCK_NONE,wxWindow* pContainer=NULL);
    wxAuiManager* GetOwnerManager() const;
	wxAuiManager* GetManager(){return &m_mgr;};
    // Allow processing accelerators to the parent frame
    virtual bool IsTopNavigationDomain(NavigationKind kind) const wxOVERRIDE;
	virtual bool Destroy() wxOVERRIDE;
protected:
    virtual void OnMoveStart();
    virtual void OnMoving(const wxRect& windowRect, wxDirection dir);
    virtual void OnMoveFinished();

private:
    void OnSize(wxSizeEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnMoveEvent(wxMoveEvent& event);
    void OnIdle(wxIdleEvent& event);
    void OnActivate(wxActivateEvent& event);
    static bool isMouseDown();

public:
	wxWindowPtrArray paneWindows; // pane windows being managed
    wxWindow* m_paneWindow;    // pane window being managed
private:
    bool m_solidDrag;          // true if system uses solid window drag
    bool m_moving;
    wxRect m_lastRect;
    wxRect m_last2Rect;
    wxRect m_last3Rect;
    wxSize m_lastSize;
    wxDirection m_lastDirection;

    wxWeakRef<wxAuiManager> m_ownerMgr;
    wxAuiManager m_mgr;

#ifndef SWIG
    wxDECLARE_EVENT_TABLE();
    wxDECLARE_CLASS(wxAuiFloatingFrame);
#endif // SWIG
};

#endif // wxUSE_AUI
#endif //_WX_FLOATPANE_H_

