#include "Arduino.h"
#include "Sim800L_Lib\Sim800L.hpp"

//SoftwareSerial SIM(RX_PIN,TX_PIN);
//String _buffer;

Sim800L::Sim800L(void):Sim800L(DEFAULT_RX_PIN, DEFAULT_TX_PIN){}

Sim800L::Sim800L(uint8_t rx, uint8_t tx) : Sim800L(rx, tx, DEFAULT_RESET_PIN){}

Sim800L::Sim800L(uint8_t rx, uint8_t tx, uint8_t rst): Sim800L(rx, tx, rst, DEFAULT_LED_PIN){}

Sim800L::Sim800L(uint8_t rx, uint8_t tx, uint8_t rst, uint8_t led)
{
    RX_PIN 		= rx;
    TX_PIN 		= tx;
    RESET_PIN 	= rst;
    LED_PIN 	= led;
    LED_FLAG 	= true;
}

void Sim800L::begin()
{
    begin(DEFAULT_BAUD_RATE);
}

void Sim800L::begin(uint32_t baud)
{
    pinMode(RESET_PIN, OUTPUT);

    _baud = baud;
    SimSerial.begin(_baud, SERIAL_8N1, RX_PIN, TX_PIN);

    _sleepMode = 0;
    _functionalityMode = 1;

    if (LED_FLAG) pinMode(LED_PIN, OUTPUT);

    _buffer.reserve(BUFFER_RESERVE_MEMORY); // Reserve memory to prevent intern fragmention
}




/*
 * AT+CSCLK=0	Disable slow clock, module will not enter sleep mode.
 * AT+CSCLK=1	Enable slow clock, it is controlled by DTR. When DTR is high, module can enter sleep mode. When DTR changes to low level, module can quit sleep mode
 */
bool Sim800L::setSleepMode(bool state)
{

    _sleepMode = state;

    if (_sleepMode) print(F("AT+CSCLK=1\r\n "));
    else 			print(F("AT+CSCLK=0\r\n "));

    if ( (_readSerial().indexOf("ER")) == -1)
    {
        return false;
    }
    else return true;
    // Error found, return 1
    // Error NOT found, return 0
}

bool Sim800L::getSleepMode()
{
    return _sleepMode;
}

/*
 * AT+CFUN=0	Minimum functionality
 * AT+CFUN=1	Full functionality (defualt)
 * AT+CFUN=4	Flight mode (disable RF function)
*/
bool Sim800L::setFunctionalityMode(uint8_t fun)
{

    if (fun==0 || fun==1 || fun==4)
    {

        _functionalityMode = fun;

        switch(_functionalityMode)
        {
        case 0:
            print(F("AT+CFUN=0\r\n "));
            break;
        case 1:
            print(F("AT+CFUN=1\r\n "));
            break;
        case 4:
            print(F("AT+CFUN=4\r\n "));
            break;
        }

        if ( (_readSerial().indexOf("ER")) == -1)
        {
            return false;
        }
        else return true;
        // Error found, return 1
        // Error NOT found, return 0
    }
    return false;
}

uint8_t Sim800L::getFunctionalityMode()
{
    return _functionalityMode;
}

String Sim800L::ping()
{
    String command;
    command  = "AT";
    command += "\r";

    // Can take up to 5 seconds

    print(command);
    return (_readSerial(5000));
}

bool Sim800L::setPIN(String pin)
{
    String command;
    command  = "AT+CPIN=\"";
    command += pin;
    command += "\"\r";

    // Can take up to 5 seconds

    print(command);

    if ( (_readSerial(5000).indexOf("ER")) == -1)
    {
        return false;
    }
    else return true;
    // Error found, return 1
    // Error NOT found, return 0
}


String Sim800L::getProductInfo()
{
    print("ATI\r");
    return (_readSerial());
}


String Sim800L::getOperatorsList()
{

    // Can take up to 45 seconds

    print("AT+COPS=?\r");

    return _readSerial(45000);

}

String Sim800L::getOperator()
{

    print("AT+COPS ?\r");

    return _readSerial();

}


bool Sim800L::calculateLocation()
{
    /*
    	Type: 1  To get longitude and latitude
    	Cid = 1  Bearer profile identifier refer to AT+SAPBR
    */

    uint8_t type = 1;
    uint8_t cid = 1;
	
	String tmp = "AT+CIPGSMLOC=" + String(type) + "," + String(cid) + "\r\n";
	print(tmp);
	
	/*
    print("AT+CIPGSMLOC=");
    print(type);
    print(",");
    print(cid);
    print("\r");
	*/

    String data = _readSerial(20000);

    if (data.indexOf("ER")!=(-1)) return false;

    uint8_t indexOne;
    uint8_t indexTwo;

    indexOne = data.indexOf(":") + 1;
    indexTwo = data.indexOf(",");

    _locationCode = data.substring(indexOne, indexTwo);

    indexOne = data.indexOf(",") + 1;
    indexTwo = data.indexOf(",", indexOne);

    _longitude = data.substring(indexOne, indexTwo);

    indexOne = data.indexOf(",", indexTwo) + 1;
    indexTwo = data.indexOf(",", indexOne);

    _latitude = data.substring(indexOne, indexTwo);

    return true;

}

String Sim800L::getLocationCode()
{
    return _locationCode;
    /*
     Location Code:
     0      Success
     404    Not Found
     408    Request Time-out
     601    Network Error
     602    No Memory
     603    DNS Error
     604    Stack Busy
     65535  Other Error
    */
}

String Sim800L::getLongitude()
{
    return _longitude;
}

String Sim800L::getLatitude()
{
    return _latitude;
}


//
//PUBLIC METHODS
//

void Sim800L::reset()
{
    if (LED_FLAG) digitalWrite(LED_PIN,1);

    digitalWrite(RESET_PIN,1);
    delay(1000);
    digitalWrite(RESET_PIN,0);
    delay(1000);
    // wait for the module response

    print(F("AT\r\n"));
    while (_readSerial().indexOf("OK")==-1 )
    {
        print(F("AT\r\n"));
    }

    //wait for sms ready
    while (_readSerial().indexOf("SMS")==-1 );

    if (LED_FLAG) digitalWrite(LED_PIN,0);

}

void Sim800L::setPhoneFunctionality()
{
    /*AT+CFUN=<fun>[,<rst>]
    Parameters
    <fun> 0 Minimum functionality
    1 Full functionality (Default)
    4 Disable phone both transmit and receive RF circuits.
    <rst> 1 Reset the MT before setting it to <fun> power level.
    */
    print (F("AT+CFUN=1\r\n"));
}


String Sim800L::signalQuality()
{
    /*Response
    +CSQ: <rssi>,<ber>Parameters
    <rssi>
    0 -115 dBm or less
    1 -111 dBm
    2...30 -110... -54 dBm
    31 -52 dBm or greater
    99 not known or not detectable
    <ber> (in percent):
    0...7 As RXQUAL values in the table in GSM 05.08 [20]
    subclause 7.2.4
    99 Not known or not detectable
    */
    print (F("AT+CSQ\r\n"));
    return(_readSerial());
}


void Sim800L::activateBearerProfile()
{
    print (F(" AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\" \r\n" ));
    _buffer=_readSerial();  // set bearer parameter
    print (F(" AT+SAPBR=3,1,\"APN\",\"internet\" \r\n" ));
    _buffer=_readSerial();  // set apn
    print (F(" AT+SAPBR=1,1 \r\n"));
    delay(1200);
    _buffer=_readSerial();			// activate bearer context
    print (F(" AT+SAPBR=2,1\r\n "));
    delay(3000);
    _buffer=_readSerial(); 			// get context ip address
}


void Sim800L::deactivateBearerProfile()
{
    print (F("AT+SAPBR=0,1\r\n "));
    delay(1500);
}



bool Sim800L::answerCall()
{
    print (F("ATA\r\n"));
    //Response in case of data call, if successfully connected
    if ( (_readSerial().indexOf("ER")) == -1)
    {
        return false;
    }
    else return true;
    // Error found, return 1
    // Error NOT found, return 0
}


void  Sim800L::callNumber(char* number)
{
    print (F("ATD"));
    print (number);
    print (F(";\r\n"));
}



uint8_t Sim800L::getCallStatus()
{
    /*
      values of return:

     0 Ready (MT allows commands from TA/TE)
     2 Unknown (MT is not guaranteed to respond to tructions)
     3 Ringing (MT is ready for commands from TA/TE, but the ringer is active)
     4 Call in progress

    */
    print (F("AT+CPAS\r\n"));
    _buffer=_readSerial();
    return _buffer.substring(_buffer.indexOf("+CPAS: ")+7,_buffer.indexOf("+CPAS: ")+9).toInt();

}



bool Sim800L::hangoffCall()
{
    print (F("ATH\r\n"));
    _buffer=_readSerial();
    if ( (_buffer.indexOf("ER")) == -1)
    {
        return false;
    }
    else return true;
    // Error found, return 1
    // Error NOT found, return 0
}


bool Sim800L::sendSms(char* number,char* text)
{

    // Can take up to 60 seconds

    print (F("AT+CMGF=1\r")); 	//set sms to text mode
    _buffer=_readSerial();
    print (F("AT+CMGS=\""));  	// command to send sms
    print (number);
    print(F("\"\r"));
    _buffer=_readSerial();
    print (text);
    print ("\r");
    _buffer=_readSerial();
    print((String)(char)26);
    _buffer=_readSerial(60000);
    // Serial.println(_buffer);
    //expect CMGS:xxx   , where xxx is a number,for the sending CPIN.
    if ((_buffer.indexOf("ER")) != -1) {
        return true;
    } else if ((_buffer.indexOf("CMGS")) != -1) {
        return false;
  	} else {
    	return true;
  	}
    // Error found, return 1
    // Error NOT found, return 0
}


bool Sim800L::prepareForSmsReceive()
{
	// Configure SMS in text mode
	print(F("AT+CMGF=1\r"));
    _buffer=_readSerial();
    //Serial.print(_buffer);
    if((_buffer.indexOf("OK")) == -1)
    {
        return false;
    }
	print(F("AT+CNMI=2,1,0,0,0\r"));
    _buffer=_readSerial();
    //Serial.print(_buffer);
    if((_buffer.indexOf("OK")) == -1)
    {
        return false;
    }
    return true;
}

const uint8_t Sim800L::checkForSMS()
{
	 _buffer = _readSerial(100);
	 if(_buffer.length() == 0)
	 {
	 	return 0;
	 }
     _buffer += _readSerial(1000);
	 // Serial.println(_buffer);
	 // +CMTI: "SM",1
	 if(_buffer.indexOf("CMTI") == -1)
	 {
	 	return 0;
	 }
	 return _buffer.substring(_buffer.indexOf(',')+1).toInt();
}


String Sim800L::getNumberSms(uint8_t index)
{
    _buffer=readSms(index);
    //Serial.println(_buffer.length());
    if (_buffer.length() > 10) //avoid empty sms
    {
        uint8_t _idx1=_buffer.indexOf("+CMGR:");
        _idx1=_buffer.indexOf("\",\"",_idx1+1);
        return _buffer.substring(_idx1+3,_buffer.indexOf("\",\"",_idx1+4));
    }
    else
    {
        return "";
    }
}



String Sim800L::readSms(uint8_t index)
{
    // Can take up to 5 seconds

    if(( _readSerial(5000).indexOf("ER")) != -1)
    {
    	return "";
    }

    print (F("AT+CMGR="));
    print ((String)index);
    print ("\r");
    _buffer=_readSerial();
    //Serial.println(_buffer);
    if (_buffer.indexOf("CMGR") == -1)
    {
    	return "";
    }

	_buffer = _readSerial(10000);
	byte first = _buffer.indexOf('\n', 2) + 1;
	byte second = _buffer.indexOf('\n', first);
    return _buffer.substring(first, second);
}


bool Sim800L::delAllSms()
{
    // Can take up to 25 seconds

    print(F("at+cmgda=\"del all\"\n\r"));
    _buffer=_readSerial(25000);
    if ( (_buffer.indexOf("ER")) == -1)
    {
        return false;
    }
    else return true;
    // Error found, return 1
    // Error NOT found, return 0
}


void Sim800L::RTCtime(int *day,int *month, int *year,int *hour,int *minute, int *second)
{
    print(F("at+cclk?\r\n"));
    // if respond with ERROR try one more time.
    _buffer=_readSerial();
    if ((_buffer.indexOf("ERR"))!=-1)
    {
        delay(50);
        print(F("at+cclk?\r\n"));
    }
    if ((_buffer.indexOf("ERR"))==-1)
    {
        _buffer=_buffer.substring(_buffer.indexOf("\"")+1,_buffer.lastIndexOf("\"")-1);
        *year=_buffer.substring(0,2).toInt();
        *month= _buffer.substring(3,5).toInt();
        *day=_buffer.substring(6,8).toInt();
        *hour=_buffer.substring(9,11).toInt();
        *minute=_buffer.substring(12,14).toInt();
        *second=_buffer.substring(15,17).toInt();
    }
}

void Sim800L::checkStatus()
{
    print(F("AT+CPIN?\r\n"));
    _readSerial();
}

//Get the time  of the base of GSM
String Sim800L::dateNet()
{
    print(F("AT+CIPGSMLOC=2,1\r\n "));
    _buffer=_readSerial();

    if (_buffer.indexOf("OK")!=-1 )
    {
        return _buffer.substring(_buffer.indexOf(":")+2,(_buffer.indexOf("OK")-4));
    }
    else
        return "0";
}

// Update the RTC of the module with the date of GSM.
bool Sim800L::updateRtc(int utc)
{

    activateBearerProfile();
    _buffer=dateNet();
    deactivateBearerProfile();

    _buffer=_buffer.substring(_buffer.indexOf(",")+1,_buffer.length());
    String dt=_buffer.substring(0,_buffer.indexOf(","));
    String tm=_buffer.substring(_buffer.indexOf(",")+1,_buffer.length()) ;

    int hour = tm.substring(0,2).toInt();
    int day = dt.substring(8,10).toInt();

    hour=hour+utc;

    String tmp_hour;
    String tmp_day;
    //TODO : fix if the day is 0, this occur when day is 1 then decrement to 1,
    //       will need to check the last month what is the last day .
    if (hour<0)
    {
        hour+=24;
        day-=1;
    }
    if (hour<10)
    {

        tmp_hour="0"+String(hour);
    }
    else
    {
        tmp_hour=String(hour);
    }
    if (day<10)
    {
        tmp_day="0"+String(day);
    }
    else
    {
        tmp_day=String(day);
    }
    //for debugging
    //Serial.println("at+cclk=\""+dt.substring(2,4)+"/"+dt.substring(5,7)+"/"+tmp_day+","+tmp_hour+":"+tm.substring(3,5)+":"+tm.substring(6,8)+"-03\"\r\n");
    print("at+cclk=\""+dt.substring(2,4)+"/"+dt.substring(5,7)+"/"+tmp_day+","+tmp_hour+":"+tm.substring(3,5)+":"+tm.substring(6,8)+"-03\"\r\n");
    if ( (_readSerial().indexOf("ER"))!=-1)
    {
        return true;
    }
    else return false;


}

void Sim800L::print(String str)
{
    if (Debug)
    {
        Serial.println("<Sim800L Debug Send: {" + str + "}>S");
    }
    SimSerial.print(str);
    SimSerial.flush();
}

//
//PRIVATE METHODS
//
String Sim800L::_readSerial()
{
    return _readSerial(TIME_OUT_READ_SERIAL);
}

String Sim800L::_readSerial(uint32_t timeout)
{

    uint64_t timeOld = millis();

    while (!SimSerial.available() && !(millis() > timeOld + timeout))
    {
        delay(13);
    }

    String str;

    while(SimSerial.available())
    {
        if (SimSerial.available()>0)
        {
            str += (char) SimSerial.read();
        }
    }
    if (Debug)
    {
        Serial.println("<Sim800L Debug Recieved: {" + str+ "}>R");
    }

    SimSerial.flush();
    return str;

}
