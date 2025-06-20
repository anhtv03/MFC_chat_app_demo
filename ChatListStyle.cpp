﻿#include "pch.h"
#include "ChatListStyle.h"
#include <gdiplus.h>
#include <filesystem>
#define CURL_STATICLIB
#include <curl.h>

#pragma comment (lib,"Gdiplus.lib")
namespace fs = std::filesystem;

Gdiplus::GraphicsPath* CreateRoundRectPath(Gdiplus::Rect rect, int radius)
{
    auto path = new Gdiplus::GraphicsPath();
    int diameter = radius * 2;
    if (diameter <= 0) {
        path->AddRectangle(rect);
        return path;
    }
    if (diameter > rect.Width) diameter = rect.Width;
    if (diameter > rect.Height) diameter = rect.Height;

    Gdiplus::Rect arcRectTL(rect.X, rect.Y, diameter, diameter);
    Gdiplus::Rect arcRectTR(rect.GetRight() - diameter, rect.Y, diameter, diameter);
    Gdiplus::Rect arcRectBR(rect.GetRight() - diameter, rect.GetBottom() - diameter, diameter, diameter);
    Gdiplus::Rect arcRectBL(rect.X, rect.GetBottom() - diameter, diameter, diameter);

    path->AddArc(arcRectTL, 180, 90);
    path->AddArc(arcRectTR, 270, 90);
    path->AddArc(arcRectBR, 0, 90);
    path->AddArc(arcRectBL, 90, 90);

    path->CloseFigure();
    return path;
}

BEGIN_MESSAGE_MAP(ChatListStyle, CWnd)
    ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_VSCROLL()
    ON_WM_MOUSEWHEEL()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

ChatListStyle::ChatListStyle()
    : m_messages(nullptr), m_totalHeight(0), m_scrollOffset(0),
    m_pMsgFont(nullptr), m_pTimeFont(nullptr), m_pTimeCenterFont(nullptr),
    m_lastTime(_T(""))
{
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, nullptr);
    m_pMsgFont = new Gdiplus::Font(L"Segoe UI Emoji", 10.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    m_pTimeFont = new Gdiplus::Font(L"Segoe UI", 8.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    m_pTimeCenterFont = new Gdiplus::Font(L"Segoe UI", 9.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
}

ChatListStyle::~ChatListStyle()
{
    delete m_pMsgFont;
    delete m_pTimeFont;
    delete m_pTimeCenterFont;
    Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

int ChatListStyle::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1) return -1;
    m_scrollBar.Create(SBS_VERT | WS_CHILD, CRect(0, 0, 0, 0), this, 1);
    return 0;
}

void ChatListStyle::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    CRect rc;
    GetClientRect(&rc);
    m_scrollBar.MoveWindow(rc.right - 16, rc.top, 16, rc.Height());
    RecalculateTotalHeight();
}

BOOL ChatListStyle::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    int clientHeight = GetClientRectHeight();
    int maxScroll = max(0, m_totalHeight - clientHeight);
    m_scrollOffset = max(0, min(maxScroll, m_scrollOffset - zDelta / 4));
    UpdateScrollInfo();
    Invalidate();
    return TRUE;
}

void ChatListStyle::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int oldOffset = m_scrollOffset;
    int clientHeight = GetClientRectHeight();
    int maxScroll = m_totalHeight - clientHeight;
    if (maxScroll < 0) maxScroll = 0;

    switch (nSBCode)
    {
    case SB_LINEUP: m_scrollOffset -= 20; break;
    case SB_LINEDOWN: m_scrollOffset += 20; break;
    case SB_PAGEUP: m_scrollOffset -= clientHeight; break;
    case SB_PAGEDOWN: m_scrollOffset += clientHeight; break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK: m_scrollOffset = nPos; break;
    case SB_TOP: m_scrollOffset = 0; break;
    case SB_BOTTOM: m_scrollOffset = maxScroll; break;
    }

    m_scrollOffset = max(0, min(maxScroll, m_scrollOffset));
    if (oldOffset != m_scrollOffset)
    {
        UpdateScrollInfo();
        Invalidate();
    }
}

BOOL ChatListStyle::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void ChatListStyle::OnPaint()
{
    CPaintDC dc(this);
    CRect client;
    GetClientRect(&client);

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap memBmp;
    memBmp.CreateCompatibleBitmap(&dc, client.Width(), client.Height());
    CBitmap* pOldBmp = memDC.SelectObject(&memBmp);

    Gdiplus::Graphics graphics(memDC.GetSafeHdc());
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    graphics.Clear(Gdiplus::Color(250, 250, 250));

    int width = client.Width() - 16 - 20;
    int clientHeight = client.Height();
    int y = -m_scrollOffset + 15;

    if (m_messages)
    {
        m_fileClickRects.clear();
        m_fileIds.clear();

        int lastMyMessageIndex = -1;
        for (int i = m_messages->size() - 1; i >= 0; i--) {
            if ((*m_messages)[i].GetMessageType() == 1) {
                lastMyMessageIndex = i;
                break;
            }
        }

        CString lastDate = _T("");
        for (int i = 0; i < m_messages->size(); i++)
        {
            const Message& msg = (*m_messages)[i];
            CString currentDate = msg.GetFormattedTime();
            bool isLastMyMessage = (i == lastMyMessageIndex);
            bool isFirstMessage = (i == 0);

            if (currentDate != lastDate) {
                if (!isFirstMessage) {
                    y += 6;
                }
                DrawCenterTime(graphics, currentDate, y, width);
                y += 40;
                lastDate = currentDate;
            }

            if (!isFirstMessage && currentDate == lastDate) {
                y += 6;
            }

            int msgHeight = CalculateMessageHeight(graphics, msg, width);
            DrawMessage(graphics, msg, y, width, isLastMyMessage);
            y += msgHeight;
        }
    }

    dc.BitBlt(0, 0, client.Width(), client.Height(), &memDC, 0, 0, SRCCOPY);
    memDC.SelectObject(pOldBmp);
}

void ChatListStyle::OnLButtonDown(UINT nFlags, CPoint point)
{
    for (size_t i = 0; i < m_fileClickRects.size(); ++i)
    {
        if (m_fileClickRects[i].Contains(Gdiplus::Point(point.x, point.y)))
        {
            CString fileId = m_fileIds[i];
            if (m_messages && !fileId.IsEmpty())
            {
                for (const Message& msg : *m_messages)
                {
                    for (const FileInfo& file : msg.GetFiles())
                    {
                        if (file.id == fileId)
                        {
                            CString tempPath = DownloadFile(file.url, file.fileName);
                            if (!tempPath.IsEmpty())
                            {
                                CFileDialog fileDialog(FALSE, file.fileName, file.fileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, _T("All Files (*.*)|*.*||"), nullptr);
                                if (fileDialog.DoModal() == IDOK)
                                {
                                    CString savePath = fileDialog.GetPathName();
                                    if (CopyFile(tempPath, savePath, FALSE))
                                    {
                                        _tremove(tempPath);
                                    }
                                }
                                else
                                {
                                    _tremove(tempPath);
                                }
                            }
                            return;
                        }
                    }
                    for (const FileInfo& image : msg.GetImages())
                    {
                        if (image.id == fileId)
                        {
                            CString tempPath = DownloadFile(image.url, image.fileName);
                            if (!tempPath.IsEmpty())
                            {
                                CFileDialog fileDialog(FALSE, image.fileName, image.fileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, _T("All Files (*.*)|*.*||"), nullptr);
                                if (fileDialog.DoModal() == IDOK)
                                {
                                    CString savePath = fileDialog.GetPathName();
                                    if (CopyFile(tempPath, savePath, FALSE))
                                    {
                                        _tremove(tempPath);
                                    }
                                }
                                else
                                {
                                    _tremove(tempPath);
                                }
                            }
                            return;
                        }
                    }
                }
            }
            break;
        }
    }
}

void ChatListStyle::SetMessages(std::vector<Message>* messages)
{
    m_messages = messages;
    m_scrollOffset = 0;
    m_lastTime = _T("");
    RecalculateTotalHeight();
}

void ChatListStyle::AddMessage(const Message& msg)
{
    if (m_messages)
    {
        m_messages->push_back(msg);
        RecalculateTotalHeight();
        ScrollToBottom();
    }
}

int ChatListStyle::CalculateMessageHeight(Gdiplus::Graphics& g, const Message& msg, int width)
{
    const int bubbleWidthMax = width * 3 / 4;
    const int spacing = 6;
    const int bubblePadding = 12;
    const int avatarSize = 32;
    const int imageSize = 150;
    const int fileHeight = 32;

    CString content = msg.GetContent();
    int totalMessageHeight = 0;
    std::vector<FileInfo> images = msg.GetImages();
    std::vector<FileInfo> files = msg.GetFiles();

    if (!content.IsEmpty()) {
        Gdiplus::RectF layoutRect(0, 0, (Gdiplus::REAL)(bubbleWidthMax - 2 * bubblePadding), 9999);
        Gdiplus::RectF boundingBox;
        Gdiplus::StringFormat format;
        format.SetTrimming(Gdiplus::StringTrimmingWord);
        g.MeasureString(content, -1, m_pMsgFont, layoutRect, &format, &boundingBox);
        totalMessageHeight = max(40, (int)boundingBox.Height + 2 * bubblePadding);
    }

    if (msg.GetMessageType() == 0) {
        totalMessageHeight = max(totalMessageHeight, avatarSize);
    }

    if (!images.empty()) {
        if (totalMessageHeight > 0) {
            totalMessageHeight += spacing;
        }
        totalMessageHeight += (images.size() * imageSize) + ((images.size() > 0) ? (images.size() - 1) * spacing : 0);
    }

    if (!files.empty()) {
        if (totalMessageHeight > 0) {
            totalMessageHeight += spacing;
        }
        totalMessageHeight += (files.size() * (fileHeight)) + ((files.size() > 0) ? (files.size() - 1) * spacing : 0);
    }

    return totalMessageHeight;
}

void ChatListStyle::RecalculateTotalHeight()
{
    if (!m_messages || !GetSafeHwnd()) return;

    CClientDC dc(this);
    Gdiplus::Graphics graphics(dc.GetSafeHdc());
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    CRect client;
    GetClientRect(&client);
    int width = client.Width() - 16 - 20;

    m_totalHeight = 15;

    CString lastDate = _T("");
    for (int i = 0; i < m_messages->size(); i++)
    {
        const Message& msg = (*m_messages)[i];
        CString currentDate = msg.GetFormattedTime();
        bool isFirstMessage = (i == 0);

        if (currentDate != lastDate) {
            if (!isFirstMessage) {
                m_totalHeight += 6; 
            }
            m_totalHeight += 40;
            lastDate = currentDate;
        }

        if (!isFirstMessage && currentDate == lastDate) {
            m_totalHeight += 6;
        }

        m_totalHeight += CalculateMessageHeight(graphics, msg, width);
    }

    m_totalHeight += 15;

    UpdateScrollInfo();
    Invalidate();
}

void ChatListStyle::ScrollToBottom()
{
    int clientHeight = GetClientRectHeight();
    m_scrollOffset = (m_totalHeight > clientHeight) ? m_totalHeight - clientHeight : 0;
    UpdateScrollInfo();
    Invalidate();
}

void ChatListStyle::UpdateScrollInfo()
{
    int clientHeight = GetClientRectHeight();
    SCROLLINFO si = { sizeof(SCROLLINFO), SIF_PAGE | SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL,
                     0, m_totalHeight, (UINT)clientHeight, m_scrollOffset };
    m_scrollBar.SetScrollInfo(&si);
    m_scrollBar.ShowWindow((m_totalHeight > clientHeight) ? SW_SHOW : SW_HIDE);
}

void ChatListStyle::DrawCenterTime(Gdiplus::Graphics& g, const CString& timeStr, int y, int width)
{
    const int timeHeight = 30;
    Gdiplus::SolidBrush brushTime(Gdiplus::Color(120, 120, 120));
    Gdiplus::StringFormat timeFormat;
    timeFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
    timeFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    Gdiplus::RectF timeRect(10, (float)y, (float)width, 30);
    g.DrawString(timeStr, -1, m_pTimeCenterFont, timeRect, &timeFormat, &brushTime);

}

CString ChatListStyle::DownloadFile(const CString& url, const CString& localPath)
{
    TCHAR tempPath[MAX_PATH];
    if (GetTempPath(MAX_PATH, tempPath) == 0) {
        return _T("");
    }

    CString fileName = localPath;
    int lastSlash = fileName.ReverseFind(_T('\\'));
    if (lastSlash >= 0) {
        fileName = fileName.Mid(lastSlash + 1);
    }
    lastSlash = fileName.ReverseFind(_T('/'));
    if (lastSlash >= 0) {
        fileName = fileName.Mid(lastSlash + 1);
    }
    fileName.Trim(_T(" \t\r\n"));

    CString assetLocalPath = CString(tempPath) + fileName;
    std::wstring wAssetLocalPath = assetLocalPath.GetString();
    std::filesystem::path fullPath(wAssetLocalPath);
    std::filesystem::path assetDir = fullPath.parent_path();
    if (!std::filesystem::exists(assetDir)) {
        std::filesystem::create_directory(assetDir);
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        return _T("");
    }

    FILE* fp = nullptr;
    errno_t err = _tfopen_s(&fp, assetLocalPath, _T("wb"));
    if (err != 0 || fp == nullptr) {
        curl_easy_cleanup(curl);
        return _T("");
    }

    CString cleanUrl = url;
    cleanUrl.Trim();
    cleanUrl.Replace(_T("\r"), _T(""));
    cleanUrl.Replace(_T("\n"), _T(""));

    CT2A urlAnsi(cleanUrl);
    const char* urlCStr = urlAnsi;

    curl_easy_setopt(curl, CURLOPT_URL, urlCStr);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        _tremove(assetLocalPath);
        fclose(fp);
        curl_easy_cleanup(curl);
        return _T("");
    }

    fclose(fp);
    curl_easy_cleanup(curl);
    return assetLocalPath;
}

void ChatListStyle::DrawMessage(Gdiplus::Graphics& g, const Message& msg, int y, int width, bool isLastMyMessage)
{
    const int padding = 15;
    const int bubbleWidthMax = width * 3 / 4;
    const int spacing = 6;
    const int bubblePadding = 12;
    const int radius = 10;
    const int avatarSize = 32;
    const int avatarMargin = 8;
    const int imageSize = 150;
    const int fileIconSize = 16;
    const int fileHeight = 32;
    const int filePadding = 8;
    const int statusIconSize = 12;
    const int statusIconMargin = 5;

    CStringW content = msg.GetContent();
    std::vector<FileInfo> files = msg.GetFiles();
    std::vector<FileInfo> images = msg.GetImages();
    bool isMyMessage = (msg.GetMessageType() == 1);
    int isSend = msg.GetIsSend();
    CString currentTime = msg.GetCreatedAt().Format(_T("%H:%M %d/%m/%Y"));

    Gdiplus::SolidBrush brushText(Gdiplus::Color(30, 30, 30));
    Gdiplus::SolidBrush brushBubbleSend(Gdiplus::Color(67, 127, 236));
    Gdiplus::SolidBrush brushBubbleReceive(Gdiplus::Color(218, 218, 218));
    Gdiplus::SolidBrush brushTextSend(Gdiplus::Color::White);
    Gdiplus::SolidBrush brushAvatar(Gdiplus::Color(100, 150, 200));
    Gdiplus::SolidBrush brushFileBubble(Gdiplus::Color(200, 200, 200));
    Gdiplus::Pen penBorder(isMyMessage ? Gdiplus::Color(50, 100, 200) : Gdiplus::Color(150, 150, 150), 1.0f);

    Gdiplus::RectF layoutRect(0, 0, (Gdiplus::REAL)(bubbleWidthMax - 2 * bubblePadding), 9999);
    Gdiplus::RectF boundingBox;
    Gdiplus::StringFormat format;
    format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);

    g.MeasureString(content, -1, m_pMsgFont, layoutRect, &format, &boundingBox);
    int contentWidth = (int)boundingBox.Width + 2 * bubblePadding + 10;
    int bubbleWidth = min(bubbleWidthMax, max(80, contentWidth));
    int bubbleHeight = content.IsEmpty() ? 0 : max(40, (int)boundingBox.Height + 2 * bubblePadding);
    int x, avatarX = 0;

    if (isMyMessage) {
        x = width - padding - bubbleWidth;
    }
    else {
        bool isDrawAvatar = m_lastTime.IsEmpty() || (currentTime != m_lastTime);
        avatarX = padding;
        x = padding + avatarSize + avatarMargin;

        if (isDrawAvatar) {
            Gdiplus::Rect avatarRect(avatarX, y, avatarSize, avatarSize);
            if (Gdiplus::GraphicsPath* avatarPath = CreateRoundRectPath(avatarRect, avatarSize / 2)) {
                g.FillPath(&brushAvatar, avatarPath);
                delete avatarPath;
            }

            Gdiplus::SolidBrush brushAvatarText(Gdiplus::Color::White);
            Gdiplus::StringFormat avatarFormat;
            avatarFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
            avatarFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);

            Gdiplus::RectF avatarTextRect(avatarX, y, avatarSize, avatarSize);
            g.DrawString(L"M", -1, m_pMsgFont, avatarTextRect, &avatarFormat, &brushAvatarText);
        }
        bubbleHeight = content.IsEmpty() ? 0 : max(bubbleHeight, avatarSize);
    }

    int currentY = y;
    int rightmostX = x + bubbleWidth;

    if (!content.IsEmpty()) {
        Gdiplus::Rect bubbleRect(x, currentY, bubbleWidth, bubbleHeight);
        if (Gdiplus::GraphicsPath* path = CreateRoundRectPath(bubbleRect, radius)) {
            g.FillPath(isMyMessage ? &brushBubbleSend : &brushBubbleReceive, path);
            delete path;
        }

        Gdiplus::RectF textRect(
            (Gdiplus::REAL)(x + bubblePadding),
            (Gdiplus::REAL)(currentY + bubblePadding),
            (Gdiplus::REAL)(bubbleWidth - 2 * bubblePadding),
            (Gdiplus::REAL)(bubbleHeight - 2 * bubblePadding)
        );
        format.SetAlignment(Gdiplus::StringAlignmentNear);
        format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        g.DrawString(content, -1, m_pMsgFont, textRect, &format, isMyMessage ? &brushTextSend : &brushText);

        currentY += bubbleHeight;
    }

    if (!images.empty()) {
        if (currentY > y) {
            currentY += spacing;
        }

        int imageX = isMyMessage ? x - (imageSize / 2) : x;
        rightmostX = max(rightmostX, imageX + bubblePadding + imageSize);

        for (const auto& image : images) {
            CString localPath = _T("temp_") + image.id + _T("_") + image.fileName;
            TCHAR tempPath[MAX_PATH];
            GetTempPath(MAX_PATH, tempPath);
            CString fullPath = CString(tempPath) + localPath;
            fs::path fsPath(fullPath.GetString());

            if (!fs::exists(fsPath)) {
                CString downloadedPath = DownloadFile(image.url, localPath);
                if (downloadedPath.IsEmpty()) {
                    currentY += imageSize + spacing;
                    continue;
                }
            }

            Gdiplus::Bitmap bitmap(fullPath);
            if (bitmap.GetLastStatus() == Gdiplus::Ok) {
                Gdiplus::Rect imageRect(imageX + bubblePadding, currentY, imageSize, imageSize);
                if (Gdiplus::GraphicsPath* imagePath = CreateRoundRectPath(imageRect, 5)) {
                    g.SetClip(imagePath);
                    g.DrawImage(&bitmap, imageRect);
                    g.ResetClip();
                    delete imagePath;

                    Gdiplus::Rect clickRect(imageX, currentY, imageSize, imageSize);
                    m_fileClickRects.push_back(clickRect);
                    m_fileIds.push_back(image.id);
                }
            }
            currentY += imageSize + spacing;
        }

        if (!images.empty()) {
            currentY -= spacing;
        }
    }

    if (!files.empty()) {
        if (currentY > y) {
            currentY += spacing;
        }

        for (const auto& file : files) {
            int fileWidth = min(bubbleWidthMax, bubbleWidth);
            int fileX = isMyMessage ? x - fileWidth : x;
            rightmostX = max(rightmostX, fileX + fileWidth * 2);

            Gdiplus::Rect fileBubbleRect(fileX, currentY, fileWidth * 2, fileHeight);
            if (Gdiplus::GraphicsPath* filePath = CreateRoundRectPath(fileBubbleRect, radius)) {
                g.FillPath(&brushFileBubble, filePath);
                g.DrawPath(&penBorder, filePath);
                delete filePath;
            }

            Gdiplus::SolidBrush iconBrush(Gdiplus::Color(100, 100, 100));
            Gdiplus::RectF iconRect(
                (Gdiplus::REAL)(fileX + filePadding),
                (Gdiplus::REAL)(currentY + (fileHeight - fileIconSize) / 2),
                (Gdiplus::REAL)fileIconSize,
                (Gdiplus::REAL)fileIconSize
            );
            g.DrawString(L"↓", -1, m_pMsgFont, iconRect, &format, &iconBrush);

            Gdiplus::RectF fileTextRect(
                (Gdiplus::REAL)(fileX + filePadding + fileIconSize + 5),
                (Gdiplus::REAL)(currentY + (fileHeight - 16) / 2),
                (Gdiplus::REAL)(fileWidth * 2 - 2 * filePadding - fileIconSize - 5),
                (Gdiplus::REAL)(fileHeight - 2 * filePadding)
            );
            g.DrawString(file.fileName, -1, m_pMsgFont, fileTextRect, &format, isMyMessage ? &brushTextSend : &brushText);

            Gdiplus::Rect clickRect(fileX, currentY, fileWidth * 2, fileHeight);
            m_fileClickRects.push_back(clickRect);
            m_fileIds.push_back(file.id);

            currentY += fileHeight + spacing;
        }

        if (!files.empty()) {
            currentY -= spacing;
        }
    }

    if (isMyMessage && isLastMyMessage && (!content.IsEmpty() || !images.empty() || !files.empty())) {
        int totalHeight = currentY - y;
        int statusIconX = rightmostX + statusIconMargin;
        int statusIconY = y + totalHeight - statusIconSize - 3;
        DrawStatusIcon(g, isSend, statusIconX, statusIconY);
    }

    m_lastTime = currentTime;
}

void ChatListStyle::DrawStatusIcon(Gdiplus::Graphics& g, int status, int x, int y) {
    if (status == 0) {
        Gdiplus::Pen pen(Gdiplus::Color(150, 150, 150), 1.5f);
        Gdiplus::Point checkMark[3] = {
            {x, y + 3},
            {x + 3, y + 6},
            {x + 8, y}
        };
        g.DrawLines(&pen, checkMark, 3);
    }
    else if (status == 1) {
        Gdiplus::Pen pen(Gdiplus::Color(67, 127, 236), 1.5f);
        Gdiplus::Point checkMark1[3] = {
            {x, y + 3},
            {x + 3, y + 6},
            {x + 8, y}
        };
        g.DrawLines(&pen, checkMark1, 3);

        Gdiplus::Point checkMark2[3] = {
            {x + 4, y + 3},
            {x + 7, y + 6},
            {x + 12, y}
        };
        g.DrawLines(&pen, checkMark2, 3);
    }
}

int ChatListStyle::GetClientRectHeight() const
{
    CRect rc;
    GetClientRect(&rc);
    return rc.Height();
}