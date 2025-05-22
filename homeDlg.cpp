// homeDlg.cpp : implementation file
//

#include "pch.h"
#include "chat_app_demo.h"
#include "afxdialogex.h"
#include "homeDlg.h"
#include <gdiplus.h>
#include <urlmon.h>
#include "chatDlg.h"
#include "TokenManager.h"
#include "util.h"
#include "models/json.hpp"
#include <atlconv.h>
#define CURL_STATICLIB
#include <curl.h>

using json = nlohmann::json;
using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "urlmon.lib")


// homeDlg dialog

IMPLEMENT_DYNAMIC(homeDlg, CDialogEx)

homeDlg::homeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HOME_DIALOG, pParent),
	m_avatarImage(nullptr),
	m_gdiplusToken(0)
{
}

homeDlg::~homeDlg()
{
	if (m_avatarImage)
	{
		delete m_avatarImage;
	}
	GdiplusShutdown(m_gdiplusToken);
	curl_global_cleanup();
}

void homeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_TITLE, _txt_title);
	DDX_Control(pDX, IDC_EDT_SEARCH, _edt_search);
	DDX_Control(pDX, IDC_TXT_LIST_TITLE, _txt_list_title);
	DDX_Control(pDX, IDC_TXT_NAME, _txt_name);
	DDX_Control(pDX, IDC_LIST_FRIEND, _idc_list_friend);
	DDX_Control(pDX, IDC_AVATAR, _idc_avatar);
}

BEGIN_MESSAGE_MAP(homeDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FRIEND, &homeDlg::OnNMDblclkListFriend)
	ON_EN_CHANGE(IDC_EDT_SEARCH, &homeDlg::OnEnChangeEdtSearch)
END_MESSAGE_MAP()

BOOL homeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	curl_global_init(CURL_GLOBAL_ALL);
	this->SetBackgroundColor(RGB(255, 255, 255));

	//set avatar for picture control
	GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);

	CString url = _T("https://res.cloudinary.com/djj5gopcs/image/upload/v1744612363/download20230704194701_ult1ta.png");
	CString localPath = _T("avatar.png");
	URLDownloadToFile(NULL, url, localPath, 0, NULL);

	m_avatarImage = Image::FromFile(localPath);

	//set font for title bkav chat
	_font_title.CreateFont(
		24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);
	_txt_title.SetFont(&_font_title);

	//set font for list friend title
	_font_list_title.CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);
	_txt_list_title.SetFont(&_font_list_title);

	//set font for name
	_font_name.CreateFont(
		20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Inter")
	);
	_txt_name.SetFont(&_font_name);

	//=================get user information==========
	CString token = TokenManager::getToken();
	json response;
	CString errorMessage;
	if (getRequest(_T("/api/user/info"), token, response, errorMessage)) {
		if (response.contains("data") && response["data"].is_object())
		{
			json data = response["data"];
			CString fullName = Utf8ToCString(data["FullName"].get<std::string>());
			//CString avatar = Utf8ToCString(data["Avatar"].get<std::string>());
			_txt_name.SetWindowText(fullName);
		}
	}

	//===============set list friend=================
	_idc_list_friend.ModifyStyle(LVS_SORTASCENDING | LVS_SORTDESCENDING, LVS_OWNERDRAWFIXED | LVS_REPORT);
	_idc_list_friend.InsertColumn(0, _T(""), LVCFMT_LEFT, 600);
	_idc_list_friend.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	CImageList imageList;
	imageList.Create(50, 50, ILC_COLOR32, 0, 10);
	_idc_list_friend.SetImageList(&imageList, LVSIL_SMALL);

	if (getRequest(_T("/api/message/list-friend"), token, response, errorMessage)) {
		if (response.contains("data") && response["data"].is_array()) {
			json data = response["data"];

			_idc_list_friend.DeleteAllItems();
			for (auto& item : data) {
				CString name = Utf8ToCString(item["FullName"].get<std::string>());
				CString friendId = Utf8ToCString(item["FriendID"].get<std::string>());
				_idc_list_friend.AddFriend(name, friendId, localPath);
			}
		}
	}

	return TRUE;
}

HBRUSH homeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_TITLE)
	{
		pDC->SetTextColor(RGB(20, 106, 224));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}
	return hbr;
}

void homeDlg::OnPaint()
{
	CPaintDC dc(this);
	CDialogEx::OnPaint();

	if (m_avatarImage)
	{
		CRect rect;
		_idc_avatar.GetWindowRect(&rect);
		ScreenToClient(&rect);

		int size = min(rect.Width(), rect.Height());

		Graphics graphics(dc);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		GraphicsPath path;
		path.AddEllipse(rect.left, rect.top, size, size);
		graphics.SetClip(&path);

		graphics.DrawImage(m_avatarImage, rect.left, rect.top, size, size);
	}
}

void homeDlg::OnNMDblclkListFriend(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int itemIndex = pNMItemActivate->iItem;

	if (itemIndex != -1)
	{
		DWORD_PTR dataIndex = _idc_list_friend.GetItemData(itemIndex);
		if (dataIndex < _idc_list_friend.m_friendIds.size()) {
			CString friendId = _idc_list_friend.m_friendIds[dataIndex];
			CString friendName = _idc_list_friend.m_Names[dataIndex];

			chatDlg dlg(friendId, friendName);
			dlg.DoModal();
		}
	}
	*pResult = 0;
}

void homeDlg::OnEnChangeEdtSearch()
{
	CString key;
	CString localPath = _T("avatar.png"); 
	_edt_search.GetWindowText(key);
	key.MakeLower();

	_idc_list_friend.DeleteAllItems();
	if (key.IsEmpty()) {
		for (size_t i = 0; i < _idc_list_friend.m_Names.size(); ++i) {
			int itemIndex = _idc_list_friend.InsertItem(_idc_list_friend.GetItemCount(), _idc_list_friend.m_Names[i]);
			_idc_list_friend.SetItemData(itemIndex, i);
		}
		return;
	}

	for (size_t i = 0; i < _idc_list_friend.m_Names.size(); ++i) {
		CString name = _idc_list_friend.m_Names[i];
		name.MakeLower(); 
		if (name.Find(key) != -1) { 
			int itemIndex = _idc_list_friend.InsertItem(_idc_list_friend.GetItemCount(), _idc_list_friend.m_Names[i]);
			_idc_list_friend.SetItemData(itemIndex, i);
		}
	}
}

//----------------------Get API--------------------------
BOOL homeDlg::getRequest(const CString& endpoint, const CString& token, json& response, CString& errorMessage) {
	CURL* curl = nullptr;
	CURLcode res = CURLE_OK;
	std::string response_str;
	long http_code = 0;

	try {
		curl = curl_easy_init();

		CStringA url(_T("http://30.30.30.85:8888") + endpoint);
		std::string authHeader = "Authorization: Bearer " + std::string(CT2A(token));
		struct curl_slist* headers = curl_slist_append(nullptr, authHeader.c_str());

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_str);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			throw std::runtime_error(curl_easy_strerror(res));
		}

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (http_code != 200) {
			if (!response_str.empty()) {
				response = json::parse(response_str, nullptr, false);
				if (!response.is_discarded() && response.contains("message") && response["message"].is_string())
					throw std::runtime_error(response["message"].get<std::string>());
			}
		}
		response = json::parse(response_str, nullptr, false);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return TRUE;
	}
	catch (const std::exception& e) {
		errorMessage = Utf8ToCString(e.what());
		if (curl) curl_easy_cleanup(curl);
		return FALSE;
	}
}

