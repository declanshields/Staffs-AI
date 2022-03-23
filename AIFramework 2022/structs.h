#pragma once
#include <vector>
#include "Waypoint.h"

struct Node
{
	int data;
};

struct Path
{
	std::vector<Waypoint*> path;
	float distance = 0.0f;
};