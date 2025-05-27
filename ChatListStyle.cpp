#include "pch.h"
#include "ChatListStyle.h"
#include "Message.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

IMPLEMENT_DYNAMIC(ChatListStyle, CWnd)

ChatListStyle::ChatListStyle()
{
    m_messageFont.CreateFont(20, 0, 0, 0, 
        FW_NORMAL, FALSE, FALSE, 0, 
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
        DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

    m_timeFont.CreateFont(12, 0, 0, 0, 
        FW_NORMAL, FALSE, FALSE, 0, 
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
        DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

    m_backgroundBrush.CreateSolidBrush(RGB(240, 240, 240));
    GdiplusStartupInput gdiInput;
    GdiplusStartup(&m_gdiplusToken, &gdiInput, nullptr);
}

ChatListStyle::~ChatListStyle()
{
    GdiplusShutdown(m_gdiplusToken);
}

BEGIN_MESSAGE_MAP(ChatListStyle, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_VSCROLL()
    ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

void ChatListStyle::SetMessages(std::vector<Message>* messages)
{
    m_messages = messages;
    UpdateLayout();
}

void ChatListStyle::AddMessage(const Message& msg)
{
    if (m_messages)
    {
        m_messages->push_back(msg);
        Invalidate();
        ScrollToBottom();
    }
}

void ChatListStyle::ScrollToBottom()
{
    CRect rc;
    GetClientRect(&rc);
    if (m_totalHeight > rc.Height())
    {
        m_scrollPos = m_totalHeight - rc.Height();
        SetScrollPos(SB_VERT, m_scrollPos);
        Invalidate();
    }
}

void ChatListStyle::UpdateLayout()
{
    if (!m_messages || m_messages->empty())
    {
        m_totalHeight = 0;
        ShowScrollBar(SB_VERT, FALSE);
        return;
    }

    CClientDC dc(this);
    CRect rc;
    GetClientRect(&rc);

    int yPos = 10;
    for (size_t i = 0; i < m_messages->size(); i++)
    {
        const Message& msg = (*m_messages)[i];
        if (i == 0 || msg.GetFormattedTime() != (*m_messages)[i - 1].GetFormattedTime())
            yPos += 30;
        yPos += CalculateMessageHeight(&dc, msg, rc.Width() - 80) + 10;
    }
    m_totalHeight = yPos + 10;

    SCROLLINFO si = { sizeof(si), SIF_RANGE | SIF_PAGE | SIF_POS, 0, m_totalHeight - 1, rc.Height(), m_scrollPos };
    m_scrollPos = max(0, min(m_scrollPos, m_totalHeight - rc.Height()));
    si.nPos = m_scrollPos;
    ShowScrollBar(SB_VERT, m_totalHeight > rc.Height());
    SetScrollInfo(SB_VERT, &si, TRUE);
    Invalidate();
}

BOOL ChatListStyle::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void ChatListStyle::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CRect rc;
    GetClientRect(&rc);
    int nCurPos = m_scrollPos;

    switch (nSBCode)
    {
    case SB_LINEUP: nCurPos -= 20; break;
    case SB_LINEDOWN: nCurPos += 20; break;
    case SB_PAGEUP: nCurPos -= rc.Height(); break;
    case SB_PAGEDOWN: nCurPos += rc.Height(); break;
    case SB_THUMBTRACK: case SB_THUMBPOSITION: nCurPos = nPos; break;
    }

    nCurPos = max(0, min(nCurPos, max(0, m_totalHeight - rc.Height())));
    if (nCurPos != m_scrollPos)
    {
        m_scrollPos = nCurPos;
        SetScrollPos(SB_VERT, m_scrollPos);
        Invalidate();
    }
    CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL ChatListStyle::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    CRect rc;
    GetClientRect(&rc);
    if (m_totalHeight <= rc.Height()) return TRUE;

    int newPos = m_scrollPos - (zDelta / 120) * 40;
    newPos = max(0, min(newPos, m_totalHeight - rc.Height()));
    if (newPos != m_scrollPos)
    {
        m_scrollPos = newPos;
        SetScrollPos(SB_VERT, m_scrollPos);
        UpdateLayout();
    }
    return TRUE;
}

void ChatListStyle::OnPaint()
{
    CPaintDC dc(this);
    CRect rc;
    GetClientRect(&rc);

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap memBitmap;
    memBitmap.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
    auto* pOldBitmap = memDC.SelectObject(&memBitmap);
    memDC.FillRect(&rc, &m_backgroundBrush);

    if (!m_messages || m_messages->empty())
    {
        dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
        memDC.SelectObject(pOldBitmap);
        return;
    }

    Graphics g(memDC.GetSafeHdc());
    int yPos = 10 - m_scrollPos;

    for (size_t i = 0; i < m_messages->size() && yPos <= rc.bottom + 100; i++)
    {
        const Message& msg = (*m_messages)[i];
        int msgHeight = CalculateMessageHeight(&memDC, msg, rc.Width() - 80);

        if (yPos >= -100)
        {
            if (i == 0 || msg.GetFormattedTime() != (*m_messages)[i - 1].GetFormattedTime())
            {
                if (yPos >= -30 && yPos <= rc.bottom + 30)
                {
                    CRect timeRect(0, yPos, rc.Width(), yPos + 20);
                    DrawTimestamp(&memDC, timeRect, msg.GetFormattedTime());
                }
                yPos += 30;
            }

            if (yPos >= -msgHeight && yPos <= rc.bottom + msgHeight)
            {
                CRect msgRect(msg.GetMessageType() == 1 ? rc.Width() - 300 : 60, yPos,
                    msg.GetMessageType() == 1 ? rc.Width() - 20 : 340, yPos + msgHeight);

                if (msg.GetMessageType() == 0)
                {
                    CString localPath = _T("avatar.png");
                    if (Image* avatar = Image::FromFile(localPath))
                    {
                        int avatarSize = 30, centerY = yPos + (msgHeight - avatarSize) / 2;
                        GraphicsPath path;
                        path.AddEllipse(20, centerY, avatarSize, avatarSize);
                        g.SetClip(&path);
                        g.DrawImage(avatar, 20, centerY, avatarSize, avatarSize);
                        g.ResetClip();
                        delete avatar;
                    }
                    else
                    {
                        CRect avatarRect(20, yPos, 50, yPos + 30);
                        CBrush avatarBrush(RGB(150, 150, 150));
                        memDC.FillRect(&avatarRect, &avatarBrush);
                        memDC.Ellipse(&avatarRect);
                    }
                }
                DrawMessageBubble(&memDC, msgRect, msg, msg.GetMessageType() == 1);
            }
        }
        yPos += msgHeight + 10;
    }

    dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
    memDC.SelectObject(pOldBitmap);
}


void ChatListStyle::DrawMessageBubble(CDC* pDC, CRect& rect, const Message& msg, bool isOwnMessage)
{
    COLORREF bubbleColor = isOwnMessage ? RGB(0, 132, 255) : RGB(230, 230, 230);
    COLORREF textColor = isOwnMessage ? RGB(255, 255, 255) : RGB(0, 0, 0);

    CBrush bubbleBrush(bubbleColor);
    CPen bubblePen(PS_SOLID, 1, bubbleColor);
    pDC->SelectObject(&bubbleBrush);
    pDC->SelectObject(&bubblePen);

    CRect textRect(0, 0, rect.Width() - 24, 0);
    pDC->SelectObject(&m_messageFont);
    pDC->DrawText(msg.GetContent(), &textRect, DT_CALCRECT | DT_WORDBREAK | DT_LEFT);

    rect.bottom = rect.top + textRect.Height() + 16;
    pDC->RoundRect(&rect, CPoint(15, 15));

    pDC->SetTextColor(textColor);
    pDC->SetBkMode(TRANSPARENT);
    textRect = rect;
    textRect.DeflateRect(12, 8);

    /*CString content = msg.GetContent().Trim();
    if (!content.IsEmpty())
    {*/
        pDC->DrawText(msg.GetContent(), &textRect, DT_WORDBREAK | DT_LEFT);
    //}
    //else if (msg.GetMessageType() == MessageType::IMAGE)
    //{
        //CRect imgRect = textRect;
        //imgRect.bottom = imgRect.top + 100;
        //CBrush imgBrush(RGB(200, 200, 200));
        //pDC->FillRect(&imgRect, &imgBrush);
        //pDC->DrawText(_T("🖼️ Hình ảnh"), &imgRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    //}

    //if (isOwnMessage && msg.IsDelivered())
    //{
        //CRect statusRect(rect.right - 25, rect.bottom - 20, rect.right - 5, rect.bottom - 5);
        //pDC->SetTextColor(RGB(0, 200, 0));
        //pDC->DrawText(_T("✓"), &statusRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    //}
}

void ChatListStyle::DrawTimestamp(CDC* pDC, CRect& rect, const CString& time)
{
    pDC->SetTextColor(RGB(128, 128, 128));
    pDC->SetBkMode(TRANSPARENT);
    pDC->SelectObject(&m_timeFont);
    pDC->DrawText(time, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

int ChatListStyle::CalculateMessageHeight(CDC* pDC, const Message& msg, int width)
{
    pDC->SelectObject(&m_messageFont);
    CRect textRect(0, 0, width - 24, 0);
    pDC->DrawText(msg.GetContent(), &textRect, DT_CALCRECT | DT_WORDBREAK | DT_LEFT);
    return max(textRect.Height() + 16, 40);
}