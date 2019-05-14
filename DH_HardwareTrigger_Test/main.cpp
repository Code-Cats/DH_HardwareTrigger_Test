/**
@brief 大恒相机硬触发采集测试程序BaseOn Windows 修改自CDC的无注释程序
@author 余鑫 NCUT-IFR
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

GX_DEV_HANDLE       m_hDevice;              //< 设备句柄  
BYTE                *m_pBufferRaw;          //< 原始图像数据  
BYTE                *m_pBufferRGB;          //< RGB图像数据，用于显示和保存bmp图像  
int64_t             m_nImageHeight;         //< 原始图像高  
int64_t             m_nImageWidth;          //< 原始图像宽  
int64_t             m_nPayLoadSize;
int64_t            m_nPixelColorFilter;    //< Bayer格式
Mat test;

int frame_count = 0;
///回调函数
static void GX_STDC OnFrameCallbackFun(GX_FRAME_CALLBACK_PARAM* pFrame)
{
	if (pFrame->status == 0)
	{
		frame_count++;
		memcpy(m_pBufferRaw, pFrame->pImgBuf, pFrame->nImgSize);
		// RGB转换
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
	GX_OPEN_PARAM openParam;	///用户配置的打开设备参数,参见GX_OPEN_PARAM结构体定义
	uint32_t nDeviceNum = 0;
	openParam.accessMode = GX_ACCESS_EXCLUSIVE;	 ///< 访问模式-< 独占方式
	openParam.openMode = GX_OPEN_INDEX;	///< 打开方式-< 通过Index打开
	openParam.pszContent = "1";	///< 输入参数内容,不允许为空字符串
	//初始化库   	
	emStatus = GXInitLib();
	if (emStatus != GX_STATUS_SUCCESS)
	{
		return 0;
	}
	//枚举所有设备并且获取设备个数,对于千兆网设备此接口仅能枚举同网段设备
	emStatus = GXUpdateDeviceList(&nDeviceNum, 1000); 
	if ((emStatus != GX_STATUS_SUCCESS) || (nDeviceNum <= 0))
	{
		return 0;
	}
	/**
	\brief     通过指定唯一标示打开设备，例如指定SN、IP、MAC等
	\attention 此接口调用之前需要调用GXUpdateDeviceList接口，更新库内部设备列表
	\param     [in]pOpenParam    用户配置的打开设备参数,参见GX_OPEN_PARAM结构体定义
	\param     [out]phDevice     返回设备句柄
	*/
	emStatus = GXOpenDevice(&openParam, &m_hDevice);
	//设置采集模式连续采集 
	//GXSetEnum\brief 设置枚举型值的当前值  \param[in]hDevice 设备句柄 \param[in]featureID 功能码ID \param[in]pnValue 用户设置的当前值
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);	///< 采集模式,参考GX_ACQUISITION_MODE_ENTRY-<连续模式

	emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);	//打开触发模式 
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_SWITCH, GX_TRIGGER_SWITCH_ON);	//打开外触发模式 
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE2);	//设置触发源LINE2 
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_SELECTOR, GX_ENUM_TRIGGER_SELECTOR_FRAME_START);	//触发类型：采集一帧
	///emStatus = GXSetEnum(m_hDevice, GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE2);

	/**\brief 设置Int类型值的当前值 \param [in]hDevice 设备句柄 \param [in]featureID 功能码ID \param [in]pnValue 用户设置的当前值*/
	emStatus = GXSetInt(m_hDevice, GX_INT_ACQUISITION_SPEED_LEVEL, 1);///< 采集速度级别-
	emStatus = GXSetEnum(m_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_OFF);	 ///<自动白平衡, 参考GX_BALANCE_WHITE_AUTO_ENTRY-< 关闭自动白平衡

	//设置硬件图像反转
	//emStatus=GXSetBool(m_hDevice, GX_BOOL_REVERSE_X, true);	//硬件不支持
	//emStatus=GXSetBool(m_hDevice, GX_BOOL_REVERSE_Y, true);	//硬件不支持

	bool      bColorFliter = false;
	//获取图像大小  	
	emStatus = GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &m_nPayLoadSize);
	//获取宽度  	
	emStatus = GXGetInt(m_hDevice, GX_INT_WIDTH, &m_nImageWidth);
	//获取高度  	
	emStatus = GXGetInt(m_hDevice, GX_INT_HEIGHT, &m_nImageHeight);
	test.create(m_nImageHeight, m_nImageWidth, CV_8UC3);
	//判断相机是否支持bayer格式  
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
	//为存储原始图像数据申请空间  
	m_pBufferRaw = new BYTE[(size_t)m_nPayLoadSize];
	if (m_pBufferRaw == NULL)
	{
		delete[]m_pBufferRGB;
		m_pBufferRGB = NULL;
		return false;
	}
	//注册图像处理回调函数 
	emStatus = GXRegisterCaptureCallback(m_hDevice, NULL, OnFrameCallbackFun);
	/**
	\brief      发送控制命令
		\attention  无
		\param[in]hDevice    设备句柄
		\param[in]featureID  功能码ID
	*/
	emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);	///< 开始采集
	//---------------------  
	Mat SrcImg;
	while (1)
	{
		cout << "采集帧数：" << frame_count<< endl;
		test.copyTo(SrcImg);
		imshow("raw image",SrcImg);
		waitKey(1);
	}

	//发送停采命令  
	emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
	//注销采集回调  
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





