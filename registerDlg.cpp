// registerDlg.cpp : implementation file
//

#include "pch.h"
#include "chat_app_demo.h"
#include "afxdialogex.h"
#include "registerDlg.h"
#include "models/json.hpp"
#include "util.h"
#define CURL_STATICLIB
#include <curl.h>

using json = nlohmann::json;

// registerDlg dialog

IMPLEMENT_DYNAMIC(registerDlg, CDialogEx)

registerDlg::registerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REGISTER_DIALOG, pParent)
{
	curl_global_cleanup();
}

registerDlg::~registerDlg()
{
}

void registerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDT_USERNAME, _edt_username);
	DDX_Control(pDX, IDC_EDT_PASSWORD, _edt_password);
	DDX_Control(pDX, IDC_EDT_CONFIRM_PASSWORD, _edt_confirm_password);
	DDX_Control(pDX, IDC_TXT_ERROR, _txt_error);
	DDX_Control(pDX, IDC_EDT_NAME, _edt_name);
	DDX_Control(pDX, IDC_BTN_REGISTER, _btn_register);
}


BEGIN_MESSAGE_MAP(registerDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_REGISTER, &registerDlg::OnBnClickedBtnRegister)
END_MESSAGE_MAP()


// registerDlg message handlers
BOOL registerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	curl_global_init(CURL_GLOBAL_ALL);
	this->SetBackgroundColor(RGB(255, 255, 255));

	//set font for login button
	_btn_register.ModifyStyle(0, BS_OWNERDRAW);

	_font_btn_register.CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);
	_btn_register.SetFont(&_font_btn_register);

	_txt_error.ShowWindow(SW_HIDE);

	return TRUE;
}

HBRUSH registerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_TXT_ERROR)
	{
		pDC->SetTextColor(RGB(236, 70, 34));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}

	return hbr;
}

void registerDlg::OnOK()
{
	OnBnClickedBtnRegister();
}

void registerDlg::OnBnClickedBtnRegister()
{
	CString name, username, password, confirmPassword;

	_edt_name.GetWindowText(name);
	_edt_username.GetWindowText(username);
	_edt_password.GetWindowText(password);
	_edt_confirm_password.GetWindowText(confirmPassword);
	_txt_error.ShowWindow(SW_HIDE);

	if (name.IsEmpty()) {
		_txt_error.ShowWindow(SW_SHOW);
		_txt_error.SetWindowText(_T("Tên hiển thị không được để trống"));
		return;
	}
	else if (username.Trim().IsEmpty()) {
		_txt_error.ShowWindow(SW_SHOW);
		_txt_error.SetWindowText(_T("Tên đăng nhập không được để trống"));
		return;
	}
	else if (password.IsEmpty()) {
		_txt_error.ShowWindow(SW_SHOW);
		_txt_error.SetWindowText(_T("Mật khẩu không được để trống"));
		return;
	}
	else if (confirmPassword.IsEmpty()) {
		_txt_error.ShowWindow(SW_SHOW);
		_txt_error.SetWindowText(_T("Nhập lại mật khẩu không được để trống"));
		return;
	}

	if (!password.IsEmpty() && !confirmPassword.IsEmpty()) {
		if (password.Compare(confirmPassword) != 0) {
			_txt_error.ShowWindow(SW_SHOW);
			_txt_error.SetWindowText(_T("Mật khẩu không khớp !"));
			return;
		}
		else {
			json response;
			CString errerMess;
			if (!Register(name, username, password, response, errerMess)) {
				_txt_error.ShowWindow(SW_SHOW);
				_txt_error.SetWindowText(errerMess);
				return;
			}

			MessageBox(_T("Đăng ký thành công!"), _T("Thông báo"), MB_OK | MB_ICONINFORMATION);
			this->EndDialog(IDOK);
		}
	}

}

//----------------------Get API--------------------------
BOOL registerDlg::Register(const CString& name, const CString& username, const CString& password, json& response, CString& errorMessage) 
{
	CURL* curl = nullptr;
	CURLcode res = CURLE_OK;
	std::string response_str;
	long http_code = 0;

	try {
		curl = curl_easy_init();

		json requestBody;
		requestBody["FullName"] = CStringA(name).GetString();
		requestBody["Username"] = CStringA(username).GetString();
		requestBody["Password"] = CStringA(password).GetString();
		std::string data = requestBody.dump();

		struct curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_URL, "http://30.30.30.85:8888/api/auth/register");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
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
		CString mess = Utf8ToCString(e.what());
		if (mess == "Username already exists") {
			errorMessage = _T("Tài khoản đã tồn tại !");
		}
		else
		{
			errorMessage = _T("Đăng ký thất bại do lỗi hệ thống!");
		}
		if (curl) curl_easy_cleanup(curl);
		return FALSE;
	}
}