#include "pch.h"
#include "itemFriendStyle.h"

itemFriendStyle::itemFriendStyle()
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

itemFriendStyle::~itemFriendStyle()
{
	for (auto avatar : m_Avatars) {
		delete avatar;
	}
}

BEGIN_MESSAGE_MAP(itemFriendStyle, CListCtrl)
END_MESSAGE_MAP()

void itemFriendStyle::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	Graphics graphics(pDC->GetSafeHdc());
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	int itemIndex = lpDrawItemStruct->itemID;
	if (itemIndex < 0 || itemIndex >= GetItemCount()) {
		return;
	}

	DWORD_PTR dataIndex = GetItemData(itemIndex);
	if (dataIndex >= m_Names.size()) {
		return;
	}

	CRect rect = lpDrawItemStruct->rcItem;
	CString name = m_Names[dataIndex];
	CString message = m_Messages[dataIndex];
	Image* avatar = m_Avatars[dataIndex];

	int avatarSize = 50;
	int centerY = rect.top + (rect.Height() - avatarSize) / 2;

	if (avatar && avatar->GetLastStatus() == Ok) {
		GraphicsPath path;
		path.AddEllipse(rect.left, centerY, avatarSize, avatarSize);
		graphics.SetClip(&path);
		graphics.DrawImage(avatar, rect.left, centerY, avatarSize, avatarSize);
		graphics.ResetClip();
	}
	else {
		SolidBrush brush(Color(200, 200, 200));
		graphics.FillEllipse(&brush, rect.left, centerY, avatarSize, avatarSize);
	}

	CFont nameFont;
	nameFont.CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);

	CFont messageFont;
	messageFont.CreateFont(
		14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);

	int textStartX = rect.left + avatarSize + 15; 
	int nameY = rect.top + 8; 
	int messageY = rect.top + 32; 

	CFont* pOldFont = pDC->SelectObject(&nameFont);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->DrawText(name, CRect(textStartX, nameY, rect.right - 10, nameY + 20),
		DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	if (!message.IsEmpty()) {
		pDC->SelectObject(&messageFont);
		pDC->SetTextColor(RGB(128, 128, 128));

		CString displayMessage = message;
		CRect messageRect(textStartX, messageY, rect.right - 10, messageY + 16);
		CSize textSize = pDC->GetTextExtent(displayMessage);
		int maxWidth = messageRect.Width();

		if (textSize.cx > maxWidth) {
			while (displayMessage.GetLength() > 0) {
				displayMessage = displayMessage.Left(displayMessage.GetLength() - 1);
				CString testString = displayMessage + _T("...");
				CSize testSize = pDC->GetTextExtent(testString);
				if (testSize.cx <= maxWidth) {
					displayMessage += _T("...");
					break;
				}
			}
		}
		pDC->DrawText(displayMessage, messageRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	}

	pDC->SelectObject(pOldFont);
}

void itemFriendStyle::AddFriend(const CString& fullName, const CString& friendId, const CString& imagePath, const CString& message)
{
	int dataIndex = m_friendIds.size();
	m_Names.push_back(fullName);
	m_friendIds.push_back(friendId);
	m_Messages.push_back(message);
	Image* avatar = nullptr;
	if (!imagePath.IsEmpty()) {
		avatar = Image::FromFile(imagePath);
		if (avatar && avatar->GetLastStatus() != Ok) {
			delete avatar;
			avatar = nullptr;
		}
	}
	m_Avatars.push_back(avatar);

	int itemIndex = InsertItem(GetItemCount(), fullName);
	SetItemData(itemIndex, (DWORD_PTR)dataIndex);
}