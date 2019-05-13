/**
@brief ������Ӳ�����ɼ����Գ���BaseOn Windows �޸���CDC����ע�ͳ���
@author ���� NCUT-IFR
@time 2019.5.13
*/
#include <GxIAPI.h>
#include <DxImageProc.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <math.h>

using namespace std;
using namespace cv;

GX_DEV_HANDLE       m_hDevice;              //< �豸���  
BYTE                *m_pBufferRaw;          //< ԭʼͼ������  
BYTE                *m_pBufferRGB;          //< RGBͼ�����ݣ�������ʾ�ͱ���bmpͼ��  
int64_t             m_nImageHeight;         //< ԭʼͼ���  
int64_t             m_nImageWidth;          //< ԭʼͼ���  
int64_t             m_nPayLoadSize;
int64_t            m_nPixelColorFilter;    //< Bayer��ʽ
Mat test;

int frame_count = 0;
///�ص�����
static void GX_STDC OnFrameCallbackFun(GX_FRAME_CALLBACK_PARAM* pFrame)
{
	if (pFrame->status == 0)
	{
		frame_count++;
		memcpy(m_pBufferRaw, pFrame->pImgBuf, pFrame->nImgSize);
		// RGBת��
		DxRaw8toRGB24(m_pBufferRaw
			, m_pBufferRGB
			, (VxUint32)(m_nImageWidth)
			, (VxUint32)(m_nImageHeight)
			, RAW2RGB_NEIGHBOUR
			, DX_PIXEL_COLOR_FILTER(m_nPixelColorFilter)
			, false);
		memcpy(test.data, m_pBufferRGB, m_nImageWidth*m_nImageHeight * 3);
	}
	return;
}


int main(int argc, char* argv[])
{
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	GX_OPEN_PARAM openParam;
	uint32_t nDeviceNum = 0;
	openParam.accessMode = GX_ACCESS_EXCLUSIVE;
	openParam.openMode = GX_OPEN_INDEX;
	openParam.pszContent = "1";
	//��ʼ����   	
	emStatus = GXInitLib();
	if (emStatus != GX_STATUS_SUCCESS)
	{
		return 0;
	}
	//ö���豸�б�
	emStatus = GXUpdateDeviceList(&nDeviceNum, 1000);
	if ((emStatus != GX_STATUS_SUCCESS) || (nDeviceNum <= 0))
	{
		return 0;
	}
	//���豸 
	emStatus = GXOpenDevice(&openParam, &m_hDevice);
	//���òɼ�ģʽ�����ɼ� 
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
	emStatus = GXSetInt(m_hDevice, GX_INT_ACQUISITION_SPEED_LEVEL, 1);
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF);
	bool      bColorFliter = false;
	//��ȡͼ���С  	
	emStatus = GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &m_nPayLoadSize);
	//��ȡ���  	
	emStatus = GXGetInt(m_hDevice, GX_INT_WIDTH, &m_nImageWidth);
	//��ȡ�߶�  	
	emStatus = GXGetInt(m_hDevice, GX_INT_HEIGHT, &m_nImageHeight);
	test.create(m_nImageHeight, m_nImageWidth, CV_8UC3);
	//�ж�����Ƿ�֧��bayer��ʽ  
	bool m_bColorFilter;
	emStatus = GXIsImplemented(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_bColorFilter);
	if (m_bColorFilter)
	{
		emStatus = GXGetEnum(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_nPixelColorFilter);
	}
	m_pBufferRGB = new BYTE[(size_t)(m_nImageWidth * m_nImageHeight * 3)];
	if (m_pBufferRGB == NULL)
	{
		return false;
	}
	//Ϊ�洢ԭʼͼ����������ռ�  
	m_pBufferRaw = new BYTE[(size_t)m_nPayLoadSize];
	if (m_pBufferRaw == NULL)
	{
		delete[]m_pBufferRGB;
		m_pBufferRGB = NULL;
		return false;
	}
	//ע��ͼ����ص����� 
	emStatus = GXRegisterCaptureCallback(m_hDevice, NULL, OnFrameCallbackFun);
	//���Ϳ�������
	emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);
	//---------------------  
	while (1)
	{

	}

	//����ͣ������  
	emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
	//ע���ɼ��ص�  
	emStatus = GXUnregisterCaptureCallback(m_hDevice);
	if (m_pBufferRGB != NULL)
	{
		delete[]m_pBufferRGB;
		m_pBufferRGB = NULL;
	}
	if (m_pBufferRaw != NULL)
	{
		delete[]m_pBufferRaw;
		m_pBufferRaw = NULL;
	}
	emStatus = GXCloseDevice(m_hDevice);
	emStatus = GXCloseLib();
	return 0;
}





