/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef SURVIVOR_BOT_BEHAVIORS
#define SURVIVOR_BOT_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "stdafx.h"
#include "SteeringBehaviors.h"
#include "Survivor.h"
#include "../inc/IExamInterface.h"
#include "ConsoleOutputReducer.h"
#include <chrono>

using time_point = std::chrono::time_point<std::chrono::system_clock>;
//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent);

	if (!pAgent)
		return Failure;

	pAgent->SetToWander();
	//std::cout << "Wandering..." << std::endl;
	return Success;
}

BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	Vector2 seekTarget{};
	IExamInterface* examInterface = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Target", seekTarget) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!pAgent || !examInterface)
		return Failure;

	//TODO: Implement Change to seek (Target)
	pAgent->SetToSeek(examInterface->NavMesh_GetClosestPathPoint(seekTarget));
	//std::cout << "Seeking..." << std::endl;
	return Success;
}

BehaviorState LookAround(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	Vector2 seekTarget{};
	IExamInterface* examInterface = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Target", seekTarget) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!pAgent || !examInterface)
		return Failure;

	//TODO: Implement Change to seek (Target)
	pAgent->SetToSpin(examInterface->NavMesh_GetClosestPathPoint(seekTarget));
	//std::cout << "Looking..." << std::endl;
	return Success;
}

BehaviorState FleeFromEnemy(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	Vector2 seekTarget{};
	IExamInterface* examInterface = nullptr;
	EntityInfo entity{};

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Target", seekTarget) &&
		pBlackboard->GetData("TargetEntity", entity) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!dataAvailable || !pAgent || !examInterface)
		return Failure;

	EnemyInfo enemy{};

	//TODO: Implement Change to seek (Target)
	auto coord{ GetConsoleCursorPosition() };
	if (entity.Type == eEntityType::ENEMY && examInterface->Enemy_GetInfo(entity, enemy))
	{
		GoToConsoleCursorPosition(coord);
		//Once grabbed, you can add it to a specific inventory slot
		//Slot must be empty

		pAgent->SetToFlee(examInterface->NavMesh_GetClosestPathPoint(seekTarget));
		std::cout << "Running from enemy..." << std::endl;
		return Success;

	}else
		GoToConsoleCursorPosition(coord);
	return Failure;
}

BehaviorState TargetEnemy(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	Vector2 seekTarget{};
	IExamInterface* examInterface = nullptr;
	EntityInfo entity{};

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Target", seekTarget) &&
		pBlackboard->GetData("TargetEntity", entity) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!dataAvailable || !pAgent || !examInterface)
		return Failure;

	EnemyInfo enemy{};
	BehaviorState result{ Failure };

	auto coord{ GetConsoleCursorPosition() };
	if (entity.Type == eEntityType::ENEMY && examInterface->Enemy_GetInfo(entity, enemy))
	{
		GoToConsoleCursorPosition(coord);

		ItemInfo currentItemInSlot{};
		for (int i{}; i < examInterface->Inventory_GetCapacity(); i++)
		{
			coord = GetConsoleCursorPosition();
			if (examInterface->Inventory_GetItem(i, currentItemInSlot) && currentItemInSlot.Type == eItemType::PISTOL)
			{
				GoToConsoleCursorPosition(coord);
				if (examInterface->Weapon_GetAmmo(currentItemInSlot) > 0) 
				{
					pAgent->SetToFace(enemy.Location);
					std::cout << "attacking enemy..." << std::endl;
					return Success;
					break;
				}
			}else
				GoToConsoleCursorPosition(coord);

		}

	}else
		GoToConsoleCursorPosition(coord);

	return Failure;
}

BehaviorState ShootEnemy(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	Vector2 seekTarget{};
	IExamInterface* examInterface = nullptr;
	EntityInfo entity{};

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Target", seekTarget) &&
		pBlackboard->GetData("TargetEntity", entity) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!dataAvailable || !pAgent || !examInterface)
		return Failure;

	EnemyInfo enemy{};

	//auto coord{ GetConsoleCursorPosition() };
	if (entity.Type == eEntityType::ENEMY)
	{
		if (examInterface->Enemy_GetInfo(entity, enemy))
		{
			//GoToConsoleCursorPosition(coord);
			ItemInfo currentItemInSlot{};

			for (int i{}; i < examInterface->Inventory_GetCapacity(); i++)
			{
				auto coord = GetConsoleCursorPosition();
				if (examInterface->Inventory_GetItem(i, currentItemInSlot) && currentItemInSlot.Type == eItemType::PISTOL)
				{
					GoToConsoleCursorPosition(coord);
					if (examInterface->Weapon_GetAmmo(currentItemInSlot) > 0)
					{

						float targetval{ Elite::Dot(Elite::GetNormalized(pAgent->GetAgentInfo()->LinearVelocity), Elite::GetNormalized(enemy.Location - pAgent->GetAgentInfo()->Position)) };
						std::cout << targetval << std::endl;

						//if (targetval >= 0.995f) 
						if (pAgent->CanHitEnemy(*pAgent, enemy))
						{
							examInterface->Inventory_UseItem(i);

							std::cout << "Shot at enemy..." << std::endl;
							return Success;
							break;
						}else
						{
							pAgent->SetToFace(enemy.Location);
							return Success;
						}
					}
					else if (examInterface->Weapon_GetAmmo(currentItemInSlot) <= 0)
					{
						examInterface->Inventory_RemoveItem(i);
					}
				}
				else
					GoToConsoleCursorPosition(coord);
			}
		}
		else
			//GoToConsoleCursorPosition(coord);

			return Failure;
	}
	
	return Failure;
}

BehaviorState GrabItem(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	Vector2 seekTarget{};
	IExamInterface* examInterface = nullptr;
	EntityInfo entity{};

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Target", seekTarget) &&
		pBlackboard->GetData("TargetEntity", entity) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!dataAvailable || !pAgent || !examInterface)
		return Failure;

	int nrOfGuns{0};
	int nrOfMedKits{0};
	int nrOfFoods{0};
	auto invCap{ examInterface->Inventory_GetCapacity() };

	ItemInfo item{};
	bool invFull{ false };
	//TODO: Implement Change to seek (Target)
	if (entity.Type == eEntityType::ITEM) 
	{
		auto coord{ GetConsoleCursorPosition() };
		if (Elite::DistanceSquared(pAgent->GetAgentInfo()->Position, entity.Location) <= Elite::Square(pAgent->GetAgentInfo()->GrabRange) && examInterface->Item_GetInfo(entity, item))
		{
			GoToConsoleCursorPosition(coord);
			if (item.Type == eItemType::GARBAGE)
			{
				examInterface->Item_Destroy(entity);
			}

			for (int i{}; i < invCap; i++)
			{
				ItemInfo tempItem{};
				coord = GetConsoleCursorPosition();
				if (examInterface->Inventory_GetItem(i, tempItem))
				{
					GoToConsoleCursorPosition(coord);
					if (tempItem.Type == eItemType::PISTOL)
					{
						++nrOfGuns;
					}
					else if (tempItem.Type == eItemType::FOOD)
					{
						++nrOfFoods;
					}
					else if (tempItem.Type == eItemType::MEDKIT)
					{
						++nrOfMedKits;
					}
				}
				else
					GoToConsoleCursorPosition(coord);
			}

			if (!((item.Type == eItemType::PISTOL && nrOfGuns >= 2)
				|| (item.Type == eItemType::FOOD && nrOfFoods >= 2)
				|| (item.Type == eItemType::MEDKIT && nrOfMedKits >= 2)))
			{

				//Once grabbed, you can add it to a specific inventory slot
				//Slot must be empty
				ItemInfo currentItemInSlot{};
				for (int i{}; i < invCap; i++)
				{
					coord = GetConsoleCursorPosition();
					if (!examInterface->Inventory_GetItem(i, currentItemInSlot))
					{
						GoToConsoleCursorPosition(coord);
						invFull = false;

						if (examInterface->Item_Grab(entity, item))
						{
							std::cout << "Grabbed Item..." << std::endl;

							examInterface->Inventory_AddItem(i, item);
							invFull = true;
							break;
						}
					}
					else
					{
						invFull = true;
						GoToConsoleCursorPosition(coord);
					}
				}
			}

		}
		else
			GoToConsoleCursorPosition(coord);

		if (!invFull)
		{
			pAgent->SetToSeek(examInterface->NavMesh_GetClosestPathPoint(seekTarget));
		}
		return Success;
	}
	else
		return Failure;
}

BehaviorState EatFood(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	IExamInterface* examInterface = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!dataAvailable || !pAgent || !examInterface)
		return Failure;


	int ItemWithBestFoodValue{-1};
	ItemInfo bestFood{};

	for (int i{}; i < examInterface->Inventory_GetCapacity(); i++)
	{
		auto coord{ GetConsoleCursorPosition() };
		if (examInterface->Inventory_GetItem(i, bestFood))
		{
			GoToConsoleCursorPosition(coord);

			if(bestFood.Type == eItemType::FOOD)
			{
				ItemWithBestFoodValue = i;
				break;
			}
		}else
			GoToConsoleCursorPosition(coord);

	}
	
	ItemInfo currentItemInSlot{};
	for (int i{}; i < examInterface->Inventory_GetCapacity(); i++)
	{
		auto coord{ GetConsoleCursorPosition() };
		if (i != ItemWithBestFoodValue && examInterface->Inventory_GetItem(i, currentItemInSlot))
		{
			GoToConsoleCursorPosition(coord);
			if(currentItemInSlot.Type == eItemType::FOOD && (examInterface->Food_GetEnergy(bestFood) < examInterface->Food_GetEnergy(currentItemInSlot)))
			{
				ItemWithBestFoodValue = i;
				bestFood = currentItemInSlot;
			}
		}else
			GoToConsoleCursorPosition(coord);

	}

	if (ItemWithBestFoodValue > -1) 
	{

		examInterface->Inventory_UseItem(ItemWithBestFoodValue);
		examInterface->Inventory_RemoveItem(ItemWithBestFoodValue);
		std::cout << "Ate Food..." << std::endl;
		return Success;
	}
	return Failure;
}

BehaviorState UseMedKit(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	IExamInterface* examInterface = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!dataAvailable || !pAgent || !examInterface)
		return Failure;


	int ItemWithBestHealthValue{-1};
	ItemInfo bestKit{};

	for (int i{}; i < examInterface->Inventory_GetCapacity(); i++)
	{
		auto coord{ GetConsoleCursorPosition() };
		if (examInterface->Inventory_GetItem(i, bestKit))
		{
			GoToConsoleCursorPosition(coord);
			if (bestKit.Type == eItemType::MEDKIT)
			{
				ItemWithBestHealthValue = i;
				break;
			}
		}else
			GoToConsoleCursorPosition(coord);
	}

	ItemInfo currentItemInSlot{};
	for (int i{}; i < examInterface->Inventory_GetCapacity(); i++)
	{
		auto coord{ GetConsoleCursorPosition() };
		if (i != ItemWithBestHealthValue && examInterface->Inventory_GetItem(i, currentItemInSlot))
		{
			GoToConsoleCursorPosition(coord);
			if (currentItemInSlot.Type == eItemType::MEDKIT && (examInterface->Medkit_GetHealth(bestKit) < examInterface->Medkit_GetHealth(currentItemInSlot)))
			{
				ItemWithBestHealthValue = i;
				bestKit = currentItemInSlot;
			}
		}else
			GoToConsoleCursorPosition(coord);
	}

	if (ItemWithBestHealthValue > -1)
	{
		examInterface->Inventory_UseItem(ItemWithBestHealthValue);
		examInterface->Inventory_RemoveItem(ItemWithBestHealthValue);
		std::cout << "Used MedKit..." << std::endl;
		return Success;
	}
	return Failure;
}

BehaviorState FleeFromTarget(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	Vector2 seekTarget{};
	IExamInterface* examInterface = nullptr;
	EntityInfo entity{};

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Target", seekTarget) &&
		pBlackboard->GetData("TargetEntity", entity) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!dataAvailable || !pAgent || !examInterface)
		return Failure;

	//TODO: Implement Change to seek (Target)
	PurgeZoneInfo pZone{};

	if (entity.Type == eEntityType::PURGEZONE)
	{
		if (examInterface->PurgeZone_GetInfo(entity, pZone) && Elite::DistanceSquared(pAgent->GetAgentInfo()->Position, pZone.Center) <= Elite::Square(pZone.Radius)) 
		{
			pAgent->SetToFlee(examInterface->NavMesh_GetClosestPathPoint(pZone.Center));
			std::cout << "Running from Purge..." << std::endl;
			return Success;
		}
		//Once grabbed, you can add it to a specific inventory slot
		//Slot must be empty


	}

	return Failure;
}

//==================
// Conditionals
//==================
bool HasDefaultPoint(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	IExamInterface* examInterface = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("Interface", examInterface);
	;

	if (!pAgent || !examInterface)
		return false;

	if (examInterface->World_GetStats().TimeSurvived < 10) 
	{
		//TODO: Check for food closeby and set target accordingly
		if (pBlackboard->ChangeData("Target", pAgent->GetDefaultSeekPoint()))
		{
			return true;
		}
	}

	return false;
}

bool FoundHouse(Elite::Blackboard* pBlackboard)
{

	Survivor* pAgent = nullptr;
	std::vector<House>* housesVec = nullptr;
	IExamInterface* examInterface = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("HousesVec", housesVec) &&
		pBlackboard->GetData("Interface", examInterface);

	if (!pAgent || !housesVec || !examInterface)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float detectionRange{ 100.f };
	auto housesIt = std::find_if(housesVec->begin(), housesVec->end(), [&pAgent, &detectionRange](const House& house) {
		//return Elite::DistanceSquared(pAgent->GetAgentInfo()->Position, house.Center) < Elite::Square(detectionRange);
		if (house.visited == false)
			return true;

		return false;
		});

	if (housesIt != housesVec->end() && !(*housesIt).visited)
	{
		//examInterface->Draw_SolidCircle((*housesIt).house.Center, .7f, { 0,0 }, { 1, 0, 0 });
		//std::cout << "found house..." << std::endl;

		pBlackboard->ChangeData("Target", (*housesIt).house.Center);
		return true;
	}

	return false;
}

bool ReachedHouse(Elite::Blackboard* pBlackboard)
{


	Survivor* pAgent = nullptr;
	std::vector<House>* housesVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("HousesVec", housesVec);

	if (!pAgent || !housesVec)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float detectionRange{ 2.f };
	auto housesIt = std::find_if(housesVec->begin(), housesVec->end(), [&pAgent, &detectionRange](const House& house) {
		if (house.visited == false) 
		{
			return true;
		};

		return false;

		});

	if (housesIt != housesVec->end())
	{

		if(Elite::DistanceSquared(pAgent->GetAgentInfo()->Position, housesIt->house.Center) < Elite::Square(detectionRange) && pAgent->GetAgentInfo()->IsInHouse)
		{
			housesIt->visited = true;
			std::cout << "Reached House..." << std::endl;
			return true;
		}
		else {
			pBlackboard->ChangeData("Target", (*housesIt).house.Center);
			return false;
		}

	}



	return false;

	return false;
}

bool FoundItemInGrabRange(Elite::Blackboard* pBlackboard)
{


	Survivor* pAgent = nullptr;
	std::vector<EntityInfo>* entityVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("EntityVec", entityVec);

	if (!pAgent || !entityVec)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float detectionRange{ 2.f };
	auto entityIt = std::find_if(entityVec->begin(), entityVec->end(), [&pAgent](const EntityInfo& entity) {
		if(entity.Type == eEntityType::ITEM && (Elite::DistanceSquared(pAgent->GetAgentInfo()->Position, entity.Location) < Elite::Square(pAgent->GetAgentInfo()->GrabRange)) )
		{
			return true;
		}

		});

	if (entityIt != entityVec->end())
	{
		if ((*entityIt).Type == eEntityType::ITEM)
		{
			pBlackboard->ChangeData("TargetEntity", (*entityIt));
			pBlackboard->ChangeData("Target", (*entityIt).Location);
			//std::cout << "found item..." << std::endl;

			return true;
		}
	}

	return false;
}

bool IsInWorldBoundaries(Elite::Blackboard* pBlackboard)
{


	Survivor* pAgent = nullptr;
	WorldInfo* worldInfo = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("WorldInfo", worldInfo);

	if (!pAgent || !worldInfo)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float detectionRange{ 2.f };
	if( pAgent->GetAgentInfo()->Position.x <= (worldInfo->Center.x - worldInfo->Dimensions.x / 2.f) ||
		pAgent->GetAgentInfo()->Position.y <= (worldInfo->Center.y - worldInfo->Dimensions.y / 2.f) ||
		pAgent->GetAgentInfo()->Position.x >= (worldInfo->Center.x + worldInfo->Dimensions.x / 2.f) ||
		pAgent->GetAgentInfo()->Position.y >= (worldInfo->Center.y + worldInfo->Dimensions.y / 2.f))
	{

		pBlackboard->ChangeData("Target", worldInfo->Center);
		return false;

	}

	//std::cout << "out of world..." << std::endl;
	return true;
}

bool FoundEnemy(Elite::Blackboard* pBlackboard)
{

	Survivor* pAgent = nullptr;
	std::vector<EntityInfo>* entityVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("EntityVec", entityVec);

	if (!pAgent || !entityVec)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float detectionRange{ 2.f };
	auto entityIt = std::find_if(entityVec->begin(), entityVec->end(), [&pAgent](const EntityInfo& entity) {
		if (entity.Type == eEntityType::ENEMY)
		{
			return true;
		}
		//return pAgent->GetAgentInfo()->IsInHouse;
		/*if (Elite::DistanceSquared(pAgent->GetAgentInfo()->Position, house.house.Center) < Elite::Square(detectionRange))
		{
			house.visited = true;
			return true;
		}*/
		});

	if (entityIt != entityVec->end() && (*entityIt).Type == eEntityType::ENEMY)
	{
		pBlackboard->ChangeData("TargetEntity", (*entityIt));
		pBlackboard->ChangeData("Target", (*entityIt).Location);
		std::cout << "enemy found..." << std::endl;

		return true;
	}

	return false;
}

bool IsLowOnEnergy(Elite::Blackboard* pBlackboard)
{

	Survivor* pAgent = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent);

	if (!pAgent)
		return false;

	if(pAgent->GetAgentInfo()->Energy < 2.5f)
	{
		//std::cout << "low energy..." << std::endl;

		return true;
	}

	return false;
}

bool WasBittenOrLowOnHealth(Elite::Blackboard* pBlackboard)
{

	Survivor* pAgent = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent);

	if (!pAgent)
		return false;

	if(pAgent->GetAgentInfo()->Health < 2.5f || pAgent->GetAgentInfo()->WasBitten)
	{
		//std::cout << "Bitten or low..." << std::endl;

		return true;
	}

	return false;

}

bool IsInPurgeZone(Elite::Blackboard* pBlackboard)
{

	Survivor* pAgent = nullptr;
	std::vector<EntityInfo>* entityVec = nullptr;
	IExamInterface* examInterface = nullptr;

	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("EntityVec", entityVec) &&
		pBlackboard->GetData("Interface", examInterface);


	if (!pAgent || !entityVec || !examInterface)
		return false;

	//TODO: Check for food closeby and set target accordingly
	auto entityIt = std::find_if(entityVec->begin(), entityVec->end(), [](const EntityInfo& entityInfo) {
		return entityInfo.Type == eEntityType::PURGEZONE;
		});
	
	
	PurgeZoneInfo pZone{};
	if (entityIt != entityVec->end())
	{
		if (entityIt->Type == eEntityType::PURGEZONE) {
			if (examInterface->PurgeZone_GetInfo(*entityIt, pZone))
			{
				if (Elite::DistanceSquared(pAgent->GetAgentInfo()->Position, pZone.Center) < Elite::Square(pZone.Radius * 1.2f))
				{
					std::cout << "Inside of PurgeZone..." << std::endl;
					return true;
				}
			}
		}
	}

	return false;
}

bool WasBitten(Elite::Blackboard* pBlackboard)
{
	Survivor* pAgent = nullptr;
	time_point currentTime{ std::chrono::system_clock::now() };
	time_point lastBite{};
	IExamInterface* examInterface = nullptr;
			
	auto dataAvailable = pBlackboard->GetData("Survivor", pAgent) &&
		pBlackboard->GetData("lastBiteTime", lastBite) &&
		pBlackboard->GetData("Interface", examInterface);


	if (!dataAvailable || !pAgent || !examInterface)
		return false;


	ItemInfo currentItemInSlot{};
	for (int i{}; i < examInterface->Inventory_GetCapacity(); i++)
	{
		auto coord = GetConsoleCursorPosition();
		if (examInterface->Inventory_GetItem(i, currentItemInSlot) && currentItemInSlot.Type == eItemType::PISTOL)
		{
			GoToConsoleCursorPosition(coord);
			if (examInterface->Weapon_GetAmmo(currentItemInSlot) > 0)
			{
				if (pAgent->GetAgentInfo()->WasBitten)
				{
					pBlackboard->ChangeData("lastBiteTime", currentTime);
					return true;
				}

				float elapsedTime{ float(std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastBite).count()) };
				if (elapsedTime <= 2.f)
				{
					return true;
				}
			}
			else if (examInterface->Weapon_GetAmmo(currentItemInSlot) <= 0)
			{
				return false;
			}
		}
		else
			GoToConsoleCursorPosition(coord);
	}
	return false;



}

#endif