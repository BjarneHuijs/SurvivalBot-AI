/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "../inc/Exam_HelperStructs.h"
#include "../inc/IExamInterface.h"
using namespace Elite;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) = 0;

	//Seek Functions
	void SetTarget(const Elite::Vector2& target) { m_Target = target; }
	Elite::Vector2 GetTarget() const { return m_Target;  }
	void SetExamInterface(IExamInterface* pExamInterface) { m_pExamInterface = pExamInterface; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	Elite::Vector2 m_Target;
	IExamInterface* m_pExamInterface = nullptr; // Internal

};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;
	
	//void SetTarget(const TargetData& pTarget) override {};
	
	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) override;
};

///////////////////////////////////////
//FLEE
//****
//class Flee : public Seek
class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) override;

private:
	float m_FleeRadius{ 15.f };
};

///////////////////////////////////////
//ARRIVE
//****
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	void SetTargetRadius(float radius);
	void SetSlowRadius(float radius);
	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) override;

private:
	float m_TargetRadius;
	float m_SlowRadius;
};

///////////////////////////////////////
//FACE
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//void SetTarget(const TargetData* pTarget);

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) override;
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) override;

	void SetWanderOffset(float offset) { m_Offset = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetWanderAngleChange(float rad) { m_AngleChange = rad; }

protected:
	float m_Offset = 6.f; // Offset (Agent direction)
	float m_Radius = 4.f; // Wander radius
	float m_AngleChange = ToRadians(45); // Max Wander angle change per frame
	float m_WanderAngle = 0.f; // Internal

private:
	//void SetTarget(const TargetData* pTarget) override {} // Hide SetTarget, wander has no target
};

///////////////////////////////////////
//EVADE
//****
class Evade : public Flee
{
public:
	Evade() = default;
	virtual ~Evade() = default;


	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) override;
private:
	float m_FleeRadius = 15.f;
};

///////////////////////////////////////
//PURSUIT
//****
class Pursuit : public Seek
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;


	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) override;
};

///////////////////////////////////////
//ATTACK
//****
class Attack : public Seek
{
public:
	Attack() = default;
	virtual ~Attack() = default;

	//void SetTarget(const TargetData& pTarget) override {};

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) override;
};

///////////////////////////////////////
//SPIN
//****
class Spin : public Face
{
public:
	Spin() = default;
	virtual ~Spin() = default;

	//void SetTarget(const TargetData& pTarget) override {};

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo* agentInfo) override;
};


#endif
