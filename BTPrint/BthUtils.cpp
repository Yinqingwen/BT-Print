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


#include "stdafx.h"
#include <windows.h>
#include <assert.h>
#include "BthUtils.h"

#pragma comment(lib,"ws2.lib")
#pragma comment(lib,"btdrt.lib")
#pragma comment(lib,"bthguid.lib")

unsigned char rgbSdpRecord[SDP_RECORD_SIZE] = {
	0x35, 0x3d, 0x09, 0x00, 0x01, 0x35, 0x11, 0x1c, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // service UUID goes here (+8)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x09, 0x00, 0x04, 0x35, 0x0c, 0x35, 0x03, 0x19, 
	0x01, 0x00, 0x35, 0x05, 0x19, 0x00, 0x03, 0x08, 
	0x00,											// server channel goes here (+40)
	0x09, 0x00, 0x06, 0x35, 0x09, 0x09, 0x65,
	0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, 0x00, 0x09, 
	0x01, 0x00, 0x25, 0x03, 0x53, 0x53, 0x41
};

//#pragma comment(lib,"btdrt.lib")
//Function: BthUtils (constructor)
//Purpose:	Initialize: Winsock and class data members
//				Turn on Bluetooth and set it discoverable mode, if not on already
BthUtils::BthUtils()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD( 2, 2 );
	WSAStartup( wVersionRequested, &wsaData );

	m_pDeviceList			= NULL;
	m_pStart				= NULL;
	m_pEnd					= NULL;
	m_pCurrentDevice		= NULL;
	m_iNumDevices			= 0;
	pCallBackFunction		= NULL;
	m_hReadThread			= NULL;
	m_socketServer			= INVALID_SOCKET;
	m_socketClient			= INVALID_SOCKET;
	hCom					= NULL;
	hWnd					= NULL;
#if 0
	BthGetMode(&m_dwBluetoothMode);
	if(m_dwBluetoothMode==BTH_POWER_OFF)
	{
		BthSetMode(BTH_DISCOVERABLE);
	}
#endif
}

//Function: ~BthUtils (destructor)
//Purpose:	Set radio mode back to original state
//				release the linked list, sockets
//				terminate the ReadThread thread


BthUtils::~BthUtils()
{
	//Set radio mode back to original state
#if 0
	BthSetMode(m_dwBluetoothMode);
#endif
	if(m_pStart)
	{
		for(m_pCurrentDevice	= m_pStart;m_pCurrentDevice;)
		{
			DeviceList *temp	= m_pCurrentDevice;
			m_pCurrentDevice	= m_pCurrentDevice->NextDevice;
			free(temp);
		}
		m_pStart=NULL;
	}

	if(m_socketClient)
		closesocket (m_socketClient);
	if(m_socketServer)
		closesocket (m_socketServer);

	//Terminate the read thread that receives chat messages from the client
	if(m_hReadThread)
	{
		DWORD dwExitCode = 0;
		TerminateThread(m_hReadThread, dwExitCode);
	}

	WSACleanup();

}

//Function: ReadThread
//Purpose: Receives messages from the peer device
DWORD WINAPI BthUtils::ReadData(LPVOID voidArg) 
{
	int iSize=0, cbBytesRecd=0 ;
	WCHAR szMessage[MAX_MESSAGE_SIZE];
	char pbuf[MAX_MESSAGE_SIZE];
	BthUtils *functionInfo = (BthUtils*) voidArg;
	iSize = sizeof(functionInfo->m_saClient);
	BTH_REMOTE_NAME remotename;
	memset(&remotename, sizeof(remotename), 0);

	SOCKET s = accept (functionInfo->m_socketServer, (SOCKADDR*)&(functionInfo->m_saClient), &iSize);
	remotename.bt = functionInfo->m_saClient.btAddr;

	setsockopt(functionInfo->m_socketServer,SOL_RFCOMM, SO_BTH_SET_READ_REMOTE_NAME, (char*)&remotename, sizeof(remotename));

	if (s != INVALID_SOCKET) 
	{
		for ( ; ; ) 
		{
			//receive data in pbuf
			cbBytesRecd = recv (s, pbuf, MAX_MESSAGE_SIZE, 0);
			//if error occured in receiving, return error code
			if (cbBytesRecd == SOCKET_ERROR) 
			{
				return WSAGetLastError();
			}
			else
			{
				// something was received, then copy the contents in szMessage
				if(cbBytesRecd>0)
				{
					StringCchPrintf (szMessage, sizeof(szMessage), L"%s", pbuf);
					(*functionInfo->pCallBackFunction)(szMessage);
				}
			}
		}
	}
	return 0;
}

//Function: DiscoverDevices
//Purpose:	Searches Bluetooth devices in range
//				Populates the link list with the name and address of the devices found
//Return: If error occurs, returns the appropriate WSAGetLastError, otherwise returns zero.

int BthUtils::DiscoverDevices()
{
	WSAQUERYSET		wsaq;
	HANDLE			hLookup;
	DeviceList *	tempDevice;

	union {
		CHAR buf[5000];
		double __unused;	// ensure proper alignment
	};

	LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
	DWORD dwSize  = sizeof(buf);
	BOOL bHaveName;

	ZeroMemory(&wsaq, sizeof(wsaq));
	wsaq.dwSize = sizeof(wsaq);
	wsaq.dwNameSpace = NS_BTH;
	wsaq.lpcsaBuffer = NULL;

	if (ERROR_SUCCESS != WSALookupServiceBegin (&wsaq, LUP_CONTAINERS, &hLookup))
	{
		return WSAGetLastError();
	}

	ZeroMemory(pwsaResults, sizeof(WSAQUERYSET));
	pwsaResults->dwSize = sizeof(WSAQUERYSET);
	pwsaResults->dwNameSpace = NS_BTH;
	pwsaResults->lpBlob = NULL;

	if(m_pStart)
	{
		for(m_pCurrentDevice=m_pStart;m_pCurrentDevice;)
		{
			DeviceList *temp=m_pCurrentDevice;
			m_pCurrentDevice=m_pCurrentDevice->NextDevice;
			free(temp);
		}
	}
	m_pEnd=m_pStart=NULL;
	m_iNumDevices=0;
	while (true)
	{	
		if(WSALookupServiceNext (hLookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pwsaResults)!=ERROR_SUCCESS)
			break;
		ASSERT (pwsaResults->dwNumberOfCsAddrs == 1);
		//Populate the link list		
		tempDevice=(DeviceList*)malloc(sizeof(DeviceList));
		tempDevice->NextDevice=NULL;
		if(m_pStart==NULL)
		{
			m_pStart = tempDevice;
			m_pEnd=m_pStart;
		}
		else
		{
			m_pEnd->NextDevice =tempDevice;
			m_pEnd=tempDevice;
		}
		m_iNumDevices++;
		m_pEnd->bthAddress = ((SOCKADDR_BTH *)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr)->btAddr;
		bHaveName = pwsaResults->lpszServiceInstanceName && *(pwsaResults->lpszServiceInstanceName);
		//If device name is available, add to node
		StringCchPrintf(m_pEnd->bthName, sizeof(m_pEnd->bthName),L"%s",bHaveName ? pwsaResults->lpszServiceInstanceName : L"");
	}

	WSALookupServiceEnd(hLookup);
	//	LeaveCriticalSection(&criticalSection);
	return 0;
}

//Function: OpenServerConnection
//Purpose:	Opens a server socket for listening. Registers the service. Creates a thread, ReadThread for reading incoming messages.
//Input:	The SDP record of the service to register, size of the SDP record, channel offset in the record, pointer to the UI function that displays the messages in the UI
//Return: If error occurs, returns the appropriate WSAGetLastError, otherwise returns zero.
//用于接收蓝牙数据
int BthUtils::OpenServerConnection(WCHAR *strGUID,void (*funcCallBackPtr)(WCHAR *))
{
	int iNameLen=0;
	if(m_socketServer==INVALID_SOCKET)
	{
		m_socketServer = socket (AF_BT, SOCK_STREAM, BTHPROTO_RFCOMM);
		if (m_socketServer  == INVALID_SOCKET) 
		{
			return WSAGetLastError ();
		}

		SOCKADDR_BTH sa;
		memset (&sa, 0, sizeof(sa));
		sa.addressFamily = AF_BT;
		sa.port = 0xFE;//0;
		if (bind (m_socketServer, (SOCKADDR *)&sa, sizeof(sa))) 
		{
			return WSAGetLastError ();
		}
		iNameLen = sizeof(sa);
		if (getsockname(m_socketServer, (SOCKADDR *)&sa, &iNameLen))	
		{
			return WSAGetLastError ();
		}

		GUID serviceClassId;
		GUID bigEndianGUID;
		GetGUID(strGUID,&serviceClassId);
		bigEndianGUID.Data1 =   ((serviceClassId.Data1 << 24) & 0xff000000) |
			((serviceClassId.Data1 << 8)  & 0x00ff0000) |
			((serviceClassId.Data1 >> 8)  & 0x0000ff00) |
			((serviceClassId.Data1 >> 24) & 0x000000ff);
		bigEndianGUID.Data2 =   ((serviceClassId.Data2 << 8) & 0xff00) |
			((serviceClassId.Data2 >> 8) & 0x00ff);
		bigEndianGUID.Data3 =   ((serviceClassId.Data3 << 8) & 0xff00) |
			((serviceClassId.Data3 >> 8) & 0x00ff);
		memcpy (bigEndianGUID.Data4, serviceClassId.Data4, sizeof(serviceClassId.Data4));
		memcpy (rgbSdpRecord + 8, &bigEndianGUID, sizeof(bigEndianGUID));

		if(RegisterService(rgbSdpRecord, SDP_RECORD_SIZE, ICHANNELOFFSET, (UCHAR)sa.port)!=0)
			return WSAGetLastError();

		if (listen (m_socketServer, SOMAXCONN)) 
		{
			return WSAGetLastError ();
		}
	}
	pCallBackFunction=funcCallBackPtr;
	m_hReadThread= CreateThread(NULL, 0, ReadData, (LPVOID)this, 0, NULL);

	return 0;
}

//Function: RegisterService
//Purpose:	Publishes the SDP record.
//Input:	The SDP record of the service to register, size of the SDP record, channel offset in the record, channel number assigned automatically by OpenServerConnection
//Return: If error occurs, returns the appropriate WSAGetLastError, otherwise returns zero.

int BthUtils::RegisterService(BYTE *rgbSdpRcd, int cSdpRecord, int iChannelOffset, UCHAR channel)
{
	ULONG recordHandle = 0;

	struct bigBlob
	{
		BTHNS_SETBLOB   b;

	}*pBigBlob;

	pBigBlob = (bigBlob *)malloc(sizeof(struct bigBlob)+cSdpRecord);
	ULONG ulSdpVersion = BTH_SDP_VERSION;
	pBigBlob->b.pRecordHandle   = &recordHandle;
	pBigBlob->b.pSdpVersion     = &ulSdpVersion;
	pBigBlob->b.fSecurity       = 0;
	pBigBlob->b.fOptions        = 0;
	pBigBlob->b.ulRecordLength  = cSdpRecord;

	memcpy (pBigBlob->b.pRecord, rgbSdpRcd, cSdpRecord);
	pBigBlob->b.pRecord[iChannelOffset] = (unsigned char)channel;
	BLOB blob;
	blob.cbSize    = sizeof(BTHNS_SETBLOB) + cSdpRecord - 1;
	blob.pBlobData = (PBYTE) pBigBlob;

	WSAQUERYSET Service;
	memset (&Service, 0, sizeof(Service));
	Service.dwSize = sizeof(Service);
	Service.lpBlob = &blob;
	Service.dwNameSpace = NS_BTH;
	if (WSASetService(&Service,RNRSERVICE_REGISTER,0) == SOCKET_ERROR)
	{
		free(pBigBlob);
		return WSAGetLastError();
	}
	else
	{
		free(pBigBlob);
		return 0;
	}
}



//Function: SendMessageToServer
//Purpose:	Opens a client socket to connect to the server. Called when the local device initiates the chat.
//Input:	string containing the GUID of the service running on the server that the client wants to connect.
//			iSelectedDeviceIndex is the selected device in the UI that the local device wants to connect. If the peer device initiates the chat, the this parameter is set to -1.							
//Return: If error occurs, returns the appropriate WSAGetLastError, otherwise returns zero.

int BthUtils::SendMessageToServer(WCHAR *strGUID, WCHAR *szMessage, DWORD dwCounts, int iSelectedDeviceIndex)
{
	int iRetVal=0, iBytesSent=0 ;
	if(m_socketClient==INVALID_SOCKET)
	{
		iRetVal=OpenClientConnection(strGUID, iSelectedDeviceIndex);
		if(iRetVal!=0)
		{
			return iRetVal;
		}
	}

	if (dwCounts > 0) 
	{
		iBytesSent = send (m_socketClient, (char *)szMessage, dwCounts, 0);
		if (iBytesSent != dwCounts)
		{
			return WSAGetLastError();
		}
	}
	return 0;
}

//Function: OpenClientConnection
//Purpose:	Opens a client socket to connect to the server.
//Input:	string containing the GUID of the service running on the server that the client wants to connect.
//			iSelectedDeviceIndex is the selected device in the UI that the local device wants to connect. If the peer device initiates the chat, the this parameter is set to -1.							
//Return: If error occurs, returns the appropriate WSAGetLastError, otherwise returns zero.
int BthUtils::OpenClientConnection(WCHAR *strGUID, int iSelectedDeviceIndex)
{
	if (m_socketClient==INVALID_SOCKET)
	{
		GUID ServerGuid;

		if(GetGUID(strGUID, &ServerGuid))
			return -1;
		m_socketClient = socket (AF_BT, SOCK_STREAM, BTHPROTO_RFCOMM);

		if (m_socketClient == INVALID_SOCKET) 
		{
			return WSAGetLastError();
		}

		SOCKADDR_BTH sa;

		memset (&sa, 0, sizeof(sa));
		sa.addressFamily = AF_BT;			
		//Search for the selected device in the list box in the link list
		m_pCurrentDevice=m_pStart;
		sa.serviceClassId=ServerGuid;

		if(iSelectedDeviceIndex==-1)
		{
			sa.btAddr=m_saClient.btAddr;
		}
		else
		{
			for (int iCount = 0 ;(m_pCurrentDevice)&&iCount!=iSelectedDeviceIndex;m_pCurrentDevice=m_pCurrentDevice->NextDevice,iCount++);
			sa.btAddr = m_pCurrentDevice->bthAddress;
		}

		if (connect (m_socketClient, (SOCKADDR *)&sa, sizeof(sa)) == SOCKET_ERROR) 
		{
			m_socketClient=INVALID_SOCKET;
			return WSAGetLastError();
		}
	}
	return 0;
}

//Function: GetGUID
//Purpose:	Conversts a string containing the GUID into a GUID datatype.
//Input:		string cotaining the GUID
//Output:	GUID type
//Return: Returns -1 in case of an error, otherwise returns zero.

int BthUtils::GetGUID(WCHAR *psz, GUID *pGUID) 
{
	int data1, data2, data3;
	int data4[8];

	if (11 ==  swscanf(psz, L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
		&data1, &data2, &data3,
		&data4[0], &data4[1], &data4[2], &data4[3], 
		&data4[4], &data4[5], &data4[6], &data4[7])) {
			pGUID->Data1 = data1;
			pGUID->Data2 = data2 & 0xffff;
			pGUID->Data3 = data3 & 0xffff;

			for (int i = 0 ; i < 8 ; ++i)
				pGUID->Data4[i] = data4[i] & 0xff;

			return TRUE;
	}
	return FALSE;
}

//Function: GetLocalDeviceName
//Purpose:	Returns the name of the owner set in the registry
//Output:	DeviceInfo: (only)name
//Return: Returns -1 in case of an error, otherwise returns zero.
//如果需要,从注册表中读取历史的蓝牙连接设备
int BthUtils::GetLocalDevice(DeviceInfo *pLocalDeviceInfo)
{
	HKEY hKey;
	int iRetVal=0;
	DWORD dwRegType, dwRegSize;
	if(RegOpenKeyEx(HKEY_CURRENT_USER,L"ControlPanel\\Owner",0,0,&hKey)==ERROR_SUCCESS)
	{
		dwRegSize=sizeof(pLocalDeviceInfo->DeviceName);
		if(RegQueryValueEx(hKey,L"Name",0,&dwRegType,(LPBYTE)pLocalDeviceInfo->DeviceName,&dwRegSize)==ERROR_SUCCESS)
		{
			if(dwRegSize <=0)
			{
				iRetVal=-1;
			}
		}

		dwRegSize=sizeof(pLocalDeviceInfo->DeviceAddr);
		if(RegQueryValueEx(hKey,L"Addr",0,&dwRegType,(LPBYTE)&pLocalDeviceInfo->DeviceAddr,&dwRegSize)==ERROR_SUCCESS)
		{
			if(dwRegSize <=0)
			{
				iRetVal=-1;
			}
		}

		dwRegSize=sizeof(pLocalDeviceInfo->nChannel);
		if(RegQueryValueEx(hKey,L"Channel",0,&dwRegType,(LPBYTE)&pLocalDeviceInfo->nChannel,&dwRegSize)==ERROR_SUCCESS)
		{
			if(dwRegSize <=0)
			{
				iRetVal=-1;
			}
		}

		RegCloseKey(hKey);
	}else
	{
		iRetVal=-1;
	}
	return iRetVal;
}

//Function: SetLocalDevice
//Purpose:	Store the name of the owner set in the registry
//Input:	DeviceInfo *
//Return: Returns -1 in case of an error, otherwise returns zero.
int BthUtils::SetLocalDevice(DeviceInfo *pLocalDeviceInfo)
{
	HKEY hKey;
	DWORD dwRegType, dwRegSize,dwDisp;

	if(RegOpenKeyEx(HKEY_CURRENT_USER,L"ControlPanel\\Owner",0,0,&hKey)==ERROR_SUCCESS)
	{
		if(RegQueryValueEx(hKey,L"Name",0,&dwRegType,(LPBYTE)pLocalDeviceInfo->DeviceName,&dwRegSize)==ERROR_SUCCESS)
		{
			if (dwRegSize>0) 
			{
				RegDeleteValue(hKey,L"Name");			
			}
			RegSetValueEx(hKey,L"Name",0,dwRegType,(LPBYTE)pLocalDeviceInfo->DeviceName,sizeof(pLocalDeviceInfo->DeviceName));
			//RegCloseKey(hKey);
		}
		if(RegQueryValueEx(hKey,L"Addr",0,&dwRegType,(LPBYTE)pLocalDeviceInfo->DeviceAddr,&dwRegSize)==ERROR_SUCCESS)
		{
			if (dwRegSize>0) 
			{
				RegDeleteValue(hKey,L"Addr");			
			}
			RegSetValueEx(hKey,L"Addr",0,dwRegType,(LPBYTE)pLocalDeviceInfo->DeviceAddr,sizeof(pLocalDeviceInfo->DeviceAddr));
			//RegCloseKey(hKey);
		}
		if(RegQueryValueEx(hKey,L"Channel",0,&dwRegType,(LPBYTE)pLocalDeviceInfo->nChannel,&dwRegSize)==ERROR_SUCCESS)
		{
			if(dwRegSize > 0)
			{
				RegDeleteValue(hKey,L"Channel");
			}
			RegSetValueEx(hKey,L"Channel",0,dwRegType,(LPBYTE)pLocalDeviceInfo->nChannel,sizeof(pLocalDeviceInfo->nChannel));
			//RegCloseKey(hKey);
		}

		RegCloseKey(hKey);
	}else
	{
		if(ERROR_SUCCESS == RegCreateKeyEx (HKEY_CURRENT_USER,L"ControlPanel\\Owner", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
		{
			RegSetValueEx (hKey, L"Name", 0, REG_SZ, (BYTE *)pLocalDeviceInfo->DeviceName, sizeof(pLocalDeviceInfo->DeviceName));
			RegSetValueEx (hKey, L"Addr", 0, REG_DWORD, (BYTE *)&pLocalDeviceInfo->DeviceAddr, sizeof(pLocalDeviceInfo->DeviceAddr));
			RegSetValueEx (hKey, L"Channel",0,REG_DWORD, (BYTE *)&pLocalDeviceInfo->nChannel, sizeof(pLocalDeviceInfo->nChannel));
		}
	}
	return 0;
}

int BthUtils::GetChannel (NodeData *pChannelNode) 
{
	if (pChannelNode->specificType == SDP_ST_UINT8)
		return pChannelNode->u.uint8;
	else if (pChannelNode->specificType == SDP_ST_INT8)
		return pChannelNode->u.int8;
	else if (pChannelNode->specificType == SDP_ST_UINT16)
		return pChannelNode->u.uint16;
	else if (pChannelNode->specificType == SDP_ST_INT16)
		return pChannelNode->u.int16;
	else if (pChannelNode->specificType == SDP_ST_UINT32)
		return pChannelNode->u.uint32;
	else if (pChannelNode->specificType == SDP_ST_INT32)
		return pChannelNode->u.int32;

	return 0;
}

HRESULT BthUtils::FindRFCOMMChannel (unsigned char *pStream, int cStream, int *pChann) {
	ISdpRecord **pRecordArg;
	int cRecordArg = 0;
	*pChann = 0;
	HRESULT hr = ServiceAndAttributeSearch (pStream, cStream, &pRecordArg, (ULONG *)&cRecordArg);
	if (FAILED(hr))
		return hr;

	for (int i = 0; (! *pChann) && (i < cRecordArg); i++) {
		ISdpRecord *pRecord = pRecordArg[i];    // particular record to examine in this loop
		CNodeDataFreeString protocolList;     // contains SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST data, if available

		if (ERROR_SUCCESS != pRecord->GetAttribute(SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST,&protocolList) ||
			(protocolList.type != SDP_TYPE_CONTAINER))
			continue;

		ISdpNodeContainer *pRecordContainer = protocolList.u.container;
		int cProtocols = 0;
		NodeData protocolDescriptor; // information about a specific protocol (i.e. L2CAP, RFCOMM, ...)

		pRecordContainer->GetNodeCount((DWORD *)&cProtocols);
		for (int j = 0; (! *pChann) && (j < cProtocols); j++) {
			pRecordContainer->GetNode(j,&protocolDescriptor);

			if (protocolDescriptor.type != SDP_TYPE_CONTAINER)
				continue;

			ISdpNodeContainer *pProtocolContainer = protocolDescriptor.u.container;
			int cProtocolAtoms = 0;
			pProtocolContainer->GetNodeCount((DWORD *)&cProtocolAtoms);

			for (int k = 0; (! *pChann) && (k < cProtocolAtoms); k++) {
				NodeData nodeAtom;  // individual data element, such as what protocol this is or RFCOMM channel id.

				pProtocolContainer->GetNode(k,&nodeAtom);

				if (IsRfcommUuid(&nodeAtom))  {
					if (k+1 == cProtocolAtoms) {
						// misformatted response.  Channel ID should follow RFCOMM uuid
						break;
					}

					NodeData channelID;
					pProtocolContainer->GetNode(k+1,&channelID);

					*pChann = (int)GetChannel(&channelID);
					break; // formatting error
				}
			}
		}
	}

	int i;
	for (i = 0; i < cRecordArg; i++) 
		pRecordArg[i]->Release();

	CoTaskMemFree(pRecordArg);

	return (*pChann != 0) ? NO_ERROR : E_FAIL;
}

int BthUtils::IsRfcommUuid(NodeData *pNode)  
{
	if (pNode->type != SDP_TYPE_UUID)
		return FALSE;

	if (pNode->specificType == SDP_ST_UUID16)
		return (pNode->u.uuid16 == RFCOMM_PROTOCOL_UUID16);
	else if (pNode->specificType == SDP_ST_UUID32)
		return (pNode->u.uuid32 == RFCOMM_PROTOCOL_UUID16);
	else if (pNode->specificType == SDP_ST_UUID128)
		return (0 == memcmp(&RFCOMM_PROTOCOL_UUID,&pNode->u.uuid128,sizeof(GUID)));

	return FALSE;
}

HRESULT BthUtils::ServiceAndAttributeSearch(
	UCHAR *szResponse,             // in - response returned from SDP ServiceAttribute query
	DWORD cbResponse,            // in - length of response
	ISdpRecord ***pppSdpRecords, // out - array of pSdpRecords
	ULONG *pNumRecords           // out - number of elements in pSdpRecords
	)
{
	*pppSdpRecords = NULL;
	*pNumRecords = 0;
	ISdpStream *pIStream = NULL;

	HRESULT hres = CoCreateInstance(__uuidof(SdpStream),NULL,CLSCTX_INPROC_SERVER,
		__uuidof(ISdpStream),(LPVOID *) &pIStream);

	if (FAILED(hres))
		return hres;  

	ULONG ulError;

	hres = pIStream->Validate (szResponse,cbResponse,&ulError);

	if (SUCCEEDED(hres)) {
		hres = pIStream->VerifySequenceOf(szResponse,cbResponse,
			SDP_TYPE_SEQUENCE,NULL,pNumRecords);

		if (SUCCEEDED(hres) && *pNumRecords > 0) {
			*pppSdpRecords = (ISdpRecord **) CoTaskMemAlloc(sizeof(ISdpRecord*) * (*pNumRecords));

			if (pppSdpRecords != NULL) {
				hres = pIStream->RetrieveRecords(szResponse,cbResponse,*pppSdpRecords,pNumRecords);

				if (!SUCCEEDED(hres)) {
					CoTaskMemFree(*pppSdpRecords);
					*pppSdpRecords = NULL;
					*pNumRecords = 0;
				}
			}
			else {
				hres = E_OUTOFMEMORY;
			}
		}
	}

	if (pIStream != NULL) {
		pIStream->Release();
		pIStream = NULL;
	}

	return hres;
}

//Function: GetDeviceInfo
//Purpose:	Returns name and address of all the devices in the link list in DeviceInfo. This is used by the UI to display the names and addresses of the devices found
//Output:	DeviceInfo: name and address
//Return: Success returns zero.
//获取已经搜索到的蓝牙的名字和地址,仅用于UI界面显示
int BthUtils::GetDeviceInfo(DeviceInfo *pPeerDevicesInfo)
{
	int iCtr=0;
	for (m_pCurrentDevice = m_pStart;(m_pCurrentDevice);m_pCurrentDevice=m_pCurrentDevice->NextDevice,iCtr++) 
	{
		StringCchPrintf(pPeerDevicesInfo[iCtr].DeviceName,sizeof(pPeerDevicesInfo[iCtr].DeviceName),L"%s",m_pCurrentDevice->bthName);
		pPeerDevicesInfo[iCtr].DeviceAddr=m_pCurrentDevice->bthAddress;
	}
	return 0;
}

int BthUtils::GetChannel(BT_ADDR *bt_addr)
{
	int iResult=0;
	int cChannel=0;
	//根据蓝牙地址获取channel
	CoInitializeEx (0, COINIT_MULTITHREADED);
	BTHNS_RESTRICTIONBLOB RBlob;
	memset (&RBlob, 0, sizeof(RBlob));
	RBlob.type = SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST;
	RBlob.numRange = 1;
	RBlob.pRange[0].minAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;
	RBlob.pRange[0].maxAttribute = SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST;
	RBlob.uuids[0].uuidType = SDP_ST_UUID16;
	RBlob.uuids[0].u.uuid16 = SerialPortServiceClassID_UUID16;

	BLOB blob;
	blob.cbSize = sizeof(RBlob);
	blob.pBlobData = (BYTE *)&RBlob;

	SOCKADDR_BTH    sa;
	memset (&sa, 0, sizeof(sa));
	*(BT_ADDR *)(&sa.btAddr) = *bt_addr;
	sa.addressFamily = AF_BT;
	CSADDR_INFO     csai;
	memset (&csai, 0, sizeof(csai));
	csai.RemoteAddr.lpSockaddr = (sockaddr *)&sa;
	csai.RemoteAddr.iSockaddrLength = sizeof(sa);

	WSAQUERYSET     wsaq;
	memset (&wsaq, 0, sizeof(wsaq));
	wsaq.dwSize      = sizeof(wsaq);
	wsaq.dwNameSpace = NS_BTH;
	wsaq.lpBlob      = &blob;
	wsaq.lpcsaBuffer = &csai;

	HANDLE hLookup;
	int iRet = WSALookupServiceBegin (&wsaq, 0, &hLookup);
	if (ERROR_SUCCESS == iRet) {
		CHAR buf[5000];
		LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
		DWORD dwSize  = sizeof(buf);
		memset(pwsaResults,0,sizeof(WSAQUERYSET));
		pwsaResults->dwSize      = sizeof(WSAQUERYSET);
		pwsaResults->dwNameSpace = NS_BTH;
		pwsaResults->lpBlob      = NULL;
		iRet = WSALookupServiceNext (hLookup, 0, &dwSize, pwsaResults);
		if (iRet == ERROR_SUCCESS) {    // Success - got the stream
			if (ERROR_SUCCESS == FindRFCOMMChannel (pwsaResults->lpBlob->pBlobData,
				pwsaResults->lpBlob->cbSize, &cChannel))
				iResult = cChannel;
		}
		WSALookupServiceEnd(hLookup);
	}
	CoUninitialize ();
	return iResult;
}

int GetBA (WCHAR *pp, BT_ADDR *pba) {
	while (*pp == ' ')
		++pp;
	for (int i = 0 ; i < 4 ; ++i, ++pp) {
		if (! iswxdigit (*pp))
			return FALSE;
		int c = *pp;
		if (c >= 'a')
			c = c - 'a' + 0xa;
		else if (c >= 'A')
			c = c - 'A' + 0xa;
		else c = c - '0';

		if ((c < 0) || (c > 16))
			return FALSE;
		*pba = *pba * 16 + c;
	}

	int i;
	for (i = 0 ; i < 8 ; ++i, ++pp) {
		if (! iswxdigit (*pp))
			return FALSE;
		int c = *pp;
		if (c >= 'a')
			c = c - 'a' + 0xa;
		else if (c >= 'A')
			c = c - 'A' + 0xa;
		else c = c - '0';

		if ((c < 0) || (c > 16))
			return FALSE;

		*pba = *pba * 16 + c;
	}

	if ((*pp != ' ') && (*pp != '\0'))
		return FALSE;
	return TRUE;
}

static unsigned long BlueToothSearchThread(LPVOID param)
{
	BthUtils *bth=reinterpret_cast<BthUtils*>(param);
	bth->BlueToothSearchThread(bth->hWnd);
	return 0;
}

int BthUtils::PairAndConnect(BT_ADDR bt_addr,int cChannel,CHAR *szPIN)
{
	//检查是否已经配对
	
	//从局部变量改为使用类成员变量
	m_socketClient = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if ( m_socketClient==INVALID_SOCKET ) {
		WCHAR szString[MAX_PATH];
		wsprintf(szString,L"socket() error is %d",GetLastError());
		MessageBox(NULL,szString,0,MB_OK|MB_TOPMOST); 
		return -1;
	}

	//密码配对
	BTH_SOCKOPT_SECURITY bth_sockopt;
	memset(&bth_sockopt, 0, sizeof(bth_sockopt));
	if(!szPIN)
	{
		bth_sockopt.iLength = strlen(DEFAULT_PIN);
		bth_sockopt.btAddr = bt_addr;
		memcpy(bth_sockopt.caData,DEFAULT_PIN,strlen(DEFAULT_PIN));
	}else
	{
		bth_sockopt.iLength = strlen(szPIN);
		bth_sockopt.btAddr = bt_addr;
		memcpy(bth_sockopt.caData,szPIN,bth_sockopt.iLength);
	}

	if(setsockopt(m_socketClient, SOL_RFCOMM, SO_BTH_SET_PIN, (char *)&bth_sockopt, sizeof(BTH_SOCKOPT_SECURITY)) == SOCKET_ERROR )
	{
		WCHAR szString[MAX_PATH];
		wsprintf(szString,L"配对失败,error is %d",GetLastError());
		MessageBox(NULL,szString,0,MB_OK|MB_TOPMOST); 
		return -1;
	}//配对结束

	GUID   guid;
	CoCreateGuid(&guid);			//guid自动生成
	SOCKADDR_BTH sa;
	memset (&sa, 0, sizeof(sa));
	sa.btAddr = bt_addr;
	sa.addressFamily = AF_BT;

	sa.port = cChannel & 0xff;
	sa.serviceClassId = guid;
	if (connect(m_socketClient, (SOCKADDR *)&sa, sizeof(sa))) {	//失败的原因是channel未设置
		WCHAR szString[MAX_PATH];
		closesocket (m_socketClient);
		wsprintf(szString,L"连接失败,error is %d",GetLastError());
		MessageBox(NULL,szString,0,MB_OK|MB_TOPMOST); 
		return -1 ;
	}
	closesocket (m_socketClient);		//不关闭的话虚拟串口创建成功了,但是会导致打印机端口打开失败
	CloseHandle((LPVOID)m_socketClient);
	m_socketClient=INVALID_SOCKET;
	return 0;
}

UINT BthUtils::BlueToothSearchThread(LPVOID param)
{
	int iRetVal=0,iNumDevices=0;
	HWND hWnd=(HWND)param;
	iRetVal= DiscoverDevices();
	iNumDevices = GetNumDevices();
	if(!iRetVal)
	PostMessage(hWnd,WM_SCAN_BLUETOOTH_DEVICE_FINISHED,(WPARAM)hWnd,(LPARAM)iNumDevices);

	return 0;
}

//搜索附近的蓝牙列表
BOOL BthUtils::BlueToothSearch(HWND hWnd)
{
	DWORD m_dwThreadID_ScanBth;
	this->hWnd=hWnd;
	::CreateThread(NULL,0,::BlueToothSearchThread,this,0, &m_dwThreadID_ScanBth);
	//AfxBeginThread((AFX_THREADPROC)::BlueToothSearchThread,this,0,0,0,0);
	return 0;
}

int BthUtils::BlueToothPairingAndCreateVirtualCom(BT_ADDR bt_addr,int cChannel,CHAR *szPIN)
{
	if(bt_addr <= 0)
	{
		MessageBox(NULL,_T("蓝牙地址为空,请从列表选择蓝牙!"),_T("Error"),MB_OK);
		return -1;
	}

	if(cChannel <= 0)
	{
		MessageBox(NULL,_T("通道错误,请检查参数!"),_T("Error"),MB_OK);
		return -1;
	}

	//配对
	int ComPort=0;
	if(!PairAndConnect(bt_addr,cChannel,szPIN))
	{
		PORTEMUPortParams pp;
		memset (&pp, 0, sizeof(pp));
		pp.device = bt_addr;
		pp.channel = cChannel;
		pp.uiportflags = 0;

		hCom = NULL;
		for (ComPort=9;ComPort>=0;ComPort--)
		{
			hCom = RegisterDevice (L"COM", ComPort, L"btd.dll", (DWORD)&pp);
			if(hCom != NULL)
				break;
		}
	}
	return ComPort;
}

void BthUtils::BluetoothVirtualComDestroy(void)
{
	if(hCom)
	DeregisterDevice(hCom);
	hCom=NULL;
}