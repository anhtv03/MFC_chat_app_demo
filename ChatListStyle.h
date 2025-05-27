#pragma once
#include "Message.h"
#include <vector>
#include <gdiplus.h>

class ChatListStyle : public CWnd
{
    DECLARE_DYNAMIC(ChatListStyle)

public:
    ChatListStyle();
    virtual ~ChatListStyle();

    void SetMessages(std::vector<Message>* pMessages);
    void AddMessage(const Message& msg);
    void ScrollToBottom();

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

private:
    std::vector<Message>* m_messages;
    CBrush m_backgroundBrush;
    CFont m_messageFont;
    CFont m_timeFont;
    int m_totalHeight;
    int m_scrollPos;
    ULONG_PTR m_gdiplusToken;

    void DrawMessageBubble(CDC* pDC, CRect& rect, const Message& msg, bool isOwn);
    void DrawTimestamp(CDC* pDC, CRect& rect, const CString& time);
    int CalculateMessageHeight(CDC* pDC, const Message& msg, int width);
    void UpdateLayout();
};