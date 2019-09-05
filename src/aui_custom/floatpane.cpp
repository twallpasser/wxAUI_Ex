///////////////////////////////////////////////////////////////////////////////
// Name:        src/aui/floatpane.cpp
// Purpose:     wxaui: wx advanced user interface - docking window manager
// Author:      Benjamin I. Williams
// Modified by:
// Created:     2005-05-17
// Copyright:   (C) Copyright 2005-2006, Kirix Corporation, All Rights Reserved
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_AUI

#include "wx/aui_custom/framemanager.h"
#include "wx/aui_custom/floatpane.h"
#include "wx/aui_custom/dockart.h"

#ifndef WX_PRECOMP
#endif

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

#include <wx/arrimpl.cpp> // This is a magic incantation which must be done!
WX_DEFINE_OBJARRAY(wxWindowPtrArray);

wxIMPLEMENT_CLASS(wxAuiFloatingFrame, wxAuiFloatingFrameBaseClass);

wxAuiFloatingFrame::wxAuiFloatingFrame(wxWindow* parent,
                wxAuiManager* owner_mgr,
                const wxAuiPaneInfo& pane,
                wxWindowID id /*= wxID_ANY*/,
                long style /*=wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION |
                              wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT |
                              wxCLIP_CHILDREN
                           */)
                : wxAuiFloatingFrameBaseClass(parent, id, wxEmptyString,
                        pane.floating_pos, pane.floating_size,
                        style |
                        (pane.HasCloseButton()?wxCLOSE_BOX:0) |
                        (pane.HasMaximizeButton()?wxMAXIMIZE_BOX:0) |
                        (pane.IsFixed()?0:wxRESIZE_BORDER)
                        )
{
	m_paneWindow = NULL;

    m_ownerMgr = owner_mgr;
    m_moving = false;
    m_mgr.SetManagedWindow(this);
	m_mgr.SetMainManager(false);
    m_solidDrag = true;

    // find out if the system supports solid window drag.
    // on non-msw systems, this is assumed to be the case
#ifdef __WXMSW__
    BOOL b = TRUE;
    SystemParametersInfo(38 /*SPI_GETDRAGFULLWINDOWS*/, 0, &b, 0);
    m_solidDrag = b ? true : false;
#endif

    SetExtraStyle(wxWS_EX_PROCESS_IDLE);
}

wxAuiFloatingFrame::~wxAuiFloatingFrame()
{
    // if we do not do this, then we can crash...
    if (m_ownerMgr && m_ownerMgr->m_actionWindow == this)
    {
        m_ownerMgr->m_actionWindow = NULL;
    }

    m_mgr.UnInit();
}
extern int GetMaxRow(const wxAuiPaneInfoArray& panes, int direction, int layer);
void wxAuiFloatingFrame::SetPaneWindow(wxAuiPaneInfo& pane,wxAuiManagerDock dir,wxWindow* pContainer)
{
	if(dir!=wxAuiManagerDock::wxAUI_DOCK_NONE)
	{
		switch(dir)
		{
		case wxAUI_DOCK_CENTER:
			{
				wxAuiPaneInfo& pane_container = m_mgr.GetPane(pContainer);
				if(pane_container.HasTab())
				{
					wxAuiPaneInfo copy_pane = pane;

					copy_pane.window->Reparent(pane_container.notebook);
					if(pane.IsFloating() && pane.frame)
					{
						pane.frame->Destroy();
						// 					pane.frame = NULL;
					}

					copy_pane.buttons.Clear();
					pane_container.notebook->AddPage(copy_pane.window,copy_pane.caption,copy_pane);

					m_ownerMgr->DetachPane(copy_pane.window);
				}
				else
				{
					wxAuiPaneInfo copy_pane = pane_container;
					copy_pane.buttons.Clear();

					wxWindow *pParent = this;
					wxNotebookEx *notebook = new wxNotebookEx(pParent,m_ownerMgr);
					copy_pane.window = notebook;
					copy_pane.notebook = notebook;
					copy_pane.Name(wxEmptyString);

					// reparent
					pane_container.window->Reparent(notebook);
					notebook->AddPage(pane_container.window,pane_container.caption,copy_pane);
					if(pane.HasTab())
					{
						for(int i=0,cnt=pane.notebook->GetPageCount();i<cnt;i++)
						{
							wxAuiPaneInfo nb_tab_pane = pane.notebook->page_panes[0];
							wxWindow* nb_tab_wnd = pane.notebook->GetPage(0);
							nb_tab_pane.window = nb_tab_wnd;
							nb_tab_pane.window->Reparent(notebook);
							notebook->AddPage(nb_tab_pane.window,nb_tab_pane.caption,nb_tab_pane);

							pane.notebook->page_panes.RemoveAt(0);
							pane.notebook->RemovePage(0);
						}
						wxWindow* pn_nb_wnd = (wxWindow*)pane.notebook;
						m_ownerMgr->DetachPane(pane.notebook);
						pn_nb_wnd->Destroy();
					}
					else
					{
						pane.window->Reparent(notebook);
						notebook->AddPage(pane.window,pane.caption,pane);

						m_ownerMgr->DetachPane(pane.window);
					}

					// detach pane
					m_mgr.DetachPane(pContainer);
					wxAuiPaneInfo& copy_pane_in_main = m_ownerMgr->GetPane(pContainer);

					m_mgr.AddPane(notebook,copy_pane);
					wxAuiPaneInfo &info = m_mgr.GetPane((wxWindow*)notebook);
					notebook->SetPaneInfo(info);

					copy_pane_in_main.window = notebook;
					copy_pane_in_main.notebook = notebook;
					copy_pane_in_main.name = info.name;
					
					// 				wxAuiFloatingFrame *float_frame = wxDynamicCast(pParent,wxAuiFloatingFrame);
					// 				if(float_frame)
					// 					float_frame->ResetPaneWindow(notebook);
					m_mgr.Update();
				}
			}
			break;
		default:
			{
				paneWindows.Add(pane.window);
				wxWindow* pane_window = paneWindows.Last();

				wxAuiPaneInfo contained_pane = pane;
				contained_pane.buttons.Clear();
				contained_pane.Dock().Direction(dir).Show().
					Layer(0).Row(GetMaxRow(m_mgr.m_panes,dir,0)+1).Position(0);
				m_mgr.AddPane(pane_window,contained_pane);

				wxAuiPaneInfo& center_pane = m_mgr.GetPane(m_paneWindow);
				center_pane.CaptionVisible(true).PaneBorder(true);
				pane_window->GetContainingSizer()->Detach(pane_window);
				pane_window->SetContainingSizer(NULL);

				m_mgr.Update();

				SetTitle(wxEmptyString);
			}
			break;
		}
		
	}
	else
	{
		paneWindows.Add(pane.window);
// 		wxWindow* pane_window = paneWindows.Last();
// 		pane_window->Reparent(this);
		m_paneWindow = pane.window;
		m_paneWindow->Reparent(this);

		wxAuiPaneInfo contained_pane = pane;
		contained_pane.buttons.Clear();
		contained_pane.Dock().Center().Show().
			CaptionVisible(false).
			PaneBorder(false).
			Layer(0).Row(0).Position(0);

		// Carry over the minimum size
		wxSize pane_min_size = pane.window->GetMinSize();

		// if the frame window's max size is greater than the min size
		// then set the max size to the min size as well
		wxSize cur_max_size = GetMaxSize();
		if (cur_max_size.IsFullySpecified() &&
			(cur_max_size.x < pane.min_size.x ||
			cur_max_size.y < pane.min_size.y)
			)
		{
			SetMaxSize(pane_min_size);
		}

		SetMinSize(pane.window->GetMinSize());

		m_mgr.AddPane(m_paneWindow/*pane_window*/, contained_pane);
		m_mgr.Update();

		if (pane.min_size.IsFullySpecified())
		{
			// because SetSizeHints() calls Fit() too (which sets the window
			// size to its minimum allowed), we keep the size before calling
			// SetSizeHints() and reset it afterwards...
			wxSize tmp = GetSize();
			GetSizer()->SetSizeHints(this);
			SetSize(tmp);
		}

		SetTitle(pane.caption);

		// This code is slightly awkward because we need to reset wxRESIZE_BORDER
		// before calling SetClientSize() below as doing it after setting the
		// client size would actually change it, at least under MSW, where the
		// total window size doesn't change and hence, as the borders size changes,
		// the client size does change.
		//
		// So we must call it first but doing it generates a size event and updates
		// pane.floating_size from inside it so we must also record its original
		// value before doing it.
		const bool hasFloatingSize = pane.floating_size != wxDefaultSize;
		if (pane.IsFixed())
		{
			SetWindowStyleFlag(GetWindowStyleFlag() & ~wxRESIZE_BORDER);
		}

		if ( hasFloatingSize )
		{
			SetSize(pane.floating_size);
		}
		else
		{
			wxSize size = pane.best_size;
			if (size == wxDefaultSize)
				size = pane.min_size;
			if (size == wxDefaultSize)
				size = m_paneWindow/*pane_window*/->GetSize();
			if (m_ownerMgr && pane.HasGripper())
			{
				if (pane.HasGripperTop())
					size.y += m_ownerMgr->m_art->GetMetric(wxAUI_DOCKART_GRIPPER_SIZE);
				else
					size.x += m_ownerMgr->m_art->GetMetric(wxAUI_DOCKART_GRIPPER_SIZE);
			}

			SetClientSize(size);
		}
	}
}

wxAuiManager* wxAuiFloatingFrame::GetOwnerManager() const
{
    return m_ownerMgr;
}

bool wxAuiFloatingFrame::IsTopNavigationDomain(NavigationKind kind) const
{
    switch ( kind )
    {
        case Navigation_Tab:
            break;

        case Navigation_Accel:
            // Floating frames are often used as tool palettes and it's
            // convenient for the accelerators defined in the parent frame to
            // work in them, so don't block their propagation.
            return false;
    }

    return wxAuiFloatingFrameBaseClass::IsTopNavigationDomain(kind);
}

void wxAuiFloatingFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
    if (m_ownerMgr)
    {
// 		for(int i=0,cnt=paneWindows.GetCount();i<cnt;i++)
// 		{
// 			m_ownerMgr->OnFloatingPaneResized(paneWindows[i], paneWindows[i]->GetRect());
// 		}
// qiugan comment 20190902
//        m_ownerMgr->OnFloatingPaneResized(m_paneWindow, GetRect());
    }
}

void wxAuiFloatingFrame::OnClose(wxCloseEvent& evt)
{
    if (m_ownerMgr)
    {
// 		for(int i=0,cnt=paneWindows.GetCount();i<cnt;i++)
// 		{
// 			m_ownerMgr->OnFloatingPaneClosed(paneWindows[i], evt);
// 		}
       m_ownerMgr->OnFloatingPaneClosed(m_paneWindow, evt);
    }
    if (!evt.GetVeto())
    {
// 		for(int i=0,cnt=paneWindows.GetCount();i<cnt;i++)
// 		{
// 			m_mgr.DetachPane(paneWindows[i]);
// 		}
        m_mgr.DetachPane(m_paneWindow);
        Destroy();
    }
}

void wxAuiFloatingFrame::OnMoveEvent(wxMoveEvent& event)
{
    if (!m_solidDrag)
    {
        // systems without solid window dragging need to be
        // handled slightly differently, due to the lack of
        // the constant stream of EVT_MOVING events
        if (!isMouseDown())
            return;
        OnMoveStart();
        OnMoving(event.GetRect(), wxNORTH);
        m_moving = true;
        return;
    }


    wxRect winRect = GetRect();

    if (winRect == m_lastRect)
        return;

    // skip the first move event
    if (m_lastRect.IsEmpty())
    {
        m_lastRect = winRect;
        return;
    }

    // as on OSX moving windows are not getting all move events, only sporadically, this difference
    // is almost always big on OSX, so avoid this early exit opportunity
#ifndef __WXOSX__
    // skip if moving too fast to avoid massive redraws and
    // jumping hint windows
    // TODO: Should 3x3px threshold increase on Retina displays?
    if ((abs(winRect.x - m_lastRect.x) > 3) ||
        (abs(winRect.y - m_lastRect.y) > 3))
    {
        m_last3Rect = m_last2Rect;
        m_last2Rect = m_lastRect;
        m_lastRect = winRect;

        // However still update the internally stored position to avoid
        // snapping back to the old one later.
        if (m_ownerMgr)
        {
            m_ownerMgr->GetPane(m_paneWindow).
                floating_pos = winRect.GetPosition();
        }

        return;
    }
#endif

    // prevent frame redocking during resize
    if (m_lastRect.GetSize() != winRect.GetSize())
    {
        m_last3Rect = m_last2Rect;
        m_last2Rect = m_lastRect;
        m_lastRect = winRect;
        return;
    }

    wxDirection dir = wxALL;

    int horiz_dist = abs(winRect.x - m_last3Rect.x);
    int vert_dist = abs(winRect.y - m_last3Rect.y);

    if (vert_dist >= horiz_dist)
    {
        if (winRect.y < m_last3Rect.y)
            dir = wxNORTH;
        else
            dir = wxSOUTH;
    }
    else
    {
        if (winRect.x < m_last3Rect.x)
            dir = wxWEST;
        else
            dir = wxEAST;
    }

    m_last3Rect = m_last2Rect;
    m_last2Rect = m_lastRect;
    m_lastRect = winRect;

    if (!isMouseDown())
        return;

    if (!m_moving)
    {
        OnMoveStart();
        m_moving = true;
    }

    if (m_last3Rect.IsEmpty())
        return;

    if ( event.GetEventType() == wxEVT_MOVING )
        OnMoving(event.GetRect(), dir);
    else
        OnMoving(wxRect(event.GetPosition(),GetSize()), dir);
}

void wxAuiFloatingFrame::OnIdle(wxIdleEvent& event)
{
    if (m_moving)
    {
        if (!isMouseDown())
        {
            m_moving = false;
            OnMoveFinished();
        }
        else
        {
            event.RequestMore();
        }
    }
}

void wxAuiFloatingFrame::OnMoveStart()
{
    // notify the owner manager that the pane has started to move
    if (m_ownerMgr && m_mgr.GetAllPanes().GetCount()==1)
    {
		m_ownerMgr->OnFloatingPaneMoveStart(m_paneWindow);
    }
}

void wxAuiFloatingFrame::OnMoving(const wxRect& WXUNUSED(window_rect), wxDirection dir)
{
    // notify the owner manager that the pane is moving
    if (m_ownerMgr && m_mgr.GetAllPanes().GetCount()==1)
    {
        m_ownerMgr->OnFloatingPaneMoving(m_paneWindow, dir);
    }
    m_lastDirection = dir;
}

void wxAuiFloatingFrame::OnMoveFinished()
{
    // notify the owner manager that the pane has finished moving
    if (m_ownerMgr && m_mgr.GetAllPanes().GetCount()==1)
    {		
       m_ownerMgr->OnFloatingPaneMoved(m_paneWindow, m_lastDirection);
    }
}

void wxAuiFloatingFrame::OnActivate(wxActivateEvent& event)
{
    if (m_ownerMgr && event.GetActive())
    {
        m_ownerMgr->OnFloatingPaneActivated(m_paneWindow);
    }
}

// utility function which determines the state of the mouse button
// (independent of having a wxMouseEvent handy) - utimately a better
// mechanism for this should be found (possibly by adding the
// functionality to wxWidgets itself)
bool wxAuiFloatingFrame::isMouseDown()
{
    return wxGetMouseState().LeftIsDown();
}

bool wxAuiFloatingFrame::Destroy()
{
	return wxAuiFloatingFrameBaseClass::Destroy();
}

wxBEGIN_EVENT_TABLE(wxAuiFloatingFrame, wxAuiFloatingFrameBaseClass)
    EVT_SIZE(wxAuiFloatingFrame::OnSize)
    EVT_MOVE(wxAuiFloatingFrame::OnMoveEvent)
    EVT_MOVING(wxAuiFloatingFrame::OnMoveEvent)
    EVT_CLOSE(wxAuiFloatingFrame::OnClose)
    EVT_IDLE(wxAuiFloatingFrame::OnIdle)
    EVT_ACTIVATE(wxAuiFloatingFrame::OnActivate)
wxEND_EVENT_TABLE()


#endif // wxUSE_AUI
