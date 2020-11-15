/*
* TrackPoint.h
*
* Authors: Cong Nguyen and Felix Kee.
*
* Expanding on Felix Kee's TrackPoint class: https://github.com/feklee/arduino-trackpoint
*
* Interface with a TrackPoint, supports stream mode using interrupt
* Parity checks slow down move movements thus are removed
*/


#ifndef __TRACKPOINT_H__
#define __TRACKPOINT_H__

#include "Arduino.h"

class TrackPoint
{
	public:
	struct DataReport {
		uint8_t state;
		int8_t x;
		int8_t y;
	};
  volatile uint8_t bitpos;
  volatile uint8_t highcount;
	void write(uint8_t data);
	uint8_t read(void);
	uint8_t readFromRamLocation(uint8_t);
	void writeToRamLocation(uint8_t, uint8_t);
	void setSensitivityFactor(uint8_t);
	uint8_t sensitivityFactor();
	void setRemoteMode();
	void reset();
  void resetStream();
	DataReport readData();
  DataReport readDataStream();
	
	TrackPoint(uint8_t, uint8_t, uint8_t, uint8_t);
	~TrackPoint();
		
	void getDataBit(void);
	uint8_t reportAvailable(void);
	void setStreamMode(void);
	DataReport getStreamReport(void);
	
	void gohi(uint8_t pin);
	void golo(uint8_t pin);
	
	protected:
	
	private:
	TrackPoint( const TrackPoint &c );
	TrackPoint& operator=( const TrackPoint &c );
	
	
	//TrackPoint pins
	uint8_t _clkPin;
	uint8_t _dataPin;
	uint8_t _resetPin;
	
	//PS2 data
	DataReport data;
	
	//getDataBit() variables
	volatile uint8_t bitcount;
  volatile uint8_t streamPos;
	volatile uint8_t n;
	volatile uint8_t val;
	volatile uint8_t incoming;
	volatile uint8_t counter;
	volatile uint8_t dataAvailable;
	uint8_t usingSeparateResetPin;

}; //TrackPoint

#endif //__TRACKPOINT_H__
