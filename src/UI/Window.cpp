/*
 * Window.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#include "Window.h"
#include "../Script/Kiss.h"

namespace kai
{

Window::Window()
{
	m_textPos.init();
	m_textStart.x = 15;
	m_textStart.y = 15;
	m_size.x = 1280;
	m_size.y = 720;
	m_bFullScreen = false;
	m_pixTab = TAB_PIX;
	m_lineHeight = LINE_HEIGHT;
	m_textSize = 0.5;
	m_textCol = Scalar(0, 255, 0);
	m_bWindow = true;
	m_gstOutput = "";
	m_fileRec = "";
}

Window::~Window()
{
	if (m_VW.isOpened())
	{
		m_VW.release();
	}
}

bool Window::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	F_ERROR_F(pK->root()->o("APP")->v("bWindow", &m_bWindow));
	if (!m_bWindow)
	{
		LOG_E("Window mode is disabled. Turn \"bWindow\":1 to enable");
	}

	pK->m_pInst = this;

	F_INFO(pK->v("w", &m_size.x));
	F_INFO(pK->v("h", &m_size.y));
	F_INFO(pK->v("bFullScreen", &m_bFullScreen));

	if (m_size.area() <= 0)
	{
		LOG_E("Window size too small");
		return false;
	}

	KISSm(pK,fileRec);
	if (!m_fileRec.empty())
	{
		int recFPS = 30;
		string reCodec = "MJPG";
		F_INFO(pK->v("recFPS", &recFPS));
		F_INFO(pK->v("recCodec", &reCodec));

		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		char strTime[128];
		strftime(strTime, sizeof(strTime), "%c", tm);
		m_fileRec += strTime;
		m_fileRec += ".avi";

		if (!m_VW.open(m_fileRec,
						CV_FOURCC(reCodec.at(0),
						reCodec.at(1),
						reCodec.at(2),
						reCodec.at(3)),
						recFPS,
						cv::Size(m_size.x, m_size.y)))
		{
			LOG_E("Cannot open file recording");
			return false;
		}
	}

	KISSm(pK,gstOutput);
	if (!m_gstOutput.empty())
	{
		if (!m_gst.open(m_gstOutput,
						CAP_GSTREAMER,
						0,
						30,
						cv::Size(m_size.x, m_size.y),
						true))
		{
			LOG_E("Cannot open GStreamer output");
			return false;
		}
	}

	F_INFO(pK->v("textX", &m_textStart.x));
	F_INFO(pK->v("textY", &m_textStart.y));
	F_INFO(pK->v("pixTab", &m_pixTab));
	F_INFO(pK->v("lineH", &m_lineHeight));
	F_INFO(pK->v("textSize", &m_textSize));
	F_INFO(pK->v("textB", &m_textCol[0]));
	F_INFO(pK->v("textG", &m_textCol[1]));
	F_INFO(pK->v("textR", &m_textCol[2]));

	m_frame.allocate(m_size.x, m_size.y);

	IF_T(!m_bWindow);

	if (m_bFullScreen)
	{
		namedWindow(*this->getName(), CV_WINDOW_NORMAL);
		setWindowProperty(*this->getName(), CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	}
	else
	{
		namedWindow(*this->getName(), CV_WINDOW_AUTOSIZE);
	}

	return true;
}

bool Window::link(void)
{
	return true;
}

bool Window::draw(void)
{
	IF_F(m_frame.bEmpty());

	if (m_bWindow)
	{
		imshow(*this->getName(), *m_frame.m());
	}

	Frame* pSrc;
	Frame* pDest;
	Frame* pTmp;
	pSrc = &m_F;
	pDest = &m_F2;

	m_F.copy(m_frame);
	Size fSize = pSrc->size();

	if (fSize.width != m_size.x || fSize.height != m_size.y)
	{
		*pDest = pSrc->resize(m_size.x, m_size.y);
		SWAP(pSrc, pDest, pTmp);
	}

	if (pSrc->m()->type() != CV_8UC3)
	{
		*pSrc = pDest->f8UC3();
		SWAP(pSrc, pDest, pTmp);
	}

	if (m_VW.isOpened())
	{
		m_VW << *pSrc->m();
	}

	if (m_gst.isOpened())
	{
		m_gst << *pSrc->m();
	}

	m_frame.allocate(m_size.x, m_size.y);
	tabReset();
	lineReset();

	return true;
}

Frame* Window::getFrame(void)
{
	return &m_frame;
}

Point* Window::getTextPos(void)
{
	m_tPoint.x = m_textPos.x;
	m_tPoint.y = m_textPos.y;
	return &m_tPoint;
}

void Window::tabNext(void)
{
	m_textPos.x += m_pixTab;
}

void Window::tabPrev(void)
{
	m_textPos.x -= m_pixTab;
	if (m_textPos.x < m_textStart.x)
		m_textPos.x = m_textStart.x;
}

void Window::tabReset(void)
{
	m_textPos.x = m_textStart.x;
}

void Window::lineNext(void)
{
	m_textPos.y += m_lineHeight;
}

void Window::lineReset(void)
{
	m_textPos.y = m_textStart.y;
}

double Window::textSize(void)
{
	return m_textSize;
}

Scalar Window::textColor(void)
{
	return m_textCol;
}

void Window::addMsg(string* pMsg)
{
	IF_(!m_bDraw);

	putText(*m_frame.m(), *pMsg, *getTextPos(), FONT_HERSHEY_SIMPLEX,
			m_textSize, m_textCol, 1);
	lineNext();
}

}
