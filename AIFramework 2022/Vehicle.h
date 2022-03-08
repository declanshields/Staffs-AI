#pragma once

#include "DrawableGameObject.h"
#include "Vector2D.h"
#include "Collidable.h"
#include "Pathfinding.h"


enum class carColour
{
	redCar,
	blueCar,
};

enum class state
{
	Seek,
	Flee,
	Arrive,
	Pursuit,
	Evade,
	Wander,
	ObstacleAvoid,
	Idle,
};

class Vehicle : public DrawableGameObject, public Collidable
{
public:
	virtual HRESULT initMesh(ID3D11Device* pd3dDevice, carColour colour);
	virtual void update(const float deltaTime);

	void setMaxSpeed(const float maxSpeed) { m_maxSpeed = maxSpeed; }
	void setCurrentSpeed(const float speed); // a ratio: a value between 0 and 1 (1 being max speed)
	void setPositionTo(Vector2D positionTo); // a position to move to
	Vector2D getPositionTo() { return m_positionTo; } // returns the position the car is moving towards
	void setVehiclePosition(Vector2D position); // the current position - this resets positionTo
	void setWaypointManager(WaypointManager* wpm);
	void hasCollided() {}

	float getCurrentSpeed() { return m_currentSpeed; }
	Pathfinding* GetPathfinderManager() { return m_pathfinding; }

	//function for setting state
	void setState(state newState) { m_currentState = newState; }
	state getState() { return m_currentState; }

protected: // protected methods


protected: // preotected properties
	float m_maxSpeed;
	float m_currentSpeed;

	float m_mass = 1.0f;
	Vector2D m_velocity;
	Vector2D m_totalForce;
	float m_deceleration = 0.75f;

	state m_currentState = state::Idle;
	
	Vector2D m_currentPosition;
	Vector2D m_startPosition;
	Vector2D m_positionTo;
	Vector2D m_lastPosition;
	WaypointManager* m_waypointManager;
	Pathfinding* m_pathfinding;
};

