#ifndef __PRINT_SPRT_H__
#define __PRINT_SPRT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define RETVAL extern "C" __declspec(dllexport) long _stdcall

	/////////////////////////////////////////
// result code
#define SPRT_FAILED				0x00000000
#define SPRT_SUCCESS			0x00000001

// COM status
#define SPRT_COMCOFIG_FAILED	0x00000012
#define SPRT_COMCOFIG_SUCCEED	0x00000013

// connection status
#define SPRT_CS_NOINITED		0x00001000
#define SPRT_CS_ERRSTATUS		0x00001001

// printer status
#define SPRT_PS_NOINITED		0x00002000
#define SPRT_PS_IDLE			0x00002001
#define SPRT_PS_BUSY			0x00002002

// device capability
#define SPRT_DC_HORZRES			0x00004001
#define SPRT_DC_VERTRES			0x00004002
#define SPRT_DC_LOGPIXELSX		0x00004003
#define SPRT_DC_LOGPIXELSY		0x00004004
#define SPRT_DC_PHYSICALWIDTH	0x00004005
#define SPRT_DC_PHYSICALHEIGHT	0x00004006

/////////////////////////////////////////
//Enum
typedef enum
{
	SPRT_PT_TIII_TIV	= 0,
	SPRT_PT_T5,
	SPRT_PT_T8
} PrinterType;

typedef enum
{
	SPRT_PO_PORTRAIT,
	SPRT_PO_LANDSCAPE
} PrintOrientation;

typedef enum
{
	SPRT_CS_NOTINITED,
	SPRT_CS_NORMAL,
	SPRT_CS_IDLE,
	SPRT_CS_BUSY
} ConnectedStatus;

typedef enum
{
	SPRT_PS_NORMAL,
	SPRT_PS_OUTPAPER,
	SPRT_PS_ERROR
} PrinterStatus;

typedef enum
{
	SPRT_MM_DOT,
	SPRT_MM_INCH,
	SPRT_MM_MM
} MapMode;

typedef struct tagComState
{
	DWORD dwBaudRate;
	BYTE Parity;
	BYTE StopBits;
}ComState;

typedef struct tagPrinterInfo
{
	PrinterType printType;
	PrintOrientation  printOrient;
	double paperWidth;
	double paperHeight;
	INT bRoll;
	INT port;
	ComState *pcs;
} PrinterInfo;


typedef struct tagFontProperty
{
	bool bDefault;
	bool bUnderLine;
	bool bItalic;
	bool bStrikeout;
	INT nWidth;
	INT nHeight;
	INT iCharSet;
	INT iWeight;
} FontProperty;


RETVAL DC_StartDoc();
RETVAL DC_StartPage();
RETVAL DC_EndDoc();
RETVAL DC_PrintText( LPCTSTR lpData, double x, double y);
RETVAL DC_PrintTextBlock( LPCTSTR lpData, 
						 double x, 
						 double y, 
						 double width, 
						 double height, 
						 INT iFormat);
RETVAL DC_GetPictureSize(LPCTSTR lpFilePath, double *width, double *height);
RETVAL DC_GetPictureBitCount(LPCTSTR lpFilePath, int *bitcount);
RETVAL DC_PrintPicture( LPCTSTR lpFilePath, double xP, double yP, double iWidth, double iHeight);
RETVAL DC_ConvertMonoPicture( LPCTSTR lpPath, LPCTSTR lpPath1);
RETVAL DC_GetConnectedStatus();
RETVAL DC_SetMapMode(MapMode mm);
RETVAL DC_GetMapMode();
RETVAL DC_PrinterSet(PrinterInfo printerInfo);
RETVAL DC_SetFont(FontProperty fontProperty);
RETVAL DC_GetDevCaps(LONG devCap);
RETVAL DC_DrawLine(double x1, double x2, double y1, double y2);
RETVAL DC_DrawRect(double x1, double x2, double y1, double y2);
RETVAL DC_DrawRoundRect(double x1, double x2, double y1, double y2, double iWidth, double iHeight);
RETVAL DC_DrawEllipse(double x1, double x2, double y1, double y2);
RETVAL DC_DrawCircle(double x, double y, double radius);
RETVAL DC_SetLineWidth(double dVal);
RETVAL DC_GetLineWidth(double *dVal);
RETVAL DC_SetFillMode(bool bVal);
RETVAL DC_SetBorderMode(bool bVal);
RETVAL DC_SetBarcodeHeight(double height);
RETVAL DC_Draw2of5(LPCTSTR str,
    double pos_x,
    double pos_y,
    BOOL checksum, 
    BOOL add_text,
    int thickness
);
RETVAL DC_DrawEAN8( LPCTSTR str,
    double pos_x,
    double pos_y,
    BOOL add_text,
    int thickness);
RETVAL DC_DrawEAN13( LPCTSTR str,
    double pos_x,
    double pos_y,
    BOOL add_text,
    int thickness);
RETVAL DC_DrawCodaBar( LPCTSTR str,
    double pos_x,
    double pos_y,
    BOOL add_text,
    int thickness);
RETVAL DC_DrawCode39( LPCTSTR str,
    double pos_x,
    double pos_y,
    BOOL checksum,
    BOOL add_text,
    int thickness);
RETVAL DC_DrawCode93(LPCTSTR str,
    double pos_x,
    double pos_y,
    BOOL add_text,
    int thickness);
RETVAL DC_DrawCode128(LPCTSTR str,
    double pos_x,
    double pos_y,
    BOOL add_text,
	BOOL advance,
    int thickness);
RETVAL DC_DrawUPCA(LPCTSTR str,
					double pos_x,
					double pos_y,
					BOOL add_text,
					int thickness);
RETVAL DC_DrawUPCE(LPCTSTR str,
				   double pos_x,
					double pos_y,
					BOOL add_text,
					int thickness);
RETVAL ASCII_SelectPrinter(LONG pType);
RETVAL ASCII_OpenPrinter(INT iPortName);
RETVAL ASCII_SendString(LPCTSTR lpStr);
RETVAL ASCII_DirectData(LPCSTR lpData, LONG SendNum);
RETVAL ASCII_SetComPortParam(ComState *cs);
RETVAL ASCII_ClosePrinter();
RETVAL ASCII_GetPrinterStatus(LPCSTR StusBuf, int n);
RETVAL GetVersion();

#ifdef __cplusplus
}
#endif

#endif
