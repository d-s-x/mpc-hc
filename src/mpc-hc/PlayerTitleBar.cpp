/*
* (C) 2014 see Authors.txt
*
* This file is part of MPC-HC.
*
* MPC-HC is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* MPC-HC is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerTitleBar.h"
#include "MainFrm.h"
#include "DSUtil.h"


// CPlayerTitleBar

IMPLEMENT_DYNAMIC(CPlayerTitleBar, CDialogBar)

CPlayerTitleBar::CPlayerTitleBar(CMainFrame* pMainFrame)
    : m_pMainFrame(pMainFrame)
    , m_title(false, true)
{
}

CPlayerTitleBar::~CPlayerTitleBar()
{
}

BOOL CPlayerTitleBar::Create(CWnd* pParentWnd)
{
    BOOL ret = CDialogBar::Create(pParentWnd, IDD_PLAYERTITLEBAR, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP, IDD_PLAYERTITLEBAR);
    return ret;
}

BOOL CPlayerTitleBar::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CDialogBar::PreCreateWindow(cs)) {
        return FALSE;
    }

    m_dwStyle &= ~CBRS_BORDER_TOP;
    m_dwStyle &= ~CBRS_BORDER_BOTTOM;

    return TRUE;
}

CSize CPlayerTitleBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
    CSize ret = __super::CalcFixedLayout(bStretch, bHorz);
    LONG height;

    if (CDC* pDC = GetDC()) {
        CFont* pOld = pDC->SelectObject(&m_title.GetFont());
        TEXTMETRIC tm;
        pDC->GetTextMetrics(&tm);
        pDC->SelectObject(pOld);
        ReleaseDC(pDC);
        height = tm.tmHeight;
    } else {
        ASSERT(FALSE);
    }
    ret.cy = height + m_pMainFrame->m_dpi.ScaleY(4) * 2;
    return ret;
}

int CPlayerTitleBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialogBar::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    CRect r;
    r.SetRectEmpty();

    m_title.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | SS_NOTIFY,
                   r, this, IDC_PLAYERSTATUS);

    Relayout();

    return 0;
}

void CPlayerTitleBar::Relayout()
{
    CString str;
    CRect r, r2;

    GetClientRect(r);

    int y = m_pMainFrame->m_dpi.ScaleY(4);


    r.DeflateRect(8, y, 8, y);

    if (CDC* pDC = m_title.GetDC()) {
        CFont* pOld = pDC->SelectObject(&m_title.GetFont());
        m_title.GetWindowText(str);
        r2 = r;
        r2.right = r2.left + pDC->GetTextExtent(str).cx;
        if (r2.right > r.right) {
            r2.right = r.right;
        }
        m_title.MoveWindow(&r2, FALSE);
        pDC->SelectObject(pOld);
        m_title.ReleaseDC(pDC);
    } else {
        ASSERT(FALSE);
    }

    InvalidateRect(r);
    UpdateWindow();
}

void CPlayerTitleBar::Clear()
{
    m_title.SetWindowText(_T(""));
}

CString CPlayerTitleBar::GetMediaTitle() const
{
    CString strResult;

    m_title.GetWindowText(strResult);

    return strResult;
}

void CPlayerTitleBar::SetMediaTitle(CString str)
{
    str.Trim();
    if (GetMediaTitle() != str) {
        m_title.SetRedraw(FALSE);
        m_title.SetWindowText(str);
        m_title.SetRedraw(TRUE);
        Relayout();
    }
}

BEGIN_MESSAGE_MAP(CPlayerTitleBar, CDialogBar)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_LBUTTONDOWN()
    ON_WM_SETCURSOR()
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CPlayerTitleBar message handlers

BOOL CPlayerTitleBar::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void CPlayerTitleBar::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    for (CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetNextWindow()) {
        if (!pChild->IsWindowVisible()) {
            continue;
        }

        CRect r;
        pChild->GetClientRect(&r);
        pChild->MapWindowPoints(this, &r);
        dc.ExcludeClipRect(&r);
    }

    CRect r;
    GetClientRect(&r);

    if (m_pMainFrame->m_pLastTitlebar != this || m_pMainFrame->m_fFullScreen) {
        r.InflateRect(0, 1, 0, 0);
    }

    if (m_pMainFrame->m_fFullScreen) {
        r.InflateRect(1, 0, 1, 0);
    }

    dc.Draw3dRect(&r, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));

    r.DeflateRect(1, 1);

    dc.FillSolidRect(&r, 0);
}

void CPlayerTitleBar::OnSize(UINT nType, int cx, int cy)
{
    CDialogBar::OnSize(nType, cx, cy);

    Invalidate();
    Relayout();
}

void CPlayerTitleBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

    WINDOWPLACEMENT wp;
    wp.length = sizeof(wp);
    pFrame->GetWindowPlacement(&wp);

    if (!pFrame->m_fFullScreen && wp.showCmd != SW_SHOWMAXIMIZED) {
        CRect r;
        GetClientRect(r);
        CPoint p = point;

        MapWindowPoints(pFrame, &point, 1);

        pFrame->PostMessage(WM_NCLBUTTONDOWN,
                            (p.x >= r.Width() - r.Height() && !pFrame->IsCaptionHidden()) ? HTBOTTOMRIGHT :
                            HTCAPTION,
                            MAKELPARAM(point.x, point.y));
    }
}

BOOL CPlayerTitleBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    return CDialogBar::OnSetCursor(pWnd, nHitTest, message);
}

HBRUSH CPlayerTitleBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogBar::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  Return a different brush if the default is not desired
    return hbr;
}

BOOL CPlayerTitleBar::PreTranslateMessage(MSG* pMsg)
{

    return __super::PreTranslateMessage(pMsg);
}

