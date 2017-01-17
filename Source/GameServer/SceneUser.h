#ifndef __CHARACTER_H_
#define __CHARACTER_H_

#include "Includes.h"

#include "SceneNpc.h"

#include "MyCounters.h"
#include "SrvEngine.h"
#include "character.pb.h"
#include "itemslots.pb.h"
#include "chatmsg.pb.h"

#include "ConfigBase.h" 
#include "MyUniqueID.h"
#include "Object.h"
#include "ObjectManager.h"
#include "MessageMgr.h"
#include "RelationCtrl.h"


/*
 *	Detail: ��ɫ��  
 *  Created by hzd 2014-12-17
 */
class SceneUser : public SceneEntryPk
{
public:
	SceneUser();
	~SceneUser();

	bool Serialize(::protobuf::UserBinary& proto);
	void UnSerialize(const ::protobuf::UserBinary& proto);

public:

	int64 sessid;				/* payersessid*/
	int32 fepsid;				/* Fep�Ự */
	bool initDataFinish;		/* �Ƿ�װ�������� */
	bool clientReady;			/* ǰ��״̬ 0 ok,1 ������Դ�� */ 
	bool userModity;			/* �Ƿ����ݱ䶯(0��,1��) */

	/* ��ɫ�������� */
public:


	UserSceneBase userbase;


public:

	bool ReqMove(Cmd::stUserMoveMoveUserCmd *rev);
	void CheckSaveToDb(int32 nSrvTime);
	bool loadFromDb(const D2SLoadUser* packet, int32 nSize);
	bool SaveToDb(int32 nScoketEventCode = 0);

	void Online(); 

public:

	// ���ظ������Ҫ���� 

	// ��Ϣ���� 
	void Update(const zTaskTimer* timer);

	/* �붨ʱ�� */
	void Timer(int32 curTime); 

public:

	/* ���볡����Ҫ�������� */ 
	void sendMainToMe();

	// ���¼���PK����
	void setupCharBase(); 

	int64 GetUid()
	{ 
		return id;
	}

	int32 GetType()
	{
		return 0;
	}

	int32 GetLandMapID() const
	{
		return 0;
	}

	int32 GetInstanceMapID() const
	{
		return 0;
	}

	int32 GetCurrMapID() const
	{
		if (GetInstanceMapID())
		{
			return GetInstanceMapID();
		}
		else
		{
			return GetLandMapID();
		}
	}

	int32 GetSceneID() const
	{
		return 0;
	}

	int32 GetZoneID() const
	{
		return 0;
	}

	int32 GetTeamID() const 
	{
		return 0;
	}

	void sendCmdToMe(NetMsgSS* pMsg, int32 nSize);

	void SendToFep(NetMsgSS* pMsg, int32 nSize);

	void SendToDp(NetMsgSS* pMsg, int32 nSize);

	void SendToWs(NetMsgSS* pMsg, int32 nSize);
	
//-------------------------------���Ը��Ļص�����------------------------------------------
private:

	void OnExpChange(const ValueType& vOldValue, const ValueType& vNewValue);
	void OnMapIDChange(const ValueType& vOldValue, const ValueType& vNewValue);
	void OnClothesChange(const ValueType& vOldValue, const ValueType& vNewValue);
	void OnWeaponChange(const ValueType& vOldValue, const ValueType& vNewValue);
	void OnMoneyChange(const ValueType& vOldValue, const ValueType& vNewValue);

	/* ͬ���Լ�������(����б䶯�ŷ��͸���) */ 
	void UpdateAttribute();


public:

	bool CheckMoneyEnough(int32 type,int32 num);
	bool TrySubMoney(int32 type, int32 num);
	bool SubMoney(int32 type, int32 num, bool notify = true, bool isTry = false);



//----------------------------------����������------------------------------------------
public:

	/* ������(֧��ÿʱÿ��ÿ��ÿ�µ����㴦��) id=>ֵ <��ʼʱ��,����ʱ��> */ 
	MyCounters ucm; 

	/* ΨһID������ */
	MyUniqueID uuid;

	/* ���ܹ����� */
	UserSkillM usm;

	/* ��Ʒ������ */
	ObjectManager objM;

	/* ������Ϣ������ */
	MessageMgr mesM;

	/* ����ģ�鹦�� */  
public:

	void getAllItemSlots(std::vector<qObject*>& slots);

	bool TryAddObject(int32 itemID, int32 itemNum);
	bool AddObject(int32 itemID, int32 itemNum,bool notify = true,bool isTry = false);

	bool TryUseObject(int32 uniqueID,int32 num);
	bool UseObject(int32 uniqueID, int32 num, bool notify = true, bool isTry = false);


private:

	bool UseObject(qObject* obj, int32 num, bool notify = true, bool isTry = false);
	bool UseItemObj(qObject* obj, int32 num, bool notify = true, bool isTry = false);
	bool UseEquip(qObject* obj, int32 num, bool notify = true, bool isTry = false);
	bool UseQuest(qObject* obj, int32 num, bool notify = true, bool isTry = false);
	bool UsePking(qObject* obj, int32 num, bool notify = true, bool isTry = false);
	bool ExecEffectMoney(const zItemB::Effect& effect, int32 num);
	
public:

	/**
	* \brief �ı��ɫ��hp
	* \param hp �����HP
	*/
	virtual void changeHP(const SDWORD &hp);

	/**
	* \brief �ı��ɫ��sp
	* \param sp �����SP
	*/
	virtual void changeSP(const SDWORD &sp);

	/**
	* \brief �ı��ɫ��mp
	* \param mp �����MP
	*/
	virtual void changeMP(const SDWORD &mp);

	/**
	* \brief �ж��Ƿ��ǵ���
	* \return true �� false ����
	*/
	virtual int isEnemy(SceneEntryPk *entry, bool notify = false, bool good = false);

	/**
	* \brief ��ȡ�Լ�������,һ�����NPC����,Player���������Լ�
	* \return NULL�������˵Ķ���ָ��
	*/
	virtual SceneEntryPk *getMaster();

	/**
	* \brief �õ����ϲ������
	*
	* \return ����
	*/
	virtual SceneEntryPk *getTopMaster();

	/**
	* \brief �жϽ�ɫ�Ƿ�����
	*/
	virtual bool isDie();

	/**
	* \brief ��ȡ��ɫ�ļ���
	*/
	virtual DWORD getLevel() const;

	int32 getVip() const
	{
		return 0;
	}

	int32 getCountry() const
	{
		return 0;
	}

	/**
	* \brief ��ȡ���е���
	* \return ���ظ����͵����е���
	*/
	virtual DWORD getFiveType() { return 0; }

	/**
	* \brief ��Ҫ��ְҵ����,��������ʹ�õļ�������
	*/
	virtual bool needType(const DWORD &needtype);

	/**
	* \brief ��Ҫ��ְҵ����,��������ʹ�õļ�������
	*/
	virtual bool addSkillToMe(zSkill *skill);

	/**
	* \brief �Ƿ��иü�����Ҫ������
	* \return true �� false û��
	*/
	virtual bool needWeapon(DWORD skillid);

	/**
	* \brief �Ƿ�Pk����
	* \param other PK�����
	* \return true �� false ��
	*/
	virtual bool isPkZone(SceneEntryPk *other = NULL);

	/**
	* \brief ������Ʒ�����ͷ���
	* \param object ������Ʒ������
	* \param num ������Ʒ������
	* \return true ���ĳɹ� false ʧ��
	*/
	virtual bool reduce(const DWORD &object, const BYTE num);

	/**
	* \brief ����������Ʒ�Ƿ��㹻
	* \param object ������Ʒ������
	* \param num ������Ʒ������
	* \return true �㹻 false ����
	*/
	virtual bool checkReduce(const DWORD &object, const BYTE num);
	/**
	* \brief ��ȡװ���˺��ӳ�
	* \return �˺��ӳ�
	*/
	virtual WORD getDamageBonus() { return 0; }

	/**
	* \brief ʩ�ż��������µ�����MP,HP,SP
	* \param base ���ܻ������Զ���
	* \return true ���ĳɹ� false ʧ��
	*/
	virtual bool doSkillCost(const zSkillB *base);

	virtual void showCurrentEffect(const WORD &state, bool isShow, bool notify = true);

	virtual bool preAttackMe(SceneEntryPk *pUser, const Cmd::stAttackMagicUserCmd *rev, bool physics = true, const bool good = false);

	/**
	* \brief �ý�ɫ����
	* \param dwTempID �����ߵ���ʱID
	*/
	virtual void toDie(const DWORD &dwTempID);

	virtual SWORD directDamage(SceneEntryPk *pAtt, const SDWORD &dam, bool notify = false);


public:
	
	Relation relM;


};

#endif


