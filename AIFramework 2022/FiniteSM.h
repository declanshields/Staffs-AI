#pragma once
#include <vector>

class Vehicle;
class PickupItem;

using namespace std;

typedef vector<PickupItem*> vecPickups;

class FiniteSM
{
public:
	FiniteSM(Vehicle* car, Vehicle* car2, vecPickups& pickups);
	~FiniteSM();

	void StateMachine();

private:
	Vehicle*   m_car          = nullptr;
	Vehicle*   m_opposingCar  = nullptr;

	float      m_deadzone     = 30.0f;
	float      m_deceleration = 0.5f;
	
	
	float m_acceptableDistance = 300.0f;

	vecPickups m_items;
};

