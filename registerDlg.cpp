// registerDlg.cpp : implementation file
//

#include "pch.h"
#include "chat_app_demo.h"
#include "afxdialogex.h"
#include "registerDlg.h"
#include "models/json.hpp"
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include "util.h"

using json = nlohmann::json;
using namespace web::http;
using namespace web::http::client;


// registerDlg dialog

IMPLEMENT_DYNAMIC(registerDlg, CDialogEx)

registerDlg::registerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REGISTER_DIALOG, pParent)
{

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
BOOL registerDlg::Register(const CString& name, const CString& username, const CString& password, json& response, CString& errorMessage) {
	try {
		http_client client(U("http://30.30.30.85:8888"));

		web::json::value requestBody;
		requestBody[U("FullName")] = web::json::value::string(std::wstring(name));
		requestBody[U("Username")] = web::json::value::string(std::wstring(username));
		requestBody[U("Password")] = web::json::value::string(std::wstring(password));

		auto requestTask = client.request(methods::POST, U("/api/auth/register"), requestBody)
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
		if (mess == "Username already exists") {
			errorMessage = _T("Tài khoản đã tồn tại !");
		}
		else
		{
			errorMessage = _T("Đăng ký thất bại do lỗi hệ thống!");
		}
		return FALSE;
	}
}