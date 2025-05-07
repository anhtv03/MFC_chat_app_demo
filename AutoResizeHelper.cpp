#include "pch.h"
#include "AutoResizeHelper.h"

void CAutoResizeHelper::Init(CDialog* pDlg)
{
    m_pDlg = pDlg;
    m_pDlg->GetClientRect(&m_origDlg);
}

void CAutoResizeHelper::Save(CWnd* pCtrl)
{
    ControlInfo info;
    info.pWnd = pCtrl;
    CRect rect;
    pCtrl->GetWindowRect(&rect);
    m_pDlg->ScreenToClient(&rect);
    info.origRect = rect;
    m_controls.push_back(info);
}

void CAutoResizeHelper::Resize(int cx, int cy)
{
    for (auto& ctrl : m_controls)
    {
        CRect newRect;
        newRect.left = ctrl.origRect.left * cx / m_origDlg.Width();
        newRect.top = ctrl.origRect.top * cy / m_origDlg.Height();
        newRect.right = ctrl.origRect.right * cx / m_origDlg.Width();
        newRect.bottom = ctrl.origRect.bottom * cy / m_origDlg.Height();
        ctrl.pWnd->MoveWindow(&newRect);
    }
}
