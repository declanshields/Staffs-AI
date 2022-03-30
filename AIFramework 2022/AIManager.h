#pragma once

#include "structs.h"
#include "WaypointManager.h"
#include <chrono>

using namespace std;

class   Vehicle;
class   PickupItem;
typedef vector<PickupItem*> vecPickups;

class AIManager
{
public:
	AIManager();
	virtual  ~AIManager();
	void	 release();
	HRESULT  initialise(ID3D11Device* pd3dDevice);
	void	 update(const float fDeltaTime);
	void	 mouseUp(int x, int y);
	void	 keyDown(WPARAM param);
	void	 keyUp(WPARAM param);
	void     handleStates();

protected:
	bool	 checkForCollisions(Vehicle* car);
	void	 setRandomPickupPosition(PickupItem* pickup);

private:
	vecPickups              m_pickups;
	Vehicle*				m_pCar  = nullptr;
	Vehicle*                m_pCar2 = nullptr;
	WaypointManager			m_waypointManager;

	      float m_deceleration = 0.5f;
	const float m_deadZone     = 30.0f;

	Path m_path;
	Path m_fuelPath;

	bool m_clear             = false;

	//time points to store the start and end points of the timer
	chrono::time_point<chrono::system_clock> start, end;
};

