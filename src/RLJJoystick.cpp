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
#include "RLJJoystick.h"

#include <assert.h>
#include <cstring>   // for memset
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <sstream>
#include <errno.h>
#include <stdio.h>

namespace RLJ
{

Joystick::Joystick( const char* device )
	: mDeviceName(device),
	  mJoystickHandle(-1),
	  mDriverVersion(0),
	  mName(),
	  mAxisValues(),
	  mButtonValues()
{
	int handle = open( device, O_RDONLY|O_NONBLOCK );
	if ( handle>=0 )
	{
		int driverVersion = 0;
		std::string name;
		char numAxes = 0; 
		char numButtons = 0;
		if ( getJoystickInfo( handle, driverVersion, name, numAxes, numButtons ) )
		{
			mJoystickHandle = handle;
			mDriverVersion = driverVersion;
			mName = name;
			mAxisValues.resize(numAxes, 0);
			mButtonValues.resize(numButtons, false);
		}
		else
		{
			printf("Can't get information for joystick %s\n", device);
			close( handle );
		}
	}
	else
	{
		printf("Can't open joystick %s\n", device);
		return;
	}
}

Joystick::~Joystick()
{
	if ( isValid() )
		close( mJoystickHandle );
}

bool Joystick::getJoystickInfo( int handle, int& driverVersion, std::string& name, char& numAxes, char& numButtons )
{
	driverVersion = 0;
	name = "";
	numAxes = 0;
	numButtons = 0;

	if ( ioctl( handle, JSIOCGAXES, &driverVersion )<0 )
		return false;
	
	char nameBuf[256];
	memset( nameBuf, 0, sizeof(nameBuf) );
	if ( ioctl( handle, JSIOCGNAME(sizeof(nameBuf)), nameBuf )<0 )
		return false;
	name = nameBuf;

	if ( ioctl( handle, JSIOCGAXES, &numAxes )<0 )
		return false;
	
	if ( ioctl( handle, JSIOCGBUTTONS, &numButtons )<0 )
		return false;
	
	return true;
}

bool Joystick::isValid() const
{
	return mJoystickHandle!=-1;
}

short int Joystick::getAxisValue( std::size_t axisIndex ) const
{
	if ( !isValid() )
		return 0;
	if ( axisIndex>=mAxisValues.size() )
		return 0;
	return mAxisValues[axisIndex];
}

bool Joystick::getButtonValue( std::size_t buttonIndex ) const
{
	if ( !isValid() )
		return 0;
	if ( buttonIndex>=mButtonValues.size() )
		return 0;
	return mButtonValues[buttonIndex];
}

bool Joystick::update()
{
	if ( !isValid() )
		return false;
	return processEvents();
}

bool Joystick::processEvents()
{
	js_event event;
	bool error = false;
	bool finished = false;
	do
	{
		int bytesRead = read( mJoystickHandle, &event, sizeof(js_event) );
		if ( bytesRead!=-1 )
		{
			assert( bytesRead==sizeof(js_event) );				
			processEvent( event );
		}
		else
		{
			finished = true;
			if ( errno!=EAGAIN )
				error = true;
		}
	}
	while ( !finished );
	
	if ( error )
		return false;
	return true;
}

void Joystick::processEvent( const js_event& event )
{
	if ( event.type & JS_EVENT_BUTTON )
		setButtonValue( event.number, (event.value!=0) );
	else if ( event.type & JS_EVENT_AXIS )
		setAxisValue( event.number, event.value );
}

void Joystick::setAxisValue( std::size_t axisIndex, short int value )
{
	assert(isValid());
	if (axisIndex>=getNumAxes())
		return;
	mAxisValues[axisIndex]=value;
}

void Joystick::setButtonValue( std::size_t buttonIndex, bool value )
{
	assert(isValid());
	if (buttonIndex>=getNumButtons())
		return;
	mButtonValues[buttonIndex]=value;
}

std::string Joystick::toString() const
{
	std::stringstream stream;
	stream << "Joystick:\n";
	stream << "driverVersion:" << getDriverVersion() << "\n";
	stream << "name:" << getName() << "\n";
	for ( std::size_t i=0; i<getNumAxes(); ++i )
		stream << "axis #:" << i << " value:" << getAxisValue(i) << "\n";
	for ( std::size_t i=0; i<getNumButtons(); ++i )
		stream << "button #:" << i << " value:" << getButtonValue(i) << "\n";
	return stream.str();
}

}
