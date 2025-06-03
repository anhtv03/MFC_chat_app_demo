// EmojiModal.cpp : implementation file
//

#include "pch.h"
#include "chat_app_demo.h"
#include "afxdialogex.h"
#include "EmojiModal.h"
#include <fstream>
#include <sstream>


// EmojiModal dialog

IMPLEMENT_DYNAMIC(EmojiModal, CDialogEx)

EmojiModal::EmojiModal(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EMOJI_MODAL, pParent)
{

}

EmojiModal::~EmojiModal()
{
}

void EmojiModal::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_EMOJI, idc_list_emoji);
}


BEGIN_MESSAGE_MAP(EmojiModal, CDialogEx)
	ON_LBN_SELCHANGE(IDC_LIST_EMOJI, &EmojiModal::OnLbnSelchangeListEmoji)
END_MESSAGE_MAP()

BOOL EmojiModal::OnInitDialog() {
	CDialog::OnInitDialog();

	if (!idc_list_emoji.SubclassDlgItem(IDC_LIST_EMOJI, this))
	{
		AfxMessageBox(_T("Không thể khởi tạo ListBox cho emoji!"));
		EndDialog(IDCANCEL);
		return FALSE;
	}

	for (const auto& item : m_emojiList) {
		std::wstring emojiStr = ConvertCodeEmoji(item.codePoints);
		CString text;
		text.Format(L"%s - %s", emojiStr.c_str(), item.name.c_str());
		idc_list_emoji.AddString(text);
	}

	return TRUE;
}

//-----------------event handlers-------------------------
void EmojiModal::OnLbnSelchangeListEmoji()
{
	int sel = idc_list_emoji.GetCurSel();
	if (sel != LB_ERR) {
		std::wstring emojiStr = ConvertCodeEmoji(m_emojiList[sel].codePoints);
		CWnd* pParent = GetParent();
		if (pParent) {
			CString msg(emojiStr.c_str());
			pParent->SendMessage(WM_USER + 1, (WPARAM)msg.GetString(), 0);
		}
		EndDialog(IDOK);
	}
}

//-----------------emoji handlers-------------------------
void EmojiModal::LoadEmojiData(const CString& filePath)
{
	std::wfstream file(filePath);
	if (!file.is_open()) return;

	std::wstring line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == L'#') continue;

		std::wstringstream ss(line);
		EmojiData data;

		std::getline(ss, data.codePoints, L';');
		std::getline(ss, data.status, L';');
		std::getline(ss, data.name, L'#');

		data.codePoints.erase(0, data.codePoints.find_first_not_of(L" "));
		data.status.erase(0, data.codePoints.find_first_not_of(L" "));
		data.name.erase(0, data.codePoints.find_first_not_of(L" "));
		m_emojiList.push_back(data);
	}
	file.close();
}

std::wstring EmojiModal::ConvertCodeEmoji(const std::wstring& code) {
	std::wstringstream ss;
	std::wstringstream iss(code);
	std::wstring token;
	while (std::getline(iss, token)) {
		if (!token.empty()) {
			wchar_t wc = (wchar_t)wcstol(token.c_str(), nullptr, 16);
			ss << wc;
		}
	}
	return ss.str();
}

