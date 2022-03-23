#pragma once
#include "WaypointManager.h"
#include "structs.h"
#include <queue>
#include <unordered_map>
#include <vector>

typedef std::pair<float, Waypoint*> wayPair;

class Pathfinding
{
private:
	WaypointManager* m_waypointManager = nullptr;

public:
	Pathfinding(WaypointManager* wpManager);
	~Pathfinding();

	Path BreadthFirst(Vector2D startPos, Vector2D endPos);
	Path Dijkstras(Vector2D startPos, Vector2D endPos);
	Path AStar(Vector2D startPos, Vector2D endPos);
};

