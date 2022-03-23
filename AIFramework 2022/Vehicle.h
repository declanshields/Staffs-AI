#pragma once

#include "DrawableGameObject.h"
#include "Vector2D.h"
#include "Collidable.h"
#include "MovementManager.h"
#include "Pathfinding.h"


enum class carColour
{
	redCar,
	blueCar,
};

enum class state
{
	Seek,
	MouseSeek,
	Flee,
	Arrive,
	Pursuit,
	Evade,
	Wander,
	ObstacleAvoid,
	Idle,
	Pathfinding,
	CollectFuel,
};

class Vehicle : public DrawableGameObject, public Collidable
{
public:
	virtual HRESULT  initMesh(ID3D11Device* pd3dDevice, carColour colour);
	virtual void     update(const float deltaTime);

	void             setMaxSpeed(const float maxSpeed) { m_maxSpeed = maxSpeed; }
	void             setCurrentSpeed(const float speed);       // a ratio: a value between 0 and 1 (1 being max speed)
	void             setPositionTo(Vector2D positionTo);       // a position to move to
	void             setVehiclePosition(Vector2D position);    // the current position - this resets positionTo
	void             setWaypointManager(WaypointManager* wpm);
	void             setAcceleration(Vector2D force) { m_acceleration = force / m_mass; }
	void             hasCollided() {}
	float            setFuel() { m_fuel = m_fuelMax; }
	Vector2D         getPositionTo() { return m_positionTo; }  // returns the position the car is moving towards

	float            getCurrentSpeed()      { return m_currentSpeed; }
	float            getMaxSpeed()          { return m_maxSpeed; }
	float            getFuelLeft()          { return m_fuel; }
	Vector2D         getVelocity()          { return m_velocity; }
	Pathfinding*     getPathfinderManager() { return m_pathfinding; }
	MovementManager* getMovementManager()   { return m_movementManager; }

	//function for setting state
	void  setState(state newState) { m_currentState = newState; }
	state getState()               { return m_currentState; }

protected: // protected methods


protected: // preotected properties
	float     m_currentSpeed;

	float     m_mass         = 1.0f;
	Vector2D  m_velocity;
	Vector2D  m_acceleration;
	float     m_maxForce     = 500.0f;
	float     m_maxSpeed     = 100.0f;
	float     m_deceleration = 0.5f;
	state     m_currentState = state::Idle;
	float     m_fuel		 = 1000.0f;
	float     m_fuelMax      = 1000.0f;
	
	Vector2D         m_currentPosition;
	Vector2D         m_startPosition;
	Vector2D         m_positionTo;
	Vector2D         m_lastPosition;
	WaypointManager* m_waypointManager;
	Pathfinding*     m_pathfinding;
	MovementManager* m_movementManager;
};