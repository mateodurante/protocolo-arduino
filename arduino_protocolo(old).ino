// =============================================================================
// GIANA Framework | Home Automation Made Easy. (LAMP || WAMP) + Arduino UNO r3.
// =============================================================================
// Copyright (C) 2013 Federico Pfaffendorf (www.federicopfaffendorf.com.ar)
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version. 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/gpl.txt
// =============================================================================
// Please, compile using Arduino 1.0.5 or greater.
// =============================================================================
// MESSAGES 
// =============================================================================
// REQUEST
//   <G(T)(PP)>             Get the value of a specific pin
// RESPONSE
//   <D(PP)(V)>             The value of a specific digital pin
//   <A(PP)(VVVV)>          The value of a specific analogue pin
// REQUEST
//   <SD(PP)(V)>            Set the value of a specific digital pin
// RESPONSE
//   <SD(PP)(V)(OK|ER)>     The result of setting a digital pin
// -----------------------------------------------------------------------------
// (T) = Pin Type: A for Analogue, D for Digital
// (PP) = Pin Number: 
//          If digital: 02, 03, 04, ... 13
//          If analogue: 00, 01, 02, 03, 04, 05
// (V) = Digital Value: 0 or 1
// (VVVV) = Analogue Value: 0000, 0001, 0002, ... 1023
// (OK|ER) = OK for OK, ER for Error
// =============================================================================
// EXAMPLES
// =============================================================================
// REQUEST 
//   <GD07>             Get the digital value of pin 7
// RESPONSE
//   <D071>             The value of digital pin 7 is 1
// REQUEST 
//   <GA00>             Get the analogue value of pin 0
// RESPONSE
//   <A000527>          The value of analogue pin 0 is 527
// REQUEST 
//   <SD131>            Set the digital value of pin 13 to 1
// RESPONSE
//   <SD131OK>          Succeed setting the value of digital pin 13 to 1
// =============================================================================
// CONSTANTS
// =============================================================================
const int SERIAL_BAUD = 9600;
const int REQUEST_MAX_LENGTH = 8;
const int RESPONSE_MAX_LENGTH = 14;
const int D_PINS_IN_LENGTH = 3;
const int A_PINS_IN_LENGTH = 6;
const int D_PINS_OUT_LENGTH = 3;
const int A_PINS_OUT_LENGTH = 6;
const int D_PINS_IN[] = {2,4,7};
const int A_PINS_IN[] = {23,24,25,26,27,28};
const int D_PINS_OUT[] = {8,12,13};
const int A_PINS_OUT[] = {3,5,6,9,10,11};
//const int PIN_NUMBER[] = {/*0*/-1,/*1*/-1,/*2*/0,/*3*/0,/*4*/1,/*5*/1,/*6*/2,
///*7*/2,/*8*/0,/*9*/3,/*10*/4,/*11*/5,}
const char *SERIAL_ID = "XXX";

/*** Protocol data position definition ***/
/*** Report State Message (Letter: S) ***/
const int STATE_LENGTH = 60;
/** long 60 **/
const int POS_S_FIRST_CONTROL_CHAR = 0;
const int POS_S_SERIAL_NUMBER = 1;
const int POS_S_SEQ_NUMBER = 4;
const int POS_S_MESSAGE_TYPE = 6; // Report
const int POS_S_OPERATION_TYPE = 7; // Free Byte [Ok | Error]
const int POS_S_D_PIN_IN[] = {8,9,10};
const int POS_S_A_PIN_IN[] = {11,15,19,23,27,31};
const int POS_S_D_PIN_OUT[] = {32,33,34};
const int POS_S_A_PIN_OUT[] = {35,39,43,47,51,55};
const int POS_S_LAST_CONTROL_CHAR = 59;

/*** Response Order Message (Letter: R) ***/
const int RESPONSE_LENGTH = 13;
/** long 13 **/
const int POS_R_FIRST_CONTROL_CHAR = 0;
const int POS_R_SERIAL_NUMBER = 1;
const int POS_R_SEQ_NUMBER = 4;
const int POS_R_MESSAGE_TYPE = 6; // Get - Set - Multiple
const int POS_R_OPERATION_TYPE = 7; // Digital - Analog - #MultipleSeq
const int POS_R_DATA = 8;
const int POS_R_LAST_CONTROL_CHAR = 12;

/*** Request Order Message (Letter: O) ***/
const int REQUEST_LENGTH = 13;
/** Specification **/
const int POS_O_FIRST_CONTROL_CHAR = 0;
const int POS_O_SERIAL_NUMBER = 1;
const int POS_O_SEQ_NUMBER = 4;
const int POS_O_MESSAGE_TYPE = 6; // Get - Set - Multiple
const int POS_O_OPERATION_TYPE = 7; // Digital - Analog - #MultipleSeq
const int POS_O_DATA = 8;
const int POS_O_LAST_CONTROL_CHAR = 12;

//const char *TEST = "<XXX00R010101102310231023102310231023102310231023102310231023>";
// =============================================================================
// TYPEDEFS
// =============================================================================
typedef char tState[STATE_LENGTH];
typedef char tResponse[RESPONSE_LENGTH];
typedef char tRequest[REQUEST_LENGTH];

/******************************************************************************/
// SECTION Class Header
/******************************************************************************/

// =============================================================================
// CLASS HEADER Pins
// =============================================================================
class Pins
{
  private:
    static int dpo_values[];
    static int apo_values[];
    static int dpi_values[];
    static int api_values[];
  public:
    static void setup();     
    static int read(char pinType, byte pin);
    static void write(char pinType, byte pin, int value);
    static int readDigitalIn(byte pin);
    static int readAnalogIn(byte pin);
    static int readDigitalOut(byte pin);
    static int readAnalogOut(byte pin);
};
// =============================================================================
// CLASS HEADER State Message
// =============================================================================
class State
{
  private:
    tState state_;
  public:
    State();
    void getState(tState &state);
    void setControlData();
    char getDigitalChar(int value);
    char getAnalogChar0(int value);
    char getAnalogChar1(int value);
    char getAnalogChar2(int value);
    char getAnalogChar3(int value);
};
// =============================================================================
// CLASS HEADER Response
// =============================================================================
class Response
{
  private:
    tResponse response_;
  public:
    Response (char action, char pinType, byte pin, int value, int seqNumber);
    Response (char action, char pinType, byte pin, int value, boolean result, 
                int seqNumber);
    void getResponse (tResponse &response);
    void setFinalControlData(int seqNumber, int index);
};
// =============================================================================
// CLASS HEADER Request
// =============================================================================
class Request
{
  private:
    tRequest request_;
  public:
    Request(tRequest request);
    void process();
};
// =============================================================================
// CLASS HEADER SerialCommunication
// =============================================================================
int _seqNumber;
class SerialCommunication
{
  private:
    static boolean available();
    static char read();
  public:
    static int getNextSeqNumber();
    static int getSeqNumber();
    static void checkSeqNumber();
    static void setup();
    static void processRequests();
    static void sendResponse(Response *response);
    static void sendState(State *state);
};

/******************************************************************************/
// SECTION Class Definition
/******************************************************************************/

// =============================================================================
// CLASS DEFINITION Pins
// =============================================================================
int Pins::dpi_values[]={0,0,0};
int Pins::api_values[]={0,0,0,0,0,0};
int Pins::dpo_values[]={0,0,0};
int Pins::apo_values[]={0,0,0,0,0,0};

void Pins::setup()
{
  //Digital Pin In
  for(int pin = 0; pin <= 2; pin++)
  {
    pinMode(D_PINS_IN[pin], INPUT); 
//    digitalWrite(A_PINS_OUT[pin], apo_values[pin]);
  }
  //Analog Pin In
/*  for(int pin = 0; pin <= 5; pin++)
  {
//    pinMode(pin, OUTPUT); 
    digitalWrite(pin, LOW);
  }
  */
  //Digital Pin Out
  for(int pin = 0; pin <= 2; pin++)
  {
    pinMode(D_PINS_OUT[pin], OUTPUT);   
    digitalWrite(D_PINS_OUT[pin], dpo_values[pin]);
  }
  //Analog Pin Out
  for(int pin = 0; pin <= 5; pin++)
  {
    pinMode(A_PINS_OUT[pin], OUTPUT);
    analogWrite(A_PINS_OUT[pin], apo_values[pin]);
  }

}

int Pins::read(char pinType, byte pin)
{
  switch (pinType)  
  {
    case 'D':
        return digitalRead(pin);        
      break;
    case 'A':
        return analogRead(pin);        
      break;
  }
}

int Pins::readDigitalIn(byte pin)
{
  dpi_values[pin] = digitalRead(D_PINS_IN[pin]);
  return dpi_values[pin];
}

int Pins::readAnalogIn(byte pin)
{
  api_values[pin] = analogRead(A_PINS_IN[pin]);
  return api_values[pin];
}

int Pins::readDigitalOut(byte pin)
{
  return dpo_values[pin];
}

int Pins::readAnalogOut(byte pin)
{
  return apo_values[pin];
}

void Pins::write(char pinType, byte pin, int value)
{
  switch (pinType)  
  {
    case 'D':
        dpo_values[pin]=value;
        digitalWrite(D_PINS_OUT[pin], value);
      break;
    case 'A':
        analogWrite(pin, value);
      break;
  }
}
// =============================================================================
// CLASS DEFINITION State
// =============================================================================
State::State ()
{
  State::setControlData();
  state_[POS_S_MESSAGE_TYPE] = 'S';
  state_[POS_S_OPERATION_TYPE] = 'O';


  for(int i = 0; i < D_PINS_IN_LENGTH; i++)
  {
    state_[POS_S_D_PIN_IN[i]] = State::getDigitalChar(Pins::readDigitalIn(i));
  }
  //Analog Pin Out
  for(int i = 0; i < A_PINS_IN_LENGTH; i++)
  {
    int value = Pins::readAnalogIn(i);
    state_[POS_S_A_PIN_IN[i]] = State::getAnalogChar0(value);
    state_[POS_S_A_PIN_IN[i]+1] = State::getAnalogChar1(value);
    state_[POS_S_A_PIN_IN[i]+2] = State::getAnalogChar2(value);
    state_[POS_S_A_PIN_IN[i]+3] = State::getAnalogChar3(value);
  }
  //Digital Pin In
  for(int i = 0; i < D_PINS_OUT_LENGTH; i++)
  {
    state_[POS_S_D_PIN_OUT[i]] = State::getDigitalChar(Pins::readDigitalOut(i));
  }
  //Analog Pin In
  for(int i = 0; i < A_PINS_OUT_LENGTH; i++)
  {
    int value = Pins::readAnalogOut(i);
    state_[POS_S_A_PIN_OUT[i]] = State::getAnalogChar0(value);
    state_[POS_S_A_PIN_OUT[i]+1] = State::getAnalogChar1(value);
    state_[POS_S_A_PIN_OUT[i]+2] = State::getAnalogChar2(value);
    state_[POS_S_A_PIN_OUT[i]+3] = State::getAnalogChar3(value);
  }

}

char State::getDigitalChar(int value){
  if(value)
    return '1';
  return '0';
}

char State::getAnalogChar0(int value){
  if(value>=1000)
    return '1';
  return '0';
}

char State::getAnalogChar1(int value){
  return value / 100 % 10 + 48;
}

char State::getAnalogChar2(int value){
  return value / 10 % 10 + 48;
}

char State::getAnalogChar3(int value){
  return value % 10 + 48;
}

void State::setControlData()
{
  int seqNumber = SerialCommunication::getSeqNumber();

  state_[POS_S_FIRST_CONTROL_CHAR] = '<';

  state_[POS_S_SERIAL_NUMBER] = SERIAL_ID[0];
  state_[POS_S_SERIAL_NUMBER+1] = SERIAL_ID[1];
  state_[POS_S_SERIAL_NUMBER+2] = SERIAL_ID[2];

  state_[POS_S_SEQ_NUMBER] = '0';
  if ((seqNumber < 100) && (seqNumber >= 10))
  {
    byte b = seqNumber / 10;
    state_[POS_S_SEQ_NUMBER] = b + 48;
    seqNumber -= (b * 10);
  }
  state_[POS_S_SEQ_NUMBER+1] = seqNumber + 48;

  state_[POS_S_LAST_CONTROL_CHAR] = '>';
  
}

void State::getState(tState &state)
{
  for (int i = 0; i < STATE_LENGTH; i++)
    state[i] = state_[i];  
}
// =============================================================================
// CLASS DEFINITION Response
// =============================================================================
Response::Response (char action, char pinType, byte pin, int value, 
                      int seqNumber)
{
  response_[0] = '<';
  response_[1] = pinType;  
  if (pin < 10) 
  {
    response_[2] = '0';
  }
  else 
  {
    response_[2] = '1';
    pin -= 10;    
  }
  response_[3] = pin + 48;
  if (pinType == 'D')
  {
    response_[4] = value + 48;
    response_[5] = '>';
  Response::setFinalControlData(seqNumber,6);
  }
  else
  {
    response_[4] = '0';
    response_[5] = '0';
    response_[6] = '0';
    response_[7] = '0';
    if (value >= 1000) 
    {
      response_[4] = '1';
      value -= 1000;
    }    
    if ((value < 1000) && (value >= 100))
    {
      byte b = value / 100;
      response_[5] = b + 48;
      value -= (b * 100);
    }
  Response::setFinalControlData(seqNumber,8);
  }
}

Response::Response (char action, char pinType, byte pin, int value, 
                      boolean result, int seqNumber)
{
  response_[0] = '<';
  response_[1] = action;
  response_[2] = pinType;
  if (pin < 10) 
  {
    response_[3] = '0';
  }
  else 
  {
    response_[3] = '1';
    pin -= 10;    
  }
  response_[4] = pin + 48;
  response_[5] = value + 48;
  if (result)
  {
    response_[6] = 'O';
    response_[7] = 'K';
  } 
  else 
  {
    response_[6] = 'E';
    response_[7] = 'R';
  }
  Response::setFinalControlData(seqNumber,8);
}

void Response::setFinalControlData(int seqNumber, int index){
  response_[index] = '0';
  if ((seqNumber < 100) && (seqNumber >= 10))
  {
    byte b = seqNumber / 10;
    response_[index] = b + 48;
    seqNumber -= (b * 10);
  }
  index++;
  response_[index++] = seqNumber + 48;
  response_[index++] = SERIAL_ID[0];
  response_[index++] = SERIAL_ID[1];
  response_[index++] = SERIAL_ID[2];
  response_[index] = '>';
}

void Response::getResponse(tResponse &response)
{
  for (int i = 0; i < RESPONSE_MAX_LENGTH; i ++)
    response[i] = response_[i];  
}
// =============================================================================
// CLASS DEFINITION Request
// =============================================================================
Request::Request (tRequest request)
{
  for (int i = 0; i < REQUEST_MAX_LENGTH; i ++)
    request_ [i] = request [i];
}

void Request::process()
{
  SerialCommunication::checkSeqNumber();
  byte pin = (request_[2] - 48) * 10 + (request_[3] - 48);
  switch (request_[0])
  {
    // GET
    case 'G':
        switch (request_[1])
        {
          // Digital
          case 'D':
              SerialCommunication::sendResponse
                (new Response ('G', 'D', pin, Pins::read ('D', pin),
                    SerialCommunication::getNextSeqNumber()));
            break;              
          // Analogue
          case 'A':
              SerialCommunication::sendResponse
                (new Response ('G', 'A', pin, Pins::read ('A', pin),
                    SerialCommunication::getNextSeqNumber()));
            break;              
        }            
      break;
    // SET
    case 'S':
        switch (request_[1])
        {
          // Digital
          case 'D':
              Pins::write ('D', pin, request_[4] == '1' ? HIGH : LOW);
              SerialCommunication::sendResponse
                (new Response 
                  ('S', 'D', pin, request_[4] - 48, (boolean)true,
                    SerialCommunication::getNextSeqNumber()
                   )
                );
            break;              
        }
      break;         
  }    
}
// =============================================================================
// CLASS DEFINITION SerialCommunication
// =============================================================================
boolean SerialCommunication::available()
{
  return(Serial.available() > 0);  
}

char SerialCommunication::read()
{
  char c[1];
  Serial.readBytes(c, 1); 
  return c[0];
}

void SerialCommunication::checkSeqNumber()
{
  if (true){}
}

void SerialCommunication::setup()
{
  Serial.begin(SERIAL_BAUD);
  _seqNumber = 0;
}

int SerialCommunication::getSeqNumber(){
  return _seqNumber;
}

int SerialCommunication::getNextSeqNumber(){
  return _seqNumber++;
}

void SerialCommunication::processRequests()
{
  tRequest request;
  while (SerialCommunication::available())
  {
    char charRead = SerialCommunication::read();
    if (charRead == '<')
    {
      int index = 0;
      while (charRead != '>')
      {
        if (charRead != '<')
        {
          request[index] = charRead;
          index++;
        }
        charRead = SerialCommunication::read();
      }
      Request *request_ = new Request (request);
      request_->process();
      free(request_);
    }    
  }  
}

void SerialCommunication::sendResponse(Response *response)
{
  tResponse response_;
  response->getResponse(response_);
  byte index = 0;
  while (response_[index] != '>')
  {
    Serial.print(response_[index]);
    index++; 
  }
  Serial.println(response_[index]);
  free(response);
}

void SerialCommunication::sendState(State *state)
{
  tState state_;
  state->getState(state_);
  for(int i=0; i<STATE_LENGTH-2; i++){
    Serial.print(state_[i]);
  }
  Serial.println(state_[STATE_LENGTH-1]);
  free(state);
}

// =============================================================================
// SKETCH
// =============================================================================
void setup()
{
  SerialCommunication::setup();
  Pins::setup();
}

void loop()
{ 
  SerialCommunication::sendState(new State());

  //delay(500);
    //Serial.print(TEST);
//    SerialCommunication::processRequests();
//    SerialCommunication::sendStatus(new Response 
//                  ('F', 'D', 9, request[4] - 48, (boolean)true));
}
// =============================================================================

