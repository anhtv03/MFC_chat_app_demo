#pragma once
#include "Message.h"
#include <vector>

class ChatListStyle : public CListCtrl
{
    DECLARE_DYNAMIC(ChatListStyle)

public:
    ChatListStyle();
    virtual ~ChatListStyle();

    void SetMessages(std::vector<Message>* pMessages);
    void AddMessage(const Message& msg);
    void ScrollToBottom();
    int CalculateTotalHeight();

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
    std::vector<Message>* m_messages;
    CBrush m_backgroundBrush;
    CFont m_messageFont;
    CFont m_timeFont;

    void DrawMessageBubble(CDC* pDC, CRect& rect, const Message& msg, bool isOwnMessage);
    void DrawTimestamp(CDC* pDC, CRect& rect, const CString& time);
    int CalculateMessageHeight(CDC* pDC, const Message& msg, int width);
    CRect GetTextRect(CDC* pDC, const CString& text, int maxWidth);
};