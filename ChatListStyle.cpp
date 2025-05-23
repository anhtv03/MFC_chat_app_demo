#include "pch.h"
#include "ChatListStyle.h"
#include "Message.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

IMPLEMENT_DYNAMIC(ChatListStyle, CListCtrl)

ChatListStyle::ChatListStyle()
{
    m_messages = nullptr;
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
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

ChatListStyle::~ChatListStyle()
{
}

BEGIN_MESSAGE_MAP(ChatListStyle, CListCtrl)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void ChatListStyle::SetMessages(std::vector<Message>* pMessages)
{
    m_messages = pMessages;
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
    if (GetItemCount() > 0)
    {
		EnsureVisible(GetItemCount() - 1, FALSE);
    }
}

BOOL ChatListStyle::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);
    pDC->FillRect(&rect, &m_backgroundBrush);
    return TRUE;
}

void ChatListStyle::OnPaint()
{
    CPaintDC dc(this);

    if (!m_messages || m_messages->empty())
        return;

    CRect clientRect;
    GetClientRect(&clientRect);

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap memBitmap;
    memBitmap.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
    CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

    memDC.FillRect(&clientRect, &m_backgroundBrush);
    Graphics graphics(memDC.GetSafeHdc());

    int yPos = 0;
    CString currentDate = _T("");

    for (size_t i = 0; i < m_messages->size(); i++)
    {
        const Message& msg = (*m_messages)[i];

        if (i == 0 || msg.GetFormattedTime() != (*m_messages)[i - 1].GetFormattedTime())
        {
            CRect timeRect(0, yPos, clientRect.Width(), yPos + 20);
            DrawTimestamp(&memDC, timeRect, msg.GetFormattedTime());
            yPos += 30;
        }

        int msgHeight = CalculateMessageHeight(&memDC, msg, clientRect.Width() - 80);
        CRect msgRect;
        if (msg.GetMessageType() == 1)
        {
            msgRect.SetRect(clientRect.Width() - 300, yPos, clientRect.Width() - 20, yPos + msgHeight);
        }
        else
        {
            msgRect.SetRect(60, yPos, 340, yPos + msgHeight);
        }

        if (msg.GetMessageType() == 0)
        {
            CString localPath = _T("avatar.png");
            Image* avatar = nullptr;
            try {
                avatar = Image::FromFile(localPath);
                if (avatar) {
                    int avatarSize = 30;
                    int centerY = yPos + (msgHeight - avatarSize) / 2;
                    GraphicsPath path;
                    path.AddEllipse(20, centerY, avatarSize, avatarSize); 
                    graphics.SetClip(&path);
                    graphics.DrawImage(avatar, 20, centerY, avatarSize, avatarSize);
                    graphics.ResetClip();
                    delete avatar;
                }
            }
            catch (...) {
                CRect avatarRect(20, yPos, 50, yPos + 30);
                CBrush avatarBrush(RGB(150, 150, 150));
                memDC.FillRect(&avatarRect, &avatarBrush);
                memDC.Ellipse(&avatarRect);
            }
        }

        DrawMessageBubble(&memDC, msgRect, msg, msg.GetMessageType() == 1);
        yPos += msgHeight + 10;
    }

    dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(pOldBitmap);
    memDC.DeleteDC();
}

void ChatListStyle::DrawMessageBubble(CDC* pDC, CRect& rect, const Message& msg, bool isOwnMessage)
{
    COLORREF bubbleColor = isOwnMessage ? RGB(0, 132, 255) : RGB(230, 230, 230);
    COLORREF textColor = isOwnMessage ? RGB(255, 255, 255) : RGB(0, 0, 0);

    CBrush bubbleBrush(bubbleColor);
    CBrush* pOldBrush = pDC->SelectObject(&bubbleBrush);
    CPen bubblePen(PS_SOLID, 1, bubbleColor);
    CPen* pOldPen = pDC->SelectObject(&bubblePen);

    pDC->RoundRect(&rect, CPoint(15, 15));

    pDC->SetTextColor(textColor);
    pDC->SetBkMode(TRANSPARENT);
    CFont* pOldFont = pDC->SelectObject(&m_messageFont);

    CRect textRect = rect;
    textRect.DeflateRect(12, 8);

    //if (msg.GetMessageType() == MessageType::TEXT)
    //{
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

    pDC->SelectObject(pOldFont);
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
}

void ChatListStyle::DrawTimestamp(CDC* pDC, CRect& rect, const CString& time)
{
    pDC->SetTextColor(RGB(128, 128, 128));
    pDC->SetBkMode(TRANSPARENT);
    CFont* pOldFont = pDC->SelectObject(&m_timeFont);

    pDC->DrawText(time, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(pOldFont);
}

int ChatListStyle::CalculateMessageHeight(CDC* pDC, const Message& msg, int width)
{
    CFont* pOldFont = pDC->SelectObject(&m_messageFont);

    CRect textRect(0, 0, width - 24, 0); 
    pDC->DrawText(msg.GetContent(), &textRect, DT_CALCRECT | DT_WORDBREAK);

    int height = textRect.Height() + 16;  

    //if (msg.GetMessageType() == MessageType::IMAGE)
    //{
        //height = max(height, 116);  
    //}

    pDC->SelectObject(pOldFont);
    return max(height, 40);  
}

CRect ChatListStyle::GetTextRect(CDC* pDC, const CString& text, int maxWidth)
{
    CRect rect(0, 0, maxWidth, 0);
    pDC->DrawText(text, &rect, DT_CALCRECT | DT_WORDBREAK);
    return rect;
}