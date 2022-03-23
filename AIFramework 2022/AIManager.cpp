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

    return hr;
}


void AIManager::update(const float fDeltaTime)
{
    for (unsigned int i = 0; i < m_waypointManager.getWaypointCount(); i++) {
        m_waypointManager.getWaypoint(i)->update(fDeltaTime);
       // AddItemToDrawList(m_waypointManager.getWaypoint(i)); // if you uncomment this, it will display the waypoints
    }

    for (int i = 0; i < m_waypointManager.getQuadpointCount(); i++)
    {
        Waypoint* qp = m_waypointManager.getQuadpoint(i);
        qp->update(fDeltaTime);
        //AddItemToDrawList(qp); // if you uncomment this, it will display the quad waypoints
    }

    // update and display the pickups
    for (unsigned int i = 0; i < m_pickups.size(); i++) {
        m_pickups[i]->update(fDeltaTime);
        AddItemToDrawList(m_pickups[i]);
    }

	// draw the waypoints nearest to the car
	/*
    Waypoint* wp = m_waypointManager.getNearestWaypoint(m_pCar->getPosition());
	if (wp != nullptr)
	{
		vecWaypoints vwps = m_waypointManager.getNeighbouringWaypoints(wp);
		for (Waypoint* wp : vwps)
		{
			AddItemToDrawList(wp);
		}
	}
    */

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

    if (!m_path.empty())
    {
        for (Waypoint* wps : m_path)
        {
            AddItemToDrawList(wps);
        }
    }

    //update cars based off current state
    handleStates();
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

void AIManager::handleStates()
{
    //handle car 1 states
    if (m_pCar != nullptr)
    {
        switch (m_pCar->getState())
        {
        case state::Arrive:
        {
            if ((m_pCar->getPosition() - m_pCar->getPositionTo()).Length() <= m_deadZone)
            {
                int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
                int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
                Vector2D position(x, y);

                m_pCar->setPositionTo(m_waypointManager.getNearestWaypoint(position)->getPosition());
            }
            m_pCar->getMovementManager()->Arrive();
            break;
        }
        case state::Pursuit:
        {
            m_pCar->getMovementManager()->Pursuit(m_pCar2);
            break;
        }
        case state::Seek:
        {
            if ((m_pCar->getPosition() - m_pCar->getPositionTo()).Length() <= m_deadZone)
            {
                int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
                int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
                Vector2D position(x, y);

                m_pCar->setPositionTo(m_waypointManager.getNearestWaypoint(position)->getPosition());
            }

        	m_pCar->getMovementManager()->Seek();

            break;
        }
        case state::MouseSeek:
	    {
            if ((m_pCar->getPosition() - m_pCar->getPositionTo()).Length() <= m_deadZone)
            {
                m_pCar->setState(state::Idle);
            }
            else
        		m_pCar->getMovementManager()->Seek();
            break;
	    }
        case state::Evade:
        {
            m_pCar->getMovementManager()->Evade(m_pCar2);
            break;
        }
        case state::Flee:
        {
            Vector2D length;
            length.x = (m_pCar2->getPosition().x - m_pCar->getPosition().x);
            length.y = (m_pCar2->getPosition().y - m_pCar->getPosition().y);

            if (length.Length() <= 300)
            {
                m_pCar->setPositionTo(m_pCar->getPosition() - m_pCar2->getPosition());
                m_pCar->getMovementManager()->Flee();
            }
            else
            {
                m_pCar->setPositionTo(m_pCar->getPosition());
                m_pCar->setState(state::Idle);
            }

            break;
        }
        case state::Wander:
        {
            m_pCar->getMovementManager()->Wander();
            break;
        }
        case state::ObstacleAvoid:
        {
            break;
        }
        case state::Idle:
        {
            m_pCar->setPositionTo(m_pCar->getPosition());
            m_pCar->getMovementManager()->Seek();

            break;
        }
        default:
            break;
        }
    }

    //handle car 2 states
    if (m_pCar2 != nullptr)
    {
        switch (m_pCar2->getState())
        {
        case state::Arrive:
        {
            m_path.clear();

            if ((m_pCar2->getPosition() - m_pCar2->getPositionTo()).Length() <= m_deadZone)
            {
                int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
                int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
                Vector2D position(x, y);

                m_pCar2->setPositionTo(m_waypointManager.getNearestWaypoint(position)->getPosition());
                m_pCar2->getMovementManager()->Arrive();
            }
            break;
        }
        case state::Pursuit:
        {
            m_path.clear();

            m_pCar2->getMovementManager()->Pursuit(m_pCar);
            break;
        }
        case state::Seek:
        {
            m_path.clear();

            if ((m_pCar2->getPosition() - m_pCar2->getPositionTo()).Length() <= m_deadZone)
            {
                int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
                int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);
                Vector2D position(x, y);

                m_pCar2->setPositionTo(m_waypointManager.getNearestWaypoint(position)->getPosition());
                m_pCar2->getMovementManager()->Seek();
            }
            break;
        }
        case state::Evade:
        {
            m_path.clear();

            m_pCar2->getMovementManager()->Evade(m_pCar);
            break;
        }
        case state::Flee:
        {
            m_path.clear();

            Vector2D length;
            length.x = (m_pCar->getPosition().x - m_pCar2->getPosition().x);
            length.y = (m_pCar->getPosition().y - m_pCar2->getPosition().y);

            if (length.Length() <= 300)
            {
                m_pCar2->setPositionTo(m_pCar2->getPosition() - m_pCar->getPosition());
                m_pCar2->getMovementManager()->Flee();
            }
            else
            {
                m_pCar2->setPositionTo(m_pCar2->getPosition());
                m_pCar2->setState(state::Idle);
            }

            break;
        }
        case state::Wander:
        {
            m_pCar2->getMovementManager()->Wander();
            break;
        }
        case state::ObstacleAvoid:
        {
            break;
        }
        case state::Idle:
        {
            m_pCar2->setPositionTo(m_pCar2->getPosition());
            m_pCar2->getMovementManager()->Seek();

            break;
        }
        case state::Pathfinding:
        {
            if (m_path.empty() && !m_pickups.empty())
            {
                //defaults the end position to the cars current position if there is no passengers to pick up
                Vector2D endPos = m_pCar2->getPosition();

                //loops through vector to find any passengers
                for (int i = 0; i <= m_pickups.size(); i++)
                {
                    //if the object is a passenger, set its position to the end point
                    if (m_pickups[i]->getType() == pickuptype::Passenger)
                    {
                        endPos = m_pickups[i]->getPosition();
                        break;
                    }
                }

                m_path = m_pCar2->getPathfinderManager()->AStar(m_pCar2->getPosition(), endPos);
            }

            if (m_pCar2->getCurrentSpeed() == 0.0f || (m_pCar2->getPosition() - m_pCar2->getPositionTo()).Length() <= m_deadZone)
            {
                if (m_path.size() > 0 && m_path.size() != 1)
                {
                    Waypoint* nextPoint = m_path[m_path.size() - 1];
                    m_pCar2->setPositionTo(nextPoint->getPosition());
                    m_pCar2->getMovementManager()->Seek();
                }
                if (m_path.size() == 1)
                {
                    Waypoint* finalPoint = m_path[m_path.size() - 1];
                    m_pCar2->setPositionTo(finalPoint->getPosition());
                    m_pCar2->getMovementManager()->Arrive();
                }
            }
            if ((m_pCar2->getPosition() - m_path[m_path.size() - 1]->getPosition()).Length() <= m_deadZone)
                m_path.pop_back();
            break;
        }
        case state::CollectFuel:
        {

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
    XMVECTOR puPos;
    XMVECTOR puScale;
    XMMatrixDecompose(
        &puScale,
        &dummy,
        &puPos,
        XMLoadFloat4x4(m_pickups[0]->getTransform())
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
        m_pickups[0]->hasCollided();
        setRandomPickupPosition(m_pickups[0]);

        // you will need to test the type of the pickup to decide on the behaviour
        // m_pCar->dosomething(); ...

        return true;
    }

    return false;
}





