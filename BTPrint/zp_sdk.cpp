#include "stdafx.h"
#include "zp_sdk.h"

typedef void (*t_zp_set_serial_device_handle)(HANDLE serialDeviceHandle);
typedef bool (*t_zp_open_wifi)(WCHAR *ip);
typedef bool (*t_zp_open)(int Port, unsigned long Baudrate);
typedef bool (*t_zp_open_auto)(void);
typedef void (*t_zp_close)(void);
typedef void (*t_zp_show_error_msg)(bool Enable);
typedef WCHAR *(*t_zp_get_error_msg)(void);
typedef int  (*t_zp_check_no_paper)(void);
typedef int  (*t_zp_check_cover_opened)(void);
typedef int  (*t_zp_error_status)(int timeout);
typedef void (*t_zp_printer_status_detect)(void);
typedef int  (*t_zp_printer_status_get)(int timeout);
typedef void (*t_zp_set_date_ziped)(bool Enabled);

typedef bool (*t_zp_page_create)(double PageWidthMM,double PageHeightMM,bool Rotate);
typedef void (*t_zp_page_free)(void);
typedef void (*t_zp_page_clear)(void);
typedef bool (*t_zp_page_print)(void);
typedef HDC  (*t_zp_page_get_dc)(void);
typedef void (*t_zp_page_set_pen)(int PenWidth);
typedef void (*t_zp_page_set_pen_color)(bool IsBlack);
typedef void (*t_zp_page_set_font)(WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
typedef void (*t_zp_draw_rect)(double left,double top,double right,double bottom,int PenWidth);
typedef void (*t_zp_draw_line)(double x0,double y0,double x1,double y1,int PenWidth);
typedef void (*t_zp_draw_bitmap)(WCHAR *RelateBitmapFileName,double left,double top,double right,double bottom);
typedef void (*t_zp_draw_text)(double x,double y,WCHAR *text);
typedef void (*t_zp_draw_convert_text_ex)(double x,double y,double width,double height,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
typedef void (*t_zp_draw_text_ex)(double x,double y,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
typedef void (*t_zp_draw_text_box)(double x,double y,double width,double height,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
typedef bool (*t_zp_draw_barcode)(double x,double y,WCHAR *pData,BARCODE_TYPE type,double Height,int LineWidth,int Rotate);
typedef bool (*t_zp_goto_feed_back)(int MaxFeedMM);
typedef bool (*t_zp_goto_feed)(int MaxFeedMM);
typedef bool (*t_zp_goto_mark_left)(int MaxFeedMM);
typedef bool (*t_zp_goto_mark_right)(int MaxFeedMM);
typedef bool (*t_zp_goto_mark_label)(int MaxFeedMM);
typedef bool (*t_zp_goto_mark_any)(int MaxFeedMM);
typedef bool (*t_zp_goto_mark_lable_begin)(int MaxFeedMM);
typedef bool (*t_zp_goto_mark_lable_end)(int MaxFeedMM);
typedef bool (*t_zp_cut_pagehead)(void);
typedef bool (*t_zp_draw_barcode2d_pdf417)(double x,double y,WCHAR *pData,int ColSize,int Size,int Rotate);
typedef bool (*t_zp_draw_barcode2d_qrcode)(double x,double y,WCHAR *pData,int Ver,int Size,int Rotate);
typedef bool (*t_zp_draw_barcode2d_datamatrix)(double x,double y,WCHAR *pData,int Size,int Rotate);

t_zp_set_serial_device_handle p_zp_set_serial_device_handle;
t_zp_open_wifi p_zp_open_wifi;
t_zp_open p_zp_open;
t_zp_open_auto p_zp_open_auto;
t_zp_close p_zp_close;
t_zp_show_error_msg p_zp_show_error_msg;
t_zp_get_error_msg p_zp_get_error_msg;
t_zp_check_no_paper p_zp_check_no_paper;
t_zp_check_cover_opened p_zp_check_cover_opened;
t_zp_error_status p_zp_error_status;
t_zp_printer_status_detect p_zp_printer_status_detect;
t_zp_printer_status_get p_zp_printer_status_get;
t_zp_set_date_ziped p_zp_set_date_ziped;

t_zp_page_create p_zp_page_create;
t_zp_page_free p_zp_page_free;
t_zp_page_clear p_zp_page_clear;
t_zp_page_print p_zp_page_print;
t_zp_page_get_dc p_zp_page_get_dc;
t_zp_page_set_pen p_zp_page_set_pen;
t_zp_page_set_pen_color p_zp_page_set_pen_color;
t_zp_page_set_font p_zp_page_set_font;
t_zp_draw_rect p_zp_draw_rect;
t_zp_draw_line p_zp_draw_line;
t_zp_draw_bitmap p_zp_draw_bitmap;
t_zp_draw_text p_zp_draw_text;
t_zp_draw_convert_text_ex p_zp_draw_convert_text_ex;
t_zp_draw_text_ex p_zp_draw_text_ex;
t_zp_draw_text_box p_zp_draw_text_box;
t_zp_draw_barcode p_zp_draw_barcode;
t_zp_goto_feed_back p_zp_goto_feed_back;
t_zp_goto_feed p_zp_goto_feed;
t_zp_goto_mark_left p_zp_goto_mark_left;
t_zp_goto_mark_right p_zp_goto_mark_right;
t_zp_goto_mark_label p_zp_goto_mark_label;
t_zp_goto_mark_any p_zp_goto_mark_any;
t_zp_goto_mark_lable_begin p_zp_goto_mark_lable_begin;
t_zp_goto_mark_lable_end p_zp_goto_mark_lable_end;
t_zp_cut_pagehead p_zp_cut_pagehead;
t_zp_draw_barcode2d_pdf417 p_zp_draw_barcode2d_pdf417;
t_zp_draw_barcode2d_qrcode p_zp_draw_barcode2d_qrcode;
t_zp_draw_barcode2d_datamatrix p_zp_draw_barcode2d_datamatrix;

static HINSTANCE  hDLL=NULL;
#define LOAD_FUNC(func)  if(!(p_##func = (t_##func)GetProcAddress(hDLL,L"##func"))){MessageBox(GetActiveWindow(),L"##func",L"error",MB_OK);goto err;}

bool zp_sdk_init(HWND hWnd)
{
	WCHAR ZP_SDK_FILE[]=L"zp_sdk.dll";
	if(hDLL)return true;
	if(!(hDLL=LoadLibrary(ZP_SDK_FILE)))goto err;
	if(!(p_zp_set_serial_device_handle = (t_zp_set_serial_device_handle)GetProcAddress(hDLL,L"zp_set_serial_device_handle")))goto err;
	if(!(p_zp_open_wifi = (t_zp_open_wifi)GetProcAddress(hDLL,L"zp_open_wifi")))goto err;
	if(!(p_zp_open = (t_zp_open)GetProcAddress(hDLL,L"zp_open")))goto err;
	if(!(p_zp_open_auto = (t_zp_open_auto)GetProcAddress(hDLL,L"zp_open_auto")))goto err;
	if(!(p_zp_close = (t_zp_close)GetProcAddress(hDLL,L"zp_close")))goto err;
	if(!(p_zp_show_error_msg = (t_zp_show_error_msg)GetProcAddress(hDLL,L"zp_show_error_msg")))goto err;
	if(!(p_zp_get_error_msg = (t_zp_get_error_msg)GetProcAddress(hDLL,L"zp_get_error_msg")))goto err;
	if(!(p_zp_check_no_paper = (t_zp_check_no_paper)GetProcAddress(hDLL,L"zp_check_no_paper")))goto err;
	if(!(p_zp_check_cover_opened = (t_zp_check_cover_opened)GetProcAddress(hDLL,L"zp_check_cover_opened")))goto err;
	if(!(p_zp_error_status = (t_zp_error_status)GetProcAddress(hDLL,L"zp_error_status")))goto err;
	if(!(p_zp_printer_status_detect = (t_zp_printer_status_detect)GetProcAddress(hDLL,L"zp_printer_status_detect")))goto err;
	if(!(p_zp_printer_status_get = (t_zp_printer_status_get)GetProcAddress(hDLL,L"zp_printer_status_get")))goto err;
	if(!(p_zp_set_date_ziped = (t_zp_set_date_ziped)GetProcAddress(hDLL,L"zp_set_date_ziped")))goto err;
		
	if(!(p_zp_page_create = (t_zp_page_create)GetProcAddress(hDLL,L"zp_page_create")))goto err;
	if(!(p_zp_page_free = (t_zp_page_free)GetProcAddress(hDLL,L"zp_page_free")))goto err;
	if(!(p_zp_page_clear = (t_zp_page_clear)GetProcAddress(hDLL,L"zp_page_clear")))goto err;
	if(!(p_zp_page_print = (t_zp_page_print)GetProcAddress(hDLL,L"zp_page_print")))goto err;
	if(!(p_zp_page_get_dc = (t_zp_page_get_dc)GetProcAddress(hDLL,L"zp_page_get_dc")))goto err;
	if(!(p_zp_page_set_pen = (t_zp_page_set_pen)GetProcAddress(hDLL,L"zp_page_set_pen")))goto err;
	if(!(p_zp_page_set_font = (t_zp_page_set_font)GetProcAddress(hDLL,L"zp_page_set_font")))goto err;
	if(!(p_zp_draw_rect = (t_zp_draw_rect)GetProcAddress(hDLL,L"zp_draw_rect")))goto err;
	if(!(p_zp_draw_line = (t_zp_draw_line)GetProcAddress(hDLL,L"zp_draw_line")))goto err;
	if(!(p_zp_draw_bitmap = (t_zp_draw_bitmap)GetProcAddress(hDLL,L"zp_draw_bitmap")))goto err;
	if(!(p_zp_draw_text = (t_zp_draw_text)GetProcAddress(hDLL,L"zp_draw_text")))goto err;
	if(!(p_zp_draw_convert_text_ex = (t_zp_draw_convert_text_ex)GetProcAddress(hDLL,L"zp_draw_convert_text_ex")))goto err;
	if(!(p_zp_draw_text_ex = (t_zp_draw_text_ex)GetProcAddress(hDLL,L"zp_draw_text_ex")))goto err;
	if(!(p_zp_draw_text_box = (t_zp_draw_text_box)GetProcAddress(hDLL,L"zp_draw_text_box")))goto err;
	if(!(p_zp_draw_barcode = (t_zp_draw_barcode)GetProcAddress(hDLL,L"zp_draw_barcode")))goto err;
	if(!(p_zp_goto_feed_back = (t_zp_goto_feed_back)GetProcAddress(hDLL,L"zp_goto_feed_back")))goto err;
	if(!(p_zp_goto_feed = (t_zp_goto_feed)GetProcAddress(hDLL,L"zp_goto_feed")))goto err;
	if(!(p_zp_goto_mark_left = (t_zp_goto_mark_left)GetProcAddress(hDLL,L"zp_goto_mark_left")))goto err;
	if(!(p_zp_goto_mark_right = (t_zp_goto_mark_right)GetProcAddress(hDLL,L"zp_goto_mark_right")))goto err;
	if(!(p_zp_goto_mark_label = (t_zp_goto_mark_label)GetProcAddress(hDLL,L"zp_goto_mark_label")))goto err;
	if(!(p_zp_goto_mark_lable_begin = (t_zp_goto_mark_lable_begin)GetProcAddress(hDLL,L"zp_goto_mark_lable_begin")))goto err;
	if(!(p_zp_goto_mark_lable_end = (t_zp_goto_mark_lable_end)GetProcAddress(hDLL,L"zp_goto_mark_lable_end")))goto err;
	if(!(p_zp_cut_pagehead = (t_zp_cut_pagehead)GetProcAddress(hDLL,L"zp_cut_pagehead")))goto err;
	if(!(p_zp_draw_barcode2d_pdf417 = (t_zp_draw_barcode2d_pdf417)GetProcAddress(hDLL,L"zp_draw_barcode2d_pdf417")))goto err;
	if(!(p_zp_draw_barcode2d_qrcode = (t_zp_draw_barcode2d_qrcode)GetProcAddress(hDLL,L"zp_draw_barcode2d_qrcode")))goto err;
	if(!(p_zp_draw_barcode2d_datamatrix = (t_zp_draw_barcode2d_datamatrix)GetProcAddress(hDLL,L"zp_draw_barcode2d_datamatrix")))goto err;
	return true;
err:
	if(hDLL)FreeLibrary(hDLL);
	hDLL=NULL;
	WCHAR BUF[256];
	wsprintf(BUF,L"Load %s Failed!",ZP_SDK_FILE);
	MessageBox(hWnd,BUF,L"Error",MB_OK);
	return false;	
}
//-------------------------------------------------------------------------------------------------
void zp_sdk_deinit(void)
{
	if(hDLL)FreeLibrary(hDLL);
	hDLL=NULL;
}
//-------------------------------------------------------------------------------------------------

ZP_API void zp_set_serial_device_handle(HANDLE serialDebiceHandle)
{
	p_zp_set_serial_device_handle(serialDebiceHandle);
}
ZP_API bool zp_open_wifi(WCHAR *ip)
{
	return p_zp_open_wifi(ip);
}
ZP_API bool zp_open(int Port, unsigned long Baudrate)
{
	return p_zp_open(Port,Baudrate);
}
ZP_API bool zp_open_auto(void)
{
	return p_zp_open_auto();
}
ZP_API void zp_close(void)
{
	p_zp_close();
}
ZP_API void zp_show_error_msg(bool Enable)
{
	p_zp_show_error_msg(Enable);
}
ZP_API WCHAR *zp_get_error_msg(void)
{
	return p_zp_get_error_msg();
}
ZP_API int zp_check_no_paper(void)
{
	return p_zp_check_no_paper();
}
ZP_API int zp_check_cover_opened(void)
{
	return p_zp_check_cover_opened();
}
ZP_API int zp_error_status(int timeout)
{
	return p_zp_error_status(timeout);
}
ZP_API void zp_printer_status_detect(void)
{
	p_zp_printer_status_detect();
}
ZP_API int zp_printer_status_get(int timeout)
{
	return p_zp_printer_status_get(timeout);
}
ZP_API void zp_set_date_ziped(bool Enabled)
{
	p_zp_set_date_ziped(Enabled);
}



ZP_API bool zp_page_create(double PageWidthMM,double PageHeightMM,bool Rotate)
{
	return p_zp_page_create(PageWidthMM,PageHeightMM,Rotate);
}
ZP_API void zp_page_free(void)
{
	p_zp_page_free();
}
ZP_API void zp_page_clear(void)
{
	p_zp_page_clear();
}
ZP_API bool zp_page_print(void)
{
	return p_zp_page_print();
}
ZP_API HDC  zp_page_get_dc(void)
{
	return p_zp_page_get_dc();
}
ZP_API void zp_page_set_pen(int PenWidth)
{
	p_zp_page_set_pen(PenWidth);
}
ZP_API void zp_page_set_pen_color(bool IsBlack)
{
	p_zp_page_set_pen_color(IsBlack);
}
ZP_API void zp_page_set_font(WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline)
{
	p_zp_page_set_font(FontName,FontHeight,Angle,Bold,Italic,Underline);
}
ZP_API void zp_draw_rect(double left,double top,double right,double bottom,int PenWidth)
{
	p_zp_draw_rect(left,top,right,bottom,PenWidth);
}
ZP_API void zp_draw_line(double x0,double y0,double x1,double y1,int PenWidth)
{
	p_zp_draw_line(x0,y0,x1,y1,PenWidth);
}
ZP_API void zp_draw_bitmap(WCHAR *RelateBitmapFileName,double left,double top,double right,double bottom)
{
	p_zp_draw_bitmap(RelateBitmapFileName,left,top,right,bottom);
}
ZP_API void zp_draw_text(double x,double y,WCHAR *text)
{
	p_zp_draw_text(x,y,text);
}
ZP_API void zp_draw_convert_text_ex(double x,double y,double width,double height,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline)
{
	p_zp_draw_convert_text_ex(x,y,width,height,text,FontName,FontHeight,Angle,Bold,Italic,Underline);
}
ZP_API void zp_draw_text_ex(double x,double y,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline)
{
	p_zp_draw_text_ex(x,y,text,FontName,FontHeight,Angle,Bold,Italic,Underline);
}
ZP_API void zp_draw_text_box(double x,double y,double width,double height,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline)
{
	p_zp_draw_text_box(x,y,width,height,text,FontName,FontHeight,Angle,Bold,Italic,Underline);
}
ZP_API bool zp_draw_barcode(double x,double y,WCHAR *pData,BARCODE_TYPE type,double Height,int LineWidth,int Rotate)
{
	return p_zp_draw_barcode(x,y,pData,type,Height,LineWidth,Rotate);
}
ZP_API bool zp_goto_feed_back(int MaxFeedMM)
{
	return p_zp_goto_feed_back(MaxFeedMM);
}
ZP_API bool zp_goto_feed(int MaxFeedMM)
{
	return p_zp_goto_feed(MaxFeedMM);
}
ZP_API bool zp_goto_mark_left(int MaxFeedMM)
{
	return p_zp_goto_mark_left(MaxFeedMM);
}
ZP_API bool zp_goto_mark_right(int MaxFeedMM)
{
	return p_zp_goto_mark_right(MaxFeedMM);
}
ZP_API bool zp_goto_mark_label(int MaxFeedMM)
{
	return p_zp_goto_mark_label(MaxFeedMM);
}
ZP_API bool zp_goto_mark_any(int MaxFeedMM)
{
	return p_zp_goto_mark_any(MaxFeedMM);
}
ZP_API bool zp_goto_mark_lable_begin(int MaxFeedMM)
{
	return p_zp_goto_mark_lable_begin(MaxFeedMM);
}
ZP_API bool zp_goto_mark_lable_end(int MaxFeedMM)
{
	return p_zp_goto_mark_lable_end(MaxFeedMM);
}
ZP_API bool zp_cut_pagehead(void)
{
	return p_zp_cut_pagehead();
}
ZP_API bool zp_draw_barcode2d_pdf417(double x,double y,WCHAR *pData,int ColSize,int Size,int Rotate)
{
	return p_zp_draw_barcode2d_pdf417(x,y,pData,ColSize,Size,Rotate);
}
ZP_API bool zp_draw_barcode2d_qrcode(double x,double y,WCHAR *pData,int Ver,int Size,int Rotate)
{
	return p_zp_draw_barcode2d_qrcode(x,y,pData,Ver,Size,Rotate);
}
ZP_API bool zp_draw_barcode2d_datamatrix(double x,double y,WCHAR *pData,int Size,int Rotate)
{
	return p_zp_draw_barcode2d_datamatrix(x,y,pData,Size,Rotate);
}

