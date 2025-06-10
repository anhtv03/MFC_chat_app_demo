#pragma once
#include <afxwin.h>
#include <gdiplus.h>
#include <vector>
#include <future>
#include "Message.h" 

class ChatListStyle : public CWnd
{
public:
    ChatListStyle();
    virtual ~ChatListStyle();

    void SetMessages(std::vector<Message>* messages);
    void AddMessage(const Message& msg);
    void RecalculateTotalHeight();
    void ScrollToBottom();
    void UpdateScrollInfo();
    int GetClientRectHeight() const;
    void DrawMessage(Gdiplus::Graphics& g, const Message& msg, int& y, int width);
    void DrawCenterTime(Gdiplus::Graphics& g, const CString& timeStr, int& y, int width);
    int CalculateMessageHeight(Gdiplus::Graphics& g, const Message& msg, int width);
    void CheckDownloadFutures();

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

private:
    std::vector<Message>* m_messages;
    int m_totalHeight;
	int m_scrollOffset;
    CString m_lastTime;
    CScrollBar m_scrollBar;
    ULONG_PTR m_gdiplusToken;
    Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
    Gdiplus::Font* m_pMsgFont;
    Gdiplus::Font* m_pTimeFont;
    Gdiplus::Font* m_pTimeCenterFont;
    std::map<CString, std::pair<CString, bool>> m_imageCache; 
    std::vector<std::future<CString>> m_downloadFutures;

};