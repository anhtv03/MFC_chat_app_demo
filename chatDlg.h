#pragma once
#include "afxdialogex.h"
#include "Message.h"
#include <afxbutton.h>
#include <cpprest/http_client.h>
#include "models/json.hpp"
#pragma comment(lib, "gdiplus.lib")

using json = nlohmann::json;
using namespace Gdiplus;

// chatDlg dialog

class chatDlg : public CDialogEx
{
	DECLARE_DYNAMIC(chatDlg)

public:
	chatDlg(CString friendId, CString friendName, CWnd* pParent = nullptr);
	virtual ~chatDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	CListCtrl _idc_list_chat;
	CEdit _idc_edt_message;
	CMFCButton _idc_btn_send;
	CMFCButton _idc_btn_emoji;
	CMFCButton _idc_btn_file;
	CMFCButton _idc_btn_image;

private:
	CString m_friendId;
	CString m_friendname;
	std::vector<Message> m_messages;
public:
	afx_msg void OnBnClickedBtnSend();
	void setIconButton(CMFCButton& _idc_button, HICON hicon);
	BOOL getMessage(CString& friendId, CString& token, json& response, CString& errorMessage);
};
