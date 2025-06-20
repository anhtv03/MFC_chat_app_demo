﻿// chatDlg.cpp : implementation file
//

#include "pch.h"
#include "chat_app_demo.h"
#include "afxdialogex.h"
#include "chatDlg.h"
#include "TokenManager.h"
#include "util.h"
#include "models/json.hpp"
#include "EmojiModal.h"
#define CURL_STATICLIB
#include <curl.h>

using json = nlohmann::json;

// chatDlg dialog

IMPLEMENT_DYNAMIC(chatDlg, CDialogEx)

chatDlg::chatDlg(CString friendId, CString friendName, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DIALOG, pParent)
{
	m_friendId = friendId;
	m_friendname = friendName;
}

chatDlg::~chatDlg()
{
	curl_global_cleanup();
}

void chatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CHAT, _idc_list_chat);
	DDX_Control(pDX, IDC_EDT_MESSAGE, _idc_edt_message);
	DDX_Control(pDX, IDC_BTN_SEND, _idc_btn_send);
	DDX_Control(pDX, IDC_BTN_EMOJI, _idc_btn_emoji);
	DDX_Control(pDX, IDC_BTN_FILE, _idc_btn_file);
	DDX_Control(pDX, IDC_BTN_IMAGE, _idc_btn_image);
}


BEGIN_MESSAGE_MAP(chatDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SEND, &chatDlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_BTN_EMOJI, &chatDlg::OnBnClickedBtnEmoji)
	ON_BN_CLICKED(IDC_BTN_IMAGE, &chatDlg::OnBnClickedBtnImage)
	ON_BN_CLICKED(IDC_BTN_FILE, &chatDlg::OnBnClickedBtnFile)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_MESSAGE(WM_USER + 1, &chatDlg::OnEmojiSelected)
END_MESSAGE_MAP()


BOOL chatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	curl_global_init(CURL_GLOBAL_ALL);
	SetWindowText(m_friendname);

	//===============set chat content=================
	_idc_edt_message.SetCueBanner(_T("Nhập tin nhắn..."), TRUE);
	CFont font;
	font.CreatePointFont(100, _T("Segoe UI Emoji"));
	_idc_edt_message.SetFont(&font);

	//===============set image button=================
	setIconButton(_idc_btn_send, AfxGetApp()->LoadIcon(IDI_ICON_SEND));
	setIconButton(_idc_btn_emoji, AfxGetApp()->LoadIcon(IDI_ICON_EMOJI));
	setIconButton(_idc_btn_image, AfxGetApp()->LoadIcon(IDI_ICON_IMAGE));
	setIconButton(_idc_btn_file, AfxGetApp()->LoadIcon(IDI_ICON_FILE));

	//===============set chat list=================
	CRect chatRect;
	GetDlgItem(IDC_LIST_CHAT)->GetWindowRect(&chatRect);
	ScreenToClient(&chatRect);
	GetDlgItem(IDC_LIST_CHAT)->DestroyWindow();
	_idc_list_chat.Create(NULL, NULL, WS_VISIBLE | WS_CHILD, chatRect, this, IDC_LIST_CHAT);

	LoadChatMessages();
	StyleInputArea();
	SetTimer(1, 5000, nullptr);

	return TRUE;
}

void chatDlg::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);

	CBrush brush(RGB(240, 240, 240));
	dc.FillRect(&rect, &brush);
}

HBRUSH chatDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_EDIT)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		return (HBRUSH)GetStockObject(WHITE_BRUSH);
	}

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(RGB(240, 240, 240));
		pDC->SetTextColor(RGB(0, 0, 0));
		return (HBRUSH)m_hbrBackground;
	}

	if (pWnd == &_idc_list_chat)
	{
		pDC->SetBkColor(RGB(240, 240, 240));
		return (HBRUSH)m_hbrBackground;
	}

	return hbr;
}

//-----------------event handlers-------------------------
void chatDlg::OnOK()
{
	OnBnClickedBtnSend();
}

void chatDlg::OnBnClickedBtnSend()
{
	CString content;
	_idc_edt_message.GetWindowText(content);

	if (content.IsEmpty()) {
		MessageBox(_T("Vui lòng nhập nội dung tin nhắn."), _T("Thông báo"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	CString token = TokenManager::getToken();
	json response;
	CString errorMessage;
	std::vector<CString> files;

	if (sendMessage(m_friendId, content, files, token, response, errorMessage)) {
		LoadChatMessages();
		_idc_edt_message.SetWindowText(_T(""));
	}
}

void chatDlg::OnBnClickedBtnEmoji()
{
	EmojiModal dlg(this);
	dlg.LoadEmojiData(_T("D:\\Source Code\\MFC_Code\\MFC_chat_app_demo\\assets\\emoji-test.txt"));
	if (dlg.DoModal() == IDOK) {}
}

void chatDlg::OnBnClickedBtnImage()
{
	CString token = TokenManager::getToken();
	json response;
	CString errorMessage;
	CString content;
	std::vector<CString> selectedFiles;

	CString filter = _T("Image Files (*.bmp; *.jpg; *.jpeg; *.png; *.gif; *.tiff)|*.bmp;*.jpg;*.jpeg;*.png;*.gif;*.tiff|")
		_T("Bitmap Files (*.bmp)|*.bmp|")
		_T("JPEG Files (*.jpg;*.jpeg)|*.jpg;*.jpeg|")
		_T("PNG Files (*.png)|*.png|")
		_T("GIF Files (*.gif)|*.gif|")
		_T("TIFF Files (*.tiff)|*.tiff|")
		_T("All Files (*.*)|*.*||");

	CFileDialog openDlg(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT,
		filter, this);

	if (openDlg.DoModal() == IDOK) {
		POSITION pos = openDlg.GetStartPosition();
		if (pos == NULL) {
			CString filePath = openDlg.GetPathName();
			selectedFiles.push_back(filePath);
		}
		else {
			while (pos != NULL) {
				CString filePath = openDlg.GetNextPathName(pos);
				selectedFiles.push_back(filePath);
			}
		}

		if (!selectedFiles.empty()) {
			if (sendMessage(m_friendId, content, selectedFiles, token, response, errorMessage)) {
				LoadChatMessages();
			}
		}
	}
}

void chatDlg::OnBnClickedBtnFile()
{
	CString token = TokenManager::getToken();
	json response;
	CString errorMessage;
	CString content;
	std::vector<CString> selectedFiles;

	CFileDialog openDlg(TRUE, _T("txt"), NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT,
		_T("Text Files (*.txt)|*.txt|Data Files (*.dat)|*.dat|All Files (*.*)|*.*||"),
		this);

	if (openDlg.DoModal() == IDOK) {
		POSITION pos = openDlg.GetStartPosition();
		if (pos == NULL) {
			CString filePath = openDlg.GetPathName();
			selectedFiles.push_back(filePath);
		}
		else {
			while (pos != NULL) {
				CString filePath = openDlg.GetNextPathName(pos);
				selectedFiles.push_back(filePath);
			}
		}

		if (!selectedFiles.empty()) {
			if (sendMessage(m_friendId, content, selectedFiles, token, response, errorMessage)) {
				LoadChatMessages();
			}
		}
	}
}

LRESULT chatDlg::OnEmojiSelected(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR emoji = (LPCTSTR)wParam;
	if (emoji)
	{
		CString currentText;
		_idc_edt_message.GetWindowText(currentText);
		_idc_edt_message.SetWindowText(currentText + CString(emoji));
		_idc_edt_message.SetFocus();
		int startPos = currentText.GetLength() + (int)wcslen(emoji);
		_idc_edt_message.SetSel(startPos, -1);
	}
	return 0;
}

//----------------------Get API--------------------------
BOOL chatDlg::getMessage(CString& friendId, CString& token, json& response, CString& errorMessage) {
	CURL* curl = nullptr;
	CURLcode res = CURLE_OK;
	std::string response_str;
	long http_code = 0;

	try {
		curl = curl_easy_init();

		CStringA url(_T("http://30.30.30.85:8888/api/message/get-message?FriendID=") + friendId);
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

		std::string jsonStr = response.dump();
		CString debugStr = Utf8ToCString(jsonStr.c_str());
		OutputDebugString(_T("ChatDlg: getMessage: ") + debugStr + _T("\n"));

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

BOOL chatDlg::sendMessage(CString& friendId, CString& content, std::vector<CString> files,
	CString& token, json& response, CString& errorMessage)
{
	CURL* curl = nullptr;
	CURLcode res = CURLE_OK;
	std::string response_str;
	long http_code = 0;
	curl_mime* mime = nullptr;

	try {
		curl = curl_easy_init();

		CStringA url(_T("http://30.30.30.85:8888/api/message/send-message"));
		std::string authHeader = "Authorization: Bearer " + std::string(CT2A(token));
		struct curl_slist* headers = curl_slist_append(nullptr, authHeader.c_str());
		mime = curl_mime_init(curl);

		curl_mimepart* part = curl_mime_addpart(mime);
		curl_mime_name(part, "FriendID");
		curl_mime_data(part, CT2A(friendId), CURL_ZERO_TERMINATED);

		if (!content.IsEmpty()) {
			part = curl_mime_addpart(mime);
			curl_mime_name(part, "Content");
			curl_mime_data(part, CW2A(content, CP_UTF8), CURL_ZERO_TERMINATED);
		}

		for (const auto& filePath : files)
		{
			if (!filePath.IsEmpty())
			{
				part = curl_mime_addpart(mime);
				curl_mime_name(part, "files");
				curl_mime_filedata(part, CT2A(filePath));
			}
		}

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
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

		std::string jsonStr = response.dump();
		CString debugStr = Utf8ToCString(jsonStr.c_str());
		OutputDebugString(_T("ChatDlg: SendChatMessage: ") + debugStr + _T("\n"));

		curl_slist_free_all(headers);
		curl_mime_free(mime);
		curl_easy_cleanup(curl);

		return TRUE;
	}
	catch (const std::exception& e) {
		errorMessage = Utf8ToCString(e.what());
		curl_mime_free(mime);
		curl_easy_cleanup(curl);
		return FALSE;
	}
	return FALSE;
}


//----------------------function design-------------------
void chatDlg::LoadChatMessages()
{
	CString token = TokenManager::getToken();
	json response;
	CString errorMessage;

	if (getMessage(m_friendId, token, response, errorMessage))
	{
		if (response.contains("data") && response["data"].is_array())
		{
			json data = response["data"];
			CString lastMessageId;
			if (!m_messages.empty()) {
				lastMessageId = m_messages.back().GetId();
			}

			std::vector<Message> newMessages;
			for (const auto& item : data)
			{
				Message msg = Message::FromJson(item);
				CString msgId = msg.GetId();
				if (lastMessageId.IsEmpty() || msgId > lastMessageId) {
					newMessages.push_back(msg);
				}
			}

			/*std::string jsonStr = response.dump();
			CString debugStr = Utf8ToCString(jsonStr.c_str());
			OutputDebugString(_T("ChatDlg: LoadChatMessage: ") + debugStr + _T("\n"));*/

			if (!newMessages.empty()) {
				m_messages.insert(m_messages.end(), newMessages.begin(), newMessages.end());
				_idc_list_chat.SetMessages(&m_messages);
				_idc_list_chat.Invalidate();
				_idc_list_chat.UpdateWindow();
				_idc_list_chat.ScrollToBottom();
			}
		}
	}
	else
	{
		OutputDebugString(_T("Lỗi: ") + errorMessage);
		AfxMessageBox(_T("Không thể tải tin nhắn: ") + errorMessage);
	}
}

void chatDlg::setIconButton(CMFCButton& _idc_button, HICON hicon) {
	_idc_button.SetIcon(hicon);
	_idc_button.SizeToContent();
	_idc_button.m_bDrawFocus = FALSE;
	_idc_button.m_bTransparent = TRUE;
	_idc_button.m_nFlatStyle = CMFCButton::BUTTONSTYLE_NOBORDERS;
	_idc_button.SetFaceColor(RGB(240, 240, 240), TRUE);
	_idc_button.SetTextColor(RGB(100, 100, 100));
	_idc_button.SetWindowPos(nullptr, 0, 0, 32, 32, SWP_NOMOVE | SWP_NOZORDER);
}

void chatDlg::StyleInputArea()
{
	CRect clientRect;
	GetClientRect(&clientRect);

	int inputHeight = 60;
	int buttonSize = 36;
	int margin = 10;

	CRect inputRect(margin, clientRect.bottom - inputHeight,
		clientRect.right - 4 * buttonSize - 5 * margin,
		clientRect.bottom - margin);
	_idc_edt_message.MoveWindow(&inputRect);

	int buttonY = clientRect.bottom - (inputHeight + buttonSize) / 2;

	CRect sendRect(inputRect.right + 5, buttonY,
		inputRect.right + 5 + buttonSize, buttonY + buttonSize);
	_idc_btn_send.MoveWindow(&sendRect);

	CRect emojiRect(sendRect.right + margin, buttonY,
		sendRect.right + margin + buttonSize, buttonY + buttonSize);
	_idc_btn_emoji.MoveWindow(&emojiRect);

	CRect imageRect(emojiRect.right + 5, buttonY,
		emojiRect.right + 5 + buttonSize, buttonY + buttonSize);
	_idc_btn_image.MoveWindow(&imageRect);

	CRect fileRect(imageRect.right + 5, buttonY,
		imageRect.right + 5 + buttonSize, buttonY + buttonSize);
	_idc_btn_file.MoveWindow(&fileRect);

	CRect chatRect(0, 0, clientRect.right, clientRect.bottom - inputHeight - margin);
	_idc_list_chat.MoveWindow(&chatRect);
}

