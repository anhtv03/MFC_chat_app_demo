#pragma once
#include "afxdialogex.h"
#include "Message.h"


// chatDlg dialog

class chatDlg : public CDialogEx
{
	DECLARE_DYNAMIC(chatDlg)

public:
	chatDlg(CWnd* pParent = nullptr);   // standard constructor
	chatDlg(int friendId, CString friendName, CWnd* pParent = nullptr);
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
	CButton _idc_btn_send;
	CMFCButton _idc_btn_emoji;
	CButton _idc_btn_file;
	CButton _idc_btn_image;

private:
	int m_friendId;
	CString m_friendName;
	std::vector<Message> m_messages;
public:
	afx_msg void OnBnClickedBtnSend();
};
