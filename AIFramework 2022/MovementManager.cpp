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

Vector2D MovementManager::Seek()
{
	Vector2D m_desiredVelocity = (m_car->getPositionTo() - m_car->getPosition());
	m_desiredVelocity.Normalize();
	m_desiredVelocity *= m_car->getMaxSpeed();

	return (m_desiredVelocity - m_car->getVelocity());
}

Vector2D MovementManager::Flee()
{
	Vector2D m_desiredVelocity = (m_car->getPosition() - m_car->getPositionTo());
	m_desiredVelocity.Normalize();
	m_desiredVelocity *= m_car->getMaxSpeed();

	return ((m_desiredVelocity - m_car->getVelocity()));
}

Vector2D MovementManager::Arrive()
{
	Vector2D m_targetOffset    = m_car->getPositionTo() - m_car->getPosition();
	float    m_distance        = m_targetOffset.Length();
	float    m_rampedSpeed     = m_car->getMaxSpeed() * (m_distance / m_decelerationDistance);
	float    m_clippedSpeed    = min(m_rampedSpeed, m_car->getMaxSpeed());
	Vector2D m_desiredVelocity = (m_clippedSpeed / m_distance) * m_targetOffset;
	
	return (m_desiredVelocity - m_car->getVelocity());
}

Vector2D MovementManager::Pursuit(Vehicle* otherCar)
{
	float m_angleBetween = ((m_car->getVelocity().Dot(otherCar->getPosition())) / m_car->getVelocity().Length()) / otherCar->getPosition().Length();
	float m_distance     = (otherCar->getPosition() - m_car->getPosition()).Length();
	if (m_angleBetween >= 0.8 && m_distance <= 150.0f)
	{
		m_car->setPositionTo(otherCar->getPosition());
		return Seek();
	}
	else
	{
		float m_lookAhead            = (otherCar->getPosition() - m_car->getPosition()).Length();
		m_lookAhead                  = m_lookAhead / (m_car->getCurrentSpeed() + otherCar->getCurrentSpeed());
		Vector2D m_predictedPosition = otherCar->getPosition() + (otherCar->getVelocity() * m_lookAhead);

		m_car->setPositionTo(m_predictedPosition);
		return Seek();
	}
}

Vector2D MovementManager::Evade(Vehicle* otherCar)
{
	float m_angleBetween = ((m_car->getVelocity().Dot(otherCar->getPosition())) / m_car->getVelocity().Length()) / otherCar->getPosition().Length();
	float m_distance = (otherCar->getPosition() - m_car->getPosition()).Length();
	if (m_angleBetween >= 0.8 && m_distance <= 150.0f)
	{
		m_car->setPositionTo(otherCar->getPosition());
		return Flee();
	}
	else
	{
		float m_lookAhead = (otherCar->getPosition() - m_car->getPosition()).Length();
		m_lookAhead = m_lookAhead / (m_car->getCurrentSpeed() + otherCar->getCurrentSpeed());
		Vector2D m_predictedPosition = otherCar->getPosition() + (otherCar->getVelocity() * m_lookAhead);

		m_car->setPositionTo(m_predictedPosition);
		return Flee();
	}
}