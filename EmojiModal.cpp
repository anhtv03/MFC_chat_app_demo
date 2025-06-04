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
	ON_NOTIFY(NM_CLICK, IDC_LIST_EMOJI, &EmojiModal::OnNMClickListEmoji)
END_MESSAGE_MAP()

BOOL EmojiModal::OnInitDialog() {
	CDialog::OnInitDialog();
	SetupEmoji();

	if (m_emojiList.empty()) {
		LoadEmojiData(_T(""));
	}

	m_image_list.Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, m_emojiList.size());

	int index = 0;
	for (const auto& item : m_emojiList) {
		std::wstring emojiStr = ConvertCodeEmoji(item.codePoints);

		HBITMAP hBitmap = CreateEmojiBitmap(emojiStr, 32);
		if (hBitmap) {
			CBitmap bitmap;
			bitmap.Attach(hBitmap);
			m_image_list.Add(&bitmap, RGB(255, 255, 255));
			bitmap.Detach();
		}
		else {
			CBitmap bitmap;
			bitmap.CreateBitmap(32, 32, 1, 1, NULL);
			m_image_list.Add(&bitmap, RGB(255, 255, 255));
		}

		LVITEM lvItem = { 0 };
		lvItem.mask = LVIF_IMAGE | LVIF_PARAM;
		lvItem.iItem = index;
		lvItem.iImage = index;
		lvItem.lParam = index;
		lvItem.pszText = const_cast<LPTSTR>(emojiStr.c_str());

		idc_list_emoji.InsertItem(&lvItem);
		index++;
	}

	idc_list_emoji.SetImageList(&m_image_list, LVSIL_NORMAL);

	return TRUE;
}

HBITMAP EmojiModal::CreateEmojiBitmap(const std::wstring& emoji, int size) {
	HDC hdc = GetDC()->GetSafeHdc();
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdc, size, size);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

	RECT rect = { 0, 0, size, size };
	FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

	HFONT hFont = CreateFont(size - 8, 0, 0, 0,
		FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Segoe UI Emoji")
	);

	HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

	SetTextColor(hdcMem, RGB(0, 0, 0));
	SetBkMode(hdcMem, TRANSPARENT);

	RECT textRect = { 0, 0, size, size };
	DrawText(hdcMem, emoji.c_str(), -1, &textRect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hdcMem, hOldFont);
	SelectObject(hdcMem, hOldBitmap);
	DeleteObject(hFont);
	DeleteDC(hdcMem);

	return hBitmap;
}

//-----------------event handlers-------------------------
void EmojiModal::OnNMClickListEmoji(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	if (pNMItemActivate->iItem >= 0 && pNMItemActivate->iItem < (int)m_emojiList.size()) {
		int emojiIndex = pNMItemActivate->iItem;
		std::wstring emojiStr = ConvertCodeEmoji(m_emojiList[emojiIndex].codePoints);

		CWnd* pParent = GetParent();
		if (pParent) {
			CString msg(emojiStr.c_str());
			pParent->SendMessage(WM_USER + 1, (WPARAM)msg.GetString(), 0);
		}

		EndDialog(IDOK);
	}

	*pResult = 0;
}


//-----------------emoji handlers-------------------------
void EmojiModal::SetupEmoji() {
	idc_list_emoji.ModifyStyle(LVS_REPORT | LVS_LIST | LVS_SMALLICON,
		LVS_ICON | LVS_AUTOARRANGE | LVS_ALIGNTOP);
	idc_list_emoji.SetExtendedStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);
	idc_list_emoji.SetIconSpacing(50, 50);
	idc_list_emoji.SetBkColor(RGB(255, 255, 255));
	idc_list_emoji.SetTextBkColor(RGB(255, 255, 255));

}

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
		data.codePoints.erase(data.codePoints.find_last_not_of(L" ") + 1);
		data.status.erase(0, data.status.find_first_not_of(L" "));
		data.status.erase(data.status.find_last_not_of(L" ") + 1);
		data.name.erase(0, data.name.find_first_not_of(L" "));
		data.name.erase(data.name.find_last_not_of(L" ") + 1);

		m_emojiList.push_back(data);
	}
	file.close();
}

std::wstring EmojiModal::ConvertCodeEmoji(const std::wstring& code) {
	std::wstringstream result;
	std::wstringstream iss(code);
	std::wstring token;

	while (iss >> token) {
		if (!token.empty()) {
			unsigned long codePoint = wcstoul(token.c_str(), nullptr, 16);

			if (codePoint <= 0xFFFF) {
				result << (wchar_t)codePoint;
			}
			else {
				codePoint -= 0x10000;
				wchar_t high = 0xD800 + (codePoint >> 10);
				wchar_t low = 0xDC00 + (codePoint & 0x3FF);
				result << high << low;
			}
		}
	}
	return result.str();
}


