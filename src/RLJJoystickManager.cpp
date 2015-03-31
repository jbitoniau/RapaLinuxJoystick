/*
   The MIT License (MIT) (http://opensource.org/licenses/MIT)
   
   Copyright (c) 2015 Jacques Menuet
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include "RLJJoystickManager.h"

#include "RLJJoystick.h"
#include <fcntl.h>
#include <assert.h>
#include <algorithm>
#include <sstream>

#include "RLJJoystickEnumerationTrigger.h"

/*
	Notes:
	- It should be possible to optimize the enumeration() method by only looking for newly added joysticks
	  since last enumeration. Removed joysticks could be handled by checking the returned value of the 
	  joystick update() method and removing it if it's false
	- A better enumeration system is needed. Something not as crude as monitoring files in /dev (registering
	  to some system events/notifications?)	  
*/
namespace RLJ
{

/*
	JoystickManager::JoystickIdentifier
*/
bool JoystickManager::JoystickIdentifier::operator==( const JoystickIdentifier& other ) const
{
	return	mDeviceName==other.mDeviceName && 
			mName==other.mName;
}

bool JoystickManager::JoystickIdentifier::operator!=( const JoystickIdentifier& other ) const
{
	return !(*this==other);
}

/*
	JoystickManager
*/
JoystickManager::JoystickManager( const std::vector<std::string>& deviceNames ) 
	:	mEnumerationTrigger(NULL),
		mJoystickDeviceNames(deviceNames),
		mJoysticks(),
		mJoystickIdentifiers(),
		mListeners()
{
	mEnumerationTrigger = new TimeBasedEnumerationTrigger( mEnumerationIntervalInMs );
	//for ( std::size_t i=0; i<mJoystickDeviceNames.size(); ++i )
	//	printf("%s\n", mJoystickDeviceNames[i].c_str());
}

JoystickManager::JoystickManager( const char* deviceNameRoot, unsigned int numDevices ) 
	:	mEnumerationTrigger(NULL),
		mJoystickDeviceNames(),
		mJoysticks(),
		mJoystickIdentifiers(),
		mListeners()
{	
	mEnumerationTrigger = new TimeBasedEnumerationTrigger( mEnumerationIntervalInMs );
	for ( unsigned int i=0; i<numDevices; ++i )
	{
		std::stringstream stream;
		stream << deviceNameRoot << i;
		mJoystickDeviceNames.push_back( stream.str() );
	}

	//for ( std::size_t i=0; i<mJoystickDeviceNames.size(); ++i )
	//	printf("%s\n", mJoystickDeviceNames[i].c_str());
}

JoystickManager::~JoystickManager()
{
	delete mEnumerationTrigger;
	mEnumerationTrigger = NULL;
}

void JoystickManager::update()
{
	if ( mEnumerationTrigger->enumerationNeeded() )
		updateEnumeration();

	for ( std::size_t i=0; i<mJoysticks.size(); ++i )
		mJoysticks[i]->update();
}

bool JoystickManager::getJoystickIdentifier( const char* deviceName, JoystickIdentifier& identifier )
{
	int handle = open( deviceName, O_RDONLY|O_NONBLOCK );
	if ( handle>=0 )
	{
		int driverVersion = 0;
		std::string name;
		char numAxes = 0; 
		char numButtons = 0;
		if ( Joystick::getJoystickInfo( handle, driverVersion, name, numAxes, numButtons ) )
		{
			identifier.mDeviceName = deviceName;
			identifier.mName = name;
			return true;
		}
	}
	return false;
}

void JoystickManager::updateEnumeration()
{
	// Get current list of joysticks
	std::vector<JoystickIdentifier> identifiers;
	for ( std::size_t i=0; i<mJoystickDeviceNames.size(); ++i )
	{
		JoystickIdentifier identifier;
		if ( getJoystickIdentifier( mJoystickDeviceNames[i].c_str(), identifier ) )
			identifiers.push_back( identifier );
	}

	// Identify joysticks that have been added since last enumeration
	std::vector<JoystickIdentifier> joysticksToAdd;
	for ( std::size_t i=0; i<identifiers.size(); ++i )
	{
		bool deviceAdded = false;
		for ( std::size_t j=0; j<mJoystickIdentifiers.size(); ++j )
		{
			if ( identifiers[i]==mJoystickIdentifiers[j] )
			{
				deviceAdded = true;
				break;
			}
		}
		if ( !deviceAdded )
			joysticksToAdd.push_back( identifiers[i] );
	}
	
	// Identify joysticks that have been removed since last enumeration
	std::vector<JoystickIdentifier> joysticksToRemove;
	for ( std::size_t i=0; i<mJoystickIdentifiers.size(); ++i )
	{
		bool deviceRemoved = true;
		for ( std::size_t j=0; j<identifiers.size(); ++j )
		{
			if ( identifiers[j]==mJoystickIdentifiers[i] )
			{
				deviceRemoved = false;
				break;
			}
		}
		if ( deviceRemoved )
			joysticksToRemove.push_back( mJoystickIdentifiers[i] );
	}

/*	printf("\nDetected: \n");
	for ( std::size_t i=0; i<identifiers.size(); ++i )
		printf("%s\n", identifiers[i].mDeviceName.c_str());
	printf("To add: \n");
	for ( std::size_t i=0; i<joysticksToAdd.size(); ++i )
		printf("%s\n", joysticksToAdd[i].mDeviceName.c_str());
	printf("To remove: \n");
	for ( std::size_t i=0; i<joysticksToRemove.size(); ++i )
		printf("%s\n", joysticksToRemove[i].mDeviceName.c_str());
	printf("\n");
*/

	for ( std::size_t i=0; i<joysticksToAdd.size(); ++i )
		addJoystick( joysticksToAdd[i] );
	for ( std::size_t i=0; i<joysticksToRemove.size(); ++i )
		removeJoystick( joysticksToRemove[i] );
}

void JoystickManager::addJoystick( const JoystickIdentifier& identifier )
{
	mJoystickIdentifiers.push_back( identifier );
	Joystick* joystick = new Joystick( identifier.mDeviceName.c_str() );
	mJoysticks.push_back( joystick );
	//printf("added %s\n", identifier.mDeviceName.c_str());

	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onJoystickConnected( this, joystick );
}

int JoystickManager::getJoystickIdentifierIndex( const std::vector<JoystickIdentifier>& identifiers, const JoystickIdentifier& identifier )
{
	for ( std::size_t i=0; i<identifiers.size(); ++i )
	{
		if ( identifier==identifiers[i] )
			return i;
	}
	return -1;	
}

void JoystickManager::removeJoystick( const JoystickIdentifier& identifier )
{
	std::size_t i = getJoystickIdentifierIndex(mJoystickIdentifiers, identifier);
	assert( i!=-1 );
	
	Joystick* joystick = mJoysticks[i];
	
	// Notify
	for ( Listeners::iterator itr=mListeners.begin(); itr!=mListeners.end(); ++itr )
		(*itr)->onJoystickDisconnecting( this, joystick );

	mJoystickIdentifiers.erase( mJoystickIdentifiers.begin() + i );
	mJoysticks.erase( mJoysticks.begin() + i );
	delete joystick;
	joystick = NULL;
	//printf("removed %s\n", identifier.mDeviceName.c_str());
}

void JoystickManager::addListener( Listener* listener )
{
	assert(listener);
	mListeners.push_back(listener);
}

bool JoystickManager::removeListener( Listener* listener )
{
	Listeners::iterator itr = std::find( mListeners.begin(), mListeners.end(), listener );
	if ( itr==mListeners.end() )
		return false;
	mListeners.erase( itr );
	return true;
}

}