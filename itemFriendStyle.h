#pragma once
#include <afxcmn.h>
#include <gdiplus.h>
#include <vector>
using namespace Gdiplus;

class itemFriendStyle : public CListCtrl
{
public:
	itemFriendStyle();
	virtual ~itemFriendStyle();

protected:
	DECLARE_MESSAGE_MAP()
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;

public:
	std::vector<Image*> m_Avatars;
	std::vector<CString> m_friendIds;
	std::vector<CString> m_Names;

	void AddFriend(const CString& fullName, const CString& friendId, const CString& imagePath);

};

