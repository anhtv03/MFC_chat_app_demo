// homeDlg.cpp : implementation file
//

#include "pch.h"
#include "chat_app_demo.h"
#include "afxdialogex.h"
#include "homeDlg.h"
#include <gdiplus.h>
#include <urlmon.h>
#include "chatDlg.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "urlmon.lib")

using namespace Gdiplus;

// homeDlg dialog

IMPLEMENT_DYNAMIC(homeDlg, CDialogEx)

homeDlg::homeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HOME_DIALOG, pParent), 
	m_avatarImage(nullptr),
	m_gdiplusToken(0)
{
}

homeDlg::~homeDlg()
{
	if (m_avatarImage)
	{
		delete m_avatarImage;
	}
	GdiplusShutdown(m_gdiplusToken);
}

void homeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_TITLE, _txt_title);
	DDX_Control(pDX, IDC_EDT_SEARCH, _edt_search);
	DDX_Control(pDX, IDC_TXT_LIST_TITLE, _txt_list_title);
	DDX_Control(pDX, IDC_TXT_NAME, _txt_name);
	DDX_Control(pDX, IDC_LIST_FRIEND, _idc_list_friend);
	DDX_Control(pDX, IDC_AVATAR, _idc_avatar);
}

BEGIN_MESSAGE_MAP(homeDlg, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FRIEND, &homeDlg::OnNMDblclkListFriend)
END_MESSAGE_MAP()

BOOL homeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	this->SetBackgroundColor(RGB(255, 255, 255));

	//set avatar for picture control
	GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);

	CString url = _T("https://res.cloudinary.com/djj5gopcs/image/upload/v1744612363/download20230704194701_ult1ta.png");
	CString localPath = _T("avatar.png");
	URLDownloadToFile(NULL, url, localPath, 0, NULL);

	m_avatarImage = Image::FromFile(localPath);

	//set font for title bkav chat
	_font_title.CreateFont(
		24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);
	_txt_title.SetFont(&_font_title);

	//set font for list friend title
	_font_list_title.CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Roboto")
	);
	_txt_list_title.SetFont(&_font_list_title);
	
	//set font for name
	_font_name.CreateFont(
		20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Inter")
	);
	_txt_name.SetFont(&_font_name);

	//===============set list friend=================
	_idc_list_friend.ModifyStyle(0, 0x0020 | LVS_REPORT );
	_idc_list_friend.InsertColumn(0, _T(""), LVCFMT_LEFT, 600);
	_idc_list_friend.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	CImageList imageList;
	imageList.Create(50, 50, ILC_COLOR32, 0, 10);
	_idc_list_friend.SetImageList(&imageList, LVSIL_SMALL);

	for (int i = 1; i <= 10; i++)
	{
		CString number;
		number.Format(_T("%d"), i + 1);
		_idc_list_friend.AddFriend(_T("Bạn ") + number, localPath);
	}

	return TRUE;
}

HBRUSH homeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_TITLE)
	{
		pDC->SetTextColor(RGB(20, 106, 224));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}
	return hbr;
}

void homeDlg::OnPaint()
{
	CPaintDC dc(this);
	CDialogEx::OnPaint();

	if (m_avatarImage)
	{
		CRect rect;
		_idc_avatar.GetWindowRect(&rect);
		ScreenToClient(&rect);

		int size = min(rect.Width(), rect.Height());

		Graphics graphics(dc);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		GraphicsPath path;
		path.AddEllipse(rect.left, rect.top, size, size);
		graphics.SetClip(&path);

		graphics.DrawImage(m_avatarImage, rect.left, rect.top, size, size);
	}
}


void homeDlg::OnNMDblclkListFriend(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int itemIndex = pNMItemActivate->iItem;

	if (itemIndex != -1) 
	{
		CString friendName = _idc_list_friend.GetItemText(9 - itemIndex, 0);
		chatDlg dlg(itemIndex + 2, friendName);
		dlg.DoModal();
	}
	*pResult = 0;
}

