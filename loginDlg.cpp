// loginDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "chat_app_demo.h"
#include "loginDlg.h"
#include "afxdialogex.h"
#include "registerDlg.h"
#include "homeDlg.h"
#include "models/json.hpp"
#include <winhttp.h>
#include <atlconv.h>

#pragma comment(lib, "winhttp.lib")
using json = nlohmann::json;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Hàm chuyển đổi từ std::string (UTF-8) sang CString
CString Utf8ToCString(const std::string& utf8Str)
{
    int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (wideCharLen == 0) return CString();

    std::vector<WCHAR> wideCharBuf(wideCharLen);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideCharBuf.data(), wideCharLen);

    return CString(wideCharBuf.data());
}

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

    _txt_error.ShowWindow(SW_HIDE);

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

    json response;
    CString errorMessage;
    if (!Login(username, password, response, errorMessage))
    {
        _txt_error.ShowWindow(SW_SHOW);
        _txt_error.SetWindowText(errorMessage);
        return;
    }

    if (response.contains("status") && response["status"].is_number())
    {
        int status = response["status"].get<int>();
        if (status == 1)
        {
            if (response.contains("data") && response["data"].is_object())
            {
                json data = response["data"];
                CString token = Utf8ToCString(data["token"].get<std::string>());
                CString username = Utf8ToCString(data["Username"].get<std::string>());
                CString fullName = Utf8ToCString(data["FullName"].get<std::string>());
            }

            homeDlg homeDlg;
            homeDlg.DoModal();
        }
        else
        {
            if (response.contains("data") && response["data"].contains("message"))
            {
                std::string message = response["data"]["message"].get<std::string>();
                CString errorMsg = Utf8ToCString(message);
                _txt_error.ShowWindow(SW_SHOW);
                _txt_error.SetWindowText(errorMsg);
            }
            else
            {
                _txt_error.ShowWindow(SW_SHOW);
                _txt_error.SetWindowText(_T("Đăng nhập thất bại."));
            }
        }
    }
    else
    {
        _txt_error.ShowWindow(SW_SHOW);
        _txt_error.SetWindowText(_T("Phản hồi từ server không hợp lệ."));
    }
}

void loginDlg::OnStnClickedTxtRegister()
{
    registerDlg registerDlg;
    if (registerDlg.DoModal() == IDOK)
    {
    }
}

BOOL loginDlg::Login(const CString& username, const CString& password, json& response, CString& errorMessage)
{
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    BOOL bResults = FALSE;

    hSession = WinHttpOpen(L"WinHTTP Example/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
    {
        errorMessage = _T("Error initializing WinHTTP session.");
        return FALSE;
    }

    hConnect = WinHttpConnect(hSession, L"10.2.44.254", 8888, 0);
    if (!hConnect)
    {
        WinHttpCloseHandle(hSession);
        errorMessage = _T("Error connecting to server.");
        return FALSE;
    }

    hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/api/auth/login", NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest)
    {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        errorMessage = _T("Error opening request.");
        return FALSE;
    }

    json requestData;
    requestData["Username"] = CW2A(username.GetString());
    requestData["Password"] = CW2A(password.GetString());
    std::string jsonStr = requestData.dump();
    LPCSTR pszPostData = jsonStr.c_str();
    DWORD dwDataLength = static_cast<DWORD>(jsonStr.length());

    LPCWSTR pszHeaders = L"Content-Type: application/json";
    bResults = WinHttpSendRequest(hRequest, pszHeaders, -1, (LPVOID)pszPostData,
        dwDataLength, dwDataLength, 0);
    if (!bResults)
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        errorMessage = _T("Error sending request.");
        return FALSE;
    }

    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults)
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        errorMessage = _T("Error receiving response.");
        return FALSE;
    }

    DWORD dwSize = 0;
    std::string responseData;
    while (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0)
    {
        LPSTR pszOutBuffer = new char[dwSize + 1];
        if (!pszOutBuffer)
        {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            errorMessage = _T("Memory allocation failed.");
            return FALSE;
        }

        ZeroMemory(pszOutBuffer, dwSize + 1);
        DWORD dwDownloaded = 0;
        if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
        {
            responseData.append(pszOutBuffer, dwDownloaded);
        }
        delete[] pszOutBuffer;
    }

    try
    {
        response = json::parse(responseData);
    }
    catch (const json::parse_error& e)
    {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        CString errorDetail = Utf8ToCString(e.what());
        errorMessage.Format(_T("JSON parse error: %s"), errorDetail);
        return FALSE;
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return TRUE;
}