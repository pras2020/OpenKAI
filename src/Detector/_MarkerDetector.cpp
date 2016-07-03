/*
 * CameraMarkerDetect.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_MarkerDetector.h"

namespace kai
{

_MarkerDetector::_MarkerDetector()
{
	_ThreadBase();
	DetectorBase();

	m_bThreadON = false;
	m_threadID = 0;
	m_cudaDeviceID = 0;

	m_numCircle = 0;

	m_minMarkerSize = MIN_MARKER_SIZE;
	m_areaRatio = MARKER_AREA_RATIO;

	m_pCamStream = NULL;
}

_MarkerDetector::~_MarkerDetector()
{
	// TODO Auto-generated destructor stub
}


bool _MarkerDetector::init(JSON* pJson, string name)
{
//	CHECK_INFO(pJson->getVal("MARKER_DETECTOR_TSLEEP_" + name, &m_tSleep));

	this->setTargetFPS(30.0);
	return true;
}

bool _MarkerDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _MarkerDetector::update(void)
{
	cuda::setDevice(m_cudaDeviceID);

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detectCircle();

		this->autoFPSto();
	}

}

void _MarkerDetector::detectCircle(void)
{
	int i,j,k;
	Point2f center;
	float radius;
	vector< vector< Point > > contours;
	vector<Vec3f> circles;
	fVector4 v4tmp;
	Mat matThresh;
	CamFrame* pHSV;
	CamFrame* pRGB;

	if(!m_pCamStream)return;
	pHSV = m_pCamStream->getHSVFrame();
	pRGB = m_pCamStream->getFrame();
	if(pRGB->empty())return;
	if(pHSV->empty())return;

	vector<GpuMat> matHSV(3);
	matHSV[0] = m_Hue;
	matHSV[1] = m_Sat;
	matHSV[2] = m_Val;

	cuda::split(*(pHSV->getGMat()), matHSV);

	m_Hue = matHSV[0];
	m_Sat = matHSV[1];
	m_Val = matHSV[2];

	//Filtering the image
	cuda::absdiff(m_Hue, Scalar(90), m_Huered);
	cuda::multiply(m_Huered, Scalar(0.25), m_Scalehuered);	//1/4
	cuda::multiply(m_Sat, Scalar(0.0625), m_Scalesat);	//1/16
	cuda::multiply(m_Scalehuered, m_Scalesat, m_Balloonyness);
	cuda::threshold(m_Balloonyness, m_Thresh, 200, 255, THRESH_BINARY);
//	cuda::threshold(m_Balloonyness, m_Thresh, 200, 255, THRESH_BINARY_INV);
	m_Thresh.download(matThresh);

	//Find the contours
	findContours(matThresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
//		drawContours(pRGB->m_uFrame, contours, -1, Scalar(255, 0, 0));

	//Find marker
	m_numCircle = 0;
	for (i=0; i<contours.size(); i++)
	{
		minEnclosingCircle(contours[i], center, radius);

//			circle(pRGB->m_uFrame, center, radius, Scalar(0, 255, 0), 1);

		//New marker found
		if (contourArea(contours[i]) < m_areaRatio*radius*radius*3.1415926)continue;
		if (radius < m_minMarkerSize)continue;

//			circle(pRGB->m_uFrame, center, radius, Scalar(0, 255, 0), 2);

		m_pCircle[m_numCircle].m_x = center.x;
		m_pCircle[m_numCircle].m_y = center.y;
		m_pCircle[m_numCircle].m_r = radius;
		m_numCircle++;

		if (m_numCircle == NUM_MARKER)
		{
			break;
		}
	}
}

bool _MarkerDetector::getCircleCenter(fVector2* pCenter)
{
	if(pCenter==NULL)return false;

	//Use num instead of m_numCircle to avoid multi-thread inconsistancy
	int num = m_numCircle;
	if(m_numCircle==0)return false;

	int i;
	MARKER_CIRCLE avr;
	MARKER_CIRCLE center;
	double x,y;

	//Find the average point
	avr.m_x = 0;
	avr.m_y = 0;

	for(i=0; i<num; i++)
	{
		avr.m_x += m_pCircle[i].m_x;
		avr.m_y += m_pCircle[i].m_y;
	}

	avr.m_x /= num;
	avr.m_x /= num;

	//Eliminate the farest ones
	center.m_x = 0;
	center.m_y = 0;
	center.m_r = 0;

	for(i=0; i<num; i++)
	{
		x = m_pCircle[i].m_x;
		y = m_pCircle[i].m_y;

		if(abs(x-avr.m_x)>500 || abs(y-avr.m_y)>500)continue;

		center.m_x += x;
		center.m_y += y;
		center.m_r += 1;
	}

	center.m_x /= center.m_r;
	center.m_y /= center.m_r;

	pCenter->m_x = center.m_x;
	pCenter->m_y = center.m_y;

	return true;
}


} /* namespace kai */
