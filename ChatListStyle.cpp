#include "pch.h"
#include "ChatListStyle.h"
#include <gdiplus.h>
#include <filesystem>
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
				}
			}
			break;
		}
	}
}

//==========================handler=================================

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
	const int fileIconSize = 16;

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

	std::vector<FileInfo> images = msg.GetImages();
	if (!images.empty()) {
		bubbleHeight += (images.size() * (imageSize + spacing)) - spacing;
	}

	std::vector<FileInfo> files = msg.GetFiles();
	if (!files.empty()) {
		bubbleHeight += (files.size() * (fileIconSize + spacing)) - spacing;
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

void ChatListStyle::UpdateScrollInfo()
{
	int clientHeight = GetClientRectHeight();

	SCROLLINFO si = { sizeof(SCROLLINFO), SIF_PAGE | SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL,
					 0, m_totalHeight, (UINT)clientHeight, m_scrollOffset };

	m_scrollBar.SetScrollInfo(&si);
	m_scrollBar.ShowWindow((m_totalHeight > clientHeight) ? SW_SHOW : SW_HIDE);
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

void ChatListStyle::DrawMessage(Gdiplus::Graphics& g, const Message& msg, int& y, int width)
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

	CStringW content = msg.GetContent();
	std::vector<FileInfo> files = msg.GetFiles();
	std::vector<FileInfo> images = msg.GetImages();
	bool isMyMessage = (msg.GetMessageType() == 1);
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

	int totalHeight = bubbleHeight;
	if (!images.empty()) totalHeight += (images.size() * (imageSize + spacing)) - spacing;
	if (!files.empty()) totalHeight += (files.size() * (fileHeight * 2 + spacing)) - spacing;

	if (!content.IsEmpty() || !images.empty() || !files.empty()) {
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
		int x_new = isMyMessage ? x - (imageSize / 2) : x;
		if (!images.empty()) {
			for (const auto& image : images) {
				CString localPath = _T("temp_") + image.fileName;
				CString downloadedPath = DownloadFile(image.url, localPath);
				if (!downloadedPath.IsEmpty()) {
					Gdiplus::Bitmap bitmap(downloadedPath);
					if (bitmap.GetLastStatus() == Gdiplus::Ok) {
						Gdiplus::Rect imageRect(x_new + bubblePadding, currentY, imageSize, imageSize);
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

		//=============draw file===========
		if (!files.empty()) {
			int fileY = y + bubbleHeight + (bubbleHeight > 0 ? spacing : 0);
			if (!images.empty()) {
				fileY = currentY;
			}

			for (const auto& file : files) {
				int fileWidth = min(bubbleWidthMax, bubbleWidth);
				int x_new = isMyMessage ? x - fileWidth : x;

				Gdiplus::Rect fileBubbleRect(
					x_new,
					fileY,
					fileWidth * 2,
					fileHeight
				);
				if (Gdiplus::GraphicsPath* filePath = CreateRoundRectPath(fileBubbleRect, radius)) {
					g.FillPath(&brushFileBubble, filePath);
					g.DrawPath(&penBorder, filePath);
					delete filePath;
				}

				Gdiplus::SolidBrush iconBrush(Gdiplus::Color(100, 100, 100));
				Gdiplus::RectF iconRect(
					(Gdiplus::REAL)(x_new + filePadding),
					(Gdiplus::REAL)(fileY + (fileHeight - fileIconSize) / 2),
					(Gdiplus::REAL)fileIconSize,
					(Gdiplus::REAL)fileIconSize
				);
				g.DrawString(L"↓", -1, m_pMsgFont, iconRect, &format, &iconBrush);

				Gdiplus::RectF fileTextRect(
					(Gdiplus::REAL)(x_new + filePadding + fileIconSize + 5),
					(Gdiplus::REAL)(fileY + (fileHeight * 2 - 16) / 4),
					(Gdiplus::REAL)(fileWidth - 2 * filePadding - fileIconSize - 5),
					(Gdiplus::REAL)(fileHeight - 2 * filePadding)
				);
				g.DrawString(file.fileName, -1, m_pMsgFont, fileTextRect, &format, isMyMessage ? &brushTextSend : &brushText);

				Gdiplus::Rect clickRect(
					x_new,
					fileY,
					fileWidth * 2,
					fileHeight
				);
				m_fileClickRects.push_back(clickRect);
				m_fileIds.push_back(file.id);
				fileY += fileHeight * 2 + spacing;
			}
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