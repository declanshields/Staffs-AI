#include "FiniteSM.h"
#include "constants.h"
#include "PickupItem.h"
#include "Vehicle.h"

FiniteSM::FiniteSM(Vehicle* car, Vehicle* car2, vecPickups& pickups)
{
	if (car != nullptr)
		m_car = car;
	if (car2 != nullptr)
		m_opposingCar = car2;

	m_items = pickups;
}

void FiniteSM::StateMachine()
{
	if (m_car != nullptr && m_opposingCar != nullptr)
	{
		state currentState = m_car->getState();

		switch (currentState)
		{
		case state::Arrive:
		{
			//If car is at its destination
			if ((m_car->getPosition() - m_car->getPositionTo()).Length() <= m_deadzone)
			{
				//Generate new destination
				int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
				int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
				Vector2D destination(x, y);

				m_car->setPositionTo(m_car->getWaypointManager()->getNearestWaypoint(destination)->getPosition());
			}
			//Arrive at the destination
			m_car->getMovementManager()->Arrive();
			break;
		}
		case state::Pursuit:
		{
			//Pursue the other car
			m_car->getMovementManager()->Pursuit(m_opposingCar);
			break;
		}
		case state::Seek:
		{
			//If car is at its destination
			if ((m_car->getPosition() - m_car->getPositionTo()).Length() <= m_deadzone)
			{
				//Generate new destination
				int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
				int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
				Vector2D destination(x, y);

				m_car->setPositionTo(m_car->getWaypointManager()->getNearestWaypoint(destination)->getPosition());
			}
			//Seek to destination
			m_car->getMovementManager()->Seek();
			break;
		}
		case state::MouseSeek:
		{
			//Works the same way as Seek, but does not generate a new destination
			if ((m_car->getPosition() - m_car->getPositionTo()).Length() <= m_deadzone)
			{
				//If already at destination, set state to pathfinding
				m_car->setState(state::Pathfinding);
			}
			else
				m_car->getMovementManager()->Seek();
			break;
		}
		case state::Evade:
		{
			//Evade the second car
			m_car->getMovementManager()->Evade(m_opposingCar);
			break;
		}
		case state::Flee:
		{
			//Calculate distance between the two cars
			Vector2D length;
			length.x = (m_opposingCar->getPosition().x - m_car->getPosition().x);
			length.y = (m_opposingCar->getPosition().y - m_car->getPosition().y);

			//If the distance is less than the acceptable distance (300)
			if (length.Length() <= m_acceptableDistance)
			{
				//Set the car to go in the opposite direction of the other car
				m_car->setPositionTo(m_car->getPosition() - m_opposingCar->getPosition());
				m_car->getMovementManager()->Flee();
			}
			else
			{
				//Else, set the car's state to Pathfinding
				m_car->setPositionTo(m_car->getPosition());
				m_car->setState(state::Pathfinding);
			}
			break;
		}
		case state::Wander:
		{
			//Make the car wander around
			m_car->getMovementManager()->Wander();
			break;
		}
		case state::ObstacleAvoid:
		{
			break;
		}
		case state::Idle:
		{
			//Make the car stay at its current position
			m_car->setPositionTo(m_car->getPosition());
			m_car->getMovementManager()->Seek();

			break;
		}
		case state::Pathfinding:
		{
			//Check to see if paths are empty, and pickups is not
			if ((m_car->m_fuelPath.path.empty() && m_car->m_passengerPath.path.empty()) && !m_items.empty())
			{
				Vector2D fuelPos, passengerPos;

				//Loop through pickups and get the position of the fuel and passenger
				for (int i = 0; i < m_items.size(); i++)
				{
					if (m_items[i]->getType() == pickuptype::Fuel)
					{
						fuelPos = m_items[i]->getPosition();
					}
					if (m_items[i]->getType() == pickuptype::Passenger)
					{
						passengerPos = m_items[i]->getPosition();
					}
				}

				//Create paths
				m_car->m_fuelPath = m_car->getPathfinderManager()->AStar(m_car->getPosition(), fuelPos);
				m_car->m_passengerPath = m_car->getPathfinderManager()->AStar(m_car->getPosition(), passengerPos);

				//Check to see if the car is at the next node
				if (m_car->getCurrentSpeed() == 0.0f || (m_car->getPosition() - m_car->getPositionTo()).Length() <= m_deadzone)
				{
					//Generate the paths again
					for (int i = 0; i < m_items.size(); i++)
					{
						if (m_items[i]->getType() == pickuptype::Fuel)
						{
							fuelPos = m_items[i]->getPosition();
						}
						if (m_items[i]->getType() == pickuptype::Passenger)
						{
							passengerPos = m_items[i]->getPosition();
						}
					}

					//Create paths
					m_car->m_fuelPath = m_car->getPathfinderManager()->AStar(m_car->getPosition(), fuelPos);
					m_car->m_passengerPath = m_car->getPathfinderManager()->AStar(m_car->getPosition(), passengerPos);

					//Check to see which pickup is closer
					//If fuel is closer
					if (m_car->m_fuelPath.distance < m_car->m_passengerPath.distance)
					{
						//Check to see if there is more than 1 node left
						if (m_car->m_fuelPath.path.size() > 0 && m_car->m_fuelPath.path.size() != 1)
						{
							//If more than one node left, seek to the next node
							Waypoint* nextPoint = m_car->m_fuelPath.path[m_car->m_fuelPath.path.size() - 1];
							m_car->setPositionTo(nextPoint->getPosition());
							m_car->getMovementManager()->Seek();
						}
						else
						{
							//If only one node left, arrive at last node
							Waypoint* finalPoint = m_car->m_fuelPath.path[m_car->m_fuelPath.path.size() - 1];
							m_car->setPositionTo(finalPoint->getPosition());
							m_car->getMovementManager()->Arrive();
						}

						//If car's position is the current node, get next node
						if ((m_car->getPosition() - m_car->m_fuelPath.path[m_car->m_fuelPath.path.size() - 1]->getPosition()).Length() <= m_deadzone)
						{
							m_car->m_fuelPath.path.pop_back();
						}
					}
					else
					{
						//Check to see if there is more than 1 node left
						if (m_car->m_passengerPath.path.size() > 0 && m_car->m_passengerPath.path.size() != 1)
						{
							//If more than one node left, seek to the next node
							Waypoint* nextPoint = m_car->m_passengerPath.path[m_car->m_passengerPath.path.size() - 1];
							m_car->setPositionTo(nextPoint->getPosition());
							m_car->getMovementManager()->Seek();
						}
						else
						{
							//If only one node left, arrive at last node
							Waypoint* finalPoint = m_car->m_passengerPath.path[m_car->m_passengerPath.path.size() - 1];
							m_car->setPositionTo(finalPoint->getPosition());
							m_car->getMovementManager()->Arrive();
						}

						//If car's position is the current node, get next node
						if ((m_car->getPosition() - m_car->m_passengerPath.path[m_car->m_passengerPath.path.size() - 1]->getPosition()).Length() <= m_deadzone)
						{
							m_car->m_passengerPath.path.pop_back();
						}
					}
				}
			}
			break;
		}
		default:
			break;
		}
	}
}