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
const int D_PINS_IN_LENGTH = 2;
const int A_PINS_IN_LENGTH = 5;
const int D_PINS_OUT_LENGTH = 2;
const int A_PINS_OUT_LENGTH = 5;
const int D_PINS_IN[] = {
  2,4,7};
const int A_PINS_IN[] = {
  A0,A1,A2,A3,A4,A5};
const int D_PINS_OUT[] = {
  8,12,13};
const int A_PINS_OUT[] = {
  3,5,6,9,10,11};
//const int PIN_NUMBER[] = {/*0*/-1,/*1*/-1,/*2*/0,/*3*/0,/*4*/1,/*5*/1,/*6*/2,
///*7*/2,/*8*/0,/*9*/3,/*10*/4,/*11*/5,}
const char *SERIAL_ID = "XXX";

/*** Protocol data position definition ***/
/*** Report State Message (Letter: S) ***/
const int STATE_LENGTH = 62;
/** long 60 **/
const int POS_S_FIRST_CONTROL_CHAR = 0;
const int POS_S_SERIAL_NUMBER = 1;
const int POS_S_SEQ_NUMBER = 4;
const int POS_S_MESSAGE_TYPE = 6; // Report
const int POS_S_OPERATION_TYPE = 7; // Free Byte [Ok | Error]
const int POS_S_D_PIN_IN[] = {
  8,9,10};
const int POS_S_A_PIN_IN[] = {
  11,15,19,23,27,31};
const int POS_S_D_PIN_OUT[] = {
  35,36,37};
const int POS_S_A_PIN_OUT[] = {
  38,42,46,50,54,58};
const int POS_S_LAST_CONTROL_CHAR = 62;

/*** Response Order Message (Letter: R) ***/
const int RESPONSE_LENGTH = 14;
/** long 13 **/
const int POS_R_FIRST_CONTROL_CHAR = 0;
const int POS_R_SERIAL_NUMBER = 1;
const int POS_R_SEQ_NUMBER = 4;
const int POS_R_MESSAGE_TYPE = 6; // Get - Set - Multiple
const int POS_R_OPERATION_TYPE = 7; // Digital - Analog - #MultipleSeq
const int POS_R_PIN = 8;
const int POS_R_DATA = 10;
const int POS_R_LAST_CONTROL_CHAR = 14;

/*** Request Order Message (Letter: O) ***/
const int REQUEST_LENGTH = 14;
/** Specification **/
const int POS_O_FIRST_CONTROL_CHAR = 0;
const int POS_O_SERIAL_NUMBER = 1;
const int POS_O_SEQ_NUMBER = 4;
const int POS_O_MESSAGE_TYPE = 6; // Get - Set - Multiple
const int POS_O_OPERATION_TYPE = 7; // Digital - Analog - #MultipleSeq
const int POS_O_PIN = 8;
const int POS_O_DATA = 10;
const int POS_O_LAST_CONTROL_CHAR = 14;

//const char *TEST = "<XXX00R010101102310231023102310231023102310231023102310231023>";
// =============================================================================
// TYPEDEFS
// =============================================================================
typedef char tState[STATE_LENGTH];
typedef char tResponse[RESPONSE_LENGTH];
typedef char tRequest[REQUEST_LENGTH];

int _delay = 0;
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
  static void write(char pinType, byte pin, int value);
  static int readDigitalIn(byte pin);
  static int readAnalogIn(byte pin);
  static int readDigitalOut(byte pin);
  static int readAnalogOut(byte pin);
  static byte getPinInt(char a, char b);
  static char getPinChar0(int value);
  static char getPinChar1(int value);
  static char getDigitalChar(int value);
  static char getAnalogChar0(int value);
  static char getAnalogChar1(int value);
  static char getAnalogChar2(int value);
  static char getAnalogChar3(int value);
  static byte getPinNumberInt(char a, char b);
  static int getValueInt(char a, char b, char c, char d);
  static void dWrite(byte pin, int value);
  static void aWrite(byte pin, int value);
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
};
// =============================================================================
// CLASS HEADER Response
// =============================================================================
class Response
{
private:
  tResponse response_;
public:
  Response (char messageType, char operationType, byte pin, int value);
  void getResponse (tResponse &response);
  void setControlData();
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
  int isOkControlData();
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
  static void setup();
  static void processRequests();
  static void sendResponse(Response *response);
  static void sendState(State *state);
  static byte getSeqNumberInt(char a, char b);
  static char getSerialNumber0(int value);
  static char getSerialNumber1(int value);
  static char getSerialNumber2(int value);
  static byte getSerialNumberInt(char a, char b, char c);
};

/******************************************************************************/
// SECTION Class Definition
/******************************************************************************/

// =============================================================================
// CLASS DEFINITION Pins
// =============================================================================
int Pins::dpi_values[]={
  0,0,0};
int Pins::api_values[]={
  0,0,0,0,0,0};
int Pins::dpo_values[]={
  0,0,0};
int Pins::apo_values[]={
  0,0,0,0,0,0};

void Pins::setup()
{
  //Digital Pin In
  for(int pin = 0; pin <= D_PINS_IN_LENGTH; pin++)
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
  for(int pin = 0; pin <= D_PINS_OUT_LENGTH; pin++)
  {
    pinMode(D_PINS_OUT[pin], OUTPUT);   
    digitalWrite(D_PINS_OUT[pin], dpo_values[pin]);
  }
  //Analog Pin Out
  for(int pin = 0; pin <= A_PINS_OUT_LENGTH; pin++)
  {
    pinMode(A_PINS_OUT[pin], OUTPUT);
    analogWrite(A_PINS_OUT[pin], apo_values[pin]);
  }

}

/** WRITE **/
void Pins::dWrite(byte pin, int value)
{
  dpo_values[pin] = value;
  Serial.println(  dpo_values[pin]);
  digitalWrite(D_PINS_OUT[pin], value);
}
void Pins::aWrite(byte pin, int value)
{
  apo_values[pin] = value;
  analogWrite(A_PINS_OUT[pin], value);
}

/** READ **/
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

/** INT/CHAR CONVERSION **/
byte Pins::getPinInt(char a, char b){
  return (a - 48) * 10 + (b - 48);
}

char Pins::getPinChar0(int value){
  if(value>=10)
    return '1';
  return '0';
}

char Pins::getPinChar1(int value){
  return value % 10 + 48;
}

char Pins::getDigitalChar(int value){
  if(value)
    return '1';
  return '0';
}

char Pins::getAnalogChar0(int value){
  if(value>=1000)
    return '1';
  return '0';
}

char Pins::getAnalogChar1(int value){
  return value / 100 % 10 + 48;
}

char Pins::getAnalogChar2(int value){
  return value / 10 % 10 + 48;
}

char Pins::getAnalogChar3(int value){
  return value % 10 + 48;
}

byte Pins::getPinNumberInt(char a, char b){
  return (a - 48) * 10 + (b - 48);
}

int Pins::getValueInt(char a, char b, char c, char d){
  return (a - 48) * 1000 + (b - 48) * 100 + (c - 48) * 10 + (d - 48);
}

// =============================================================================
// CLASS DEFINITION State
// =============================================================================
State::State ()
{
  State::setControlData();
  state_[POS_S_MESSAGE_TYPE] = 'S';
  state_[POS_S_OPERATION_TYPE] = 'O';


  for(int i = 0; i <= D_PINS_IN_LENGTH; i++)
  {
    state_[POS_S_D_PIN_IN[i]] = Pins::getDigitalChar(Pins::readDigitalIn(i));
  }
  //Analog Pin In
  for(int i = 0; i <= A_PINS_IN_LENGTH; i++)
  {
    int value = Pins::readAnalogIn(i);
    state_[POS_S_A_PIN_IN[i]] = Pins::getAnalogChar0(value);
    state_[POS_S_A_PIN_IN[i]+1] = Pins::getAnalogChar1(value);
    state_[POS_S_A_PIN_IN[i]+2] = Pins::getAnalogChar2(value);
    state_[POS_S_A_PIN_IN[i]+3] = Pins::getAnalogChar3(value);
  }
  //Digital Pin Out
  for(int i = 0; i <= D_PINS_OUT_LENGTH; i++)
  {
    state_[POS_S_D_PIN_OUT[i]] = Pins::getDigitalChar(Pins::readDigitalOut(i));
  }
  //Analog Pin Out
  for(int i = 0; i <= A_PINS_OUT_LENGTH; i++)
  {
    int value = Pins::readAnalogOut(i);
    state_[POS_S_A_PIN_OUT[i]] = Pins::getAnalogChar0(value);
    state_[POS_S_A_PIN_OUT[i]+1] = Pins::getAnalogChar1(value);
    state_[POS_S_A_PIN_OUT[i]+2] = Pins::getAnalogChar2(value);
    state_[POS_S_A_PIN_OUT[i]+3] = Pins::getAnalogChar3(value);
  }

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
  for (int i = 0; i <= STATE_LENGTH; i++)
    state[i] = state_[i];  
}
// =============================================================================
// CLASS DEFINITION Response
// =============================================================================

Response::Response (char messageType, char operationType, byte pin, int value)
{
  Response::setControlData();
  response_[POS_R_MESSAGE_TYPE] = messageType;
  response_[POS_R_OPERATION_TYPE] = operationType;
  response_[POS_R_PIN] = Pins::getPinChar0(pin);
  response_[POS_R_PIN+1] = Pins::getPinChar1(pin);
  response_[POS_R_DATA] = Pins::getAnalogChar0(value);
  response_[POS_R_DATA+1] = Pins::getAnalogChar1(value);
  response_[POS_R_DATA+2] = Pins::getAnalogChar2(value);
  response_[POS_R_DATA+3] = Pins::getAnalogChar3(value);
}

void Response::setControlData()
{
  int seqNumber = SerialCommunication::getSeqNumber();

  response_[POS_R_FIRST_CONTROL_CHAR] = '<';

  response_[POS_R_SERIAL_NUMBER] = SERIAL_ID[0];
  response_[POS_R_SERIAL_NUMBER+1] = SERIAL_ID[1];
  response_[POS_R_SERIAL_NUMBER+2] = SERIAL_ID[2];

  response_[POS_R_SEQ_NUMBER] = '0';
  if ((seqNumber < 100) && (seqNumber >= 10))
  {
    byte b = seqNumber / 10;
    response_[POS_R_SEQ_NUMBER] = b + 48;
    seqNumber -= (b * 10);
  }
  response_[POS_R_SEQ_NUMBER+1] = seqNumber + 48;

  response_[POS_R_LAST_CONTROL_CHAR] = '>';

}

void Response::getResponse(tResponse &response)
{
  for (int i = 0; i <= RESPONSE_LENGTH; i ++)
    response[i] = response_[i];  
}
// =============================================================================
// CLASS DEFINITION Request
// =============================================================================
Request::Request (tRequest request)
{
  for (int i = 0; i <= REQUEST_LENGTH; i ++)
    request_ [i] = request [i];
}

void Request::process()
{
  if(Request::isOkControlData()){
    Serial.println("ASDF");
    switch (request_[POS_O_MESSAGE_TYPE])  
    {
    case 'S':
      switch (request_[POS_O_OPERATION_TYPE])  
      {
      case 'D':
        Pins::dWrite(
        Pins::getPinNumberInt(
        request_[POS_O_PIN], 
        request_[POS_O_PIN+1]
          ), 
        Pins::getValueInt(
        request_[POS_R_DATA],
        request_[POS_R_DATA+1],
        request_[POS_R_DATA+2],
        request_[POS_R_DATA+3]
          ));

        SerialCommunication::sendResponse
          (new Response (request_[POS_O_MESSAGE_TYPE], 
        request_[POS_O_OPERATION_TYPE], 
        Pins::getPinNumberInt(
        request_[POS_O_PIN], 
        request_[POS_O_PIN+1]), 
        Pins::getValueInt(
        request_[POS_R_DATA],
        request_[POS_R_DATA+1],
        request_[POS_R_DATA+2],
        request_[POS_R_DATA+3]
          ))
          );  
        break;
      case 'A':
        //return analogRead(pin);        
        break;
      case 'T':
        _delay = Pins::getValueInt(
        request_[POS_R_DATA],
        request_[POS_R_DATA+1],
        request_[POS_R_DATA+2],
        request_[POS_R_DATA+3]
          );        
        break;
      }       
      break;
    }

  } 
  else {
    Serial.println("ERRORRRR");
  }
}

int Request::isOkControlData()
{
  if(isOkSeqNumber(request_[POS_O_SEQ_NUMBER],request_[POS_O_SEQ_NUMBER+1]) && 
    isOkSerialNumber(request_[POS_O_SERIAL_NUMBER],
  request_[POS_O_SERIAL_NUMBER+1],
  request_[POS_O_SERIAL_NUMBER+2]) && 
    isOkControlChars(request_[POS_O_FIRST_CONTROL_CHAR],
  request_[POS_O_LAST_CONTROL_CHAR])
    )
    return 1;
  return 0;
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
      request[index] = charRead;
      index++;
      while (charRead != '>')
      {
        if (charRead != '<')
        {
          request[index] = charRead;
          index++;
        }
        charRead = SerialCommunication::read();
      }
      if (charRead == '>'){
        request[index] = charRead;
      }
      Serial.println(":::::::");
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
  for(int i=0; i<=STATE_LENGTH-1; i++){
    Serial.print(state_[i]);
  }
  Serial.println(state_[STATE_LENGTH]);
  free(state);
}

byte SerialCommunication::getSeqNumberInt(char a, char b){
  return (a - 48) * 10 + (b - 48);
}

char SerialCommunication::getSerialNumber0(int value){
  return value / 100 % 10;
}
char SerialCommunication::getSerialNumber1(int value){
  return value / 10 % 10;
}
char SerialCommunication::getSerialNumber2(int value){
  return value % 10;
}
byte SerialCommunication::getSerialNumberInt(char a, char b, char c){
  return (a - 48) * 100 + (b - 48) * 10 + (c - 48);
}
// =============================================================================
// ASCO Refactor this
// =============================================================================
byte getSeqNumberChar0(){
  return _seqNumber / 10 % 10;
}
byte getSeqNumberChar1(){
  return _seqNumber % 10;
}
int isOkSeqNumber(byte a, byte b)
{
  if(a-48 != getSeqNumberChar0() || 
    b-48 != getSeqNumberChar1() ){
    // SEND ERROR MESSAGE AND RESET
    Serial.println("error SEQ NUMBER");
    return 0;
  }
  return 1;
}

int isOkControlChars(byte a, byte b)
{
  if(a != (byte) '<' || b != (byte) '>'){
    // SEND ERROR MESSAGE AND RESET
    Serial.println("error CONTROL CHAR");
    return 0;
  }
  return 1;
}

int isOkSerialNumber(char a, char b, char c)
{
  if (SERIAL_ID[0] != a || SERIAL_ID[1] != b || SERIAL_ID[2] != c){
    // NOTHING TO DO/DISCARD
    Serial.println("error SERIAL NUMBER");
    return 0;
  }
  return 1;
}
// =============================================================================
// SKETCH
// =============================================================================
void setup()
{
  SerialCommunication::setup();
  Pins::setup();
  pinMode(13, OUTPUT);
}

void loop()
{ 
  //     Serial.println(":::::::");
  SerialCommunication::processRequests();
  //      Serial.println(":::::::");
  //  analogWrite(13, LOW);
  SerialCommunication::sendState(new State());

  delay(_delay);
  //Serial.print(TEST);
  //    SerialCommunication::processRequests();
  //    SerialCommunication::sendStatus(new Response 
  //                  ('F', 'D', 9, request[4] - 48, (boolean)true));
}
// =============================================================================





