// BTPrintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CBTPrintApp.h"
#include "BTPrintDlg.h"
#include "BthUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BthUtils objBthUtils;
DeviceInfo *g_pLocalDeviceInfo=NULL;
HCURSOR hCurOld;
int DeviceNum=0;
int g_iSelectedDeviceIndex=0;
int nComPort=0;

// CBTPrintDlg dialog

CBTPrintDlg::CBTPrintDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBTPrintDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBTPrintDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBTPrintDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CBTPrintDlg::OnBnClickedButtonSearch)
	ON_BN_CLICKED(IDC_BUTTON_PRINT, &CBTPrintDlg::OnBnClickedButtonPrint)
	ON_BN_CLICKED(IDC_BUTTON_BACK, &CBTPrintDlg::OnBnClickedButtonBack)
	ON_MESSAGE(WM_SCAN_BLUETOOTH_DEVICE_FINISHED, OnTestMessage)
	ON_CBN_SELCHANGE(IDC_COMBO_BT_LIST, &CBTPrintDlg::OnCbnSelchangeComboBtList)
END_MESSAGE_MAP()

LRESULT CBTPrintDlg::OnTestMessage(WPARAM wParam, LPARAM lParam)
{
	HWND hWnd;
	DeviceNum=(int)lParam;

	g_pLocalDeviceInfo=(DeviceInfo*)malloc(sizeof(DeviceInfo)*DeviceNum);	//用于存储蓝牙搜索信息(名称,地址)
	memset(g_pLocalDeviceInfo,0,sizeof(DeviceInfo)*DeviceNum);
	objBthUtils.GetDeviceInfo(g_pLocalDeviceInfo);
	
	GetDlgItem(IDC_COMBO_BT_LIST,&hWnd);		//改变了hWnd的指向
	::SendMessage(hWnd,CB_RESETCONTENT,0,0);
	for(int i=0;i<DeviceNum;i++)
	{
		WCHAR szDeviceNameAddr[MAX_NAME_SIZE]=_T("");
		StringCchPrintf(szDeviceNameAddr, sizeof(szDeviceNameAddr), _T("%s:(%04x%08x)"), g_pLocalDeviceInfo[i].DeviceName, GET_NAP(g_pLocalDeviceInfo[i].DeviceAddr), GET_SAP(g_pLocalDeviceInfo[i].DeviceAddr));
		::SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)szDeviceNameAddr);
		::SendMessage(hWnd,CB_SETITEMDATA, i, (LPARAM)i);	//add index
	}

	if(g_pLocalDeviceInfo)
	{
		free(g_pLocalDeviceInfo);
		g_pLocalDeviceInfo=NULL;
	}
	
	SetCursor(hCurOld);
	::MessageBox(NULL,_T("搜索完毕!"),_T("OK"),0);
	hWnd=(HWND)wParam;
	::EnableWindow(::GetDlgItem(hWnd,IDC_BUTTON_SEARCH),TRUE);
	return 0L;
}

// CBTPrintDlg message handlers

BOOL CBTPrintDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	int     screenX     =     GetSystemMetrics(SM_CXSCREEN);   
	int     screenY     =     GetSystemMetrics(SM_CYSCREEN);   
	SetWindowPos(NULL,0,20,screenX,screenY,SWP_NOZORDER);

#ifdef ZP_SDK
	extern bool zp_sdk_init(HWND hWnd);
	zp_sdk_init(this->GetSafeHwnd());
#endif

	//auto load configure
	WCHAR szDeviceNameAddr[MAX_NAME_SIZE]=_T("");
	HWND hWnd=NULL;
	GetDlgItem(IDC_COMBO_BT_LIST,&hWnd);
	g_pLocalDeviceInfo=(DeviceInfo*)malloc(sizeof(DeviceInfo));
	memset(g_pLocalDeviceInfo,0,sizeof(DeviceInfo));
	if(!objBthUtils.GetLocalDevice(g_pLocalDeviceInfo))
	{
		StringCchPrintf(szDeviceNameAddr, sizeof(szDeviceNameAddr), _T("%s:(%04x%08x)"), g_pLocalDeviceInfo[0].DeviceName, GET_NAP(g_pLocalDeviceInfo[0].DeviceAddr), GET_SAP(g_pLocalDeviceInfo[0].DeviceAddr));
		::SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM)szDeviceNameAddr);
		::SendMessage(hWnd,CB_SETITEMDATA, 0, (LPARAM)0);	//add index
	}
	if(g_pLocalDeviceInfo)
		free(g_pLocalDeviceInfo);
	g_pLocalDeviceInfo=NULL;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

static void PrintExit()
{
#ifdef ZP_SDK
	extern void zp_sdk_deinit(void);
	zp_sdk_deinit();
#endif
}

void CBTPrintDlg::OnCancel()
{
	PrintExit();
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CBTPrintDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_BTPRINT_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_BTPRINT_DIALOG));
	}
}
#endif

static void GetTextFromFile(char **file)
{
	assert(*file != NULL);
	
}

static void Unicode2Ascii(unsigned short* src,char*tar)
{
	unsigned int n;
	n=WideCharToMultiByte(0,0,(WCHAR*)src,(unsigned int)-1, 0, 0,0,0);
	WideCharToMultiByte(0,0,(WCHAR*)src,(unsigned int)-1,(char*)tar,n,0,0);
	tar[n]=0;
}

static void PrintControl(void)
{
#ifdef ZP_SDK
	int MessageNum;
	while (1 == 1)
	{
again:
		MessageNum = zp_error_status(2000);
		if (MessageNum != 0)
		{
			switch (MessageNum)
			{
			case 1:
				if (MessageBox(NULL,L"打印机纸舱盖开",L"错误",MB_OKCANCEL)==IDOK)
					goto again;
				else
					goto exit;
			case 2:
				if (MessageBox(NULL,L"打印机缺纸", L"错误", MB_OKCANCEL)==IDOK)
					goto again;
				else
					goto exit;
			case 4:
				if (MessageBox(NULL,L"打印机过热", L"错误",MB_OKCANCEL)==IDOK)
					goto again;
				else
					goto exit;
			default:
				break;
			}
		}
		break;
	}
reprint:
	zp_page_print();
	zp_printer_status_detect();
	MessageNum = zp_printer_status_get(8000);
	if (MessageNum != 0)
	{
		switch (MessageNum)
		{
		case 1:
			if (MessageBox(NULL,L"打印机纸舱盖开",L"错误",MB_OKCANCEL)==IDOK)
				goto reprint;
			else
				goto exit;
		case 2:
			if (MessageBox(NULL,L"打印机缺纸", L"错误", MB_OKCANCEL)==IDOK)
				goto reprint;
			else
				goto exit;
		case 4:
			if (MessageBox(NULL,L"打印机过热", L"错误",MB_OKCANCEL)==IDOK)
				goto reprint;
			else
				goto exit;

		default:
			break;
		}
	}
exit:
	zp_page_free();
#endif

#ifdef SPRT_SDK
#endif
}

static void PrintTest(int nComPort)
{
#ifdef ZP_SDK
	WCHAR *barcode=_T("12234565");

	if(zp_open(nComPort,115200))	//打开成功
	{
		if (zp_page_create(80, 15, false) == false)  
			::MessageBox(0,L"creat page failed!",0,0);
		zp_page_clear();

		zp_page_set_font(L"宋体", 3, 0, false, false, false);
		zp_draw_barcode(2, 5, L"123456789012", BARCODE_CODE128, 6, 3, 0);
		zp_draw_text(6, 11, L"S/N:123456789012");

		zp_page_print();
		zp_page_free();
		//PrintControl();
		zp_close();
	}else
	{
		::MessageBox(0,L"Print Open failed!",0,0);
	}
#endif

#ifdef SPRT_SDK
		//打印机设置
	PrinterInfo pf;
	FontProperty fontProperty;
	memset(&pf,0,sizeof(pf));
	pf.paperHeight = 300;
	pf.paperWidth = 0;
	pf.pcs = 0;
	pf.port = nComPort;
	pf.printOrient = SPRT_PO_PORTRAIT;
	pf.printType = SPRT_PT_TIII_TIV;
	pf.bRoll = 1;

	if(!DC_PrinterSet(pf))
	{
		MessageBox(NULL,L"PrinterSet Error",L"Error",MB_OK);
	}

	if(!DC_StartDoc())
	{
		MessageBox(NULL,L"StartDoc Error",L"Error",MB_OK);
	}
	if(!DC_StartPage())
	{
		MessageBox(NULL,L"StartPage Error",L"Error",MB_OK);
	}
	if(!DC_SetMapMode(SPRT_MM_MM))
	{
		MessageBox(NULL,L"SetMapMode Error",L"Error",MB_OK);
	}

    fontProperty.bDefault = FALSE;
    fontProperty.bItalic = TRUE;
    fontProperty.bStrikeout = FALSE;
    fontProperty.bUnderLine = TRUE;
    fontProperty.iCharSet = 0;
    fontProperty.nWidth = 15;
    fontProperty.nHeight = 36;
    fontProperty.iWeight = 900;

    if(!DC_SetFont(fontProperty))
	{
		MessageBox(NULL,L"SetFont Error",L"Error",MB_OK);
	}

	DC_DrawCode128(L"12345678", 0, 30, TRUE,TRUE,1);

	if(!DC_EndDoc())
	{
		MessageBox(NULL,L"EndDoc Error",L"Error",MB_OK);
	}
#endif
}

void CBTPrintDlg::OnBnClickedButtonSearch()
{
	// TODO: Add your control notification handler code here
	//进行搜索,取得蓝牙个数
	HWND hWnd=NULL;
	hWnd=this->GetSafeHwnd();
	if(objBthUtils.BlueToothSearch(hWnd))
	{
		::MessageBox(NULL,L"BlueTooth Search Error",L"Error",MB_OK);
		return;
	}

	hCurOld=SetCursor(LoadCursor(NULL, IDC_WAIT));
	::EnableWindow(::GetDlgItem(hWnd,IDC_BUTTON_SEARCH),FALSE);
}

static BOOL CALLBACK DlgProc2 (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        SetWindowText (GetDlgItem (hWnd, IDC_CHANNEL), L"");
        SetFocus (GetDlgItem (hWnd, IDC_CHANNEL));

        return 0;

    case WM_COMMAND:
        {
            int wID = LOWORD(wParam);
            switch (wID)
            {
            case IDOK:
                {
                    WCHAR szChannel[64];
                    GetWindowText (GetDlgItem (hWnd, IDC_CHANNEL), szChannel, 64);
                    int c = _wtoi (szChannel);
                    EndDialog (hWnd, c);
                }
                return 0;

            case IDCANCEL:
                EndDialog (hWnd, 0);
                return 0;
            }
        }
        break;
    }

    return 0;
}

void CBTPrintDlg::OnBnClickedButtonPrint()
{
	// TODO: Add your control notification handler code here
	WCHAR szPIN[5] ={0};
	CHAR PIN[4]={0};
	int nComPort=0;
	int nChannel=0;
	WCHAR DeviceName[MAX_NAME_SIZE]={0};
	BT_ADDR bt_addr=0;
	GetDlgItemText(IDC_EDIT1,szPIN,sizeof(szPIN));			//Get PIN input
	szPIN[4]=_T('\0');
	//字符转换
	Unicode2Ascii((unsigned short*)szPIN,PIN);

	//搜索过
	if(DeviceNum)
	{
		g_pLocalDeviceInfo=(DeviceInfo*)malloc(sizeof(DeviceInfo)*DeviceNum);
		memset(g_pLocalDeviceInfo,0,sizeof(DeviceInfo)*DeviceNum);
		objBthUtils.GetDeviceInfo(g_pLocalDeviceInfo);

		for(int i=0;i<DeviceNum;i++)
		{
			if(g_iSelectedDeviceIndex == i)
			{
				bt_addr = g_pLocalDeviceInfo[i].DeviceAddr;
				nChannel = objBthUtils.GetChannel(&bt_addr);
				memcpy(DeviceName,g_pLocalDeviceInfo[i].DeviceName,sizeof(g_pLocalDeviceInfo[i].DeviceName));
				break;
			}
		}
	}else			//从注册表查找记录
	{
		g_pLocalDeviceInfo=(DeviceInfo*)malloc(sizeof(DeviceInfo));
		memset(g_pLocalDeviceInfo,0,sizeof(DeviceInfo));
		if(!objBthUtils.GetLocalDevice(g_pLocalDeviceInfo))
		{
			bt_addr=g_pLocalDeviceInfo->DeviceAddr;
			nChannel=g_pLocalDeviceInfo->nChannel;
			memcpy(DeviceName,g_pLocalDeviceInfo->DeviceName,sizeof(g_pLocalDeviceInfo->DeviceName));
		}else
		{
			::MessageBox(NULL,_T("请重新查找蓝牙"),0,0);
			goto Exit;
		}
	}

	if(!nChannel)
	{
		nChannel= DialogBox(AfxGetInstanceHandle(),MAKEINTRESOURCE (IDD_ENTERCHANNEL), NULL, DlgProc2);
	}

	if(bt_addr <= 0)
	{
		::MessageBox(NULL,_T("蓝牙地址错误"),_T("Error"),0);
		goto Exit;
	}

	if(nChannel <=0)
	{
		::MessageBox(NULL,_T("传输通道错误"),_T("Error"),0);
		goto Exit;
	}

	nComPort=objBthUtils.BlueToothPairingAndCreateVirtualCom(bt_addr,nChannel,(PIN != NULL)?(PIN):(DEFAULT_PIN));
	if(nComPort >=0)
	{
		PrintTest(nComPort);

		//存储记录
		objBthUtils.SetLocalDevice(&g_pLocalDeviceInfo[g_iSelectedDeviceIndex]);
		objBthUtils.BluetoothVirtualComDestroy();//释放虚拟串口
	}else
	{
		::MessageBox(NULL,_T("虚拟串口创建失败,请重试"),_T("Error"),MB_OK);
		goto Exit;
	}

Exit:
	if(g_pLocalDeviceInfo)
	{
		free(g_pLocalDeviceInfo);
		g_pLocalDeviceInfo=NULL;
	}
}

void CBTPrintDlg::OnBnClickedButtonBack()
{
	// TODO: Add your control notification handler code here
	PrintExit();
	if(g_pLocalDeviceInfo)
	{
		free(g_pLocalDeviceInfo);
		g_pLocalDeviceInfo=NULL;
	}	
}

void CBTPrintDlg::OnCbnSelchangeComboBtList()
{
	// TODO: Add your control notification handler code here
	HWND hWnd;
	GetDlgItem(IDC_COMBO_BT_LIST,&hWnd);
	g_iSelectedDeviceIndex=::SendMessage(hWnd,CB_GETCURSEL,0,0);		//Get Select item
}
