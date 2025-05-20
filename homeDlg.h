#pragma once
#include "afxdialogex.h"
#include <afxwin.h>
#include <gdiplus.h>
#include "itemFriendStyle.h"
#include <cpprest/http_client.h>
#include "models/json.hpp"
#pragma comment(lib, "gdiplus.lib")

using json = nlohmann::json;
using namespace Gdiplus;

// homeDlg dialog

class homeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(homeDlg)

public:
	homeDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~homeDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HOME_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnPaint();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnNMDblclkListFriend(NMHDR* pNMHDR, LRESULT* pResult);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	ULONG_PTR m_gdiplusToken;
	Image* m_avatarImage;
	BOOL getRequest(const web::uri& endpoint, CString& token, json& response, CString& errorMessage);

private:
	CStatic _txt_title;
	CStatic _txt_list_title;
	CFont _font_title;
	CFont _font_list_title;
	CFont _font_name;
	CEdit _edt_search;
	CStatic _txt_name;
	itemFriendStyle	_idc_list_friend;
	CStatic _idc_avatar;
	GdiplusStartupInput m_gdiplusStartupInput;
	CImageList m_ImageList;
};
