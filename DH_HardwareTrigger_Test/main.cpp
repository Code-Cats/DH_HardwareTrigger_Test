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
	GXCloseLib();
	Sleep(100);
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	GX_OPEN_PARAM openParam;	///�û����õĴ��豸����,�μ�GX_OPEN_PARAM�ṹ�嶨��
	uint32_t nDeviceNum = 0;
	openParam.accessMode = GX_ACCESS_EXCLUSIVE;	 ///< ����ģʽ-< ��ռ��ʽ
	openParam.openMode = GX_OPEN_INDEX;	///< �򿪷�ʽ-< ͨ��Index��
	openParam.pszContent = "1";	///< �����������,������Ϊ���ַ���
	//��ʼ����   	
	emStatus = GXInitLib();
	if (emStatus != GX_STATUS_SUCCESS)
	{
		return 0;
	}
	//ö�������豸���һ�ȡ�豸����,����ǧ�����豸�˽ӿڽ���ö��ͬ�����豸
	emStatus = GXUpdateDeviceList(&nDeviceNum, 1000); 
	if ((emStatus != GX_STATUS_SUCCESS) || (nDeviceNum <= 0))
	{
		return 0;
	}
	/**
	\brief     ͨ��ָ��Ψһ��ʾ���豸������ָ��SN��IP��MAC��
	\attention �˽ӿڵ���֮ǰ��Ҫ����GXUpdateDeviceList�ӿڣ����¿��ڲ��豸�б�
	\param     [in]pOpenParam    �û����õĴ��豸����,�μ�GX_OPEN_PARAM�ṹ�嶨��
	\param     [out]phDevice     �����豸���
	*/
	emStatus = GXOpenDevice(&openParam, &m_hDevice);
	//���òɼ�ģʽ�����ɼ� 
	//GXSetEnum\brief ����ö����ֵ�ĵ�ǰֵ  \param[in]hDevice �豸��� \param[in]featureID ������ID \param[in]pnValue �û����õĵ�ǰֵ
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);	///< �ɼ�ģʽ,�ο�GX_ACQUISITION_MODE_ENTRY-<����ģʽ

	emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);	//�򿪴���ģʽ 
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_SWITCH, GX_TRIGGER_SWITCH_ON);	//���ⴥ��ģʽ 
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE2);	//���ô���ԴLINE2 
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_SELECTOR, GX_ENUM_TRIGGER_SELECTOR_FRAME_START);	//�������ͣ��ɼ�һ֡
	///emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE2);

	/**\brief ����Int����ֵ�ĵ�ǰֵ \param [in]hDevice �豸��� \param [in]featureID ������ID \param [in]pnValue �û����õĵ�ǰֵ*/
	emStatus = GXSetInt(m_hDevice, GX_INT_ACQUISITION_SPEED_LEVEL, 1);///< �ɼ��ٶȼ���-
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF);	 ///<�Զ���ƽ��, �ο�GX_BALANCE_WHITE_AUTO_ENTRY-< �ر��Զ���ƽ��

	//����Ӳ��ͼ��ת
	//emStatus=GXSetBool(m_hDevice, GX_BOOL_REVERSE_X, true);	//Ӳ����֧��
	//emStatus=GXSetBool(m_hDevice, GX_BOOL_REVERSE_Y, true);	//Ӳ����֧��

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
	/**
	\brief      ���Ϳ�������
		\attention  ��
		\param[in]hDevice    �豸���
		\param[in]featureID  ������ID
	*/
	emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);	///< ��ʼ�ɼ�
	//---------------------  
	Mat SrcImg;
	while (1)
	{
		cout << "�ɼ�֡����" << frame_count<< endl;
		test.copyTo(SrcImg);
		imshow("raw image",SrcImg);
		waitKey(1);
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





