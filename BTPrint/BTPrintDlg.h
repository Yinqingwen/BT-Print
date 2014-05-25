// BTPrintDlg.h : header file
//

#pragma once

// CBTPrintDlg dialog
class CBTPrintDlg : public CDialog
{
// Construction
public:
	CBTPrintDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BTPRINT_DIALOG };


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	//virtual void OnOK();
	virtual void OnCancel();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnBnClickedButtonPrint();
	afx_msg void OnBnClickedButtonBack();
	afx_msg void OnCbnSelchangeComboBtList();
	afx_msg LRESULT OnTestMessage(WPARAM wParam, LPARAM lParam);
};
