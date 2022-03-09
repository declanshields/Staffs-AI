#pragma once
#include "Vector2D.h"

class Vehicle;

class MovementManager
{
public:
	MovementManager(Vehicle* thisCar);
	~MovementManager();

	Vector2D Seek();
	Vector2D Flee();
	Vector2D Arrive();
	Vector2D Pursuit(Vehicle* otherCar);
	Vector2D Evade(Vehicle* otherCar);

private:
	Vehicle* m_car = nullptr;
	float m_decelerationDistance = 250.0f;
};

