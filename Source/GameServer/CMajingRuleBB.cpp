#include "CMajingRuleBB.hpp"

#include "CRoom.hpp"
#include "CPlayer.hpp"
#include "CMaJiang.hpp"
#include "CHuScore.h"


CMajingRuleBB::CMajingRuleBB(CRoom* pRoom, const ::msg_maj::RoomOption& roomOption)
	: CMajingRule(pRoom,roomOption)
{
	const ::msg_maj::BBOption& option = m_roomOption.bb_option();
	m_usWanFaVal = option.wanfatype();
	m_usMaiMaType = option.maimatype();	// 买马类型
	m_bGhost = HAS_FLAG(m_usWanFaVal, WAN_FA_MASK_GHOST);
	m_bDaHuJiaBei = HAS_FLAG(m_usWanFaVal, WAN_FA_MASK_DaHuJiaBei);
	m_bCanPaoHu = HAS_FLAG(m_usWanFaVal, WAN_FA_MASK_CanPaoHu);
	m_bGangShangPaoBao3Jia = HAS_FLAG(m_usWanFaVal, WAN_FA_MASK_GangShangPaoBao3Jia);
	m_bQiangGangBao3Jia = HAS_FLAG(m_usWanFaVal, WAN_FA_MASK_QiangGangBao3Jia);
	m_bGangKaiHuaIsBao3Jia = HAS_FLAG(m_usWanFaVal, WAN_FA_MASK_GangKaiHuaBao3Jia);

	if (IsGhost())
	{
		AddGhostPai(7);
	}
}

CMajingRuleBB::~CMajingRuleBB()
{

}

uint16_t CMajingRuleBB::GetBankerSeat()
{
	const vecHuPai& vecHuPai = m_pMaj->GetVecHuPai();
	switch (vecHuPai.size())
	{
	case 0:
	{
		if (m_pRoom->GetTotalPersons() > 0)
		{
			return (m_pRoom->GetBankerSeat() + 1) % m_pRoom->GetTotalPersons();
		}

		break;
	}
	case 1: return vecHuPai[0].m_usHupaiPos; break;
	default: return m_pMaj->GetUsCurActionPos(); break;
	}
	return m_pRoom->GetBankerSeat();
}

// 检测平胡
bool CMajingRuleBB::CheckHupai_PingHu(const std::vector<uint16_t>& pailist)
{
	return CMajingRule::CheckHupai_PingHu(pailist);
}

// 检测胡牌
::msg_maj::hu_type CMajingRuleBB::CheckHupaiAll(CPlayer* pPlayer, const std::vector<uint16_t>& pailist, const vecEventPai& eventpailist, uint16_t usTingPai)
{
	return CheckHupaiAndGhost(pPlayer, pailist, eventpailist, usTingPai);
}

// 检测胡牌
::msg_maj::hu_type CMajingRuleBB::CheckHupaiAndGhost(CPlayer* pPlayer, const std::vector<uint16_t>& pailist, const vecEventPai& eventpailist, uint16_t usTingPai)
{
	Display(pailist);
	if (pailist.size() % 3 != 2)
	{
		return ::msg_maj::hu_none;
	}

	std::multimap<int16_t, ::msg_maj::hu_type, std::greater<int16_t> > mapScoreType;
	uint16_t score = 0;
	::msg_maj::hu_type huType = ::msg_maj::hu_none;
	bool bRet = m_bGhost ? CheckHupai_QiDui_Ghost(pailist) : CheckHupai_QiDui(pailist);
	if (bRet)
	{
		huType = ::msg_maj::hu_t_bb_qidui;
		score = CHuScore::Instance()->GetPaiXingScore(huType);
		mapScoreType.insert(std::make_pair(score, huType));
		if (IsQys(pailist, eventpailist, m_bGhost))
		{
			huType = ::msg_maj::hu_t_bb_qysqd;
			score = CHuScore::Instance()->GetPaiXingScore(huType);
			mapScoreType.insert(std::make_pair(score, huType));
		}
	}

	if (huType == ::msg_maj::hu_none)
	{
		bool bCanPingHu = m_bGhost ? CheckHupai_PingHu_Ghost(pailist) : CheckHupai_PingHu(pailist);
		if (bCanPingHu)
		{
			huType = ::msg_maj::hu_t_bb_pinghu;
			score = CHuScore::Instance()->GetPaiXingScore(huType);
			mapScoreType.insert(std::make_pair(score, huType));
			if (IsPPHu(pailist, eventpailist, m_bGhost))
			{
				huType = ::msg_maj::hu_t_bb_pphu;
				score = CHuScore::Instance()->GetPaiXingScore(huType);
				mapScoreType.insert(std::make_pair(score, huType));
			}
			if (IsQys(pailist, eventpailist, m_bGhost))
			{
				huType = ::msg_maj::hu_t_bb_qys;
				score = CHuScore::Instance()->GetPaiXingScore(huType);
				mapScoreType.insert(std::make_pair(score, huType));
				if (IsPPHu(pailist, eventpailist, m_bGhost))
				{
					huType = ::msg_maj::hu_t_bb_qyspphu;
					score = CHuScore::Instance()->GetPaiXingScore(huType);
					mapScoreType.insert(std::make_pair(score, huType));
				}
			}
		}
	}

	if (huType != ::msg_maj::hu_none)
	{
		if (IsDiHu(pPlayer))
		{
			huType = ::msg_maj::hu_t_bb_dihu;
			score = CHuScore::Instance()->GetPaiXingScore(huType);
			mapScoreType.insert(std::make_pair(score, huType));
		}

		if (IsTianHu(pPlayer))
		{
			huType = ::msg_maj::hu_t_bb_tianhu;
			score = CHuScore::Instance()->GetPaiXingScore(huType);
			mapScoreType.insert(std::make_pair(score, huType));
		}

		if (mapScoreType.empty())
		{
			return ::msg_maj::hu_none;
		}
		else
		{
			return mapScoreType.begin()->second;
		}
	}
	return huType;
}

// 杠分在最后的胡牌才能结算 
void CMajingRuleBB::CountEventMingGang(uint16_t usSeat)
{
	//胡牌基础分数
	m_pRoom->GetPlayer(usSeat)->AddMingGangTimes(1);
	m_pRoom->GetPlayer(m_pMaj->GetUsCurActionPos())->AddFangGangTimesThisInn(1);
	m_pRoom->GetPlayer(m_pMaj->GetUsCurActionPos())->AddFangGangTimes(1);
}

void CMajingRuleBB::CountEventAnGang(uint16_t usSeat)
{
	m_pRoom->GetPlayer(usSeat)->AddAnGangTimes(1);
}

void CMajingRuleBB::CountEventGouShouGang(uint16_t usSeat)
{
	m_pRoom->GetPlayer(usSeat)->AddNextGangTimes(1);
}

bool CMajingRuleBB::CanDianPao(CPlayer* pPlayer)
{
	return CMajingRule::CanDianPao(pPlayer);
}

bool CMajingRuleBB::IsCanCountGang()
{
	return !m_pMaj->IsNotHued();
}

void CMajingRuleBB::CountResult()
{
	// 总分 = 牌形分 * 胡牌方式
	CMaJiang* pMaj = m_pMaj;
	vecHuPai huPaies = pMaj->GetVecHuPai();
	std::pair<uint16_t, uint16_t> pairhitMaInfo;
	for (vecHuPai::const_iterator it = huPaies.begin(); it != huPaies.end(); ++it)
	{
		const stHuPai& sthupai = *it;

		// 增加其他记录
		CPlayer* pHuPlayer = m_pRoom->GetPlayer(sthupai.m_usHupaiPos);
		if (pHuPlayer == NULL)
		{
			continue;
		}

		switch (sthupai.m_eHupaiWay)
		{
		case ::msg_maj::hu_way_bb_zimo:
		case ::msg_maj::hu_way_bb_gangkaihua:
		case ::msg_maj::hu_way_bb_dianpao:
		{
			// 结算胡牌人的分
			CountHupaiScore(sthupai, pHuPlayer->GetPaiList(), pHuPlayer->GetEventPaiList(), pairhitMaInfo);
			pHuPlayer->AddZiMoTimes(1, pHuPlayer->IsGhostHu(this));
			pHuPlayer->AddHitMaTotal(pairhitMaInfo.first);
			pHuPlayer->AddHuPaiTotal(1);
			break;
		}
		case ::msg_maj::hu_way_bb_gangshangpao:
		case ::msg_maj::hu_way_bb_qiangganghu:
		{
			CountHupaiScore(sthupai, pHuPlayer->GetPaiList(), pHuPlayer->GetEventPaiList(), pairhitMaInfo);
			pHuPlayer->AddHitMaTotal(pairhitMaInfo.first);
			pHuPlayer->AddHuPaiTotal(1);
			break;
		}
		default:
			break;
		}
	}
}

void CMajingRuleBB::CountGangResult()
{
	vecHuPai huPaies = m_pMaj->GetVecHuPai();
	std::map<uint16_t, int16_t> o_mapScore;
	for (uint16_t i = 0; i < m_pRoom->GetTotalPersons(); ++i)
	{
		bool find = false;
		stHuPai sthupai;
		sthupai.m_usHupaiPos = -1;
		for (vecHuPai::const_iterator it = huPaies.begin(); it != huPaies.end(); ++it)
		{
			if (it->m_usHupaiPos == i)
			{
				sthupai = *it;
				find = true;
				break;
			}
		}
		if (!find)
		{
			sthupai.m_eHupaiType = msg_maj::hu_none;
			sthupai.m_eHupaiWay = msg_maj::hu_way_none;
			sthupai.m_usHupaiPos = i;
		}
		CountGangScore(sthupai, i);
	}
}

bool CMajingRuleBB::CanBuGang(CPlayer* pPlayer, std::vector<uint16_t>& agpailist)
{
	if (NULL == pPlayer)
	{
		//LOG(ERROR) << "CanBuGang() NULL == pPlayer";
		return false;
	}

	const std::vector<uint16_t>& paiList = pPlayer->GetPaiList();
	for (std::vector<uint16_t>::const_iterator it = paiList.begin(); it != paiList.end(); ++it)
	{
		if (pPlayer->CheckNextGang(*it))
		{
			agpailist.push_back(*it);
		}
	}
	return !agpailist.empty();
}

void CMajingRuleBB::GetHitMaDatas(const stHuPai& sthupai, uint16_t& hitMaTimes)
{
	hitMaTimes = 0;
}

int32_t CMajingRuleBB::CountHupaiBaseScore(const stHuPai& sthupai, const std::vector<uint16_t>& pailist, const vecEventPai& eventpailist, stHuDetail& huDetail)
{
	// 是否作大胡
	int32_t usBigHuMulti = m_bDaHuJiaBei ? CHuScore::Instance()->GetPaiXingScore(sthupai.m_eHupaiType) : 1;
	int32_t usExtraMulti = 0;
	int32_t item3 = 0;
	int32_t item4 = 0;
	int32_t item5 = 0;
	int32_t item6 = 0;
	switch (sthupai.getDefHuWay())
	{
	case ::msg_maj::hu_way_gangkaihua:item3 = 1; usExtraMulti += 2; break;
	case ::msg_maj::hu_way_qiangganghu:item4 = 1; usExtraMulti += 2; break;
	case ::msg_maj::hu_way_gangshangpao:item5 = 1; usExtraMulti += 2; break;
	default:
		break;
	}

	// 无鬼加倍
	if (IsGhost() && !IsHasGhostPai(pailist))
	{
		item6 = 1;
		usExtraMulti += 2;
	}

	int32_t usMaiMaCount = 0;
	CPlayer* pPlayer = m_pRoom->GetPlayer(sthupai.m_usHupaiPos);
	if (pPlayer)
	{
		CheckHitMa(sthupai.m_usHupaiPos, pPlayer->GetHitMa());
		usMaiMaCount = pPlayer->GetHitMa().size();
	}

	int32_t baseScore = GetBaseScore();
	int32_t usTotalBaseScore = baseScore * usBigHuMulti;
	if (usExtraMulti)
	{
		usTotalBaseScore *= usExtraMulti;
	}

	if (usMaiMaCount)
	{
		usTotalBaseScore *= (usMaiMaCount + 1);
	}

	huDetail.item1 = baseScore;
	huDetail.item2 = usBigHuMulti;
	huDetail.item3 = item3;
	huDetail.item4 = item4;
	huDetail.item5 = item5;
	huDetail.item6 = item6;
	huDetail.item10 = usExtraMulti;
	huDetail.item11 = usMaiMaCount;

	// 接炮 = 底分 * 大胡牌型番数 *  特殊胡牌番数 * (码数  + 1) + 杠分
	// 自摸 = 底分 * 大胡牌型番数 *  特殊胡牌番数 * (码数  + 1) + 杠分
	return usTotalBaseScore;
}

void CMajingRuleBB::CountHupaiScore(const stHuPai& sthupai, const std::vector<uint16_t>& pailist, const vecEventPai& eventpailist, std::pair<uint16_t, uint16_t>& out_mainfo)
{
	uint32_t usTopScore = CMajingRule::GetTopScore();

	stHuDetail stdetail;
	int32_t nTotalBaseScore = CountHupaiBaseScore(sthupai, pailist, eventpailist, stdetail);
	if (nTotalBaseScore == 0)
	{
		return;
	}

	CPlayer* huPlayer = m_pRoom->GetPlayer(sthupai.m_usHupaiPos);
	if (huPlayer == NULL)
	{
		return;
	}

	std::vector<int16_t> doedMultiSeat;
	int16_t winScore = 0;
	for (uint16_t i = 0; i < m_pRoom->GetTotalPersons(); ++i)
	{
		if (NULL == m_pRoom->GetPlayer(i))  continue;

		if (i == sthupai.m_usHupaiPos) continue;

		bool isDianPao = (sthupai.m_eHupaiWay == ::msg_maj::hu_way_bb_dianpao
			|| sthupai.m_eHupaiWay == ::msg_maj::hu_way_bb_gangshangpao
			|| sthupai.m_eHupaiWay == ::msg_maj::hu_way_bb_qiangganghu);

		int32_t nScoreTmp = isDianPao ? nTotalBaseScore : (nTotalBaseScore * 2);
		nScoreTmp = nScoreTmp < usTopScore ? nScoreTmp : usTopScore;
		winScore += nScoreTmp;

		switch (sthupai.m_eHupaiWay)
		{
		case ::msg_maj::hu_way_bb_zimo:
		{
			m_pRoom->GetPlayer(i)->AddTotalFan(-nScoreTmp);
			int16_t offsetSeat = getOffsetPos(sthupai.m_usHupaiPos, i);
			doedMultiSeat.push_back(offsetSeat);
			break;
		}
		case ::msg_maj::hu_way_bb_gangshangpao:
		{
			if (GangShangPaoBao3Jia())
			{
				if (i == m_pMaj->GetUsCurActionPos())
				{
					m_pRoom->GetPlayer(i)->AddTotalFan(-nScoreTmp * (m_pRoom->GetTotalPersons() - 1));
					int16_t offsetSeat = getOffsetPos(sthupai.m_usHupaiPos, i);
					doedMultiSeat.push_back(offsetSeat);
				}
			}
			else
			{
				if (i == m_pMaj->GetUsCurActionPos())
				{
					m_pRoom->GetPlayer(i)->AddTotalFan(-nScoreTmp);
					int16_t offsetSeat = getOffsetPos(sthupai.m_usHupaiPos, i);
					doedMultiSeat.push_back(offsetSeat);
				}
				else
				{
					winScore -= nScoreTmp;
					continue;
				}
			}
			break;
		}
		case ::msg_maj::hu_way_bb_dianpao:
		{
			if (i == m_pMaj->GetUsCurActionPos())
			{
				m_pRoom->GetPlayer(i)->AddTotalFan(-nScoreTmp);
				int16_t offsetSeat = getOffsetPos(sthupai.m_usHupaiPos, i);
				doedMultiSeat.push_back(offsetSeat);
			}
			else
			{
				winScore -= nScoreTmp;
				continue;
			}
			break;
		}
		case ::msg_maj::hu_way_bb_qiangganghu: //抢杠胡
		{
			if (IsQiangGangHuBao3Jia(sthupai)) // 勾先包三家，不勾，只有一家
			{
				m_pRoom->GetPlayer(m_pMaj->m_nLastGangPos)->AddTotalFan(-nScoreTmp);
				int16_t offsetSeat = getOffsetPos(sthupai.m_usHupaiPos, m_pMaj->m_nLastGangPos);
				doedMultiSeat.push_back(offsetSeat);
			}
			else
			{
				if (i == m_pMaj->m_nLastGangPos)
				{
					m_pRoom->GetPlayer(i)->AddTotalFan(-nScoreTmp);
					int16_t offsetSeat = getOffsetPos(sthupai.m_usHupaiPos, i);
					doedMultiSeat.push_back(offsetSeat);
				}
				else
				{
					winScore -= nScoreTmp;
				}
			}
			break;
		}
		case ::msg_maj::hu_way_bb_gangkaihua: // 
		{
			if (IsGangKaiHuaBao3Jia(sthupai)) // 勾先包三家，不勾，只有一家
			{
				m_pRoom->GetPlayer(m_pMaj->m_nLastGangFromPos)->AddTotalFan(-nScoreTmp);
				int16_t offsetSeat = getOffsetPos(sthupai.m_usHupaiPos, m_pMaj->m_nLastGangFromPos);
				doedMultiSeat.push_back(offsetSeat);
			}
			else
			{
				m_pRoom->GetPlayer(i)->AddTotalFan(-nScoreTmp);
				int16_t offsetSeat = getOffsetPos(sthupai.m_usHupaiPos, i);
				doedMultiSeat.push_back(offsetSeat);
			}
			break;
		}
		default:
		{
			m_pRoom->GetPlayer(i)->AddTotalFan(-nScoreTmp);
			int16_t offsetSeat = getOffsetPos(sthupai.m_usHupaiPos, i);
			doedMultiSeat.push_back(offsetSeat);
			break;
		}
		}
	}
	stdetail.doedMultiSeat = doedMultiSeat;
	m_pRoom->GetPlayer(sthupai.m_usHupaiPos)->AddTotalFan(winScore);
	stdetail.SetData(sthupai.m_usHupaiPos, sthupai.m_usHupaiPos, getOffsetPos(sthupai.m_usHupaiPos, -1), winScore, sthupai);
	m_pRoom->GetPlayer(sthupai.m_usHupaiPos)->AddScore(stdetail);

	out_mainfo.first = stdetail.item11;
	out_mainfo.second = winScore;
}

void CMajingRuleBB::CountGangScore(const stHuPai& sthupai, uint16_t gangSeat)
{
	int32_t baseScore = GetBaseScore();
	CPlayer* pPlayer = m_pRoom->GetPlayer(gangSeat);

	{
		int32_t nTotalGang = (pPlayer->GetAnGangTimes() * 2) * baseScore;
		if (nTotalGang > 0)
		{
			int32_t winScore = 0;
			for (uint16_t j = 0; j < m_pRoom->GetTotalPersons(); ++j)
			{
				if (gangSeat == j) continue;

				switch (sthupai.m_eHupaiWay)
				{
				case ::msg_maj::hu_way_bb_qiangganghu: //抢杠胡
				{
					if (false) // 勾先包三家，不勾，只有一家
						m_pRoom->GetPlayer(m_pMaj->m_nLastGangPos)->AddTotalFan(-nTotalGang);
					else
						m_pRoom->GetPlayer(j)->AddTotalFan(-nTotalGang);
					break;
				}
				case ::msg_maj::hu_way_bb_gangkaihua: // 暗杠(无三家),其他杠(有可能包三家)
				{
					if (false)	// 如果是明杠杠上花，则有可能要包三家
						m_pRoom->GetPlayer(m_pMaj->m_nLastGangFromPos)->AddTotalFan(-nTotalGang);
					else
						m_pRoom->GetPlayer(j)->AddTotalFan(-nTotalGang);
					break;
				}
				default:
				{
					m_pRoom->GetPlayer(j)->AddTotalFan(-nTotalGang);
					break;
				}
				}
				winScore += nTotalGang;
			}
			pPlayer->AddTotalFan(winScore);
			stFengYuDetail stDetail;
			stDetail.SetData(gangSeat, gangSeat, getOffsetPos(gangSeat, -1), winScore, 3);
			pPlayer->AddScore(stDetail);
		}
	}

	{
		int32_t nTotalGang = pPlayer->GetNextGangTimes() * baseScore;
		if (nTotalGang > 0)
		{
			int32_t winScore = 0;
			for (uint16_t j = 0; j < m_pRoom->GetTotalPersons(); ++j)
			{
				if (gangSeat == j) continue;

				switch (sthupai.m_eHupaiWay)
				{
				case ::msg_maj::hu_way_bb_qiangganghu: //抢杠胡
				{
					if (false) // 勾先包三家，不勾，只有一家
						m_pRoom->GetPlayer(m_pMaj->m_nLastGangPos)->AddTotalFan(-nTotalGang);
					else
						m_pRoom->GetPlayer(j)->AddTotalFan(-nTotalGang);
					break;
				}
				case ::msg_maj::hu_way_bb_gangkaihua: // 暗杠(无三家),其他杠(有可能包三家)
				{
					if (false)	// 如果是明杠杠上花，则有可能要包三家
						m_pRoom->GetPlayer(m_pMaj->m_nLastGangFromPos)->AddTotalFan(-nTotalGang);
					else
						m_pRoom->GetPlayer(j)->AddTotalFan(-nTotalGang);
					break;
				}
				default:
				{
					m_pRoom->GetPlayer(j)->AddTotalFan(-nTotalGang);
					break;
				}
				}
				winScore += nTotalGang;
			}
			pPlayer->AddTotalFan(winScore);
			stFengYuDetail stDetail;
			stDetail.SetData(gangSeat, gangSeat, getOffsetPos(gangSeat, -1), winScore, 2);
			pPlayer->AddScore(stDetail);
		}
	}

	{
		// 明杠
		uint32_t winScore = 0;
		const std::vector<uint16_t>& seated = pPlayer->GetMingGangSeated();
		for (std::vector<uint16_t>::const_iterator it2 = seated.begin(); it2 != seated.end(); ++it2)
		{
			uint32_t tmpScoer = baseScore * (m_pRoom->GetTotalPersons() - 1);
			winScore += tmpScoer;
			switch (sthupai.m_eHupaiWay)
			{
			case ::msg_maj::hu_way_bb_qiangganghu: //抢杠胡
			{
				if (false) // 勾先包三家，不勾，只有一家
					m_pRoom->GetPlayer(m_pMaj->m_nLastGangPos)->AddTotalFan(-tmpScoer);
				else
					m_pRoom->GetPlayer(*it2)->AddTotalFan(-tmpScoer);
				break;
			}
			case ::msg_maj::hu_way_bb_gangkaihua: // 暗杠(无三家),其他杠(有可能包三家)
			{
				if (false)	// 如果是明杠杠上花，则有可能要包三家
					m_pRoom->GetPlayer(m_pMaj->m_nLastGangFromPos)->AddTotalFan(-tmpScoer);
				else
					m_pRoom->GetPlayer(*it2)->AddTotalFan(-tmpScoer);
				break;
			}
			default:
			{
				m_pRoom->GetPlayer(*it2)->AddTotalFan(-tmpScoer);
				break;
			}
			}
		}
		if (winScore > 0)
		{
			pPlayer->AddTotalFan(winScore);
			stFengYuDetail stDetail;
			stDetail.SetData(gangSeat, gangSeat, getOffsetPos(gangSeat, -1), winScore, 1);
			pPlayer->AddScore(stDetail);
		}
	}

}

void CMajingRuleBB::SetHuInfo(const stHuPai& sthupai, uint16_t seat, ::msg_maj::HuInfo* huInfo, bool isFull)
{
	huInfo->set_game_type(::msg_maj::maj_t_yulin);
	huInfo->mutable_bb_info()->set_hutype(::msg_maj::hu_type(sthupai.m_eHupaiType));
	huInfo->mutable_bb_info()->set_huway(::msg_maj::hu_way(sthupai.m_eHupaiWay));
	if (!isFull)
	{
		return;
	}

	uint16_t i = seat;

	for (std::vector<uint16_t>::const_iterator iter = m_pMaj->GetMaPaiList().begin(); iter != m_pMaj->GetMaPaiList().end(); ++iter)
	{
		huInfo->mutable_bb_info()->add_ma_pai_all(*iter);
	}
	for (std::vector<uint16_t>::iterator iter = m_pRoom->GetPlayer(i)->GetHitMa().begin(); iter != m_pRoom->GetPlayer(i)->GetHitMa().end(); ++iter)
	{
		huInfo->mutable_bb_info()->add_ma_pai_hit(*iter);
	}
}

void CMajingRuleBB::SetResultSeatHuInfo(const stGameResultSeat& seatData, ::msg_maj::HuInfo* huInfo)
{
	huInfo->set_game_type(::msg_maj::maj_t_yulin);
	huInfo->mutable_bb_info()->set_huway(seatData.hu_info.hu_way);
	huInfo->mutable_bb_info()->set_hutype(seatData.hu_info.hu_type);
	for (std::vector<int16_t>::const_iterator it = seatData.hu_info.ma_pai_all.begin(); it != seatData.hu_info.ma_pai_all.end(); ++it)
	{
		huInfo->mutable_bb_info()->add_ma_pai_all(*it);
	}

	for (std::vector<int16_t>::const_iterator it = seatData.hu_info.ma_pai_hit.begin(); it != seatData.hu_info.ma_pai_hit.end(); ++it)
	{
		huInfo->mutable_bb_info()->add_ma_pai_hit(*it);
	}
}

void CMajingRuleBB::SetReplayActionHuInfo(const stReplayAction& actionData, ::msg_maj::HuInfo* huInfo)
{
	huInfo->set_game_type(::msg_maj::maj_t_yulin);
	huInfo->mutable_bb_info()->set_huway(actionData.hu_info.hu_way);
	huInfo->mutable_bb_info()->set_hutype(actionData.hu_info.hu_type);
	for (int j = 0; j < actionData.hu_info.ma_pai_all.size(); ++j)
	{
		huInfo->mutable_bb_info()->add_ma_pai_all(actionData.hu_info.ma_pai_all[j]);
	}
	for (int j = 0; j < actionData.hu_info.ma_pai_hit.size(); ++j)
	{
		huInfo->mutable_bb_info()->add_ma_pai_hit(actionData.hu_info.ma_pai_hit[j]);
	}
}

bool CMajingRuleBB::IsThisInnMyWin(CPlayer* pPlayer) const
{
	if (pPlayer->IsHued() && m_pMaj->m_usFirstSeat == pPlayer->GetSeat())
	{
		return true;
	}
	return false;
}

bool CMajingRuleBB::IsGuoPengThisPai(CPlayer* pPlayer, uint16_t usPai)
{
	if (IsGuoPeng())
	{
		std::set<uint16_t>::const_iterator it = pPlayer->m_setGuoPengPai.find(usPai);
		if (it != pPlayer->m_setGuoPengPai.end())
		{
			if (usPai == *it)
			{
				return true;
			}
		}
	}
	return false;
}

eRoomStatus CMajingRuleBB::AcceptAskAllNextStatus() const
{
	return eRoomStatus_StartGame;
}

eRoomStatus CMajingRuleBB::SendHandCardsAllNextState() const
{
	return eRoomStatus_StartGame;
}

eRoomStatus CMajingRuleBB::DisoverCardAllCheckAndDoEvent() const
{
	return eRoomStatus_StartGame;
}



