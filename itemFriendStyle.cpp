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
        OutputDebugString(L"DEBUG: Invalid DrawItem index\n");
        return;
    }

    DWORD_PTR dataIndex = GetItemData(itemIndex);
    if (dataIndex >= m_Names.size()) {
        OutputDebugString(L"DEBUG: Invalid dataIndex\n");
        return;
    }

    CRect rect = lpDrawItemStruct->rcItem;
    CString name = m_Names[dataIndex];
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

void itemFriendStyle::AddFriend(const CString& fullName, const CString& friendId, const CString& imagePath)
{
    int dataIndex = m_friendIds.size();
    m_Names.push_back(fullName);
    m_friendIds.push_back(friendId);
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