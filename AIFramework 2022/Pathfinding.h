#pragma once
#include "WaypointManager.h"
#include "structs.h"
#include <queue>
#include <unordered_map>
#include <vector>

class Pathfinding
{
private:
	WaypointManager* m_waypointManager = nullptr;

public:
	Pathfinding(WaypointManager* wpManager);
	~Pathfinding();

	vector<Waypoint*> BreadthFirst(Vector2D startPos, Vector2D endPos);
};
