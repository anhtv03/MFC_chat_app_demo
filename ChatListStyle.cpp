#include "pch.h"
#include "ChatListStyle.h"
#include <gdiplus.h>
#define CURL_STATICLIB
#include <curl.h>

#pragma comment (lib,"Gdiplus.lib")

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
	const int bubbleWidthMax = width * 2 / 3;
	const int spacing = 6;
	const int bubblePadding = 12;
	const int avatarSize = 32;

	CString content = msg.GetContent();

	Gdiplus::RectF layoutRect(0, 0, (Gdiplus::REAL)(bubbleWidthMax - 2 * bubblePadding), 9999);
	Gdiplus::RectF boundingBox;
	Gdiplus::StringFormat format;
	format.SetTrimming(Gdiplus::StringTrimmingWord);
	g.MeasureString(content, -1, m_pMsgFont, layoutRect, &format, &boundingBox);

	int bubbleHeight = max(40, (int)boundingBox.Height + 2 * bubblePadding);
	if (msg.GetMessageType() == 0) {
		bubbleHeight = max(bubbleHeight, avatarSize);
	}

	return bubbleHeight + spacing;
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
	for (const Message& msg : *m_messages)
	{
		CString currentDate = msg.GetFormattedTime();
		if (currentDate != lastDate) {
			m_totalHeight += 40;
			lastDate = currentDate;
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

void ChatListStyle::UpdateScrollInfo()
{
	int clientHeight = GetClientRectHeight();

	SCROLLINFO si = { sizeof(SCROLLINFO), SIF_PAGE | SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL,
					 0, m_totalHeight, (UINT)clientHeight, m_scrollOffset };

	m_scrollBar.SetScrollInfo(&si);
	m_scrollBar.ShowWindow((m_totalHeight > clientHeight) ? SW_SHOW : SW_HIDE);
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

	int y = 15 - m_scrollOffset;
	int width = client.Width() - 16 - 20;

	if (m_messages)
	{
		CString lastDate = _T("");
		for (const Message& msg : *m_messages)
		{
			CString currentDate = msg.GetFormattedTime();
			if (currentDate != lastDate) {
				DrawCenterTime(graphics, currentDate, y, width);
				lastDate = currentDate;
			}
			DrawMessage(graphics, msg, y, width);
		}
	}

	dc.BitBlt(0, 0, client.Width(), client.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

void ChatListStyle::DrawCenterTime(Gdiplus::Graphics& g, const CString& timeStr, int& y, int width)
{
	const int timeHeight = 30;

	Gdiplus::SolidBrush brushTime(Gdiplus::Color(120, 120, 120));
	Gdiplus::StringFormat timeFormat;
	timeFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
	timeFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::RectF timeRect(10, (float)y, (float)width, 30);
	g.DrawString(timeStr, -1, m_pTimeCenterFont, timeRect, &timeFormat, &brushTime);
	y += 40;
}

CString DownloadFile(const CString& url, const CString& localPath)
{
	CURL* curl = curl_easy_init();
	FILE* fp = nullptr;
	_tfopen_s(&fp, localPath, _T("wb"));

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
	fclose(fp);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK) {
		_tremove(localPath);
		return _T("");
	}

	return localPath;
}

void ChatListStyle::DrawMessage(Gdiplus::Graphics& g, const Message& msg, int& y, int width)
{
	const int padding = 15;
	const int bubbleWidthMax = width * 2 / 3;
	const int spacing = 6;
	const int bubblePadding = 12;
	const int radius = 10;
	const int avatarSize = 32;
	const int avatarMargin = 8;
	const int imageSize = 120;
	const int fileIconSize = 32;

	CStringW content = msg.GetContent();
	std::vector<CString> files = msg.GetFiles();
	std::vector<CString> images = msg.GetImages();
	bool isMyMessage = (msg.GetMessageType() == 1);
	CString currentTime = msg.GetCreatedAt().Format(_T("%H:%M %d/%m/%Y"));

	Gdiplus::SolidBrush brushText(Gdiplus::Color(30, 30, 30));
	Gdiplus::SolidBrush brushBubbleSend(Gdiplus::Color(67, 127, 236));
	Gdiplus::SolidBrush brushBubbleReceive(Gdiplus::Color(218, 218, 218));
	Gdiplus::SolidBrush brushTextSend(Gdiplus::Color::White);
	Gdiplus::SolidBrush brushAvatar(Gdiplus::Color(100, 150, 200));

	Gdiplus::RectF layoutRect(0, 0, (Gdiplus::REAL)(bubbleWidthMax - 2 * bubblePadding), 9999);
	Gdiplus::RectF boundingBox;
	Gdiplus::StringFormat format;
	format.SetTrimming(Gdiplus::StringTrimmingWord);
	g.MeasureString(content, -1, m_pMsgFont, layoutRect, &format, &boundingBox);

	int bubbleWidth = min(bubbleWidthMax, max(80, (int)boundingBox.Width + 2 * bubblePadding + 10));
	int bubbleHeight = content.IsEmpty() ? 0 : max(40, (int)boundingBox.Height + 2 * bubblePadding);

	int x, avatarX = 0;

	if (isMyMessage) {
		x = width - bubbleWidth - padding + 10;
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

	int totalHeight = bubbleHeight;
	if (!images.empty()) totalHeight += (images.size() * (imageSize + spacing)) - spacing;
	if (!files.empty()) totalHeight += (files.size() * (fileIconSize + spacing)) - spacing;

	//=============draw content===========
	if (!content.IsEmpty()) {
		Gdiplus::Rect bubbleRect(x, y, bubbleWidth, bubbleHeight);
		if (Gdiplus::GraphicsPath* path = CreateRoundRectPath(bubbleRect, radius)) {
			g.FillPath(isMyMessage ? &brushBubbleSend : &brushBubbleReceive, path);
			delete path;
		}

		Gdiplus::RectF textRect(
			(Gdiplus::REAL)(x + bubblePadding),
			(Gdiplus::REAL)(y + bubblePadding),
			(Gdiplus::REAL)(bubbleWidth - 2 * bubblePadding),
			(Gdiplus::REAL)(bubbleHeight - 2 * bubblePadding)
		);
		format.SetAlignment(Gdiplus::StringAlignmentNear);
		format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		g.DrawString(content, -1, m_pMsgFont, textRect, &format, isMyMessage ? &brushTextSend : &brushText);
	}

	//=============draw image===========
	int currentY = y + bubbleHeight + (bubbleHeight > 0 ? spacing : 0);
	if (!images.empty()) {
		for (const auto& imagePath : images) {
			CString localPath = _T("temp_") + imagePath.Mid(imagePath.ReverseFind(_T('/')) + 1);
			CString downloadedPath = DownloadFile(imagePath, localPath);
			if (!downloadedPath.IsEmpty()) {
				Gdiplus::Bitmap bitmap(downloadedPath);
				if (bitmap.GetLastStatus() == Gdiplus::Ok) {
					Gdiplus::Rect imageRect(x + bubblePadding, currentY, imageSize, imageSize);
					if (Gdiplus::GraphicsPath* imagePath = CreateRoundRectPath(imageRect, 5)) {
						g.SetClip(imagePath);
						g.DrawImage(&bitmap, imageRect);
						g.ResetClip();
						delete imagePath;
					}
				}
				_tremove(localPath);
			}
			currentY += imageSize + spacing;
		}
	}

	m_lastTime = currentTime;
	y += totalHeight + spacing;
}

int ChatListStyle::GetClientRectHeight() const
{
	CRect rc;
	GetClientRect(&rc);
	return rc.Height();
}