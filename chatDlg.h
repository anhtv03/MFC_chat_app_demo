#pragma once
#include "afxdialogex.h"
#include "Message.h"
#include <afxbutton.h>
#include "models/json.hpp"
#include "ChatListStyle.h"
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
	virtual void OnOK();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnBnClickedBtnEmoji();
	afx_msg void OnBnClickedBtnImage();
	afx_msg void OnBnClickedBtnFile();
	LRESULT OnEmojiSelected(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	ChatListStyle _idc_list_chat;
	CEdit _idc_edt_message;
	CMFCButton _idc_btn_send;
	CMFCButton _idc_btn_emoji;
	CMFCButton _idc_btn_file;
	CMFCButton _idc_btn_image;


private:
	CString m_friendId;
	CString m_friendname;
	std::vector<Message> m_messages;
	CBrush m_hbrBackground;
public:
	void setIconButton(CMFCButton& _idc_button, HICON hicon);
	void StyleInputArea();
	void LoadChatMessages();
	BOOL getMessage(CString& friendId, CString& token, json& response, CString& errorMessage);
	BOOL sendMessage(CString& friendId, CString& content, std::vector<CString> files,
		CString& token, json& response, CString& errorMessage);
};
