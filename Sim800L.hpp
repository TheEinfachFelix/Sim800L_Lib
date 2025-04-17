#ifndef Sim800L_h
#define Sim800L_h
#include "Arduino.h"


#define DEFAULT_RX_PIN 		10
#define DEFAULT_TX_PIN 		11
#define DEFAULT_RESET_PIN 	2		// pin to the reset pin Sim800L

#define DEFAULT_LED_FLAG	true 	// true: use led.	 false: don't user led.
#define DEFAULT_LED_PIN 	13 		// pin to indicate states.

#define BUFFER_RESERVE_MEMORY	255
#define DEFAULT_BAUD_RATE		9600
#define TIME_OUT_READ_SERIAL	5000



class Sim800L
{
private:
    HardwareSerial SimSerial = Serial2;
    void print(String str);


    uint32_t _baud;
    int _timeout;
    String _buffer;
    bool _sleepMode;
    uint8_t _functionalityMode;
    String _locationCode;
    String _longitude;
    String _latitude;

    String _readSerial();
    String _readSerial(uint32_t timeout);

public:

    uint8_t	RX_PIN;
    uint8_t TX_PIN;
    uint8_t RESET_PIN;
    uint8_t LED_PIN;
    bool	LED_FLAG;
    bool Debug;
    void checkStatus();

    Sim800L(void);
    Sim800L(uint8_t rx, uint8_t tx);
    Sim800L(uint8_t rx, uint8_t tx, uint8_t rst);
    Sim800L(uint8_t rx, uint8_t tx, uint8_t rst, uint8_t led);

    void begin();					//Default baud 9600
    void begin(uint32_t baud);
    void reset();

    String ping();
    bool setSleepMode(bool state);
    bool getSleepMode();
    bool setFunctionalityMode(uint8_t fun);
    uint8_t getFunctionalityMode();

    bool setPIN(String pin);
    String getProductInfo();

    String getOperatorsList();
    String getOperator();

    bool calculateLocation();
    String getLocationCode();
    String getLongitude();
    String getLatitude();

    bool answerCall();
    void callNumber(char* number);
    bool hangoffCall();
    uint8_t getCallStatus();

	const uint8_t checkForSMS();
	bool prepareForSmsReceive();
    bool sendSms(char* number,char* text);
    String readSms(uint8_t index);
    String getNumberSms(uint8_t index);
    bool delAllSms();


    String signalQuality();
    void setPhoneFunctionality();
    void activateBearerProfile();
    void deactivateBearerProfile();
    bool setMode();

    void RTCtime(int *day,int *month, int *year,int *hour,int *minute, int *second);
    String dateNet();
    bool updateRtc(int utc);

};

#endif