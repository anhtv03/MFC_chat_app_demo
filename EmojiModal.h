#pragma once
#include "afxdialogex.h"
#include <vector>
#include <string>


// EmojiModal dialog
struct EmojiData {
	std::wstring codePoints;
	std::wstring status;
	std::wstring name;
};


class EmojiModal : public CDialogEx
{
	DECLARE_DYNAMIC(EmojiModal)

public:
	EmojiModal(CWnd* pParent = nullptr);   // standard constructor
	virtual ~EmojiModal();

	void LoadEmojiData(const CString& filePath);
	afx_msg void OnLbnSelchangeListEmoji();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EMOJI_MODAL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	std::vector<EmojiData> m_emojiList;
	CListBox idc_list_emoji;
	std::wstring ConvertCodeEmoji(const std::wstring& code);

};
