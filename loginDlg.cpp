// loginDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "chat_app_demo.h"
#include "loginDlg.h"
#include "afxdialogex.h"
#include "registerDlg.h"
#include "homeDlg.h"
#include "TokenManager.h"
#include "util.h"
#include "models/json.hpp"
#include <cpprest/http_client.h>
#include <cpprest/json.h>

#include "chatDlg.h"

using json = nlohmann::json;
using namespace web::http;
using namespace web::http::client;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



loginDlg::loginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN_DIALOG, pParent)
{
}

void loginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDT_USERNAME, _edt_username);
	DDX_Control(pDX, IDC_EDT_PASSWORD, _edt_password);
	DDX_Control(pDX, IDC_CHK_REMEMBER, _chk_remember);
	DDX_Control(pDX, IDC_BTN_LOGIN, _btn_login);
	DDX_Control(pDX, IDC_TXT_ERROR, _txt_error);
	DDX_Control(pDX, IDC_STATIC_TITLE, _txt_title);
}

BEGIN_MESSAGE_MAP(loginDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_LOGIN, &loginDlg::OnBnClickedBtnLogin)
	ON_STN_CLICKED(IDC_TXT_REGISTER, &loginDlg::OnStnClickedTxtRegister)
END_MESSAGE_MAP()

BOOL loginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	this->SetBackgroundColor(RGB(255, 255, 255));

	_txt_error.ShowWindow(SW_HIDE);

	//set font for title Bkav chat
	_font_title.CreateFont(
		24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);
	_txt_title.SetFont(&_font_title);

	//set font for login button
	_btn_login.ModifyStyle(0, BS_OWNERDRAW);
	_font_button_login.CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);
	_btn_login.SetFont(&_font_button_login);

	//check remember auto login
	CString username = AfxGetApp()->GetProfileString(_T("Login"), _T("Username"), _T(""));
	CString password = AfxGetApp()->GetProfileString(_T("Login"), _T("Password"), _T(""));
	if (!username.IsEmpty() && !password.IsEmpty()) {
		_edt_username.SetWindowText(username);
		_edt_password.SetWindowText(password);
		_chk_remember.SetCheck(BST_CHECKED);
		OnBnClickedBtnLogin();
	}

	return TRUE;
}

HBRUSH loginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	//set color for text error 
	if (pWnd->GetDlgCtrlID() == IDC_TXT_ERROR)
	{
		pDC->SetTextColor(RGB(236, 70, 34));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}
	//set color for text register 
	if (pWnd->GetDlgCtrlID() == IDC_TXT_REGISTER)
	{
		pDC->SetTextColor(RGB(4, 125, 231));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}
	//set color for text title 
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_TITLE)
	{
		pDC->SetTextColor(RGB(20, 106, 224));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(WHITE_BRUSH);
	}

	return hbr;
}

void loginDlg::OnOK()
{
	OnBnClickedBtnLogin();
}

void loginDlg::OnBnClickedBtnLogin()
{
	CString username, password;
	_edt_username.GetWindowText(username);
	_edt_password.GetWindowText(password);
	_txt_error.ShowWindow(SW_HIDE);

	//check username empty
	if (username.Trim().IsEmpty())
	{
		_txt_error.ShowWindow(SW_SHOW);
		_txt_error.SetWindowText(_T("Tên đăng nhập không được để trống"));
		return;
	}
	else if (password.IsEmpty())
	{
		_txt_error.ShowWindow(SW_SHOW);
		_txt_error.SetWindowText(_T("Mật khẩu không được để trống"));
		return;
	}

	//if (username == "ad" && password == "1") {
	//	chatDlg chatDlg(1, _T("AnhTV"));
	//	chatDlg.DoModal();
	//}

	json response;
	CString errorMessage;
	if (!Login(username, password, response, errorMessage))
	{
		_txt_error.ShowWindow(SW_SHOW);
		_txt_error.SetWindowText(errorMessage);
		return;
	}

	if (response.contains("data") && response["data"].is_object())
	{
		json data = response["data"];
		CString token = Utf8ToCString(data["token"].get<std::string>());
		TokenManager::setToken(token);

		if (_chk_remember.GetCheck()) {
			AfxGetApp()->WriteProfileString(_T("Login"), _T("Username"), username);
			AfxGetApp()->WriteProfileString(_T("Login"), _T("Password"), password);
		}
		else {
			AfxGetApp()->WriteProfileString(_T("Login"), _T("Username"), _T(""));
			AfxGetApp()->WriteProfileString(_T("Login"), _T("Password"), _T(""));
		}
	}

	homeDlg homeDlg;
	homeDlg.DoModal();	
}

void loginDlg::OnStnClickedTxtRegister()
{
	registerDlg registerDlg;
	if (registerDlg.DoModal() == IDOK)
	{
	}
}

//----------------------Get API--------------------------
BOOL loginDlg::Login(const CString& username, const CString& password, json& response, CString& errorMessage)
{
	try {
		http_client client(U("http://30.30.30.85:8888"));

		web::json::value requestBody;
		requestBody[U("Username")] = web::json::value::string(std::wstring(username));
		requestBody[U("Password")] = web::json::value::string(std::wstring(password));

		auto requestTask = client.request(methods::POST, U("/api/auth/login"), requestBody)
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
		CString mess = Utf8ToCString(e.what());
		if (mess == "Incorrect password" || mess == "Username not found") {
			errorMessage = _T("Bạn nhập sai tên tài khoản hoặc mật khẩu! ");
		}
		else
		{
			errorMessage = _T("Đăng nhập thất bại do lỗi hệ thống!");
		}
		return FALSE;
	}
}


