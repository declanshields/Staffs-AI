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

Path Pathfinding::BreadthFirst(Vector2D startPos, Vector2D endPos)
{
	Waypoint* start = m_waypointManager->getNearestWaypoint(startPos);
	Waypoint* goal = m_waypointManager->getNearestWaypoint(endPos);

	Path returnPath;

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

	current = nullptr;

	returnPath.path = path;
	returnPath.distance = 0;

	return returnPath;
}

Path Pathfinding::Dijkstras(Vector2D startPos, Vector2D endPos)
{
	Waypoint* start = m_waypointManager->getNearestWaypoint(startPos);
	Waypoint* goal = m_waypointManager->getNearestWaypoint(endPos);

	wayPair initial;
	initial.first = 0;
	initial.second = start;

	priority_queue<wayPair, std::vector<wayPair>, greater<wayPair>> frontier;
	frontier.push(initial);

	unordered_map<Waypoint*, Waypoint*> came_from;
	unordered_map<Waypoint*, float> cost_so_far;

	cost_so_far.insert({ start, 0.0f });


	Path returnPath;

	while (!frontier.empty())
	{
		Waypoint* current = frontier.top().second;
		frontier.pop();

		if (current == goal)
		{
			break;
		}

		vecWaypoints currentNeighbours = m_waypointManager->getNeighbouringWaypoints(current);

		for (Waypoint* neighbour : currentNeighbours)
		{
			float costToNeighbour = cost_so_far.find(current)->second + (neighbour->getPosition() - current->getPosition()).Length();
			if ((came_from.find(neighbour) == came_from.end()) || costToNeighbour < cost_so_far.find(neighbour)->second)
			{
				cost_so_far.insert({ neighbour, costToNeighbour });
				came_from.insert({ neighbour, current });

				wayPair temp;
				temp.first = costToNeighbour;
				temp.second = neighbour;

				frontier.push(temp);
			}
		}
	}

	Waypoint* current = new Waypoint();

	vector<Waypoint*> path;
	path.push_back(goal);
	current = goal;		

	while (came_from[current] != current)
	{		
		returnPath.distance += cost_so_far[current];
		Waypoint* temp = came_from[current];

		path.push_back(temp);
		current = temp;
	}

	current = nullptr;

	returnPath.path = path;

	return returnPath;
}

Path Pathfinding::AStar(Vector2D startPos, Vector2D endPos)
{
	Waypoint* start = m_waypointManager->getNearestWaypoint(startPos);
	Waypoint* goal = m_waypointManager->getNearestWaypoint(endPos);

	wayPair initial;
	initial.first = 0;
	initial.second = start;

	Path returnPath;

	priority_queue<wayPair, std::vector<wayPair>, greater<wayPair>> frontier;
	frontier.push(initial);

	unordered_map<Waypoint*, Waypoint*> came_from;
	unordered_map<Waypoint*, float> cost_so_far;

	cost_so_far.insert({ start, 0.0f });

	while (!frontier.empty())
	{
		Waypoint* current = frontier.top().second;
		frontier.pop();

		if (current == goal)
			break;

		vecWaypoints currentNeighbours = m_waypointManager->getNeighbouringWaypoints(current);

		for (Waypoint* neighbour : currentNeighbours)
		{
			float costToNeighbour = cost_so_far.find(current)->second + (neighbour->getPosition() - current->getPosition()).Length();
			if ((came_from.find(neighbour) == came_from.end()) || costToNeighbour < cost_so_far.find(neighbour)->second)
			{
				cost_so_far.insert({ neighbour, costToNeighbour });
				float priority = costToNeighbour + (goal->getPosition() - current->getPosition()).Length();

				came_from.insert({ neighbour, current });

				wayPair temp;
				temp.first = priority;
				temp.second = neighbour;

				frontier.push(temp);
			}
		}
	}

	Waypoint* current = new Waypoint();

	vector<Waypoint*> path;
	path.push_back(goal);
	current = goal;

	while (came_from[current] != current)
	{
		returnPath.distance += cost_so_far[current];
		Waypoint* temp = came_from[current];

		path.push_back(temp);
		current = temp;
	}

	returnPath.path = path;
	current = nullptr;

	return returnPath;
}