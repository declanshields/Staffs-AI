#pragma once

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

protected:
	bool	 checkForCollisions(Vehicle* car);
	void	 setRandomPickupPosition(PickupItem* pickup);

private:
	vecPickups              m_pickups;
	Vehicle*				m_pCar  = nullptr;
	Vehicle*                m_pCar2 = nullptr;
	WaypointManager			m_waypointManager;

	float m_deceleration = 0.5f;

	vector<Waypoint*> m_path;

	bool m_blueSeeking       = false;
	bool m_blueArriving      = false;
	bool m_bluePursuit       = false;
	bool m_blueFlee          = false;
	bool m_blueObstacleAvoid = false;

	bool m_redWander         = false;

	bool m_pathfinding       = false;
	bool m_clear             = false;

	//time points to store the start and end points of the timer
	chrono::time_point<chrono::system_clock> start, end;
};

