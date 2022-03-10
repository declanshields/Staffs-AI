#include "MovementManager.h"
#include "Vehicle.h"

MovementManager::MovementManager(Vehicle* thisCar)
{
	m_car = thisCar;
}

MovementManager::~MovementManager()
{
	if (m_car != nullptr)
	{
		delete m_car;
		m_car = nullptr;
	}
}

void MovementManager::Seek()
{
	Vector2D m_desiredVelocity = (m_car->getPositionTo() - m_car->getPosition());
	m_desiredVelocity.Normalize();
	m_desiredVelocity *= m_car->getMaxSpeed();

	Vector2D m_steeringForce = (m_desiredVelocity - m_car->getVelocity());

	m_car->setAcceleration(m_steeringForce);
}

void MovementManager::Flee()
{
	Vector2D m_desiredVelocity = (m_car->getPosition() - m_car->getPositionTo());
	m_desiredVelocity.Normalize();
	m_desiredVelocity *= m_car->getMaxSpeed();

	Vector2D m_steeringForce =  ((m_desiredVelocity - m_car->getVelocity()));

	m_car->setAcceleration(m_steeringForce);
}

void MovementManager::Arrive()
{
	Vector2D m_targetOffset    = m_car->getPositionTo() - m_car->getPosition();
	float    m_distance        = m_targetOffset.Length();
	float    m_rampedSpeed     = m_car->getMaxSpeed() * (m_distance / m_decelerationDistance);
	float    m_clippedSpeed    = min(m_rampedSpeed, m_car->getMaxSpeed());
	Vector2D m_desiredVelocity = (m_clippedSpeed / m_distance) * m_targetOffset;
	
	Vector2D m_steeringForce =  (m_desiredVelocity - m_car->getVelocity());

	m_car->setAcceleration(m_steeringForce);
}

void MovementManager::Pursuit(Vehicle* otherCar)
{
	float m_distance = (otherCar->getPosition() - m_car->getPosition()).Length();

	if (m_distance <= 150.0f)
	{
		m_car->setPositionTo(otherCar->getPosition());
		Seek();
	}
	else
	{
		float m_lookAhead = (otherCar->getPosition() - m_car->getPosition()).Length();

		float m_totalSpeed = (m_car->getVelocity().Length()) + (otherCar->getVelocity().Length());

		if (m_totalSpeed == 0.0f)
			m_lookAhead = 0.0f;
		else
			m_lookAhead = m_lookAhead / m_totalSpeed;

		Vector2D m_predictedPosition = otherCar->getPosition() + (otherCar->getVelocity() * m_lookAhead);

		m_car->setPositionTo(m_predictedPosition);
		Seek();
	}
}

void MovementManager::Evade(Vehicle* otherCar)
{
	float m_distance = (otherCar->getPosition() - m_car->getPosition()).Length();

	if (m_distance <= 250.0f)
	{
		float m_lookAhead = (otherCar->getPosition() - m_car->getPosition()).Length();

		float m_totalSpeed = (m_car->getVelocity().Length()) + (otherCar->getVelocity().Length());

		if (m_totalSpeed == 0.0f)
			m_lookAhead = 0.0f;
		else
			m_lookAhead = m_lookAhead / m_totalSpeed;

		Vector2D m_predictedPosition = otherCar->getPosition() + (otherCar->getVelocity() * m_lookAhead);

		m_car->setPositionTo(m_predictedPosition);
		Flee();
	}
	else
	{
		m_car->setPositionTo(m_car->getPosition());
		Seek();
	}
}

void MovementManager::Wander()
{
	Vector2D m_direction = m_car->getVelocity();
	m_direction.Normalize();
	Vector2D m_circleCenter = m_car->getPosition() + (m_direction * m_direction.Length());

	float m_wanderAngle = ((float(rand()) / float(RAND_MAX)) * (30.0f - -30.0f)) + (-30.0f);
	float x = cos(m_wanderAngle);
	float y = sin(m_wanderAngle);

	Vector2D m_offset = Vector2D(x, y);
	m_car->setPositionTo(m_circleCenter + m_offset);

	Seek();
}
