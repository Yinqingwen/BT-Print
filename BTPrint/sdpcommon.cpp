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
// SdpCommon.cpp: Commonly used functions...

#include <stdafx.h>
#include <windows.h>
#include <bthapi.h>
#include <bt_sdp.h>
#include <svsutil.hxx>
#include "sdpcommon.h"

#if defined (WINCE_EMULATION)
#include <assert.h>
#else
#define assert ASSERT
#endif


inline USHORT RtlUshortByteSwap(IN USHORT Source) {
    USHORT swapped;
    swapped = ((Source) << (8 * 1)) | ((Source) >> (8 * 1));
    return swapped;
}

inline ULONG RtlUlongByteSwap(IN ULONG Source) {
	ULONG swapped;

	swapped = ((Source)              << (8 * 3)) |  ((Source & 0x0000FF00) << (8 * 1)) |
	          ((Source & 0x00FF0000) >> (8 * 1)) |  ((Source)              >> (8 * 3));

	return swapped;
}

inline ULONGLONG RtlByteSwap64(ULONGLONG Source) {
	ULONGLONG swapped;
	
	swapped = (ULONGLONG) (RtlUlongByteSwap((ULONG)(Source >> 32))) << 32;
	swapped = swapped | (RtlUlongByteSwap((ULONG)(Source & 0x00000000FFFFFFFF)));
	return swapped;
}

ISdpStream * AllocateSdpStream(void);
void FreeSdpStream(ISdpStream *pStream);


ISdpRecord *CreateSDPRecordFromStream(UCHAR *szResponse, DWORD cbResponse) {
	ISdpRecord *pIRecord = NULL;

	if (FAILED(CoCreateInstance(__uuidof(SdpRecord),NULL,CLSCTX_INPROC_SERVER,
	                        __uuidof(ISdpRecord),(LPVOID *) &pIRecord))) {
		return NULL;
	}

	if (FAILED(pIRecord->CreateFromStream(szResponse,cbResponse))) {
		pIRecord->Release();
		return NULL;
	}
	return pIRecord;
}

// 
// Takes a raw stream ServiceAttribute response from the server and converts 
// it into an array of ISdpRecord elements to facilitate manipulation.
// 
STDMETHODIMP
ServiceAndAttributeSearchParse(
    UCHAR *szResponse,             // in - response returned from SDP ServiceAttribute query
    DWORD cbResponse,            // in - length of response
    ISdpRecord ***pppSdpRecords, // out - array of pSdpRecords
    ULONG *pNumRecords           // out - number of elements in pSdpRecords
    )
{
    HRESULT hres = E_FAIL;

    *pppSdpRecords = NULL;
    *pNumRecords = 0;

    ISdpStream *pIStream = NULL;

    if (NULL == (pIStream = AllocateSdpStream()))
    	return E_FAIL;

    hres = pIStream->Validate(szResponse,cbResponse,NULL);

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
        FreeSdpStream(pIStream);
        pIStream = NULL;
    }

	return hres;
}

//
//  Routines related to parsing an SDP format into human readable form.
//


const WCHAR *attribIdTable[] = {
	L"Record Handle",
	L"Class ID List",
	L"Record State",
	L"Service ID",
	L"Protocol Descriptor List",
	L"Browse Group List",
	L"Language Base",
	L"Service Info Time To Live",
	L"Service Availability",
	L"Bluetooth Profile Descriptor List",
	L"Document URL",
	L"Client Executable URL",
	L"Icon URL"
};

static const WCHAR *cszUnknownAttrib = L"Unknown Attribute";
static const WCHAR *cszMaxAttribID   = L"Maximum Attribute Value";

const WCHAR *GetAttribFromID(USHORT usAttribId) {
	if (usAttribId == 0xFFFF)
		return cszMaxAttribID;

	if (usAttribId > SDP_ATTRIB_ICON_URL)
		return cszUnknownAttrib;

	return attribIdTable[usAttribId];
}

// 
// Functions to process attribute IDs
// 

void PrintMisformatted(PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	pfnPrint(pvContext,TRUE,L"Entry is misformatted\n");
}

typedef void (*PFN_SDPPRINT_ATTRIB)(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext);

// Given a stream, creates an ISdpStream element and verifies.
BOOL LoadAndValidate(ISdpStream **ppSdpStream, UCHAR **ppStream, ULONG *pSize, BOOL fVerifyIsSequence) {
	*ppSdpStream = AllocateSdpStream();
	ULONG_PTR sdpError;
	int iIndex = 0;

	SDP_TYPE type = SDP_TYPE_NIL;
	SDP_SPECIFICTYPE specType;
	ULONG storageSize;

	if (!(*ppSdpStream))
		return FALSE;

	if (FAILED((*ppSdpStream)->Validate(*ppStream,*pSize,&sdpError)))
		return FALSE;

	if (fVerifyIsSequence) {
		(*ppSdpStream)->RetrieveElementInfo(*ppStream,&type,&specType,pSize,&storageSize,ppStream);

		if (type != SDP_TYPE_SEQUENCE)
			return FALSE;
	}
	return TRUE;
}

BOOL GetIntFromStream(ISdpStream *pSdpStream, UCHAR *pStream, ULONG size, ULONG *pulVal) {
	SDP_TYPE type;
	SDP_SPECIFICTYPE specType = SDP_ST_NONE;
	ULONG storageSize;
	ULONG recordSize;

	pSdpStream->RetrieveElementInfo(pStream,&type,&specType,&recordSize,&storageSize,&pStream);
	if (specType != SDP_ST_UINT32)
		return FALSE;

	pSdpStream->RetrieveUint32(pStream,pulVal);
	*pulVal = RtlUlongByteSwap(*pulVal);
	return TRUE;
}

void SdpPrintAttribRecordHandle(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	ISdpStream *pSdpStream = NULL;
	int iIndex = 0;

	ULONG recordHandle;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,FALSE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Record misformatted\n");
		goto done;
	}

	if ( ! GetIntFromStream(pSdpStream,pStream,size,&recordHandle)) {
		pfnPrint(pvContext,TRUE,L"  Element is not a UINT32\n");
		goto done;
	}

	pfnPrint(pvContext,FALSE,L"  Record Handle = 0x%08x\n",recordHandle);
done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}


#define MIN_PROTOCOL_UUID16	      SDP_PROTOCOL_UUID16
#define MAX_PROTOCOL_UUID16	      WSP_PROTOCOL_UUID16

const WCHAR cszUnknownUUID[] = L"Unknown UUID";


const WCHAR *g_rgszProtocolUUIDs[] = {
	L"SDP",
	L"UDP",
	L"RFCOMM",
	L"TCP",
	L"TCSBIN",
	L"TCSAT",
	cszUnknownUUID,
	L"OBEX",
	L"IP",
	L"FTP",
	cszUnknownUUID,
	L"HTTP",
	cszUnknownUUID,
	L"WSP"
};
const WCHAR g_szL2CAPProtUUID[] = L"L2CAP";


const WCHAR g_szServiceDiscovery[]     = L"Service Discovery Server";
const WCHAR g_szBrowseGroup[]          = L"Browse Group Descriptor";
const WCHAR g_szPublicBrowseGroup[]    = L"Public Browse Group";
const WCHAR g_szSerialPort[]           = L"Serial Port";
const WCHAR g_szLanAccess[]            = L"LAN Access using PPP";
const WCHAR g_szDialupNetworkService[] = L"Dialup Networking Service";
const WCHAR g_szIRMC[]                 = L"IR MC Sync";
const WCHAR g_szObexPush[]             = L"Obex Push";
const WCHAR g_szObexFile[]             = L"Obex File Transfer";
const WCHAR g_szIRMCSync[]             = L"IR MC Sync Command";
const WCHAR g_szHeadset[]              = L"Headset";
const WCHAR g_szCordlessTelephone[]    = L"Cordless Telephone";
const WCHAR g_szIntercomn[]            = L"Intercom";
const WCHAR g_szFax[]                  = L"Fax";
const WCHAR g_szHeadsetAudioGateway[]  = L"Headset Audio Gateway";

const WCHAR g_szPNP[]                  = L"PnP Information";
const WCHAR g_szGNetworking[]          = L"Generic Networking";
const WCHAR g_szGFileTransfer[]        = L"Generic File Transfer";
const WCHAR g_szGAudio[]               = L"Generic Audio";
const WCHAR g_szGTelephone[]           = L"Generic Telephony";


const WCHAR * GetUUIDString(ULONG ulGUID) {
	if (ulGUID == L2CAP_PROTOCOL_UUID16)
		return g_szL2CAPProtUUID;

	if ((ulGUID >= MIN_PROTOCOL_UUID16) && (ulGUID <= MAX_PROTOCOL_UUID16))
		return g_rgszProtocolUUIDs[ulGUID-MIN_PROTOCOL_UUID16];

	switch (ulGUID) {
	case ServiceDiscoveryServerServiceClassID_UUID16:
		return g_szServiceDiscovery ;
		break;
		
	case BrowseGroupDescriptorServiceClassID_UUID16:
		return g_szBrowseGroup ;
		break;

	case PublicBrowseGroupServiceClassID_UUID16:
		return g_szPublicBrowseGroup ;
		break;

	case SerialPortServiceClassID_UUID16:
		return g_szSerialPort ;
		break;

	case LANAccessUsingPPPServiceClassID_UUID16:
		return g_szLanAccess;
		break;

	case DialupNetworkingServiceClassID_UUID16:
		return g_szDialupNetworkService;
		break;

	case IrMCSyncServiceClassID_UUID16:
		return g_szIRMC;
		break;

	case OBEXObjectPushServiceClassID_UUID16:
		return g_szObexPush;
		break;

	case OBEXFileTransferServiceClassID_UUID16:
		return g_szObexFile ;
		break;

	case IrMcSyncCommandServiceClassID_UUID16:
		return g_szIRMCSync ;
		break;

	case HeadsetServiceClassID_UUID16:
		return g_szHeadset;
		break;

	case CordlessServiceClassID_UUID16:
		return g_szCordlessTelephone ;
		break;

	case IntercomServiceClassID_UUID16:
		return g_szIntercomn;
		break;

	case FaxServiceClassID_UUID16:
		return g_szFax ;
		break;

	case HeadsetAudioGatewayServiceClassID_UUID16:
		return g_szHeadsetAudioGateway ;
		break;

	case PnPInformationServiceClassID_UUID16:
		return g_szPNP;
		break;

	case GenericNetworkingServiceClassID_UUID16:
		return g_szGNetworking;
		break;

	case GenericFileTransferServiceClassID_UUID16:
		return g_szGFileTransfer;
		break;

	case GenericAudioServiceClassID_UUID16:
		return g_szGAudio;
		break;

	case GenericTelephonyServiceClassID_UUID16:
		return g_szGTelephone;
		break;

		default:
			break;
	}

	return cszUnknownUUID;
}

void SdpByteSwapUuid128(GUID *uuid128From, GUID *uuid128To)
{
    uuid128To->Data1 = RtlUlongByteSwap(uuid128From->Data1);
    uuid128To->Data2 = RtlUshortByteSwap(uuid128From->Data2);
    uuid128To->Data3 = RtlUshortByteSwap(uuid128From->Data3);
    memcpy(uuid128To->Data4, uuid128From->Data4, sizeof(uuid128From->Data4)); 
}

// Looks up UUID in a stream, returns one of #define xxx_UUID16 values in bt_sdp.h on success,
// otherwise will return 0
ULONG FindUUIDType(SDP_SPECIFICTYPE specType, ISdpStream *pSdpStream, PUCHAR uuidRecord) {
	ULONG ulGUID;

	// 
	switch (specType) {
		case SDP_ST_UUID128:
		{
			GUID uuid;
			pSdpStream->RetrieveUuid128(uuidRecord,&uuid);
			SdpByteSwapUuid128(&uuid,&uuid);
			ulGUID = uuid.Data1; // don't do byte swap, SdpByteSwapUuid128 did it already for us.

			// Make sure that the base of this is same as base BT UUID.  If not we don't know what it is.
			uuid.Data1 = 0;
			if (0 != memcmp(&uuid,&Bluetooth_Base_UUID,sizeof(GUID))) {
				return 0;
			}
		}
		break;

		case SDP_ST_UUID32:
		{
			pSdpStream->RetrieveUint32(uuidRecord,&ulGUID);
			ulGUID = RtlUlongByteSwap(ulGUID);
		}
		break;

		case SDP_ST_UUID16:
		{
			UINT16 uuid16;
			pSdpStream->RetrieveUint16(uuidRecord,&uuid16);
			ulGUID = RtlUshortByteSwap(uuid16);
		}
		break;

		default:
			assert(0);
	}

	// The GUIDs we know about fall into a well known ranges.

	// First check if it's a Protocol UUID
	if (((ulGUID >= MIN_PROTOCOL_UUID16) && (ulGUID <= MAX_PROTOCOL_UUID16)) || (ulGUID == L2CAP_PROTOCOL_UUID16))
		return ulGUID;

	// Otherwise check the service classes.
	switch (ulGUID) {
		case ServiceDiscoveryServerServiceClassID_UUID16:
		case BrowseGroupDescriptorServiceClassID_UUID16:
		case PublicBrowseGroupServiceClassID_UUID16:
		case SerialPortServiceClassID_UUID16:
		case LANAccessUsingPPPServiceClassID_UUID16:
		case DialupNetworkingServiceClassID_UUID16:
		case IrMCSyncServiceClassID_UUID16:
		case OBEXObjectPushServiceClassID_UUID16:
		case OBEXFileTransferServiceClassID_UUID16:
		case IrMcSyncCommandServiceClassID_UUID16:
		case HeadsetServiceClassID_UUID16:
		case CordlessServiceClassID_UUID16:
		case IntercomServiceClassID_UUID16:
		case FaxServiceClassID_UUID16:
		case HeadsetAudioGatewayServiceClassID_UUID16:
		case PnPInformationServiceClassID_UUID16:
		case GenericNetworkingServiceClassID_UUID16:
		case GenericFileTransferServiceClassID_UUID16:
		case GenericAudioServiceClassID_UUID16:
		case GenericTelephonyServiceClassID_UUID16:
			return ulGUID;
			break;

		default:
			break;
	}

	return 0;
}

#define SDP_PRINT_FLAG_INDEX   0x00000001
#define SDP_PRINT_FLAG_INDENT  0x00000002

void SdpPrintUUID(SDP_SPECIFICTYPE specType, ISdpStream *pSdpStream, PUCHAR uuidRecord, int iIndex, PFN_SDPPRINT pfnPrint, PVOID pvContext, DWORD dwFlags=(SDP_PRINT_FLAG_INDEX|SDP_PRINT_FLAG_INDENT)) {
	ULONG ulGUID = FindUUIDType(specType,pSdpStream,uuidRecord);
	const WCHAR *szName = GetUUIDString(ulGUID);
	WCHAR szBuf[MAX_PATH];
	int   iOffset = 0;

	if (dwFlags & SDP_PRINT_FLAG_INDENT) {
		wcscpy(szBuf,L"  ");
		iOffset += 2;
	}
		
	if (dwFlags & SDP_PRINT_FLAG_INDEX) {
		iOffset += wsprintf(szBuf+iOffset,L"Entry(%d) ",iIndex);
	}
	
	switch (specType) {
		case SDP_ST_UUID128:
		{
			GUID uuid;
			pSdpStream->RetrieveUuid128(uuidRecord,&uuid);
			SdpByteSwapUuid128(&uuid,&uuid);

			iOffset += wsprintf(szBuf+iOffset,L"UUID128 = " SVSUTIL_GUID_FORMAT_W L" (%s)\n",SVSUTIL_RGUID_ELEMENTS(uuid),szName);
		}
		break;

		case SDP_ST_UUID32:
		{
			ULONG uuid;
			pSdpStream->RetrieveUint32(uuidRecord,&uuid);
			uuid = RtlUlongByteSwap(uuid);

			iOffset += wsprintf(szBuf+iOffset,L"UUID32 = 0x%08x (%s)\n",uuid,szName);
		}
		break;

		case SDP_ST_UUID16:
		{
			UINT16 uuid;
			pSdpStream->RetrieveUint16(uuidRecord,&uuid);
			uuid = RtlUshortByteSwap(uuid);

			iOffset += wsprintf(szBuf+iOffset,L"UUID16 = 0x%04x (%s)\n",uuid,szName);
		}
		break;

		default:
			assert(0);
	}
	pfnPrint(pvContext,FALSE,szBuf);
}

void SdpPrintString(PUCHAR pStream, ULONG recordSize, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	WCHAR *psz = NULL;

	int iOutLen = MultiByteToWideChar(CP_ACP, 0, (PCSTR)pStream, recordSize, 0, 0);
	if(!iOutLen) {
		pfnPrint(pvContext,TRUE,L"Cannot convert string\n");
		return;
	}

	if (NULL == (psz = (WCHAR*) LocalAlloc(LMEM_FIXED,(iOutLen+1)*sizeof(WCHAR)))) {
		pfnPrint(pvContext,TRUE,L"Out of memory\n");
		return;
	}

	MultiByteToWideChar(CP_ACP, 0, (PCSTR)pStream, recordSize, psz, iOutLen);
	psz[iOutLen] = 0;
	pfnPrint(pvContext,FALSE,L"%s\n",psz);

	LocalFree(psz);
}

class SdpBufferWrite {
public:
	WCHAR *szBuf;
	int    iOffset;

	SdpBufferWrite(WCHAR *sz, int i) {
		szBuf = sz;
		iOffset = i;
	}
};

void SdpPrintToBuf(PVOID pvContext, BOOL fError, WCHAR *wszFormat,...) {
	SdpBufferWrite *pBuf = (SdpBufferWrite*) pvContext;

	int iOffset  = pBuf->iOffset;
	WCHAR *szBuf = pBuf->szBuf;

	if (fError)
		iOffset += wsprintf(szBuf + iOffset,L"Error: ");

	va_list ap;
	va_start(ap,wszFormat);

	iOffset += vswprintf(szBuf + iOffset,wszFormat,ap);
	va_end (ap);

	pBuf->iOffset = iOffset;
}

// We don't have knowledge of a attribute, so print it out in a somewhat nice format.
void SdpPrintUnknownAttribute(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext, int iIndent) {
	ISdpStream *pSdpStream = NULL;

	SDP_TYPE type;
	SDP_SPECIFICTYPE specType;
	ULONG storageSize;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,FALSE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Object misformatted\n");
		goto done;
	}

	// NOTE: Most applications should use SDP Records or ISdpWalk when parsing
	// SDP records.  However some of these routines require that the data be a data 
	// element sequence or alternative, whereas this is designed for a general purpose
	// parser that will print out *anything*.

    while (size) {
		PUCHAR pSubElement = pStream;
		ULONG recordSize = 0;
		WCHAR szBuf[1000];
		int   iOffset = 0;

		pSdpStream->RetrieveElementInfo(pStream,&type,&specType,&recordSize,&storageSize,&pSubElement);

		if (type == SDP_TYPE_SEQUENCE || type == SDP_TYPE_ALTERNATIVE) {
			// go recursive to handle this.
			int i;
			for (i = 0; i < iIndent; i++)
				iOffset += wsprintf(szBuf+iOffset,L"  ");

			iOffset += wsprintf(szBuf+iOffset,L"%s\n",(type == SDP_TYPE_SEQUENCE) ? L"SEQUENCE" : L"ALTERNATIVE");
			pfnPrint(pvContext,FALSE,szBuf);
			SdpPrintUnknownAttribute(pSubElement,recordSize,pfnPrint,pvContext,iIndent+1);

			iOffset = 0;
			for (i = 0; i < iIndent; i++)
				iOffset += wsprintf(szBuf+iOffset,L"  ");

			iOffset += wsprintf(szBuf+iOffset,L"END SEQUENCE\n");
		}
		else {
			for (int i = 0; i < iIndent; i++)
				iOffset += wsprintf(szBuf+iOffset,L"  ");
	
			switch (type) {
			case SDP_TYPE_NIL:
				iOffset += wsprintf(szBuf+iOffset,L"NIL\n");
				break;
			
			case SDP_TYPE_UINT:
				iOffset += wsprintf(szBuf+iOffset,L"UINT");
				if (specType ==  SDP_ST_UINT8) {
					iOffset += wsprintf(szBuf+iOffset,L"8: 0x%02x\n",*pSubElement);
				}
				else if (specType == SDP_ST_UINT16) {
					USHORT us;
					pSdpStream->RetrieveUint16(pSubElement,&us);
					iOffset += wsprintf(szBuf+iOffset,L"16: 0x%04x\n",RtlUshortByteSwap(us));
				}
				else if (specType == SDP_ST_UINT32) {
					ULONG ul;
					pSdpStream->RetrieveUint32(pSubElement,&ul);
					iOffset += wsprintf(szBuf+iOffset,L"32: 0x%08x\n",RtlUlongByteSwap(ul));
				}
				else if (specType == SDP_ST_UINT64) {
					ULONGLONG ull;
					pSdpStream->RetrieveUint64(pSubElement,&ull);
					ull = RtlByteSwap64(ull);
					iOffset += wsprintf(szBuf+iOffset,L"64: 0x%I64x\n",ull);
				}
				else if (specType == SDP_ST_UINT128) {
					SDP_ULARGE_INTEGER_16 ull;
					pSdpStream->RetrieveUint128(pSubElement,&ull);
					ull.LowPart  = RtlByteSwap64(ull.LowPart);
					ull.HighPart = RtlByteSwap64(ull.HighPart);
					iOffset += wsprintf(szBuf+iOffset,L"128 0x%I64x,0x%I64x\n",ull.HighPart,ull.LowPart);
				}
				break;

			case SDP_TYPE_INT:
				iOffset += wsprintf(szBuf+iOffset,L"INT: ");
				if (specType == SDP_ST_INT8) {
					iOffset += wsprintf(szBuf+iOffset,L"8: 0x%02x\n",*pSubElement);
				}
				else if (specType == SDP_ST_INT16) {
					SHORT s;
					pSdpStream->RetrieveInt16(pSubElement,&s);
					iOffset += wsprintf(szBuf+iOffset,L"16: 0x%04x\n",s);
				}
				else if (specType == SDP_ST_INT32) {
					LONG l;
					pSdpStream->RetrieveInt32(pSubElement,&l);
					iOffset += wsprintf(szBuf+iOffset,L"32: 0x%08x\n",RtlUlongByteSwap(l));
				}
				else if (specType == SDP_ST_INT64) {
					LONGLONG ll;
					pSdpStream->RetrieveInt64(pSubElement,&ll);
					ll = RtlByteSwap64(ll);
					iOffset += wsprintf(szBuf+iOffset,L"64: 0x%I64x\n",ll);
				}
				else if (specType == SDP_ST_INT128) {
					SDP_LARGE_INTEGER_16 ll;
					pSdpStream->RetrieveInt128(pSubElement,&ll);
					ll.LowPart  = RtlByteSwap64(ll.LowPart);
					ll.HighPart = RtlByteSwap64(ll.HighPart);
					iOffset += wsprintf(szBuf+iOffset,L"128 0x%I64x,0x%I64x\n",ll.HighPart,ll.LowPart);
				}
				break;

			case SDP_TYPE_UUID: 
			{
				SdpBufferWrite bufW(szBuf,iOffset);
				SdpPrintUUID(specType,pSdpStream,pSubElement,iIndent,SdpPrintToBuf,&bufW,0);
			}
			break;

			case SDP_TYPE_BOOLEAN:
				iOffset += wsprintf(szBuf+iOffset,L"BOOL: %s\n",(*pSubElement) ?  L"TRUE" : L"FALSE");
				break;


			case SDP_TYPE_STRING: 
			{
				iOffset += wsprintf(szBuf+iOffset,L"STRING: ");
				SdpBufferWrite bufW(szBuf,iOffset);
				SdpPrintString(pSubElement,recordSize,SdpPrintToBuf,&bufW);
			}
			break;

			case SDP_TYPE_URL: 
			{
				iOffset += wsprintf(szBuf+iOffset,L"URL: ");
				SdpBufferWrite bufW(szBuf,iOffset);
				SdpPrintString(pSubElement,recordSize,SdpPrintToBuf,&bufW);
			}
			break;

			default:
				assert(0); // Validating code should catch unknown elements.
				break;
			}
		}
		pfnPrint(pvContext,FALSE,szBuf);
		
		recordSize += 1 + storageSize;

		pStream += recordSize;
		size -= recordSize;
    }
done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}

// Stream is a data element sequence containing a list of UUIDs.
void SdpPrintAttribUUIDs(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	ISdpStream *pSdpStream = NULL;
	int iIndex = 0;

	SDP_TYPE type;
	SDP_SPECIFICTYPE specType;
	ULONG storageSize;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,TRUE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Object misformatted\n");
		goto done;
	}

    while (size) {
		PUCHAR uuidRecord = pStream;
		ULONG recordSize = 0;

		pSdpStream->RetrieveElementInfo(pStream,&type,&specType,&recordSize,&storageSize,&uuidRecord);

		if (type != SDP_TYPE_UUID) {
			pfnPrint(pvContext,TRUE,L"UUID stream invalid, element (%d) is not a UUID\n",iIndex);
			goto done;
		}

		SdpPrintUUID(specType,pSdpStream,uuidRecord,iIndex,pfnPrint,pvContext);
		recordSize += 1 + storageSize;

		pStream += recordSize;
		size -= recordSize;
		iIndex++;
    }
done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}

void SdpPrintAttribRecordState(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	ISdpStream *pSdpStream = NULL;
	int iIndex = 0;

	ULONG recordState;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,FALSE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Record misformatted\n");
		goto done;
	}

	if ( ! GetIntFromStream(pSdpStream,pStream,size,&recordState)) {
		pfnPrint(pvContext,TRUE,L"  Element is not a UINT32\n");
		goto done;
	}

	pfnPrint(pvContext,FALSE,L"  Record State = 0x%08x\n",recordState);
done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}

void SdpPrintAttribServiceId(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	SDP_TYPE type;
	SDP_SPECIFICTYPE specType = SDP_ST_NONE;
	ULONG storageSize;
	ULONG recordSize;

	ISdpStream *pSdpStream = NULL;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,FALSE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Record misformatted\n");
		goto done;
	}

	pSdpStream->RetrieveElementInfo(pStream,&type,&specType,&recordSize,&storageSize,&pStream);
	if (type != SDP_TYPE_UUID) {
		pfnPrint(pvContext,TRUE,L"  Element is not a UUID\n");
		goto done;
	}

	SdpPrintUUID(specType,pSdpStream,pStream,0,pfnPrint,pvContext,SDP_PRINT_FLAG_INDENT);

done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);

}


#define SDP_PROTOCOL_UNKNOWN   0


void SdpPrintAttribProtDescrList(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	ISdpStream *pSdpStream = NULL;

	SDP_TYPE type;
	SDP_SPECIFICTYPE specType;
	ULONG storageSize;
	ULONG sdpProtocol;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,TRUE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Object misformatted\n");
		goto done;
	}

    while (size) {
		PUCHAR pElementInfo = NULL;
		ULONG recordSize = 0;
		BOOL  fFirstPass = TRUE;

		pSdpStream->RetrieveElementInfo(pStream,&type,&specType,&recordSize,&storageSize,&pElementInfo);

		if (type != SDP_TYPE_SEQUENCE) {
			pfnPrint(pvContext,TRUE,L"Protocol stream invalid, element is not a Data Element Sequence\n");
			goto done;
		}

		// Adjust the main traversal pointer
		ULONG fullRecordSize = recordSize + 1 + storageSize; 
		pStream += fullRecordSize;
		size -= fullRecordSize;

		sdpProtocol = SDP_PROTOCOL_UNKNOWN;

		// Walk through the given sequence.
		while (recordSize) {
			ULONG subRecordSize;
			PUCHAR pSubElementInfo = NULL;
			
			pSdpStream->RetrieveElementInfo(pElementInfo,&type,&specType,&subRecordSize,&storageSize,&pSubElementInfo);
			subRecordSize += 1 + storageSize;
			pElementInfo += subRecordSize;
			recordSize -= subRecordSize;

			if (sdpProtocol == SDP_PROTOCOL_UNKNOWN) {
				if (type != SDP_TYPE_UUID) {
					pfnPrint(pvContext,TRUE,L"    First element in sequence is not a protocol UUID, skipping parsing on current sequence\n");
					break;
				}

				SdpPrintUUID(specType,pSdpStream,pSubElementInfo,0,pfnPrint,pvContext,SDP_PRINT_FLAG_INDENT);

				ULONG ulGUID = FindUUIDType(specType, pSdpStream, pSubElementInfo);
				if ((ulGUID != L2CAP_PROTOCOL_UUID16) && (ulGUID != RFCOMM_PROTOCOL_UUID16) && 
				    (ulGUID != OBEX_PROTOCOL_UUID16)) {
					pfnPrint(pvContext,TRUE,L"    Do not know how to parse protocol UUID... skipping.\n");
					break;
				}

				sdpProtocol = ulGUID;
				continue;
			}

			if (sdpProtocol == L2CAP_PROTOCOL_UUID16) {
				if (fFirstPass) {
					if (specType != SDP_ST_UINT16) {
						pfnPrint(pvContext,TRUE,L"    L2CAP Channel ID type is not UINT16, skipping rest of processing of this entry.\n");
						break;
					}
					USHORT cid;

					pSdpStream->RetrieveUint16(pSubElementInfo,&cid);
					pfnPrint(pvContext,FALSE,L"    L2CAP Channel ID = 0x%04x\n",cid);
					fFirstPass = FALSE;
				}
				else {
					pfnPrint(pvContext,TRUE,L"    Do not know how to parse remainder of L2CAP information... skipping.\n");
					break;
				}

			}
			else if (sdpProtocol == RFCOMM_PROTOCOL_UUID16) {
				if (fFirstPass) {
					if (specType != SDP_ST_UINT8) {
						pfnPrint(pvContext,TRUE,L"    RFCOMM Channel ID type is not UINT8, skipping rest of processing of this entry.\n");
						break;
					}
					pfnPrint(pvContext,FALSE,L"    RFCOMM Channel ID = 0x%02x\n",*pSubElementInfo);
					fFirstPass = FALSE;
				}
				else {
					pfnPrint(pvContext,TRUE,L"    Do not know how to parse remainder of RFCOMM information... skipping.\n");
					break;
				}
			}
			else {
				assert(sdpProtocol == OBEX_PROTOCOL_UUID16);
				pfnPrint(pvContext,TRUE,L"    Do not know how to parse remainder of OBEX information... skipping.\n");
				break;
			}
		}
    }
done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}



// Language base is a data element containing one or more group of 3 UINT16's.
void SdpPrintAttribLanguageBase(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	ISdpStream *pSdpStream = NULL;

	SDP_TYPE type;
	SDP_SPECIFICTYPE specType;
	ULONG storageSize;
	int   iIndex = 0;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,TRUE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Object misformatted\n");
		goto done;
	}

	while (size) {
		USHORT rgusAttribs[3];
		ULONG recordSize = 0;
		UCHAR *pAttrib;

		for (DWORD i = 0; i < SVSUTIL_ARRLEN(rgusAttribs); i++) {
			if (FAILED(pSdpStream->RetrieveElementInfo(pStream,&type,&specType,&recordSize,&storageSize,&pAttrib) ||
			    (specType != SDP_ST_UINT16))) {
				PrintMisformatted(pfnPrint,pvContext);
				goto done;
			}

			pSdpStream->RetrieveUint16(pAttrib,&rgusAttribs[i]);

			recordSize += 1 + storageSize;
			pStream += recordSize;
			size -= recordSize;

			if (size == 0 && i != (SVSUTIL_ARRLEN(rgusAttribs)-1)) {
				PrintMisformatted(pfnPrint,pvContext);
				goto done;
			}
		}

		pfnPrint(pvContext,FALSE,L"  Entry(%d)\n",iIndex);
		pfnPrint(pvContext,FALSE,L"    Language           = 0x%04x\n",RtlUshortByteSwap(rgusAttribs[0]));
		pfnPrint(pvContext,FALSE,L"    Character Encoding = 0x%04x\n",RtlUshortByteSwap(rgusAttribs[1]));
		pfnPrint(pvContext,FALSE,L"    Base AttributeID   = 0x%04x\n",RtlUshortByteSwap(rgusAttribs[2]));
		iIndex++;
	}

done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}

void SdpPrintAttribServiceInfoTTL(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	ISdpStream *pSdpStream = NULL;
	int iIndex = 0;

	ULONG infoTTL;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,FALSE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Record misformatted\n");
		goto done;
	}

	if ( ! GetIntFromStream(pSdpStream,pStream,size,&infoTTL)) {
		pfnPrint(pvContext,TRUE,L"  Element is not a UINT32\n");
		goto done;
	}

	pfnPrint(pvContext,FALSE,L"  ServiceInfoTimeToLive = 0x%08x\n",infoTTL);
done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}

void SdpPrintAttribServiceAvailability(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	SDP_TYPE type;
	SDP_SPECIFICTYPE specType = SDP_ST_NONE;
	ULONG storageSize;
	ULONG recordSize;

	ISdpStream *pSdpStream = NULL;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,FALSE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Record misformatted\n");
		goto done;
	}

	pSdpStream->RetrieveElementInfo(pStream,&type,&specType,&recordSize,&storageSize,&pStream);
	if (specType != SDP_ST_UINT8) {
		pfnPrint(pvContext,TRUE,L"  Element is not a UINT8\n");
		goto done;
	}

	pfnPrint(pvContext,FALSE,L"  Service Availability = 0x%02x\n",*pStream);

done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}

void SdpPrintAttribBluetoothProfileDescriptorList(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	ISdpStream *pSdpStream = NULL;

	SDP_TYPE type;
	SDP_SPECIFICTYPE specType;
	ULONG storageSize;
	ULONG sdpProtocol;
	int   iIndexNumber = 0;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,TRUE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Object misformatted\n");
		goto done;
	}

	// The Profile descriptor list is a data element sequence containing 
	// one or more data element sequences containing information about a BT profile
	// supported by this record.  The first element of each list is a UUID specifying
	// the service, the second is a UINT16 representing the version #.

    while (size) {
		PUCHAR pElementInfo = NULL;
		ULONG recordSize = 0;
		BOOL  fFirstPass = TRUE;


		pSdpStream->RetrieveElementInfo(pStream,&type,&specType,&recordSize,&storageSize,&pElementInfo);

		if (type != SDP_TYPE_SEQUENCE) {
			pfnPrint(pvContext,TRUE,L"Protocol stream invalid, element is not a Data Element Sequence\n");
			goto done;
		}

		// Adjust the main traversal pointer
		ULONG fullRecordSize = recordSize + 1 + storageSize; 
		pStream += fullRecordSize;
		size -= fullRecordSize;

		sdpProtocol = SDP_PROTOCOL_UNKNOWN;

		pfnPrint(pvContext,FALSE,L"  Protocol Descriptor(%d)\n",iIndexNumber);

		// Walk through the given sequence.
		while (recordSize) {
			ULONG subRecordSize;
			PUCHAR pSubElementInfo = NULL;
			
			pSdpStream->RetrieveElementInfo(pElementInfo,&type,&specType,&subRecordSize,&storageSize,&pSubElementInfo);
			subRecordSize += 1 + storageSize;
			pElementInfo += subRecordSize;
			recordSize -= subRecordSize;

			if (fFirstPass) {
				if (type != SDP_TYPE_UUID) {
					pfnPrint(pvContext,TRUE,L"  First element is not a UUID, skipping this entry\n");
					break;
				}
				SdpPrintUUID(specType, pSdpStream, pSubElementInfo, 0, pfnPrint, pvContext,SDP_PRINT_FLAG_INDENT);
				fFirstPass = FALSE;
			}
			else {
				if (specType != SDP_ST_UINT16) {
					pfnPrint(pvContext,TRUE,L"  Second element is not a UINT16, skipping this entry\n");
					break;
				}
				USHORT usVersion;
				pSdpStream->RetrieveUint16(pSubElementInfo,&usVersion);
				usVersion = RtlUshortByteSwap(usVersion);

				pfnPrint(pvContext,FALSE,L"    Version = %d.%d\n",HIBYTE(usVersion),LOBYTE(usVersion));
			}
		}
		iIndexNumber++;
    }
done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}

void SdpPrintAttribURL(UCHAR *pStream, ULONG size, PFN_SDPPRINT pfnPrint, PVOID pvContext) { 
	SDP_TYPE type;
	SDP_SPECIFICTYPE specType = SDP_ST_NONE;
	ULONG storageSize;
	ULONG recordSize;

	ISdpStream *pSdpStream = NULL;
	int iIndex = 0;

	if (! LoadAndValidate(&pSdpStream,&pStream,&size,FALSE)) {
		pfnPrint(pvContext,TRUE,L"Attribute Record misformatted\n");
		goto done;
	}

	pSdpStream->RetrieveElementInfo(pStream,&type,&specType,&recordSize,&storageSize,&pStream);
	if (type != SDP_TYPE_URL) {
		pfnPrint(pvContext,TRUE,L"  Element is not a URL\n");
		goto done;
	}

	pfnPrint(pvContext,FALSE,L"  URL=");
	SdpPrintString(pStream,recordSize,pfnPrint,pvContext);

done:
	if (pSdpStream)
		FreeSdpStream(pSdpStream);
}

static const PFN_SDPPRINT_ATTRIB g_PrintTable[] = {
	SdpPrintAttribRecordHandle,
	SdpPrintAttribUUIDs, // Class ID is a list of UUIDs
	SdpPrintAttribRecordState,
	SdpPrintAttribServiceId,
	SdpPrintAttribProtDescrList,
	SdpPrintAttribUUIDs, // Browse group list is a list of UUIDs
	SdpPrintAttribLanguageBase,
	SdpPrintAttribServiceInfoTTL,
	SdpPrintAttribServiceAvailability,
	SdpPrintAttribBluetoothProfileDescriptorList,
	SdpPrintAttribURL, // document URL is an URL
	SdpPrintAttribURL, // client exe URL is an URL
	SdpPrintAttribURL  // icon URL is an URL
};



void PrintRecordInfo(ISdpRecord *pRecord, int iRecordIndex, PFN_SDPPRINT pfnPrint, PVOID pvContext) {
	USHORT *pAttributeList = NULL;
	ULONG  nAttributes;
	ULONG  i;

	pfnPrint(pvContext,FALSE,L"*** Record %d ***\n",iRecordIndex+1);

	if (FAILED(pRecord->GetAttributeList(&pAttributeList,&nAttributes))) {
		pfnPrint(pvContext,TRUE,L"Unable to retrieve attributes from record\n");
		goto done;
	}

	for (i = 0; i < nAttributes; i++) {
		USHORT usAttribId = pAttributeList[i];
		UCHAR  *pStream = NULL;
		ULONG  size;

		pfnPrint(pvContext,FALSE,L"AttribID 0x%04x (%s)\n",usAttribId,GetAttribFromID(usAttribId));
	
		if (FAILED(pRecord->GetAttributeAsStream(usAttribId,&pStream,&size))) {
			if (pStream)
				CoTaskMemFree(pStream);

			// since this is almost certainly an OOM or an internal error, no point continuing processing.
			pfnPrint(pvContext,TRUE,L"Unable to parse attribute stream for attribute id %d.  Stopping processing.\n",usAttribId);
			goto done;
		}
	
		if (usAttribId > SDP_ATTRIB_ICON_URL) {
			SdpPrintUnknownAttribute(pStream,size,pfnPrint,pvContext,1);
		}
		else {
			g_PrintTable[usAttribId](pStream,size,pfnPrint,pvContext);
		}

		CoTaskMemFree(pStream);
	}

done:
	if (pAttributeList)
		CoTaskMemFree(pAttributeList);
}

// Error code -> string lookup
static const WCHAR *g_szErrorCodes[] = {
      L"Invalid/unsupported SDP version",
      L"Invalid Service Record Handle",
      L"Invalid request syntax",
      L"Invalid PDU size",
      L"Invalid Continuation State",
      L"Insufficient Resources to satisfy request"
};
static const WCHAR cszUnknownErr[] = L"Unknown Error Code";

const WCHAR *GetErrorFromId(USHORT usErrCode) {
	if ((usErrCode >= SDP_ERROR_INVALID_SDP_VERSION) && (usErrCode <= SDP_ERROR_INSUFFICIENT_RESOURCES))
		return g_szErrorCodes[usErrCode-1];

	return cszUnknownErr;
}

// Protocol packet type -> string lookup
static const WCHAR *g_szPDUTypes[] = {
     L"Error PDU",
     L"Service Search Request PDU",
     L"Service Search Response PDU",
     L"Attribute Request PDU",
     L"Attribute Response PDU",
     L"Service Attribute Search Request PDU",
     L"Service Attribute Search Response PDU"
};
static const WCHAR cszUnknownPDU[] = L"Unknown PDU packet";

const WCHAR *GetPduNameFromId(UCHAR id) {
	if ((id >= 1) && (id <= SVSUTIL_ARRLEN(g_szPDUTypes))) // PDU Error to PDU Service Attribute Response
		return g_szPDUTypes[id-1];

	return cszUnknownPDU;
}

ISdpStream * AllocateSdpStream(void) {
	ISdpStream *pIStream = NULL;

	if (FAILED(CoCreateInstance(__uuidof(SdpStream),NULL,CLSCTX_INPROC_SERVER,
	                        __uuidof(ISdpStream),(LPVOID *) &pIStream))) {
		return NULL;
	}
	return pIStream;
}

void FreeSdpStream(ISdpStream *pStream) {
	pStream->Release();
}

