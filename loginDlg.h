#pragma once
#include "afxwin.h"
#include "afxbutton.h"
#include "models/json.hpp"

using json = nlohmann::json;

class loginDlg : public CDialogEx
{
public:
    loginDlg(CWnd* pParent = nullptr);
    enum { IDD = IDD_LOGIN_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()
    CEdit _edt_username;
    CEdit _edt_password;
    CButton _chk_remember;
    CMFCButton _btn_login;
    CStatic _txt_error;
    CStatic _txt_title;
    CFont _font_title;
    CFont _font_button_login;

public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBnClickedBtnLogin();
    afx_msg void OnStnClickedTxtRegister();

    BOOL Login(const CString& username, const CString& password, json& response, CString& errorMessage);
};