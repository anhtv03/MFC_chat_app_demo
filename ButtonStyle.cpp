#include "pch.h"
#include "ButtonStyle.h"

void ButtonStyle::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    CRect rect = lpDrawItemStruct->rcItem;
    UINT state = lpDrawItemStruct->itemState;

    pDC->FillSolidRect(&rect, RGB(28, 127, 217));

    CString text;
    GetWindowText(text);
    pDC->SetTextColor(RGB(255, 255, 255));
    pDC->SetBkMode(TRANSPARENT);
    pDC->DrawText(text, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}