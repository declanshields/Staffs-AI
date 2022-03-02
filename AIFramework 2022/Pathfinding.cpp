#include "Pathfinding.h"
#include "Waypoint.h"

Pathfinding::Pathfinding(WaypointManager* wpManager)
{
	m_waypointManager = wpManager;
}

Pathfinding::~Pathfinding()
{
	if (m_waypointManager != nullptr) 
	{
		delete m_waypointManager;
		m_waypointManager = nullptr;
	}
}

vector<Waypoint*> Pathfinding::BreadthFirst(Vector2D startPos, Vector2D endPos)
{
	Waypoint* start = m_waypointManager->getNearestWaypoint(startPos);
	Waypoint* goal = m_waypointManager->getNearestWaypoint(endPos);

	queue<Waypoint*> frontier;
	frontier.push(start);

	unordered_map<Waypoint*, Waypoint*> came_from;

	came_from.insert({ start, start });

	while (!frontier.empty())
	{
		Waypoint* current = frontier.front();
		frontier.pop();

		if (current == goal)
			break;

		vecWaypoints currentNeighbours = m_waypointManager->getNeighbouringWaypoints(current);

		for (Waypoint* neighbour : currentNeighbours)
		{
			unordered_map<Waypoint*, Waypoint*>::iterator val = came_from.find(neighbour);

			if (val != came_from.end())
				continue;
			else
			{
				frontier.push(neighbour);

				came_from.insert({ neighbour, current });
			}
		}
	}

	Waypoint* current = new Waypoint();

	vector<Waypoint*> path;
	path.push_back(goal);
	current = goal;

	while (came_from[current] != current)
	{
		Waypoint* temp = came_from[current];

		path.push_back(temp);
		current = temp;
	}

	return path;
}