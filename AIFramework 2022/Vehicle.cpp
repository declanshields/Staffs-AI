#include "Vehicle.h"

#define HALF_MAX_SPEED 100
#define NORMAL_MAX_SPEED 200
#define DOUBLE_MAX_SPEED 400

HRESULT	Vehicle::initMesh(ID3D11Device* pd3dDevice, carColour colour)
{
	m_scale = XMFLOAT3(30, 20, 1);

	if (colour == carColour::redCar)
	{
		setTextureName(L"Resources\\car_red.dds");
	}
	else if (colour == carColour::blueCar)
	{
		setTextureName(L"Resources\\car_blue.dds");
	}

	HRESULT hr = DrawableGameObject::initMesh(pd3dDevice);

	m_maxSpeed = NORMAL_MAX_SPEED;
	m_currentSpeed = 0;
	setVehiclePosition(Vector2D(0, 0));

	m_lastPosition = Vector2D(0, 0);

	return hr;
}

void Vehicle::update(const float deltaTime)
{
	//// consider replacing with force based acceleration / velocity calculations
	//Vector2D vecTo = m_positionTo - m_currentPosition;
	//float velocity = 0;

	//float length = (float)vecTo.Length();
	//// if the distance to the end point is less than the car would move, then only move that distance. 
	//if (length > 0) {
	//	vecTo.Normalize();
	//	velocity = m_currentSpeed + (deltaTime * 2.5f);

	//	if (velocity >= m_maxSpeed)
	//		velocity = m_maxSpeed;

	//	if(length > velocity)
	//		vecTo *= velocity;
	//	else
	//		vecTo *= length;

	//	m_currentPosition += vecTo;
	//}

	//// rotate the object based on its last & current position
	//Vector2D diff = m_currentPosition - m_lastPosition;
	//if (diff.Length() > 0) { // if zero then don't update rotation
	//	diff.Normalize();
	//	m_radianRotation = atan2f((float)diff.y, (float)diff.x); // this is used by DrawableGameObject to set the rotation
	//}
	//m_lastPosition = m_currentPosition;
	//m_currentSpeed = velocity;

	//force based movement
	Vector2D acceleration = m_totalForce / m_mass;
	m_velocity += acceleration * deltaTime;
	m_currentPosition += m_velocity * deltaTime;

	switch(m_currentState)
	{
	case state::Seek:
		{
		Vector2D vecTo = m_positionTo - m_currentPosition;
		vecTo.Normalize();
		vecTo *= m_maxSpeed;

		m_totalForce += (vecTo - m_velocity);
		break;
		}
	case state::Flee:
		{
		Vector2D vecTo = m_currentPosition - m_positionTo;
		vecTo.Normalize();
		if (vecTo.Length() >= 250.0f)
		{
			vecTo *= m_maxSpeed;

			m_totalForce += (vecTo - m_velocity);
		}
		break;
		}
	case state::Arrive:
		{
		Vector2D vecTo = m_positionTo - m_currentPosition;
		vecTo.Normalize();
		float speed = vecTo.Length() * m_deceleration;

		vecTo = (vecTo * speed) / vecTo.Length();
		m_totalForce += (vecTo - m_velocity);
		break;
		}
	case state::Pursuit:
		{
		break;
		}
	}

	// set the current poistion for the drawablegameobject
	setPosition(Vector2D(m_currentPosition));

	DrawableGameObject::update(deltaTime);
}


// a ratio: a value between 0 and 1 (1 being max speed)
void Vehicle::setCurrentSpeed(const float speed)
{
	m_currentSpeed = m_maxSpeed * speed;
	m_currentSpeed = max(0, m_currentSpeed);
	m_currentSpeed = min(m_maxSpeed, m_currentSpeed);
}

// set a position to move to
void Vehicle::setPositionTo(Vector2D position)
{
	m_startPosition = m_currentPosition;
	
	m_positionTo = position;
}

// set the current position
void Vehicle::setVehiclePosition(Vector2D position)
{
	m_currentPosition = position;
	m_positionTo = position;
	m_startPosition = position;
	setPosition(position);
}

void Vehicle::setWaypointManager(WaypointManager* wpm)
{
	m_waypointManager = wpm;
	m_pathfinding = new Pathfinding(wpm);
}


