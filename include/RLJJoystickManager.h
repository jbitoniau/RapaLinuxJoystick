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
#pragma once

#include <vector>
#include <string>

namespace RLJ
{

class Joystick;
class JoystickEnumerationTrigger;

class JoystickManager
{
public:
	// Pass an array of device names (such as "/dev/input/js0", "/dev/input/js1", ...) that 
	// will be monitored on a regular interval. If a device appears a Joystick object 
	// will be automatically created. If it disappears it will be removed.
	// The more names to monitor, the more expensive the enumeration is.
	JoystickManager( const std::vector<std::string>& deviceNames );

	// Same as above except that the list is constructed from a root name suffixed with 
	// a value going from 0 to numDevices-1. 
	JoystickManager( const char* deviceNameRoot, unsigned int numDevices );

	virtual ~JoystickManager();

	const std::vector<Joystick*>& getJoysticks() const { return mJoysticks; }

	void        update();
	void        updateEnumeration();

	class Listener
	{
	public:
		virtual void onJoystickConnected( JoystickManager* joystickManager, Joystick* joystick ) {}
		virtual void onJoystickDisconnecting( JoystickManager* joystickManager, Joystick* joystick ) {}
	};

	void        addListener( Listener* listener );
	bool        removeListener( Listener* listener );

private:
	struct JoystickIdentifier
	{
		std::string mDeviceName;
		std::string mName;
		bool operator==( const JoystickIdentifier& other ) const;
		bool operator!=( const JoystickIdentifier& other ) const;
	};

	static bool getJoystickIdentifier( const char* deviceName, JoystickIdentifier& identifier );
	static int  getJoystickIdentifierIndex( const std::vector<JoystickIdentifier>& identifiers, const JoystickIdentifier& identifier );

	void        addJoystick( const JoystickIdentifier& identifier );

	void        removeJoystick(const JoystickIdentifier& identifier );

	static const unsigned int           mEnumerationIntervalInMs = 2000;
	JoystickEnumerationTrigger*         mEnumerationTrigger;
	std::vector<std::string>            mJoystickDeviceNames;
	std::vector<Joystick*>              mJoysticks;
	std::vector<JoystickIdentifier>     mJoystickIdentifiers;

	// Listeners
	typedef std::vector<Listener*>      Listeners;
	Listeners                           mListeners;
};

}