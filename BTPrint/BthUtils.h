//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//#define SPRT_SDK
#define ZP_SDK

#pragma once
#include <winsock2.h>
#include <ws2bth.h>
#include <bthapi.h>
#include <bt_api.h>
#include <bt_sdp.h>
#include <assert.h>
#include "sdpcommon.h"
#ifdef ZP_SDK
#include "zp_sdk.h"		//sdk for printer
#endif
#ifdef SPRT_SDK
#include "PrintDll.h"
#pragma comment(lib,"PrintDll.lib")
#endif

#define MAX_NAME_SIZE 128
#define MAX_ADDR_SIZE 15
#define MAX_MESSAGE_SIZE 256
#define WM_SCAN_BLUETOOTH_DEVICE_FINISHED (WM_USER+100)
#define SDP_RECORD_SIZE 0x0000003f
#define ICHANNELOFFSET 40
#define DEFAULT_PIN	"0000"

struct DeviceList
{ 
	TCHAR bthName[40];
	BT_ADDR bthAddress;
	int nChannel;
	DeviceList *NextDevice;
};

struct DeviceInfo
{
	WCHAR DeviceName[MAX_NAME_SIZE];
	BT_ADDR DeviceAddr;
	int nChannel;
};
class BthUtils
{
public:
	BthUtils();
	~BthUtils();
	HWND hWnd;
	int GetLocalDevice(DeviceInfo *pLocalDeviceInfo);
	int SetLocalDevice(DeviceInfo *pLocalDeviceInfo);
	int OpenServerConnection(WCHAR *strGUID, void (*funcCallBackPtr)( WCHAR*));		//后续开发
	int SendMessageToServer(WCHAR *strGUID, WCHAR *szMessage, DWORD dwCounts,int iSelectedDeviceIndex);//后续开发
	UINT BlueToothSearchThread(LPVOID param);

	BOOL BlueToothSearch(HWND hWnd);
	int GetDeviceInfo(DeviceInfo *pPeerDevicesInfo);
	int BlueToothPairingAndCreateVirtualCom(BT_ADDR bt_addr,int cChannel,CHAR *szPIN);
	void BluetoothVirtualComDestroy(void);

private:
	DeviceList *m_pDeviceList, *m_pStart, *m_pEnd, *m_pCurrentDevice;
	int m_iNumDevices;
	void (*pCallBackFunction)( WCHAR* ) ;
	HANDLE m_hReadThread;
	SOCKET m_socketServer, m_socketClient;
	SOCKADDR_BTH m_saClient;
	DWORD m_dwBluetoothMode;
	HANDLE hCom;
	int RegisterService(BYTE *rgbSdpRecord, int cSdpRecord, int iChannelOffset, UCHAR channel);
	int OpenClientConnection(WCHAR *strGUID, int iSelectedDeviceIndex);
	int GetGUID(WCHAR *psz, GUID *pGUID) ;

	int DiscoverDevices();
	HRESULT FindRFCOMMChannel (unsigned char *pStream, int cStream, int *pChann);
	inline int GetNumDevices(){return m_iNumDevices;};
	int PairAndConnect(BT_ADDR bt_addr,int cChannel,CHAR *szPIN);
	static DWORD WINAPI ReadData(LPVOID voidArg);
	int IsRfcommUuid(NodeData *pNode) ;
	int GetChannel (NodeData *pChannelNode) ;
	int GetChannel(BT_ADDR *bt_addr);
	HRESULT ServiceAndAttributeSearch(UCHAR *szResponse, DWORD cbResponse,ISdpRecord ***pppSdpRecords,ULONG *pNumRecords
    );
};