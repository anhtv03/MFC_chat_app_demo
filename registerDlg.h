#pragma once
#include "afxdialogex.h"
#include "ButtonStyle.h"
#include "models/json.hpp"

using json = nlohmann::json;
// registerDlg dialog

class registerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(registerDlg)

public:
	registerDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~registerDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REGISTER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
	CEdit _edt_username;
	CEdit _edt_password;
	CEdit _edt_confirm_password;
	CStatic _txt_error;
	CEdit _edt_name;
	ButtonStyle _btn_register;
	CFont _font_btn_register;
public:
	afx_msg void OnBnClickedBtnRegister();
	BOOL Register(const CString& name, const CString& username,
		const CString& password, json& response, CString& errorMessage);
};
