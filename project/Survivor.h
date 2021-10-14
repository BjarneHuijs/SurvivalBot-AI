#pragma once
#include "stdafx.h"
#include "SteeringBehaviors.h"
#include "../inc/Exam_HelperStructs.h"

struct House
{
	bool visited;
	HouseInfo house;
};

class IExamInterface;
class Survivor
{

public:
	//Survivor(AgentInfo* agent);
	Survivor();
	~Survivor();

	void Init(AgentInfo* pAgent, IExamInterface* pInterface);
	void Update(float deltaTime, AgentInfo* agentInfo, std::vector<HouseInfo>* housesInView, std::vector<EntityInfo>* entitiesInView);

	Elite::Blackboard* CreateBlackboard(IExamInterface* pInterface);
	SteeringPlugin_Output CalculateSteering(float deltaTime, AgentInfo* pAgent);

	void SetTarget(const Elite::Vector2& target);
	Elite::Vector2 GetTarget() const;
	void SetToWander();
	void SetToSeek(const Elite::Vector2& target);
	void SetToFlee(const Elite::Vector2& target);
	void SetToFace(const Elite::Vector2& target);
	void SetToSpin(const Elite::Vector2& target);
	void SetToAttack(const Elite::Vector2& target);

	Elite::Vector2 GetDefaultSeekPoint() const;
	AgentInfo* GetAgentInfo() const;

	void SetDecisionMaking(IDecisionMaking* decisionMakingStructure);

	bool CanHitEnemy(const Survivor& pAgent, const EnemyInfo& enemy) const;

private:
	AgentInfo* m_pAgent = nullptr;
	IDecisionMaking* m_pDecisionMaking = nullptr;
	ISteeringBehavior* m_pSteeringBehavior = nullptr;

	Wander* m_pWander = nullptr;
	Seek* m_pSeek = nullptr;
	Face* m_pFace = nullptr;
	Spin* m_pSpin = nullptr;
	Flee* m_pFlee = nullptr;
	Attack* m_pAttack = nullptr;

	std::vector<House> m_FoundHousesVec;
	std::vector<EntityInfo> m_EntityVec;

	//std::vector<ItemInfo> m_Inventory;
	std::vector<ItemInfo> m_FoundItems;

	Elite::Vector2 m_defaultHousePos{};

	Elite::Vector2 ClosestIntersection(const Survivor& pAgent, const EnemyInfo& enemy) const;
	int FindLineCircleIntersections(float cx, float cy, float radius, Elite::Vector2 point1, Elite::Vector2 point2, Elite::Vector2& intersection1, Elite::Vector2& intersection2) const;

private:

	// make uncopyable
	Survivor(const Survivor&) {};
	Survivor& operator=(const Survivor&) {};
};

