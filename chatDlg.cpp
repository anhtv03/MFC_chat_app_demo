// chatDlg.cpp : implementation file
//

#include "pch.h"
#include "chat_app_demo.h"
#include "afxdialogex.h"
#include "chatDlg.h"
#include "Message.h"
#include "TokenManager.h"
#include "util.h"
#include "models/json.hpp"
#include <cpprest/http_client.h>
#include <cpprest/json.h>

using json = nlohmann::json;
using namespace web::http;
using namespace web::http::client;


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
END_MESSAGE_MAP()


BOOL chatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText(m_friendname);

	//===============set chat content=================
	CEdit* pEditCtrl = (CEdit*)GetDlgItem(IDC_EDT_MESSAGE);
	pEditCtrl->SetCueBanner(_T("Nhập tin nhắn..."));

	//===============set image button=================
	setIconButton(_idc_btn_send, AfxGetApp()->LoadIcon(IDI_ICON_SEND));
	setIconButton(_idc_btn_emoji, AfxGetApp()->LoadIcon(IDI_ICON_EMOJI));
	setIconButton(_idc_btn_image, AfxGetApp()->LoadIcon(IDI_ICON_IMAGE));
	setIconButton(_idc_btn_file, AfxGetApp()->LoadIcon(IDI_ICON_FILE));

	//===============set chat list=================
	_idc_list_chat.ModifyStyle(LVS_LIST | LVS_ICON | LVS_SMALLICON, LVS_REPORT | WS_VSCROLL);
	_idc_list_chat.SetExtendedStyle(_idc_list_chat.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	_idc_list_chat.InsertColumn(0, _T(""), LVCFMT_LEFT, 600);

	CString token = TokenManager::getToken();
	json response;
	CString errorMessage;
	if (getMessage(m_friendId, token, response, errorMessage)) {
		if (response.contains("data") && response["data"].is_array())
		{
			json data = response["data"];
			for (const auto& item : data)
			{
				CString content = item.contains("Content") ? Utf8ToCString(item["Content"].get<std::string>()) : CString();
				_idc_list_chat.InsertItem(_idc_list_chat.GetItemCount(), content);
			}
		}
	}

	return TRUE;
}

void chatDlg::OnBnClickedBtnSend()
{
	CString content;
	_idc_edt_message.GetWindowText(content);

	if (content.IsEmpty()) {
		MessageBox(_T("Vui lòng nhập nội dung tin nhắn."), _T("Thông báo"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	Message newMsg;
	/*newMsg.id = _T("11");
	newMsg.myId = _T("1");
	CString friendIdStr;
	friendIdStr.Format(_T("%d"), m_friendId);
	newMsg.friendId = friendIdStr;
	newMsg.content = content;
	newMsg.isSend = 1;
	newMsg.createdAt = CTime::GetCurrentTime();
	newMsg.messageType = 0; */

	//m_messages.push_back(newMsg);
	//_idc_list_chat.InsertItem(_idc_list_chat.GetItemCount(), newMsg.content);
	_idc_edt_message.SetWindowText(_T(""));

}

//----------------------Get API--------------------------
BOOL chatDlg::getMessage(CString& friendId, CString& token, json& response, CString& errorMessage) {
	try {
		http_client client(U("http://30.30.30.87:8888"));
		uri_builder builder(U("/api/message/get-message"));
		builder.append_query(U("FriendID"), std::wstring(friendId));

		http_request request(methods::GET);
		request.set_request_uri(builder.to_uri());
		request.headers().add(U("Authorization"), U("Bearer ") + std::wstring(token));

		auto requestTask = client.request(request)
			.then([&](http_response res) {
			auto status = res.status_code();
			return res.extract_json().then([status](web::json::value jsonResponse) {
				return std::make_pair(status, jsonResponse);
				});
				})
			.then([&](std::pair<int, web::json::value> result) {
			int status = result.first;
			web::json::value jsonResponse = result.second;
			response = json::parse(jsonResponse.serialize());
			OutputDebugString(L"DEBUG: Parsed JSON: " + Utf8ToCString(response.dump()) + L"\n");

			if (status != status_codes::OK) {
				if (response.contains("message") && response["message"].is_string()) {
					throw std::runtime_error(response["message"].get<std::string>());
				}
			}
				});
		requestTask.wait();
		return TRUE;
	}
	catch (const std::exception& e) {
		errorMessage = Utf8ToCString(e.what());
		return FALSE;
	}
}



//----------------------function design-------------------
void chatDlg::setIconButton(CMFCButton& _idc_button, HICON hicon) {
	_idc_button.SetIcon(hicon);
	_idc_button.SizeToContent();
	_idc_button.m_bDrawFocus = FALSE;
	_idc_button.SetWindowPos(nullptr, 0, 0, 32, 32, SWP_NOMOVE | SWP_NOZORDER);
}