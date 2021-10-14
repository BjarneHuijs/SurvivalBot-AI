#include "stdafx.h"
#include "Behaviors.h"
#include "Survivor.h"
#include "IExamInterface.h"

using namespace Elite;
//Survivor::Survivor(AgentInfo* agent)
Survivor::Survivor()
	: m_defaultHousePos{ Elite::Vector2{33.5f, 56.f } }
{
	//if (agent)
		//m_pAgent = agent;

	m_pWander = new Wander();
	m_pSeek = new Seek();
	m_pFace = new Face();
	m_pSpin = new Spin();
	m_pFlee = new Flee();
	m_pAttack = new Attack();

	m_pSeek->SetTarget(m_defaultHousePos);
}

Survivor::~Survivor()
{
	SAFE_DELETE(m_pDecisionMaking);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pFace);
	SAFE_DELETE(m_pSpin);
	SAFE_DELETE(m_pFlee);
	SAFE_DELETE(m_pAttack);
}



//void Survivor::Init()
void Survivor::Init(AgentInfo* pAgent, IExamInterface* pInterface)
{
	m_pAgent = pAgent;
	m_pSeek->SetExamInterface(pInterface);
	m_pWander->SetExamInterface(pInterface);
	m_pAttack->SetExamInterface(pInterface);

	// Decision making for user agent
	Blackboard* pBlackboard = CreateBlackboard(pInterface);


	//BT
	BehaviorTree* pBehaviourTree = new BehaviorTree(pBlackboard,
		new BehaviorSelector(
		{
			new BehaviorSequence(
				{
					new BehaviorConditional(FoundEnemy),
					new BehaviorSequence(
						{
							//new BehaviorAction(TargetEnemy),
							new BehaviorAction(ShootEnemy)
						}),

				}),

			new BehaviorSequence(
				{
					new BehaviorConditional(IsInPurgeZone),
					new BehaviorAction(FleeFromTarget)
				}),

			new BehaviorSequence(
				{
					new BehaviorConditional(FoundItemInGrabRange),
					new BehaviorAction(GrabItem)
				}),

			

			new BehaviorSequence(
				{
					new BehaviorConditional(IsLowOnEnergy),
					new BehaviorAction(EatFood)
				}),
			new BehaviorSequence(
				{
					new BehaviorConditional(WasBittenOrLowOnHealth),
					new BehaviorAction(UseMedKit)
				}),

			new BehaviorSequence(
				{
					new BehaviorConditional(WasBitten),
					new BehaviorAction(LookAround),
				}),


			new BehaviorSequence(
				{
					new BehaviorConditional(ReachedHouse),
					new BehaviorPartialSequence(
						{
							new BehaviorAction(LookAround),
							new BehaviorAction(ChangeToWander)
						}),
				}),
			new BehaviorSequence(
				{
					new BehaviorConditional(FoundHouse),
					new BehaviorAction(ChangeToSeek)
				}),
			
			new BehaviorSequence(
				{
					new BehaviorConditional(IsInWorldBoundaries),
					new BehaviorAction(ChangeToSeek)
				}),
			
			new BehaviorSequence(
				{
					new BehaviorConditional(HasDefaultPoint),
					new BehaviorAction(ChangeToSeek)
				}),

			new BehaviorAction(ChangeToWander)
		}) 
	);
	
	m_pDecisionMaking = pBehaviourTree;	
}


void Survivor::Update(float deltaTime, AgentInfo* agentInfo, std::vector<HouseInfo>* housesInView, std::vector<EntityInfo>* entitiesInView)
{
	for (const HouseInfo& house : *housesInView) 
	{
		auto houseIt = std::find_if(m_FoundHousesVec.begin(), m_FoundHousesVec.end(), [&house](const House& foundHouse) 
			{
				return house.Center == foundHouse.house.Center && house.Size == foundHouse.house.Size;
			});

		if(houseIt == m_FoundHousesVec.end())
		{
			//House newHouse{};
			//newHouse.visited = false;
			//newHouse.house = house;
			m_FoundHousesVec.push_back(House{false, house});
		}
	}

	m_pAgent = agentInfo;
	m_EntityVec = *entitiesInView;

	if (m_pDecisionMaking)
		m_pDecisionMaking->Update(deltaTime);
}

Blackboard* Survivor::CreateBlackboard(IExamInterface* pInterface)
{
	Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("Survivor", this);
	//pBlackboard->AddData("AgentInfo", m_pAgent);
	pBlackboard->AddData("HousesVec", &m_FoundHousesVec);
	pBlackboard->AddData("EntityVec", &m_EntityVec);
	pBlackboard->AddData("Interface", pInterface);
	//pBlackboard->AddData("WorldSize", m_TrimWorldSize);
	pBlackboard->AddData("Target", m_defaultHousePos);
	pBlackboard->AddData("lastBiteTime", time_point{});
	pBlackboard->AddData("TargetEntity", EntityInfo{});
	pBlackboard->AddData("WorldInfo", &pInterface->World_GetInfo());

	return pBlackboard;
}

SteeringPlugin_Output Survivor::CalculateSteering(float deltaTime, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};
	if (m_pSteeringBehavior)
	{
		steering = m_pSteeringBehavior->CalculateSteering(deltaTime, pAgent);
	}

	return steering;
}

void Survivor::SetTarget(const Elite::Vector2& target)
{
	if (m_pSteeringBehavior)
		m_pSteeringBehavior->SetTarget(target);
}

Elite::Vector2 Survivor::GetTarget() const
{
	return m_pSteeringBehavior->GetTarget();
}

void Survivor::SetToWander()
{
	m_pSteeringBehavior = m_pWander;
}

void Survivor::SetToSeek(const Elite::Vector2& target)
{
	if (m_pSteeringBehavior != m_pSeek) 
	{
		m_pSteeringBehavior = m_pSeek;
	}
	m_pSeek->SetTarget(target);
}

void Survivor::SetToFlee(const Elite::Vector2& target)
{
	if (m_pSteeringBehavior != m_pFlee) 
	{
		m_pSteeringBehavior = m_pFlee;
	}
	m_pFlee->SetTarget(target);
}

void Survivor::SetToFace(const Elite::Vector2& target)
{
	if (m_pSteeringBehavior != m_pFace) 
	{
		m_pSteeringBehavior = m_pFace;
	}
	m_pFace->SetTarget(target);
}

void Survivor::SetToSpin(const Elite::Vector2& target)
{
	if (m_pSteeringBehavior != m_pSpin) 
	{
		m_pSteeringBehavior = m_pSpin;
	}
	m_pSpin->SetTarget(target);
}

void Survivor::SetToAttack(const Elite::Vector2& target)
{
	m_pSteeringBehavior = m_pAttack;
	SetTarget(target);
}

Elite::Vector2 Survivor::GetDefaultSeekPoint() const
{
	return m_defaultHousePos;
}

AgentInfo* Survivor::GetAgentInfo() const
{
	return m_pAgent;
}

void Survivor::SetDecisionMaking(IDecisionMaking* decisionMakingStructure)
{
	if (decisionMakingStructure)
		m_pDecisionMaking = decisionMakingStructure;
}

//HELPER FUNCTIONS
bool Survivor::CanHitEnemy(const Survivor& pAgent, const EnemyInfo& enemy) const 
{
	Elite::Vector2 hitPoint{ ClosestIntersection(pAgent, enemy) };

	if (hitPoint != Elite::Vector2{})
	{
		return true;
	}

	return false;
}

Elite::Vector2 Survivor::ClosestIntersection(const Survivor& pAgent, const EnemyInfo& enemy) const
{
	const float fireRange{  2 * pAgent.GetAgentInfo()->FOV_Range };
	const Elite::Vector2 fireLine{ Elite::GetNormalized(Elite::OrientationToVector(pAgent.GetAgentInfo()->Orientation)) * fireRange };

	//cx,cy is center point of the circle 
	float cx = enemy.Location.x;
	float cy = enemy.Location.y;
	float radius = enemy.Size;

	Elite::Vector2 lineStart{ pAgent.GetAgentInfo()->Position };
	Elite::Vector2 lineEnd{ pAgent.GetAgentInfo()->Position + fireLine };

	Elite::Vector2 intersection1{};
	Elite::Vector2 intersection2{};
	int intersections = FindLineCircleIntersections(cx, cy, radius, lineStart, lineEnd, intersection1, intersection2);

	if (intersections == 1)
		return intersection1; // one intersection

	if (intersections == 2)
	{
		double dist1 = Distance(intersection1, lineStart);
		double dist2 = Distance(intersection2, lineStart);

		if (dist1 < dist2)
			return intersection1;
		else
			return intersection2;
	}

	return Elite::Vector2{}; // no intersections at all
}

// Find the points of intersection.
int Survivor::FindLineCircleIntersections(float cx, float cy, float radius, Elite::Vector2 point1, Elite::Vector2 point2, Elite::Vector2& intersection1, Elite::Vector2& intersection2) const
{
	float dx, dy, A, B, C, det, t{};

	dx = point2.x - point1.x;
	dy = point2.y - point1.y;

	A = dx * dx + dy * dy;
	B = 2 * (dx * (point1.x - cx) + dy * (point1.y - cy));
	C = (point1.x - cx) * (point1.x - cx) + (point1.y - cy) * (point1.y - cy) - radius * radius;

	det = B * B - 4 * A * C;
	if ((A <= 0.0000001) || (det < 0))
	{
		// No real solutions.
		intersection1 = Elite::Vector2(0.f, 0.f);
		intersection2 = Elite::Vector2(0.f, 0.f);
		return 0;
	}
	else if (det == 0)
	{
		// One solution.
		t = -B / (2 * A);
		intersection1 = Elite::Vector2(point1.x + t * dx, point1.y + t * dy);
		intersection2 = Elite::Vector2(0.f, 0.f);
		return 1;
	}
	else
	{
		// Two solutions.
		t = (float)((-B + std::sqrt(det)) / (2 * A));
		intersection1 = Elite::Vector2(point1.x + t * dx, point1.y + t * dy);

		t = (float)((-B - std::sqrt(det)) / (2 * A));
		intersection2 = Elite::Vector2(point1.x + t * dx, point1.y + t * dy);
		return 2;
	}
}