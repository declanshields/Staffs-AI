#include "AIManager.h"
#include "Vehicle.h"
#include "DrawableGameObject.h"
#include "PickupItem.h"
#include "Waypoint.h"
#include "main.h"
#include "constants.h"

AIManager::AIManager()
{
	m_pCar  = nullptr;
    m_pCar2 = nullptr;
}

AIManager::~AIManager()
{
	release();
}

void AIManager::release()
{
	clearDrawList();

	for (PickupItem* pu : m_pickups)
	{
		delete pu;
	}
	m_pickups.clear();

	delete m_pCar;
	m_pCar = nullptr;

    delete m_pCar2;
    m_pCar2 = nullptr;
}

HRESULT AIManager::initialise(ID3D11Device* pd3dDevice)
{
    // create the vehicle 
    float xPos = -500; // an abtrirary start point
    float yPos = 300;

    //second vehicle pos
    float xPos2 = 500;
    float yPos2 = -300;

    m_pCar = new Vehicle();
    HRESULT hr = m_pCar->initMesh(pd3dDevice, carColour::blueCar);
    m_pCar->setVehiclePosition(Vector2D(xPos, yPos));
    if (FAILED(hr))
        return hr;

    m_pCar2 = new Vehicle();
    hr = m_pCar2->initMesh(pd3dDevice, carColour::redCar);
    if (FAILED(hr))
        return hr;

    // setup the waypoints
    m_waypointManager.createWaypoints(pd3dDevice);
    m_pCar->setWaypointManager(&m_waypointManager);
    m_pCar2->setWaypointManager(&m_waypointManager);

    // create a passenger pickup item
    PickupItem* pPickupPassenger = new PickupItem();
    hr = pPickupPassenger->initMesh(pd3dDevice, pickuptype::Passenger);
    m_pickups.push_back(pPickupPassenger);

    // NOTE!! for fuel and speedboost - you will need to create these here yourself
    PickupItem* pFuel = new PickupItem();
    hr = pFuel->initMesh(pd3dDevice, pickuptype::Fuel);
    m_pickups.push_back(pFuel);

    // (needs to be done after waypoint setup)
    setRandomPickupPosition(pPickupPassenger);
    setRandomPickupPosition(pFuel);

    m_pCar->setState(state::Idle);
    m_pCar2->setState(state::Idle);

    return hr;
}


void AIManager::update(const float fDeltaTime)
{
    // update and display the pickups
    for (unsigned int i = 0; i < m_pickups.size(); i++) {
        m_pickups[i]->update(fDeltaTime);
        AddItemToDrawList(m_pickups[i]);
    }

    // update and draw the car (and check for pickup collisions)
	if (m_pCar != nullptr)
	{
        if (m_pCar->getFuelLeft() == 0.0f)
            m_pCar->setState(state::Idle);

		m_pCar->update(fDeltaTime);
        checkForCollisions(m_pCar);
		AddItemToDrawList(m_pCar);
	}
    if (m_pCar2 != nullptr)
    {
        if (m_pCar2->getFuelLeft() == 0.0f)
            m_pCar2->setState(state::Idle);

        m_pCar2->update(fDeltaTime);
        checkForCollisions(m_pCar2);
        AddItemToDrawList(m_pCar2);
    }

    //Display the path the cars are currently taking
    if(!m_pCar->m_fuelPath.path.empty() && !m_pCar->m_passengerPath.path.empty())
    {
        if (m_pCar->m_fuelPath.distance <= m_pCar->m_passengerPath.distance)
        {
            for (int i = 0; i < m_pCar->m_fuelPath.path.size(); i++)
            {
                if (m_pCar->m_fuelPath.path[i] != nullptr)
                {
                    m_pCar->m_fuelPath.path[i]->update(fDeltaTime);
                    AddItemToDrawList(m_pCar->m_fuelPath.path[i]);
                }
            }
        }
        else
        {
            for (int i = 0; i < m_pCar->m_passengerPath.path.size(); i++)
            {
                if (m_pCar->m_passengerPath.path[i] != nullptr)
                {
                    m_pCar->m_passengerPath.path[i]->update(fDeltaTime);
                    AddItemToDrawList(m_pCar->m_passengerPath.path[i]);
                }
            }
        }
    }
    if (!m_pCar2->m_fuelPath.path.empty() && !m_pCar2->m_passengerPath.path.empty())
    {
	    if (m_pCar2->m_fuelPath.distance <= m_pCar2->m_passengerPath.distance)
	    {
		    for (int i = 0; i < m_pCar2->m_fuelPath.path.size(); i++)
		    {
			    if(m_pCar2->m_fuelPath.path[i] != nullptr)
			    {
                    m_pCar2->m_fuelPath.path[i]->update(fDeltaTime);
                    AddItemToDrawList(m_pCar2->m_fuelPath.path[i]);
			    }
		    }
	    }
        else
        {
	        for (int i = 0; i < m_pCar2->m_passengerPath.path.size(); i++)
	        {
		        if(m_pCar2->m_passengerPath.path[i] != nullptr)
		        {
                    m_pCar2->m_passengerPath.path[i]->update(fDeltaTime);
                    AddItemToDrawList(m_pCar2->m_passengerPath.path[i]);
		        }
	        }
        }
    }


    StateMachine();
}

void AIManager::mouseUp(int x, int y)
{
	// get a waypoint near the mouse click, then set the car to move to the this waypoint
	Waypoint* wp = m_waypointManager.getNearestWaypoint(Vector2D(x, y));
	if (wp == nullptr)
		return;

    // steering mode
    m_pCar->setPositionTo(wp->getPosition());
    m_pCar->setState(state::MouseSeek);
}

void AIManager::keyUp(WPARAM param)
{

}

void AIManager::keyDown(WPARAM param)
{
	// hint 65-90 are a-z
    const WPARAM key_space = 32;
	const WPARAM key_a     = 65;
    const WPARAM key_c     = 67;
    const WPARAM key_e     = 69;
    const WPARAM key_f     = 70;
    const WPARAM key_p     = 80;
	const WPARAM key_s     = 83;
    const WPARAM key_w     = 87;

    switch (param)
    {
        //a for arrive
        case key_a:
        {
            m_pCar->setState(state::Arrive);
            m_pCar->setPositionTo(m_pCar->getPosition());

            break;
        }
        //c for center
        case key_c: 
        {
            m_pCar->setState(state::Idle);

            m_pCar->setPositionTo(Vector2D(0, 0));
            break;
        }
        //e for evade
        case key_e:
        {
            m_pCar->setState(state::Evade);
            break;
        }
        //f for flee
        case key_f: 
        {
            m_pCar->setState(state::Flee);

            break;
        }
        //s for seek
		case key_s:
		{
            m_pCar->setState(state::Seek);

			break;
		}
        //w for wander
        case key_w:
        {
            m_pCar2->setState(state::Wander);

            break;
        }
        case key_p:
        {
            m_pCar->setState(state::Pursuit);

            break;
        }
        //space for pathfinding
        case key_space:
        {
            m_pCar2->setState(state::Pathfinding);

            break;
        }
        // etc
        default:
            break;
    }
}

void AIManager::StateMachine()
{
    if (m_pCar != nullptr && m_pCar2 != nullptr)
    {
        state currentState = m_pCar->getState();

        switch (currentState)
        {
        case state::Arrive:
        {
            //If car is at its destination
            if ((m_pCar->getPosition() - m_pCar->getPositionTo()).Length() <= m_deadZone)
            {
                //Generate new destination
                int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
                int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
                Vector2D destination(x, y);

                m_pCar->setPositionTo(m_pCar->getWaypointManager()->getNearestWaypoint(destination)->getPosition());
            }
            //Arrive at the destination
            m_pCar->getMovementManager()->Arrive();
            break;
        }
        case state::Pursuit:
        {
            //Pursue the other car
            m_pCar->getMovementManager()->Pursuit(m_pCar2);
            break;
        }
        case state::Seek:
        {
            //If car is at its destination
            if ((m_pCar->getPosition() - m_pCar->getPositionTo()).Length() <= m_deadZone)
            {
                //Generate new destination
                int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
                int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
                Vector2D destination(x, y);

                m_pCar->setPositionTo(m_pCar->getWaypointManager()->getNearestWaypoint(destination)->getPosition());
            }
            //Seek to destination
            m_pCar->getMovementManager()->Seek();
            break;
        }
        case state::MouseSeek:
        {
            //Works the same way as Seek, but does not generate a new destination
            if ((m_pCar->getPosition() - m_pCar->getPositionTo()).Length() <= m_deadZone)
            {
                //If already at destination, set state to pathfinding
                m_pCar->setState(state::Pathfinding);
            }
            else
                m_pCar->getMovementManager()->Seek();
            break;
        }
        case state::Evade:
        {
            //Evade the second car
            m_pCar->getMovementManager()->Evade(m_pCar2);
            break;
        }
        case state::Flee:
        {
            //Calculate distance between the two cars
            Vector2D length;
            length.x = (m_pCar2->getPosition().x - m_pCar->getPosition().x);
            length.y = (m_pCar2->getPosition().y - m_pCar->getPosition().y);

            //If the distance is less than the acceptable distance (300)
            if (length.Length() <= m_deadZone)
            {
                //Set the car to go in the opposite direction of the other car
                m_pCar->setPositionTo(m_pCar->getPosition() - m_pCar2->getPosition());
                m_pCar->getMovementManager()->Flee();
            }
            else
            {
                //Else, set the car's state to Idle
                m_pCar->setPositionTo(m_pCar->getPosition());
                m_pCar->setState(state::Idle);
            }
            break;
        }
        case state::Wander:
        {
            //Make the car wander around
            m_pCar->getMovementManager()->Wander();
            break;
        }
        case state::ObstacleAvoid:
        {
            break;
        }
        case state::Idle:
        {
            //Make the car stay at its current position
            m_pCar->setPositionTo(m_pCar->getPosition());
            m_pCar->getMovementManager()->Seek();

            break;
        }
        case state::Pathfinding:
        {
            Vector2D fuelPos, passengerPos;
            //Check to see if paths are empty, and pickups is not
            if ((m_pCar->m_fuelPath.path.empty() && m_pCar->m_passengerPath.path.empty()) && !m_pickups.empty())
            {

                //Loop through pickups and get the position of the fuel and passenger
                for (int i = 0; i < m_pickups.size(); i++)
                {
                    if (m_pickups[i]->getType() == pickuptype::Fuel)
                    {
                        fuelPos = m_pickups[i]->getPosition();
                    }
                    if (m_pickups[i]->getType() == pickuptype::Passenger)
                    {
                        passengerPos = m_pickups[i]->getPosition();
                    }
                }

                //Create paths
                m_pCar->m_fuelPath = m_pCar->getPathfinderManager()->AStar(m_pCar->getPosition(), fuelPos);
                m_pCar->m_passengerPath = m_pCar->getPathfinderManager()->AStar(m_pCar->getPosition(), passengerPos);
            }
            if (!m_pCar->m_fuelPath.path.empty() && !m_pCar->m_passengerPath.path.empty())
            {
                //Check to see which pickup is closer
                //If fuel is closer
                if (m_pCar->m_fuelPath.distance < m_pCar->m_passengerPath.distance)
                {
                    //Check to see if there is more than 1 node left
                    if (m_pCar->m_fuelPath.path.size() > 0 && m_pCar->m_fuelPath.path.size() != 1)
                    {
                        //If more than one node left, seek to the next node
                        Waypoint* nextPoint = m_pCar->m_fuelPath.path[m_pCar->m_fuelPath.path.size() - 1];
                        m_pCar->setPositionTo(nextPoint->getPosition());
                        m_pCar->getMovementManager()->Seek();
                    }
                    else
                    {
                        //If only one node left, arrive at last node
                        Waypoint* finalPoint = m_pCar->m_fuelPath.path[m_pCar->m_fuelPath.path.size() - 1];
                        m_pCar->setPositionTo(finalPoint->getPosition());
                        m_pCar->getMovementManager()->Arrive();
                    }

                    //If car's position is the current node, get next node
                    if ((m_pCar->getPosition() - m_pCar->m_fuelPath.path[m_pCar->m_fuelPath.path.size() - 1]->getPosition()).Length() <= m_deadZone)
                    {
                        m_pCar->m_fuelPath.path.pop_back();
                    }
                }
                else
                {
                    //Check to see if there is more than 1 node left
                    if (m_pCar->m_passengerPath.path.size() > 0 && m_pCar->m_passengerPath.path.size() != 1)
                    {
                        //If more than one node left, seek to the next node
                        Waypoint* nextPoint = m_pCar->m_passengerPath.path[m_pCar->m_passengerPath.path.size() - 1];
                        m_pCar->setPositionTo(nextPoint->getPosition());
                        m_pCar->getMovementManager()->Seek();
                    }
                    else
                    {
                        //If only one node left, arrive at last node
                        Waypoint* finalPoint = m_pCar->m_passengerPath.path[m_pCar->m_passengerPath.path.size() - 1];
                        m_pCar->setPositionTo(finalPoint->getPosition());
                        m_pCar->getMovementManager()->Arrive();
                    }

                    //If car's position is the current node, get next node
                    if ((m_pCar->getPosition() - m_pCar->m_passengerPath.path[m_pCar->m_passengerPath.path.size() - 1]->getPosition()).Length() <= m_deadZone)
                    {
                        m_pCar->m_passengerPath.path.pop_back();
                    }
                }
            }
            break;
        }
        default:
            break;
        }

        currentState = m_pCar2->getState();

        switch (currentState)
        {
        case state::Arrive:
        {
            //If car is at its destination
            if ((m_pCar2->getPosition() - m_pCar2->getPositionTo()).Length() <= m_deadZone)
            {
                //Generate new destination
                int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
                int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
                Vector2D destination(x, y);

                m_pCar2->setPositionTo(m_pCar->getWaypointManager()->getNearestWaypoint(destination)->getPosition());
            }
            //Arrive at the destination
            m_pCar2->getMovementManager()->Arrive();
            break;
        }
        case state::Pursuit:
        {
            //Pursue the other car
            m_pCar2->getMovementManager()->Pursuit(m_pCar);
            break;
        }
        case state::Seek:
        {
            //If car is at its destination
            if ((m_pCar2->getPosition() - m_pCar2->getPositionTo()).Length() <= m_deadZone)
            {
                //Generate new destination
                int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
                int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
                Vector2D destination(x, y);

                m_pCar2->setPositionTo(m_pCar->getWaypointManager()->getNearestWaypoint(destination)->getPosition());
            }
            //Seek to destination
            m_pCar2->getMovementManager()->Seek();
            break;
        }
        case state::MouseSeek:
        {
            //Works the same way as Seek, but does not generate a new destination
            if ((m_pCar2->getPosition() - m_pCar2->getPositionTo()).Length() <= m_deadZone)
            {
                //If already at destination, set state to pathfinding
                m_pCar2->setState(state::Pathfinding);
            }
            else
                m_pCar2->getMovementManager()->Seek();
            break;
        }
        case state::Evade:
        {
            //Evade the second car
            m_pCar2->getMovementManager()->Evade(m_pCar);
            break;
        }
        case state::Flee:
        {
            //Calculate distance between the two cars
            Vector2D length;
            length.x = (m_pCar->getPosition().x - m_pCar2->getPosition().x);
            length.y = (m_pCar->getPosition().y - m_pCar2->getPosition().y);

            //If the distance is less than the acceptable distance (300)
            if (length.Length() <= m_deadZone)
            {
                //Set the car to go in the opposite direction of the other car
                m_pCar2->setPositionTo(m_pCar->getPosition() - m_pCar->getPosition());
                m_pCar2->getMovementManager()->Flee();
            }
            else
            {
                //Else, set the car's state to Pathfinding
                m_pCar2->setPositionTo(m_pCar->getPosition());
                m_pCar2->setState(state::Pathfinding);
            }
            break;
        }
        case state::Wander:
        {
            //Make the car wander around
            m_pCar2->getMovementManager()->Wander();
            break;
        }
        case state::ObstacleAvoid:
        {
            break;
        }
        case state::Idle:
        {
            //Make the car stay at its current position
            m_pCar2->setPositionTo(m_pCar2->getPosition());
            m_pCar2->getMovementManager()->Seek();

            break;
        }
        case state::Pathfinding:
        {
            Vector2D fuelPos, passengerPos;
            //Check to see if paths are empty, and pickups is not
            if ((m_pCar2->m_fuelPath.path.empty() || m_pCar2->m_passengerPath.path.empty()) && !m_pickups.empty())
            {
                //Loop through pickups and get the position of the fuel and passenger
                for (int i = 0; i < m_pickups.size(); i++)
                {
                    if (m_pickups[i]->getType() == pickuptype::Fuel)
                    {
                        fuelPos = m_pickups[i]->getPosition();
                    }
                    if (m_pickups[i]->getType() == pickuptype::Passenger)
                    {
                        passengerPos = m_pickups[i]->getPosition();
                    }
                }
               
                //Create paths
                m_pCar2->m_fuelPath = m_pCar2->getPathfinderManager()->AStar(m_pCar2->getPosition(), fuelPos);
                m_pCar2->m_passengerPath = m_pCar2->getPathfinderManager()->AStar(m_pCar2->getPosition(), passengerPos);
            }
            if (!m_pCar2->m_fuelPath.path.empty() && !m_pCar2->m_passengerPath.path.empty())
            {
                //Check to see which pickup is closer
                //If fuel is closer
                if (m_pCar2->m_fuelPath.distance < m_pCar2->m_passengerPath.distance)
                {
                    //Check to see if there is more than 1 node left
                    if (m_pCar2->m_fuelPath.path.size() != 1)
                    {
                        //If more than one node left, seek to the next node
                        Waypoint* nextPoint = m_pCar2->m_fuelPath.path[m_pCar2->m_fuelPath.path.size() - 1];
                        m_pCar2->setPositionTo(nextPoint->getPosition());
                        m_pCar2->getMovementManager()->Seek();
                    }
                    else
                    {
                        //If only one node left, arrive at last node
                        Waypoint* finalPoint = m_pCar2->m_fuelPath.path[0];
                        m_pCar2->setPositionTo(finalPoint->getPosition());
                        m_pCar2->getMovementManager()->Arrive();
                    }

                    //If car's position is the current node, get next node
                    if ((m_pCar2->getPosition() - m_pCar2->m_fuelPath.path[m_pCar2->m_fuelPath.path.size() - 1]->getPosition()).Length() <= m_deadZone)
                    {
                        m_pCar2->m_fuelPath.path.pop_back();
                    }
                }
                else
                {
                    //Check to see if there is more than 1 node left
                    if (m_pCar2->m_passengerPath.path.size() > 0 && m_pCar2->m_passengerPath.path.size() != 1)
                    {
                        //If more than one node left, seek to the next node
                        Waypoint* nextPoint = m_pCar2->m_passengerPath.path[m_pCar2->m_passengerPath.path.size() - 1];
                        m_pCar2->setPositionTo(nextPoint->getPosition());
                        m_pCar2->getMovementManager()->Seek();
                    }
                    else
                    {
                        //If only one node left, arrive at last node
                        Waypoint* finalPoint = m_pCar2->m_passengerPath.path[m_pCar2->m_passengerPath.path.size() - 1];
                        m_pCar2->setPositionTo(finalPoint->getPosition());
                        m_pCar2->getMovementManager()->Arrive();
                    }

                    //If car's position is the current node, get next node
                    if ((m_pCar2->getPosition() - m_pCar2->m_passengerPath.path[m_pCar2->m_passengerPath.path.size() - 1]->getPosition()).Length() <= m_deadZone)
                    {
                        m_pCar2->m_passengerPath.path.pop_back();
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


void AIManager::setRandomPickupPosition(PickupItem* pickup)
{
    if (pickup == nullptr)
        return;

    int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
    int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);

    Waypoint* wp = m_waypointManager.getNearestWaypoint(Vector2D(x, y));
    if (wp) {
        pickup->setPosition(wp->getPosition());
    }
}

/*
// IMPORTANT
// hello. This is hopefully the only time you are exposed to directx code 
// you shouldn't need to edit this, but marked in the code below is a section where you may need to add code to handle pickup collisions (speed boost / fuel)
// the code below does the following:
// gets the *first* pickup item "m_pickups[0]"
// does a collision test with it and the car
// creates a new random pickup position for that pickup

// the relevant #includes are already in place, but if you create your own collision class (or use this code anywhere else) 
// make sure you have the following:
#include <d3d11_1.h> // this has the appropriate directx structures / objects
#include <DirectXCollision.h> // this is the dx collision class helper
using namespace DirectX; // this means you don't need to put DirectX:: in front of objects like XMVECTOR and so on. 
*/

bool AIManager::checkForCollisions(Vehicle* car)
{
    if (m_pickups.size() == 0)
        return false;

    XMVECTOR dummy;

    // get the position and scale of the car and store in dx friendly xmvectors
    XMVECTOR carPos;
    XMVECTOR carScale;
    XMMatrixDecompose(
        &carScale,
        &dummy,
        &carPos,
        XMLoadFloat4x4(car->getTransform())
    );

    // create a bounding sphere for the car
    XMFLOAT3 scale;
    XMStoreFloat3(&scale, carScale);
    BoundingSphere boundingSphereCar;
    XMStoreFloat3(&boundingSphereCar.Center, carPos);
    boundingSphereCar.Radius = scale.x;

    // do the same for a pickup item
    // a pickup - !! NOTE it is only referring the first one in the list !!
    // to get the passenger, fuel or speedboost specifically you will need to iterate the pickups and test their type (getType()) - see the pickup class

    for (int i = 0; i < m_pickups.size(); i++)
    {
        XMVECTOR puPos;
        XMVECTOR puScale;
        XMMatrixDecompose(
            &puScale,
            &dummy,
            &puPos,
            XMLoadFloat4x4(m_pickups[i]->getTransform())
        );

        // bounding sphere for pickup item
        XMStoreFloat3(&scale, puScale);
        BoundingSphere boundingSpherePU;
        XMStoreFloat3(&boundingSpherePU.Center, puPos);
        boundingSpherePU.Radius = scale.x;

        // THIS IS generally where you enter code to test each type of pickup
        // does the car bounding sphere collide with the pickup bounding sphere?
        if (boundingSphereCar.Intersects(boundingSpherePU))
        {
            OutputDebugStringA("Pickup Collision\n");
            m_pickups[i]->hasCollided();
            setRandomPickupPosition(m_pickups[i]);

            // you will need to test the type of the pickup to decide on the behaviour
            // m_pCar->dosomething(); ...

            if (m_pickups[i]->getType() == pickuptype::Fuel)
                car->setFuel();

            return true;
        }
    }
    return false;
}





