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

#include <string>
#include <vector>

struct js_event;

namespace RLJ
{

class Joystick
{
public:
	Joystick( const char* device );
	virtual ~Joystick();
	bool                    isValid() const;

	const std::string&      getDeviceName() const               { return mDeviceName; }
	int                     getDriverVersion() const            { return mDriverVersion; }
	const std::string&      getName() const                     { return mName; }


	std::size_t             getNumAxes() const                  { return mAxisValues.size(); }
	short int               getAxisValue( std::size_t axisIndex ) const;    // Returns the axis value in the range [-32767..32767]

	std::size_t             getNumButtons() const               { return mButtonValues.size(); }
	bool                    getButtonValue( std::size_t buttonIndex ) const;

	bool                    update();       // Returns false if the joystick couldn't be read (device wasn't opened, or closed abruptly, etc...)

	std::string             toString() const;

protected:
	friend class JoystickManager;
	static bool             getJoystickInfo( int handle, int& driverVersion, std::string& name, char& numAxes, char& numButtons );
	bool                    processEvents();
	void                    processEvent( const js_event& event );
	void                    setAxisValue( std::size_t axisIndex, short int value );
	void                    setButtonValue( std::size_t buttonIndex, bool value );

private:
	std::string             mDeviceName;
	int                     mJoystickHandle;
	int                     mDriverVersion;
	std::string             mName;
	std::vector<short int>  mAxisValues;
	std::vector<bool>       mButtonValues;
};

}

