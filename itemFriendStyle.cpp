#include "pch.h"
#include "itemFriendStyle.h"

itemFriendStyle::itemFriendStyle() {}
itemFriendStyle::~itemFriendStyle()
{
	//for (auto avatar : m_Avatars) delete avatar;
}

BEGIN_MESSAGE_MAP(itemFriendStyle, CListCtrl)
END_MESSAGE_MAP()

void itemFriendStyle::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	Graphics graphics(pDC->GetSafeHdc());
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	int index = lpDrawItemStruct->itemID;

	if (index >= m_Names.size()) return;

	CRect rect = lpDrawItemStruct->rcItem;
	CString name = m_Names[index];
	Image* avatar = m_Avatars[index];

	int avatarSize = 50;
	int centerY = rect.top + (rect.Height() - avatarSize) / 2;

	GraphicsPath path;
	path.AddEllipse(rect.left, centerY, avatarSize, avatarSize);
	graphics.SetClip(&path);
	graphics.DrawImage(avatar, rect.left, centerY, avatarSize, avatarSize);
	graphics.ResetClip();

	CFont font;
	font.CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);
	CFont* pOldFont = pDC->SelectObject(&font);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(name, CRect(rect.left + avatarSize + 50, rect.top, rect.right, rect.bottom), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	pDC->SelectObject(pOldFont);
}

void itemFriendStyle::AddFriend(const CString& name, const CString& imagePath) {
	m_Names.push_back(name);
	m_Avatars.push_back(Image::FromFile(imagePath));
	int index = InsertItem(GetItemCount(), _T(""));
	SetItemText(index, 0, name);
}

