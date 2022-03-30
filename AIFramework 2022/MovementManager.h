#pragma once
#include "Vector2D.h"

class Vehicle;

class MovementManager
{
public:
	MovementManager(Vehicle* thisCar);
	~MovementManager();

	void Seek();
	void Flee();
	void Arrive();
	void Pursuit(Vehicle* otherCar); //Kind of work now i think
	void Evade(Vehicle* otherCar);	 //Kind of work now i think
	void Wander();

private:
	Vehicle* m_car = nullptr;
	float    m_decelerationDistance = 150.0f;
};

