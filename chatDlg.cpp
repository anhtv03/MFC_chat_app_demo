// chatDlg.cpp : implementation file
//

#include "pch.h"
#include "chat_app_demo.h"
#include "afxdialogex.h"
#include "chatDlg.h"
#include "Message.h"
#include "JsonHelper.h"


// chatDlg dialog

IMPLEMENT_DYNAMIC(chatDlg, CDialogEx)

chatDlg::chatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DIALOG, pParent)
{

}

chatDlg::chatDlg(int friendId, CString friendName, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_DIALOG, pParent)
{
	m_friendId = friendId;
	m_friendName = friendName;
}

chatDlg::~chatDlg()
{
	m_friendId = 0;
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


BOOL chatDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();
	SetWindowText(m_friendName);

	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);

	CString dirPath(exePath);
	dirPath = dirPath.Left(dirPath.ReverseFind(_T('\\')));
	CString filePath = dirPath + _T("\\assets\\messages.txt");

	CString friendID;
	friendID.Format(_T("%d"), m_friendId);

	m_messages = JsonHelper::ReadMessagesFromJsonFile(filePath, _T("1"), friendID);

	//===============set chat list=================
	_idc_list_chat.InsertColumn(0, _T(""), LVCFMT_LEFT, 600);

	for (const auto& item : m_messages)
	{
		_idc_list_chat.InsertItem(_idc_list_chat.GetItemCount(), item.content);
	}

	//===============set chat content=================
	CEdit* pEditCtrl = (CEdit*)GetDlgItem(IDC_EDT_MESSAGE);
	pEditCtrl->SetCueBanner(_T("Nhập tin nhắn..."));

	HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON_SEND);
	_idc_btn_send.SetIcon(hIcon);
	_idc_btn_send.SetWindowPos(nullptr, 0, 0, 30, 30, SWP_NOMOVE | SWP_NOZORDER);


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
	newMsg.id = _T("11");
	newMsg.myId = _T("1");
	CString friendIdStr;
	friendIdStr.Format(_T("%d"), m_friendId);
	newMsg.friendId = friendIdStr;
	newMsg.content = content;
	newMsg.isSend = 1;
	newMsg.createdAt = CTime::GetCurrentTime();
	newMsg.messageType = 0; 

	m_messages.push_back(newMsg);
	_idc_list_chat.InsertItem(_idc_list_chat.GetItemCount(), newMsg.content);
	_idc_edt_message.SetWindowText(_T(""));

}
