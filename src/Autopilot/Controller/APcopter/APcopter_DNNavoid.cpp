#include "../../Controller/APcopter/APcopter_DNNavoid.h"

namespace kai
{

APcopter_DNNavoid::APcopter_DNNavoid()
{
	m_pAP = NULL;
	m_pIN = NULL;
	m_nVision = 0;
	m_action = DA_UNKNOWN;
}

APcopter_DNNavoid::~APcopter_DNNavoid()
{
}

bool APcopter_DNNavoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool APcopter_DNNavoid::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));
	if (!m_pIN)
	{
		LOG_E(iName << " not found");
		return false;
	}

	Kiss** pItr = pK->getChildItr();
	OBJECT tO;
	tO.init();
	tO.m_fBBox.z = 1.0;
	tO.m_fBBox.w = 1.0;

	m_nVision = 0;
	while (pItr[m_nVision])
	{
		Kiss* pKT = pItr[m_nVision];
		IF_F(m_nVision >= DNNAVOID_N_VISION);

		DNN_AVOID_VISION* pV = &m_pVision[m_nVision];
		m_nVision++;
		pV->init();

		F_ERROR_F(pKT->v("orientation", (int* )&pV->m_orientation));
		F_INFO(pKT->v("rMin", &pV->m_rMin));
		F_INFO(pKT->v("rMax", &pV->m_rMax));
		F_INFO(pKT->v("l", &tO.m_fBBox.x));
		F_INFO(pKT->v("t", &tO.m_fBBox.y));
		F_INFO(pKT->v("r", &tO.m_fBBox.z));
		F_INFO(pKT->v("b", &tO.m_fBBox.w));

		double deg;
		if(pKT->v("angleDeg", &deg))
		{
			pV->m_angleTan = tan(deg*DEG_RAD);
		}

		pV->m_pObj = m_pIN->add(&tO);
		NULL_F(pV->m_pObj);

		Kiss** pItrAct = pKT->getChildItr();

		pV->m_nAction = 0;
		while (pItrAct[pV->m_nAction])
		{
			Kiss* pKA = pItrAct[pV->m_nAction];
			IF_F(pV->m_nAction >= DNNAVOID_N_ACTION);

			DNN_AVOID_ACTION* pA = &pV->m_pAction[pV->m_nAction];
			pA->init();
			pA->m_nClass = pKA->array("class", pA->m_pClass,
					DNNAVOID_N_PLACE_CLASS);
			IF_CONT(pA->m_nClass <= 0);
			pV->m_nAction++;

			string strAction = "";
			F_ERROR_F(pKA->v("action", &strAction));
			pA->m_action = str2actionType(strAction);
		}
	}

	return true;
}

DNN_AVOID_ACTION_TYPE APcopter_DNNavoid::str2actionType(string& strAction)
{
	if (strAction == "safe")
		return DA_SAFE;
	else if (strAction == "warn")
		return DA_WARN;
	else if (strAction == "forbid")
		return DA_FORBID;

	return DA_UNKNOWN;
}

string APcopter_DNNavoid::actionType2str(DNN_AVOID_ACTION_TYPE aType)
{
	static const string pType[] =
	{ "unknown", "safe", "warn", "forbid" };
	return pType[aType];
}

void APcopter_DNNavoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pIN);
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);

	_Mavlink* pMavlink = m_pAP->m_pMavlink;
	double alt = (double)pMavlink->m_msg.global_position_int.relative_alt;

	int i, j, k;
	for (i = 0; i < m_nVision; i++)
	{
		DNN_AVOID_VISION* pV = &m_pVision[i];
		string strPlace = pV->m_pObj->m_name;

		for (j = 0; j < pV->m_nAction; j++)
		{
			DNN_AVOID_ACTION* pA = &pV->m_pAction[j];

			int k;
			for (k = 0; k < pA->m_nClass; k++)
			{
				if (strPlace == pA->m_pClass[k])
					break;
			}
			//class not defined in the action
			IF_CONT(k >= pA->m_nClass);

			//class definition found
			m_action = pA->m_action;
			break;
		}

		if(j >= pV->m_nAction)
		{
			//unknown action
			LOG_I("UNKNOWN:" << strPlace);
			continue;
		}

		if(m_action <= DA_SAFE)
		{
			LOG_I("SAFE:" << strPlace);
			continue;
		}

		if(m_action <= DA_WARN)
		{
			LOG_I("WARNING:" << strPlace);

			continue;
		}

		LOG_I("FORBID:" << strPlace);

		pMavlink->distanceSensor(0, //type
				pV->m_orientation,	//orientation
				pV->m_rMax,
				pV->m_rMin,
				constrain(alt*pV->m_angleTan, pV->m_rMin, pV->m_rMax));
	}

}

bool APcopter_DNNavoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}
