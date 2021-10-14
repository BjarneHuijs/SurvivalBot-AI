//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../inc/IExamInterface.h"
//#include "SteeringAgent.h"

//SEEK
//****

SteeringPlugin_Output Seek::CalculateSteering(float deltaT, AgentInfo* agentInfo)
{
	SteeringPlugin_Output steering = {};

	//Simple Seek Behaviour (towards Target)
	m_Target = m_pExamInterface->NavMesh_GetClosestPathPoint(m_Target);

	steering.LinearVelocity = m_Target - agentInfo->Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo->MaxLinearSpeed; //Rescale to Max Speed

	if (Distance(m_Target, agentInfo->Position) < 1.f)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}

	//steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	return steering;
}

//FLEE
//****

SteeringPlugin_Output Flee::CalculateSteering(float deltaT, AgentInfo* agentInfo)
{

	auto distance{ Distance(agentInfo->Position, m_Target) };
	if (distance > m_FleeRadius)
	{
		SteeringPlugin_Output steering;
		//steering.IsValid = false;
		return steering;
	}

	SteeringPlugin_Output steering = {};

	//Simple Seek Behaviour (towards Target)
	steering.LinearVelocity = m_Target - agentInfo->Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo->MaxLinearSpeed; //Rescale to Max Speed

	steering.LinearVelocity *= -1; //Invert seek velocity to flee

	if (Distance(m_Target, agentInfo->Position) < 2.f)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}

	//steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	return steering;
}

//ARRIVE
//****

SteeringPlugin_Output Arrive::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	steering.AutoOrient = true;

	const float slowRadius{ 10.f };
	const float distance{ Distance(m_Target, pAgent->Position) };



	//if((m_Target).Position.Distance(pAgent->GetPosition()) < slowRadius)
	if (distance < slowRadius)
	{
		steering.LinearVelocity = m_Target - pAgent->Position; // Desired velocity;
		steering.LinearVelocity.Normalize(); // Normalize desired velocity
		steering.LinearVelocity *= pAgent->MaxLinearSpeed; // Rescale to max speed
		steering.LinearVelocity *= (distance / slowRadius); // Scale down velocity to stop at target
	}
	else
	{
		steering.LinearVelocity = m_Target - pAgent->Position; // Desired velocity;
		steering.LinearVelocity.Normalize(); // Normalize desired velocity
		steering.LinearVelocity *= pAgent->MaxLinearSpeed; // Rescale to max speed
	}

	return steering;
}

void Arrive::SetTargetRadius(float radius)
{
	m_TargetRadius = radius;
}

void Arrive::SetSlowRadius(float radius)
{
	m_SlowRadius = radius;
}

//FACE
//****

SteeringPlugin_Output Face::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering{};

	const Elite::Vector2 toTarget{ m_Target - pAgent->Position };

	const  float to{ atan2f(toTarget.y, toTarget.x) + float(E_PI_2) };
	float from{ pAgent->Orientation };
	from = atan2f(sinf(from), cosf(from));
	float desired = to - from;

	const float Pi2 = float(E_PI) * 2.f;
	if (desired > E_PI)
		desired -= Pi2;
	else if (desired < -E_PI)
		desired += Pi2;

	// multiply desired by some value to make it go as fast as possible (30.f)
	steering.AngularVelocity = desired * 30.f;

	steering.AutoOrient = false;

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringPlugin_Output Wander::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	SteeringPlugin_Output steering = {};
	steering.AutoOrient = true;

	//SetWanderOffset(6.f);
	//SetWanderRadius(4.f);

	Elite::Vector2 targetCenter{ pAgent->Position + pAgent->LinearVelocity.GetNormalized() * m_Offset };

	const float deltaAngle = m_AngleChange * (randomFloat(2.f) - 1.f);
	m_WanderAngle += deltaAngle;
	
	Elite::Vector2 targetVector{ cos(m_WanderAngle) * m_Radius, sin(m_WanderAngle) * m_Radius };

	if (Elite::DistanceSquared(pAgent->Position, m_Target) < Elite::Square(m_Offset))
	{
		m_Target = { targetCenter.x + targetVector.x, targetCenter.y + targetVector.y };
		m_Target = m_pExamInterface->NavMesh_GetClosestPathPoint(m_Target);
	}

	steering = Seek::CalculateSteering(deltaT, pAgent);

	return steering;
}

//EVADE
//****
SteeringPlugin_Output Evade::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	const float predictionRadius{ 4.f };

	auto distance{ Distance(pAgent->Position, m_Target) };
	if (distance > m_FleeRadius)
	{
		SteeringPlugin_Output steering;

		return steering;
	}
	float offset{ distance / (pAgent->MaxLinearSpeed * 0.9f) };


	SteeringPlugin_Output steering = {};
	steering.AutoOrient = true;


	steering.LinearVelocity = -1 * (m_Target - pAgent->Position); // Desired velocity;
	//steering.LinearVelocity = (pAgent->GetPosition() - (m_Target).Position); same result
	steering.LinearVelocity.Normalize(); // Normalize desired velocity
	steering.LinearVelocity *= pAgent->MaxLinearSpeed; // Rescale to max speed

	return steering;
}

//PURSUIT
//****

SteeringPlugin_Output Pursuit::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	const float predictionRadius{ 4.f };

	float distance{ Distance(m_Target, pAgent->Position) };
	float offset{ distance / (pAgent->MaxLinearSpeed * 0.9f) };

	SteeringPlugin_Output steering = {};
	steering.AutoOrient = true;
	steering = Seek::CalculateSteering(deltaT, pAgent);
	steering.LinearVelocity *= 0.9f;

	return steering;
}

//ATTACK
//****

SteeringPlugin_Output Attack::CalculateSteering(float deltaT, AgentInfo* agentInfo)
{
	SteeringPlugin_Output steering = {};

	//Simple Seek Behaviour (towards Target)
	//m_Target = m_pExamInterface->NavMesh_GetClosestPathPoint(m_Target);

	steering = Seek::CalculateSteering(deltaT, agentInfo);

	//steering.LinearVelocity = m_Target - agentInfo->Position; //Desired Velocity
	//steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	//steering.LinearVelocity *= agentInfo->MaxLinearSpeed; //Rescale to Max Speed
	steering.AutoOrient = false;
	steering.LinearVelocity *= -1; // invert speed to walk away from enemy

	/*if (Distance(m_Target, agentInfo->Position) < 1.f)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}*/

	//steering.AngularVelocity = -agentInfo->AngularVelocity; //Rotate your character to inspect the world while walking
	//steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	return steering;
}

//SPIN
//****

SteeringPlugin_Output Spin::CalculateSteering(float deltaT, AgentInfo* agentInfo)
{
	SteeringPlugin_Output steering = {};
	steering.LinearVelocity = agentInfo->LinearVelocity;
	steering.AutoOrient = false;
	steering.AngularVelocity = 30.f;

	return steering;
}