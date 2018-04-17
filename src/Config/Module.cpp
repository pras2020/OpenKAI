/*
 * Module.cpp
 *
 *  Created on: Nov 22, 2016
 *      Author: yankai
 */

#include "Module.h"

namespace kai
{

BASE* Module::createInstance(Kiss* pK)
{
	IF_N(!pK);

	ADD_MODULE(_AutoPilot);
	ADD_MODULE(_AprilTags);
	ADD_MODULE(_Automaton);
	ADD_MODULE(_Bullseye);
	ADD_MODULE(_Camera);
	ADD_MODULE(_Canbus);
	ADD_MODULE(_ClusterNet);
	ADD_MODULE(_Flow);
	ADD_MODULE(_GPS);
	ADD_MODULE(_GStreamer);
	ADD_MODULE(_Lane);
	ADD_MODULE(_LeddarVu);
	ADD_MODULE(_Mavlink);
	ADD_MODULE(_Path);
	ADD_MODULE(_RC);
	ADD_MODULE(_RPLIDAR);
	ADD_MODULE(_SerialPort);
	ADD_MODULE(_TCPserver);
	ADD_MODULE(_TCPsocket);
	ADD_MODULE(_TCP);
	ADD_MODULE(_UDP);
	ADD_MODULE(_UDPclient);
	ADD_MODULE(_UDPserver);
	ADD_MODULE(_WebSocket);
	ADD_MODULE(Window);

	ADD_MODULE(_Augment);
	ADD_MODULE(_filterBilateralBlur);
	ADD_MODULE(_filterBlur);
	ADD_MODULE(_filterBrightness);
	ADD_MODULE(_filterContrast);
	ADD_MODULE(_filterCrop);
	ADD_MODULE(_filterFlip);
	ADD_MODULE(_filterGaussianBlur);
	ADD_MODULE(_filterHistEqualize);
	ADD_MODULE(_filterLowResolution);
	ADD_MODULE(_filterMedianBlur);
	ADD_MODULE(_filterNoise);
	ADD_MODULE(_filterRotate);
	ADD_MODULE(_filterSaturation);
	ADD_MODULE(_filterShrink);
	ADD_MODULE(_filterTone);

	ADD_MODULE(_BBoxCutOut);
	ADD_MODULE(_FrameCutOut);
	ADD_MODULE(_MultiImageNet);

#ifdef USE_OPENCV_CONTRIB
	ADD_MODULE(_ROITracker);
#endif
#ifdef USE_CAFFE
	ADD_MODULE(_Caffe);
	ADD_MODULE(_CaffeRegressionTrain);
	ADD_MODULE(_CaffeRegressionInf);
#endif
#ifdef USE_ZED
	ADD_MODULE(_ZED);
	ADD_MODULE(_DNNGen_odometry);
#endif
#ifdef USE_SSD
	ADD_MODULE(_SSD);
#endif
#ifdef USE_DARKNET
	ADD_MODULE(_YOLO);
#endif
#ifdef USE_REALSENSE
	ADD_MODULE(_RealSense);
#endif
#ifdef USE_PYLON
	ADD_MODULE(_Pylon);
#endif
#ifdef USE_CASCADE
	ADD_MODULE(_Cascade);
#endif
#ifdef USE_ORB_SLAM2
	ADD_MODULE(_ORB_SLAM2);
#endif
#ifdef USE_TENSORRT
	ADD_MODULE(_ImageNet);
	ADD_MODULE(_DetectNet);
#endif

    return NULL;
}

template <typename T> BASE* Module::createInst(Kiss* pKiss)
{
	IF_N(!pKiss);

	T* pInst = new T();
	if(!pInst->init(pKiss))
	{
		delete pInst;
		return NULL;
	}
    return pInst;
}


Module::Module()
{
}

Module::~Module()
{
}

}
