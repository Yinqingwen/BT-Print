#ifndef ZP_SDK_H
#define ZP_SDK_H

#ifdef ZP_SDK_EXPORT
#define ZP_API //extern "C" __declspec(dllexport)
#else
#define ZP_API //extern "C" __declspec(dllimport)
#endif

//ֱ�����ô���devicehandle���ɵ��õ�Ӧ�ó����𴮿ڵĴ򿪹رգ�������SDK��open��close����
ZP_API void zp_set_serial_device_handle(HANDLE serialDeviceHandle);
//-----------------------------------------------------------------------------------------------------------------
//ʵ�ִ�ӡ��TCP���ӣ�ipΪ��ӡ��ip��ַ������"192.168.1.56"
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_open_wifi(WCHAR *ip);
//-----------------------------------------------------------------------------------------------------------------
//�򿪴�ӡ���˿ڣ�portΪ�˿ںţ�����1,2,3�ȣ�BaudrateΪ�����ʣ���Ϊ115200
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_open(int Port, unsigned long Baudrate);
//-----------------------------------------------------------------------------------------------------------------
//�Զ��򿪴�ӡ���˿�
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_open_auto(void);

//�رմ�ӡ���˿�
ZP_API void zp_close(void);

//��ʾ��ӡ��������Ϣ��Enable��true:��ʾ,false:����ʾ
ZP_API void zp_show_error_msg(bool Enable);
//��ô�ӡ��������Ϣ
ZP_API WCHAR *zp_get_error_msg(void);
//���ش�ӡ�Ƿ�ȱֽ��״̬��-1�޷���ô�ӡ��״̬��0��û��ȱֽ��1��ȱֽ
ZP_API int zp_check_no_paper(void);
//���ش�ӡ�Ƿ�ֽ�ָǴ򿪵�״̬��-1�޷���ô�ӡ��״̬��0��û�д򿪣�1����
ZP_API int zp_check_cover_opened(void);
//���ش�ӡ��״̬��-1����Ϣ��0�޹��ϣ�bit0ֽ�ָǿ���bit1ȱֽ��bit2��ӡͷ����
ZP_API int zp_error_status(int timeout);
//���ʹ�ӡ��״̬��ѯָ���������
ZP_API void zp_printer_status_detect(void);
//�õ���ӡ��״̬����zp_printer_status_detect����ʹ��
ZP_API int zp_printer_status_get(int timeout);
//���ô�ӡ���ݷ���ѹ��
ZP_API void zp_set_date_ziped(bool Enabled);

//-----------------------------------------------------------------------------------------------------------------
//������ӡ��ǩҳ��PageWidthMM:��ǩҳ��ȣ���λΪmm��XT4131A�ͺţ����Ϊ80mm;
//PageHeightMM:��ǩҳ��ȣ���λΪmm
//Rotate:falseΪ����ת��trueΪ��ת����
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_page_create(double PageWidthMM,double PageHeightMM,bool Rotate);

//�ͷű�ǩҳ����zp_page_create���ʹ��
ZP_API void zp_page_free(void);

//�����ǩҳ����
ZP_API void zp_page_clear(void);

//��ӡzp_page_create�ı�ǩҳ
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_page_print(void);

//----------------------------------------------------

//��ȡ��ǰ��ǩ��DC��������Ԥ����ʾ
//���ص�ǰ��ǩ��DC
ZP_API HDC  zp_page_get_dc(void);

//���û��ʣ�PenWidth:Ϊ���ʿ�ȣ���λΪ����
ZP_API void zp_page_set_pen(int PenWidth);

//���û�����ɫ��trueΪ��ɫ��falseΪ��ɫ
ZP_API void zp_page_set_pen_color(bool IsBlack);

//��������
//FontName:��������("����")
//FontHeight:����߶ȣ���λΪmm
//Angle:��ת�Ƕ�
//Bold:����
//Italic:б��
//Underline:�»���
ZP_API void zp_page_set_font(WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
//-----------------------------------------------------------------------------------------------------------------

//������
//left���λ�ã���λΪmm
//top����λ�ã���λΪmm
//right�ұ�λ�ã���λΪmm
//bottom�ױ�λ�ã���λΪmm
//PenWidth:Ϊ���ʿ�ȣ���λΪ����
ZP_API void zp_draw_rect(double left,double top,double right,double bottom,int PenWidth);

//��ֱ��
//x0,y0Ϊ�����λ�����꣬��λΪmm
//x1,y1Ϊ��ĩ��λ�����꣬��λΪmm
//PenWidth:Ϊ���ʿ�ȣ���λΪ����
ZP_API void zp_draw_line(double x0,double y0,double x1,double y1,int PenWidth);

//���ż���λͼ��λͼΪbmp��ʽ��ɫֵΪ2λ
//RelateBitmapFileNameλͼ���ƣ�����background.bmp��λͼ����ʹ˶�̬����ͬһ·���£�
//left����λͼ���λ�ã���λΪmm
//top����λͼ����λ�ã���λΪmm
//right����λͼ�ұ�λ�ã���λΪmm
//bottom����λͼ�ױ�λ�ã���λΪmm
ZP_API void zp_draw_bitmap(WCHAR *RelateBitmapFileName,double left,double top,double right,double bottom);

//д����
//x,yΪ������ʼλ�����꣬��λΪmm
//textΪλ������
ZP_API void zp_draw_text(double x,double y,WCHAR *text);

//д��ɫ����
//x,yΪ������ʼλ�����꣬��λΪmm
//textΪ��������
//FontName:��������("����")
//FontHeight:����߶ȣ���λΪmm
//Angle:��ת�Ƕ�
//Bold:����
//Italic:б��
//Underline:�»���
ZP_API void zp_draw_convert_text_ex(double x,double y,double width,double height,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
//д����
//x,yΪ������ʼλ�����꣬��λΪmm
//textΪ��������
//FontName:��������("����")
//FontHeight:����߶ȣ���λΪmm
//Angle:��ת�Ƕ�
//Bold:����
//Italic:б��
//Underline:�»���
ZP_API void zp_draw_text_ex(double x,double y,WCHAR *text,WCHAR *FontName,double FontHeight,int Angle,bool Bold,bool Italic,bool Underline);
//�ڷ�����д���֣�����ݿ�Ĵ�С�Զ�����
//x,yΪ������ʼλ�����꣬��λΪmm
//width,heightΪ��Ŀ�Ⱥ͸߶�
//textΪ��������
//FontName:��������("����")
//FontHeight:����߶ȣ���λΪmm
//Angle:��ת�Ƕ�
//Bold:����
//Italic:б��
//Underline:�»���
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

//������
//x,yΪ������ʼλ�����꣬��λΪmm
//pDataΪ��������
//typeΪ�������ͣ�Ŀǰֻ֧�� BARCODE_CODE128
//Height����߶ȣ���λΪmm
//LineWidth�߿�ȣ���λΪ���أ��Ƽ�2
//Rotate��ת�Ƕȣ�0,90,180,270�ĸ�ѡ��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_draw_barcode(double x,double y,WCHAR *pData,BARCODE_TYPE type,double Height,int LineWidth,int Rotate);
//-----------------------------------------------------------------------------------------------------------------
//����ֽ����ֽMaxFeedMM��mm��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_goto_feed_back(int MaxFeedMM);

//��ֽ����ֽMaxFeedMM��mm��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_goto_feed(int MaxFeedMM);

//��ֽ����ڱ꣬�����MaxFeedMM��mm��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_goto_mark_left(int MaxFeedMM);

//��ֽ���Һڱ꣬�����MaxFeedMM��mm��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_goto_mark_right(int MaxFeedMM);

//��ֽ����ǩ�������MaxFeedMM��mm��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_goto_mark_label(int MaxFeedMM);

//��ֽ���κα�־����������ڱ꣬�Һڱ꣬��ǩ�������MaxFeedMM��mm��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_goto_mark_any(int MaxFeedMM);

//��ֽ����ǩ���أ���ʼ�����������MaxFeedMM��mm��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_goto_mark_lable_begin(int MaxFeedMM);

//��ֽ����ǩ���أ���β�����������MaxFeedMM��mm��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_goto_mark_lable_end(int MaxFeedMM);
//-----------------------------------------------------------------------------------------------------------------
//ȥ��ҳü
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_cut_pagehead(void);

//---------------------------------------------------------------------------------------------------------------
// V1.3
//��pdf417 ��ά����
//x,yΪ������ʼλ�����꣬��λΪmm
//pDataΪ��������
//ColSize�����ȣ���λΪmm
//Size���С����λΪ���أ��Ƽ�3
//Rotate��ת�Ƕȣ�0,90,180,270�ĸ�ѡ��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_draw_barcode2d_pdf417(double x,double y,WCHAR *pData,int ColSize,int Size,int Rotate);

//��qrcode ��ά����
//x,yΪ������ʼλ�����꣬��λΪmm
//pDataΪ��������
//Ver����Ϊ4�����Ķ�
//Size����Ĵ�С���Ƽ�5
//Rotate��ת�Ƕȣ�0,90,180,270�ĸ�ѡ��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_draw_barcode2d_qrcode(double x,double y,WCHAR *pData,int Ver,int Size,int Rotate);
//��datamatrix��ά����
//x,yΪ������ʼλ�����꣬��λΪmm
//pDataΪ��������
//Size ��������Ĵ�С
//Rotate��ת�Ƕȣ�0,90,180,270�ĸ�ѡ��
//return == false ָ��ʧ��
//return == true  ָ��ɹ�
ZP_API bool zp_draw_barcode2d_datamatrix(double x,double y,WCHAR *pData,int Size,int Rotate);



#endif


