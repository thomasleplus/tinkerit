// standard firmata version 1
// brutally hacked by Massimo Banzi <m.banzi@tinker.it>
// to support the Arduino Ethernet Shield
// Tested to work with AS3Glue
//
#include <string.h>
#include <Ethernet.h>




/*==============================================================================
 * MACROS
 *============================================================================*/

/* Version numbers for the protocol.  The protocol is still changing, so these
 * version numbers are important.  This number can be queried so that host
 * software can test whether it will be compatible with the currently
 * installed firmware. */
#define FIRMATA_MAJOR_VERSION   1 // for non-compatible changes
#define FIRMATA_MINOR_VERSION   0 // for backwards compatible changes

/* total number of pins currently supported */
#define TOTAL_ANALOG_PINS       6
#define TOTAL_DIGITAL_PINS      10

// for comparing along with INPUT and OUTPUT
#define PWM                     2

// for selecting digital inputs
#define PB  2  // digital input, pins 8-13
#define PC  3  // analog input port
#define PD  4  // digital input, pins 0-7

#define MAX_DATA_BYTES 2 // max number of data bytes in non-SysEx messages
/* message command bytes */
#define DIGITAL_MESSAGE         0x90 // send data for a digital pin
#define ANALOG_MESSAGE          0xE0 // send data for an analog pin (or PWM)
//#define PULSE_MESSAGE           0xA0 // proposed pulseIn/Out message (SysEx)
//#define SHIFTOUT_MESSAGE        0xB0 // proposed shiftOut message (SysEx)
#define REPORT_ANALOG_PIN       0xC0 // enable analog input by pin #
#define REPORT_DIGITAL_PORTS    0xD0 // enable digital input by port pair
#define START_SYSEX             0xF0 // start a MIDI SysEx message
#define SET_DIGITAL_PIN_MODE    0xF4 // set a digital pin to INPUT or OUTPUT 
#define END_SYSEX               0xF7 // end a MIDI SysEx message
#define REPORT_VERSION          0xF9 // report firmware version
#define SYSTEM_RESET            0xFF // reset from MIDI

/*==============================================================================
 * GLOBAL VARIABLES
 *============================================================================*/
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 0, 2 };
byte gw[] = { 192, 168, 0, 1 };

  
char buffer[64];
  
  

Server server(5050);
Client client(0); // dummy, will be used when a client connects

/* input message handling */
byte waitForData = 0; // this flag says the next serial input will be data
byte executeMultiByteCommand = 0; // execute this after getting multi-byte data
byte multiByteChannel = 0; // channel data for multiByteCommands
byte storedInputData[MAX_DATA_BYTES] = {
  0,0}; // multi-byte data
/* digital pins */
boolean digitalInputsEnabled = false; // output digital inputs or not
int digitalInputs;
int previousDigitalInputs; // previous output to test for change
int digitalPinStatus = 3; // bitwise array to store pin status, ignore RxTx pins
/* PWM/analog outputs */
int pwmStatus = 0; // bitwise array to store PWM status
/* analog inputs */
unsigned int analogPinsToReport = 0; // bitwise array to store pin reporting
int analogPin = 0; // counter for reading analog pins
int analogData; // storage variable for data from analogRead()
/* timer variables */
unsigned long currentMillis; 
unsigned long nextExecuteMillis; // for comparison with timer0_overflow_count

/*==============================================================================
 * FUNCTIONS                                                                
 *============================================================================*/
/* -----------------------------------------------------------------------------
 * output the version message to the serial port  */
void printVersion() {
  client.print(REPORT_VERSION, BYTE);
  client.print(FIRMATA_MINOR_VERSION, BYTE);
  client.print(FIRMATA_MAJOR_VERSION, BYTE);
}

/* -----------------------------------------------------------------------------
 * output digital bytes received from the serial port  */
void outputDigitalBytes(byte pin0_6, byte pin7_13) {
  int i;
  int mask;
  int twoBytesForPorts;

  // this should be converted to use PORTs
  twoBytesForPorts = pin0_6 + (pin7_13 << 7);
  for(i=2; i<TOTAL_DIGITAL_PINS; ++i) { // ignore Rx,Tx pins (0 and 1)
    mask = 1 << i;
    if( (digitalPinStatus & mask) && !(pwmStatus & mask) ) {
      digitalWrite(i, twoBytesForPorts & mask ? HIGH : LOW);
    } 
  }
}

/* -----------------------------------------------------------------------------
 * check all the active digital inputs for change of state, then add any events
 * to the Serial output queue using client.print() */
void checkDigitalInputs() {
  if(digitalInputsEnabled) {
    previousDigitalInputs = digitalInputs;
    digitalInputs = PINB << 8;  // get pins 8-13
    digitalInputs += PIND;      // get pins 0-7
    digitalInputs = digitalInputs &~ digitalPinStatus; // ignore pins set OUTPUT
    if(digitalInputs != previousDigitalInputs) {
      // TODO: implement more ports as channels for more than 16 digital pins
      client.print(DIGITAL_MESSAGE,BYTE);
      client.print(digitalInputs % 128, BYTE); // Tx pins 0-6
      client.print(digitalInputs >> 7, BYTE);  // Tx pins 7-13
    }
  }
}

// -----------------------------------------------------------------------------
/* sets the pin mode to the correct state and sets the relevant bits in the
 * two bit-arrays that track Digital I/O and PWM status
 */
void setPinMode(byte pin, byte mode) {
  if(pin > 1) { // ignore RxTx pins (0,1)
    if(mode == INPUT) {
      digitalPinStatus = digitalPinStatus &~ (1 << pin);
      pwmStatus = pwmStatus &~ (1 << pin);
      digitalWrite(pin,LOW); // turn off pin before switching to INPUT
      pinMode(pin,INPUT);
    }
    else if(mode == OUTPUT) {
      digitalPinStatus = digitalPinStatus | (1 << pin);
      pwmStatus = pwmStatus &~ (1 << pin);
      pinMode(pin,OUTPUT);
    }
    else if( mode == PWM ) {
      digitalPinStatus = digitalPinStatus | (1 << pin);
      pwmStatus = pwmStatus | (1 << pin);
      pinMode(pin,OUTPUT);
    }
    // TODO: save status to EEPROM here, if changed
  }
}

// -----------------------------------------------------------------------------
/* sets bits in a bit array (int) to toggle the reporting of the analogIns
 */
void setAnalogPinReporting(byte pin, byte state) {
  if(state == 0) {
    analogPinsToReport = analogPinsToReport &~ (1 << pin);
  }
  else { // everything but 0 enables reporting of that pin
    analogPinsToReport = analogPinsToReport | (1 << pin);
  }
  // TODO: save status to EEPROM here, if changed
}

/* -----------------------------------------------------------------------------
 * processInput() is called whenever a byte is available on the
 * Arduino's serial port.  This is where the commands are handled. */
void processInput( int inputData) {
  int command;

    Serial.print("received ");
    Serial.println(inputData,DEC);


  // a few commands have byte(s) of data following the command
  if( (waitForData > 0) && (inputData < 128) ) {  
    waitForData--;
    storedInputData[waitForData] = inputData;
    if( (waitForData==0) && executeMultiByteCommand ) { // got the whole message
      switch(executeMultiByteCommand) {
      case ANALOG_MESSAGE:
        setPinMode(multiByteChannel,PWM);
        analogWrite(multiByteChannel, 
        (storedInputData[0] << 7) + storedInputData[1] );
        break;
      case DIGITAL_MESSAGE:
        outputDigitalBytes(storedInputData[1], storedInputData[0]); //(LSB, MSB)
        break;
      case SET_DIGITAL_PIN_MODE:
        setPinMode(storedInputData[1], storedInputData[0]); // (pin#, mode)
        if(storedInputData[0] == INPUT) 
          digitalInputsEnabled = true; // enable reporting of digital inputs
        break;
      case REPORT_ANALOG_PIN:
        setAnalogPinReporting(multiByteChannel,storedInputData[0]);
        break;
      case REPORT_DIGITAL_PORTS:
        // TODO: implement MIDI channel as port base for more than 16 digital inputs
        if(storedInputData[0] == 0)
          digitalInputsEnabled = false;
        else
          digitalInputsEnabled = true;
        break;
      }
      executeMultiByteCommand = 0;
    }	
  } 
  else {
    // remove channel info from command byte if less than 0xF0
    if(inputData < 0xF0) {
      command = inputData & 0xF0;
      multiByteChannel = inputData & 0x0F;
    } 
    else {
      command = inputData;
      // commands in the 0xF* range don't use channel data
    }
    switch (command) { // TODO: these needs to be switched to command
    case ANALOG_MESSAGE:
    case DIGITAL_MESSAGE:
    case SET_DIGITAL_PIN_MODE:
      waitForData = 2; // two data bytes needed
      executeMultiByteCommand = command;
      break;
    case REPORT_ANALOG_PIN:
    case REPORT_DIGITAL_PORTS:
      waitForData = 1; // two data bytes needed
      executeMultiByteCommand = command;
      break;
    case SYSTEM_RESET:
      // this doesn't do anything yet
      break;
    case REPORT_VERSION:
      
      printVersion();
      break;
    }
  }
}

/* -----------------------------------------------------------------------------
 * this function checks to see if there is data waiting on the serial port 
 * then processes all of the stored data
 */


// =============================================================================
// used for flashing the pin for the version number
void pin13strobe(int count, int onInterval, int offInterval) {
  byte i;
  pinMode(13, OUTPUT);
  for(i=0; i<count; i++) {
    delay(offInterval);
    digitalWrite(13,1);
    delay(onInterval);
    digitalWrite(13,0);
  }
}

void dbg(char* message) {
  Serial.print(message);
}


/*==============================================================================
 * SETUP()
 *============================================================================*/
void setup() {
  byte i;

  Ethernet.begin(mac, ip,gw);
  
  Serial.begin(9600);


  // flash the pin 13 with the protocol version


  for(i=0; i<TOTAL_DIGITAL_PINS; ++i) {
    setPinMode(i,INPUT);
  }
  // TODO: load state from EEPROM here

  dbg("begin");

  /* TODO: send digital inputs here, if enabled, to set the initial state on the
   * host computer, since once in the loop(), the Arduino will only send data on
   * change. */
}

/*==============================================================================
 * LOOP()
 *============================================================================*/
void loop() {
  /* DIGITALREAD - as fast as possible, check for changes and output them to the
   * FTDI buffer using client.print()  */

  // a new client is connected to the server
  client = server.available();

  // while client is connected we process data


  while (client.connected()) {


    // client.print("ciao");
    // main processing function
    if (client.available() > 0) 
      processInput(client.read());


    checkDigitalInputs();  
   currentMillis = millis();
    if(currentMillis > nextExecuteMillis) {  
        nextExecuteMillis = currentMillis + 19; // run this every 20ms
      /* SERIALREAD - client.read() uses a 128 byte circular buffer, so handle
       	 * all serialReads at once, i.e. empty the buffer */
      //checkForSerialReceive();



      /* SEND FTDI WRITE BUFFER - make sure that the FTDI buffer doesn't go over
       	 * 60 bytes. use a timer to sending an event character every 4 ms to
       	 * trigger the buffer to dump. */

      /* ANALOGREAD - right after the event character, do all of the
       	 * analogReads().  These only need to be done every 4ms. */
      for(analogPin=0;analogPin<TOTAL_ANALOG_PINS;analogPin++) {
        if( analogPinsToReport & (1 << analogPin) ) {
          analogData = analogRead(analogPin);

          client.print(ANALOG_MESSAGE + analogPin, BYTE);
          // These two bytes converted back into the 10-bit value on host
          client.print(analogData % 128, BYTE);
          client.print(analogData >> 7, BYTE); 
        }
      }
    }
  }
  client.stop();
}
