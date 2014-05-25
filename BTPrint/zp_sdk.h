#ifndef ZP_SDK_H
#define ZP_SDK_H

#ifdef ZP_SDK_EXPORT
#define ZP_API //extern "C" __declspec(dllexport)
#else
#define ZP_API //extern "C" __declspec(dllimport)
#endif

//直接设置串口devicehandle，由调用的应用程序负责串口的打开关闭，不调用SDK的open与close函数
ZP_API void zp_set_serial_device_handle(HANDLE serialDeviceHandle);
//-----------------------------------------------------------------------------------------------------------------
//实现打印机TCP连接，ip为打印机ip地址，例如"192.168.1.56"
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_open_wifi(WCHAR *ip);
//-----------------------------------------------------------------------------------------------------------------
//打开打印机端口，port为端口号，例如1,2,3等，Baudrate为波特率，设为115200
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_open(int Port, unsigned long Baudrate);
//-----------------------------------------------------------------------------------------------------------------
//自动打开打印机端口
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_open_auto(void);

//关闭打印机端口
ZP_API void zp_close(void);

//显示打印机错误信息，Enable：true:显示,false:不显示
ZP_API void zp_show_error_msg(bool Enable);
//获得打印机错误信息
ZP_API WCHAR *zp_get_error_msg(void);
//返回打印是否缺纸的状态：-1无法获得打印机状态，0：没有缺纸，1：缺纸
ZP_API int zp_check_no_paper(void);
//返回打印是否纸仓盖打开的状态：-1无法获得打印机状态，0：没有打开，1：打开
ZP_API int zp_check_cover_opened(void);
//返回打印机状态：-1无信息，0无故障，bit0纸仓盖开，bit1缺纸，bit2打印头过热
ZP_API int zp_error_status(int timeout);
//发送打印机状态查询指令，立即返回
ZP_API void zp_printer_status_detect(void);
//得到打印机状态，和zp_printer_status_detect成套使用
ZP_API int zp_printer_status_get(int timeout);
//设置打印数据发送压缩
ZP_API void zp_set_date_ziped(bool Enabled);

//-----------------------------------------------------------------------------------------------------------------
//创建打印标签页，PageWidthMM:标签页宽度，单位为mm，XT4131A型号，最大为80mm;
//PageHeightMM:标签页宽度，单位为mm
//Rotate:false为不旋转，true为旋转方向
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_page_create(double PageWidthMM,double PageHeightMM,bool Rotate);

//释放标签页，与zp_page_create配对使用
ZP_API void zp_page_free(void);

//清楚标签页内容
ZP_API void zp_page_clear(void);

//打印zp_page_create的标签页
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_page_print(void);

//----------------------------------------------------

//获取当前标签的DC，可用于预览显示
//返回当前标签的DC
ZP_API HDC  zp_page_get_dc(void);

//设置画笔，PenWidth:为画笔宽度，单位为像素
ZP_API void zp_page_set_pen(int PenWidth);

//设置画笔颜色，true为黑色，false为白色
ZP_API void zp_page_set_pen_color(bool IsBlack);

//设置字体
//FontName:字体名称("宋体")
//FontHeight:字体高度，单位为mm
//Angle:旋转角度
//Bold:粗体
//Italic:斜体
//Underline:下划线
ZP_API void zp_page_set_font(WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
//-----------------------------------------------------------------------------------------------------------------

//画矩形
//left左边位置，单位为mm
//top顶边位置，单位为mm
//right右边位置，单位为mm
//bottom底边位置，单位为mm
//PenWidth:为画笔宽度，单位为像素
ZP_API void zp_draw_rect(double left,double top,double right,double bottom,int PenWidth);

//画直线
//x0,y0为线起点位置坐标，单位为mm
//x1,y1为线末点位置坐标，单位为mm
//PenWidth:为画笔宽度，单位为像素
ZP_API void zp_draw_line(double x0,double y0,double x1,double y1,int PenWidth);

//缩放加载位图，位图为bmp格式，色值为2位
//RelateBitmapFileName位图名称，例如background.bmp（位图必须和此动态库在同一路径下）
//left放置位图左边位置，单位为mm
//top放置位图顶边位置，单位为mm
//right放置位图右边位置，单位为mm
//bottom放置位图底边位置，单位为mm
ZP_API void zp_draw_bitmap(WCHAR *RelateBitmapFileName,double left,double top,double right,double bottom);

//写文字
//x,y为文字起始位置坐标，单位为mm
//text为位子内容
ZP_API void zp_draw_text(double x,double y,WCHAR *text);

//写反色文字
//x,y为文字起始位置坐标，单位为mm
//text为文字内容
//FontName:字体名称("宋体")
//FontHeight:字体高度，单位为mm
//Angle:旋转角度
//Bold:粗体
//Italic:斜体
//Underline:下划线
ZP_API void zp_draw_convert_text_ex(double x,double y,double width,double height,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
//写文字
//x,y为文字起始位置坐标，单位为mm
//text为文字内容
//FontName:字体名称("宋体")
//FontHeight:字体高度，单位为mm
//Angle:旋转角度
//Bold:粗体
//Italic:斜体
//Underline:下划线
ZP_API void zp_draw_text_ex(double x,double y,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
//在方框内写文字，会根据框的大小自动换行
//x,y为文字起始位置坐标，单位为mm
//width,height为框的宽度和高度
//text为文字内容
//FontName:字体名称("宋体")
//FontHeight:字体高度，单位为mm
//Angle:旋转角度
//Bold:粗体
//Italic:斜体
//Underline:下划线
ZP_API void zp_draw_text_box(double x,double y,double width,double height,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);

//-----------------------------------------------------------------------------------------------------------------
enum BARCODE_TYPE
{
	BARCODE_CODE128,
	BARCODE_CODE39,
	BARCODE_CODE93,
	BARCODE_CODABAR,
	BARCODE_EAN8,
	BARCODE_EAN13,
	BARCODE_UPC,
};

//画条码
//x,y为条码起始位置坐标，单位为mm
//pData为条码内容
//type为条码类型，目前只支持 BARCODE_CODE128
//Height条码高度，单位为mm
//LineWidth线宽度，单位为像素，推荐2
//Rotate旋转角度，0,90,180,270四个选择
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_draw_barcode(double x,double y,WCHAR *pData,BARCODE_TYPE type,double Height,int LineWidth,int Rotate);
//-----------------------------------------------------------------------------------------------------------------
//回走纸，回纸MaxFeedMM（mm）
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_goto_feed_back(int MaxFeedMM);

//走纸，走纸MaxFeedMM（mm）
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_goto_feed(int MaxFeedMM);

//走纸到左黑标，最多走MaxFeedMM（mm）
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_goto_mark_left(int MaxFeedMM);

//走纸到右黑标，最多走MaxFeedMM（mm）
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_goto_mark_right(int MaxFeedMM);

//走纸到标签，最多走MaxFeedMM（mm）
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_goto_mark_label(int MaxFeedMM);

//走纸到任何标志处，包括左黑标，右黑标，标签，最多走MaxFeedMM（mm）
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_goto_mark_any(int MaxFeedMM);

//走纸到标签上沿（开始处），最多走MaxFeedMM（mm）
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_goto_mark_lable_begin(int MaxFeedMM);

//走纸到标签下沿（结尾处），最多走MaxFeedMM（mm）
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_goto_mark_lable_end(int MaxFeedMM);
//-----------------------------------------------------------------------------------------------------------------
//去除页眉
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_cut_pagehead(void);

//---------------------------------------------------------------------------------------------------------------
// V1.3
//画pdf417 二维条码
//x,y为条码起始位置坐标，单位为mm
//pData为条码内容
//ColSize条码宽度，单位为mm
//Size点大小，单位为像素，推荐3
//Rotate旋转角度，0,90,180,270四个选择
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_draw_barcode2d_pdf417(double x,double y,WCHAR *pData,int ColSize,int Size,int Rotate);

//画qrcode 二维条码
//x,y为条码起始位置坐标，单位为mm
//pData为条码内容
//Ver建议为4，不改动
//Size条码的大小，推荐5
//Rotate旋转角度，0,90,180,270四个选择
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_draw_barcode2d_qrcode(double x,double y,WCHAR *pData,int Ver,int Size,int Rotate);
//画datamatrix二维条码
//x,y为条码起始位置坐标，单位为mm
//pData为条码内容
//Size 调整条码的大小
//Rotate旋转角度，0,90,180,270四个选择
//return == false 指令失败
//return == true  指令成功
ZP_API bool zp_draw_barcode2d_datamatrix(double x,double y,WCHAR *pData,int Size,int Rotate);



#endif


