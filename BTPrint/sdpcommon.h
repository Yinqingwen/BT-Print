//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
// SdpCommon.h: Commonly used functions...


ISdpRecord *CreateSDPRecordFromStream(UCHAR *szResponse, DWORD cbResponse);

STDMETHODIMP
ServiceAndAttributeSearchParse(
    UCHAR *szResponse,             // in - response returned from SDP ServiceAttribute query
    DWORD cbResponse,            // in - length of response
    ISdpRecord ***pppSdpRecords, // out - array of pSdpRecords
    ULONG *pNumRecords           // out - number of elements in pSdpRecords
    );


// Function type for when printing to PrintRecordInfo
typedef void (*PFN_SDPPRINT)(PVOID pvContext, BOOL fError, WCHAR *szFormat,...);

void PrintRecordInfo(ISdpRecord *pRecord, int iRecordIndex, PFN_SDPPRINT pfnPrint, PVOID pvContext);

const WCHAR *GetAttribFromID(USHORT usAttribId);


void SdpPrintAttribUUIDs(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext);
const WCHAR *GetErrorFromId(USHORT usErrCode);
const WCHAR *GetPduNameFromId(UCHAR id);



// When calling ISdpRecord::GetAttribute(), if the returned type is to be an URL 
// or STRING the function allocates the string.  This simple wrapper guarantees
// that the string is automatically freed.  Also use this for GetNodeStringData()

class CNodeDataFreeString : public NodeData {
public:
	CNodeDataFreeString() {
		type = SDP_TYPE_NIL;
	}

	~CNodeDataFreeString() {
		if (type == SDP_TYPE_STRING)
			CoTaskMemFree(u.str.val);
		else if (type == SDP_TYPE_URL)
			CoTaskMemFree(u.url.val);
	}
};
