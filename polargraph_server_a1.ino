/**
*  Polargraph Server for ATMEGA328-based arduino boards.
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  https://github.com/euphy/polargraph_server_a1

The program has a core part that consists of the following files:

- comms.ino
- configuration.ino
- eeprom.ino
- exec.ino
- penlift.ino
- pixel.ino
- util.ino

and the first portion of the main file, probably called
something like polargraph_server_a1.ino.

CONFIGURATION!! Read this!
==========================

Kung fu is like a game of chess. You must think first! Before you move.

This is a unified codebase for a few different versions of Polargraph Server.

You can control how it is compiled by changing the #define lines below.

Comment the lines below in or out to control what gets compiled.
*/

// Turn on some debugging code
//(we didn't need to use this deugging code at all, which is why it's still commented)
// ===========================
//#define DEBUG
//#define DEBUG_COMMS
//#define DEBUG_PENLIFT
//#define DEBUG_PIXEL

// Program features
//These define the classes that run the main methods in the program--the funcitons that lift the pen, generate the drawing, and generate the vector lines of the image
// ================
#define PIXEL_DRAWING
#define PENLIFT
#define VECTOR_LINES

// Specify what kind of motor driver you are using (we use a v
// ===============================================
// Make sure the version of motorshield you have is listed below WITHOUT "//" on the front.
// REMEMBER!!!  You need to comment out the matching library imports in the 'configuration.ino' tab too.
// Using discrete stepper drivers? (eg EasyDriver, stepstick, Pololu gear),
// choose SERIAL_STEPPER_DRIVERS and define your pins at the bottom of 'configuration.ino'.
//define ADAFRUIT_MOTORSHIELD_V1
#define ADAFRUIT_MOTORSHIELD_V2
//#define SERIAL_STEPPER_DRIVERS 




#include <AccelStepper.h>
#include <Servo.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

/*  ===========================================================  
    These variables are common to all polargraph server builds
=========================================================== */    
//this is the version of the software we are using
const String FIRMWARE_VERSION_NO = "1.10.6";

//  EEPROM addresses--variables that define the default settings of the machine; can be changed before running 
const byte EEPROM_MACHINE_WIDTH = 0;
const byte EEPROM_MACHINE_HEIGHT = 2;
const byte EEPROM_MACHINE_MM_PER_REV = 14; // 4 bytes (float)
const byte EEPROM_MACHINE_STEPS_PER_REV = 18;
const byte EEPROM_MACHINE_STEP_MULTIPLIER = 20;

const byte EEPROM_MACHINE_MOTOR_SPEED = 22; // 4 bytes float
const byte EEPROM_MACHINE_MOTOR_ACCEL = 26; // 4 bytes float
const byte EEPROM_MACHINE_PEN_WIDTH = 30; // 4 bytes float

const byte EEPROM_MACHINE_HOME_A = 34; // 4 bytes
const byte EEPROM_MACHINE_HOME_B = 38; // 4 bytes

const byte EEPROM_PENLIFT_DOWN = 42; // 2 bytes
const byte EEPROM_PENLIFT_UP = 44; // 2 bytes

// Pen raising servo--controls motors
Servo penHeight;
const int DEFAULT_DOWN_POSITION = 90;
const int DEFAULT_UP_POSITION = 180;
static int upPosition = DEFAULT_UP_POSITION; // defaults
static int downPosition = DEFAULT_DOWN_POSITION;
static int penLiftSpeed = 3; // ms between steps of moving motor
byte const PEN_HEIGHT_SERVO_PIN = 9;
boolean isPenUp = false;

//sets default motor speed, can be changed later in code
int motorStepsPerRev = 800;
float mmPerRev = 95;
byte stepMultiplier = 1;

//sets height/wideth of machine in mm, can be changed later in code
static int machineWidth = 650;
static int machineHeight = 800;

//sets default height/width (the height/width of the original settins)
static int defaultMachineWidth = 650;
static int defaultMachineHeight = 650;
static int defaultMmPerRev = 95;
static int defaultStepsPerRev = 800;
static int defaultStepMultiplier = 1;

//sets motor speed in miliseconds
float currentMaxSpeed = 800.0;
float currentAcceleration = 400.0;
boolean usingAcceleration = true;

//sets default length of one pen stroke in mm
int startLengthMM = 800;

//calculates stroke length based on motor speed
float mmPerStep = mmPerRev / multiplier(motorStepsPerRev);
float stepsPerMM = multiplier(motorStepsPerRev) / mmPerRev;

//sets wiidth of paper to be drawing on, can be changed later
long pageWidth = machineWidth * stepsPerMM;
long pageHeight = machineHeight * stepsPerMM;
long maxLength = 0;

//static char rowAxis = 'A';
const int INLENGTH = 50;
const char INTERMINATOR = 10;

//sets width of pen tip, can be changed later
float penWidth = 0.8F; // line width in mm

//sets variables to report to the driver software the position and speed of the pen at any given time
boolean reportingPosition = true;
boolean acceleration = true;

//declares motor variables
extern AccelStepper motorA;
extern AccelStepper motorB;

//declares variables to tell whether machine is on or off
boolean currentlyRunning = true;

//sets parameters of arrays which is used to generate individual pen strokes
static char inCmd[10];
static char inParam1[14];
static char inParam2[14];
static char inParam3[14];
static char inParam4[14];
//static char inParams[4][14];

//used for size of previos array
byte inNoOfParams;

//generates variables to hold commands in and to test whether the machine has confirmed comands
char lastCommand[INLENGTH+1];
boolean commandConfirmed = false;

//sets variables to broadcast to the terminal window of the arduino software whether or not the machine is ready, when the last time it operated was, whether the machine is on, and how long it will take for the motors to turn off if they are on idle
int rebroadcastReadyInterval = 5000;
long lastOperationTime = 0L;
long motorIdleTimeBeforePowerDown = 600000L;
boolean automaticPowerDown = false;
boolean powerIsOn = false;

//variable of the last time the user interacted with the machine
long lastInteractionTime = 0L;

//tests whether the last stroke was at the top of the machine (if so it must be treated differently to avoid accidentally stalling the motors
#ifdef PIXEL_DRAWING
static boolean lastWaveWasTop = true;

//  Drawing direction--sets the direction with which to draw in based on input by the user--ie, if in the driver GUI the user chooses the first option, which is to draw from top right to bottom left, they will start from dir_ne (because the top right is the northeast of the machine)
const static byte DIR_NE = 1;
const static byte DIR_SE = 2;
const static byte DIR_SW = 3;
const static byte DIR_NW = 4;

//sets the default drawing diretion--the machine will turn on expecting to start drawing from the top right and move to the bottom left
static int globalDrawDirection = DIR_NW;

//sets automatic and preset modes
const static byte DIR_MODE_AUTO = 1;
const static byte DIR_MODE_PRESET = 2;
static byte globalDrawDirectionMode = DIR_MODE_AUTO;
#endif

//difnes strings that will be sent to the arduino's terminal window depending on what the machine is doing.  
#define READY_STR "READY"
#define RESEND_STR "RESEND"
#define DRAWING_STR "DRAWING"
#define OUT_CMD_SYNC_STR "SYNC,"

//defines string arrays to send to the terminal window to declare that a message is being sent
char MSG_E_STR[] = "MSG,E,";
char MSG_I_STR[] = "MSG,I,";
char MSG_D_STR[] = "MSG,D,";

//defines more strings and chars that are names of commands. These will define commands in the command queue and will tell the machine which commands to run at any given time
const static char COMMA[] = ",";
const static char CMD_END[] = ",END";
const static String CMD_CHANGELENGTH = "C01";
const static String CMD_CHANGEPENWIDTH = "C02";
//const static String CMD_CHANGEMOTORSPEED = "C03";
//const static String CMD_CHANGEMOTORACCEL = "C04";
#ifdef PIXEL_DRAWING
const static String CMD_DRAWPIXEL = "C05";
const static String CMD_DRAWSCRIBBLEPIXEL = "C06";
//const static String CMD_DRAWRECT = "C07";
const static String CMD_CHANGEDRAWINGDIRECTION = "C08";
//const static String CMD_TESTPATTERN = "C10";
const static String CMD_TESTPENWIDTHSQUARE = "C11";
#endif
const static String CMD_SETPOSITION = "C09";
#ifdef PENLIFT
const static String CMD_PENDOWN = "C13";
const static String CMD_PENUP = "C14";
const static String CMD_SETPENLIFTRANGE = "C45";
#endif
#ifdef VECTOR_LINES
const static String CMD_CHANGELENGTHDIRECT = "C17";
#endif
const static String CMD_SETMACHINESIZE = "C24";
//const static String CMD_SETMACHINENAME = "C25";
const static String CMD_GETMACHINEDETAILS = "C26";
const static String CMD_RESETEEPROM = "C27";
const static String CMD_SETMACHINEMMPERREV = "C29";
const static String CMD_SETMACHINESTEPSPERREV = "C30";
const static String CMD_SETMOTORSPEED = "C31";
const static String CMD_SETMOTORACCEL = "C32";
const static String CMD_SETMACHINESTEPMULTIPLIER = "C37";

//starts setup of the machine, runs things that have to be done before the machine can work
void setup() 
{
  //begins the terminal window and prints messages
  Serial.begin(57600);           // set up Serial library at 57600 bps
  Serial.print("POLARGRAPH ON!");
  Serial.println();
  
  //runs methods from other included files that set up the machine
  configuration_motorSetup();
  eeprom_loadMachineSpecFromEeprom();
  configuration_setup();
 
  //sets max speed and acceleration of both motors
  motorA.setMaxSpeed(currentMaxSpeed);
  motorA.setAcceleration(currentAcceleration);  
  motorB.setMaxSpeed(currentMaxSpeed);
  motorB.setAcceleration(currentAcceleration);
  
  //calculates and sets starting length (time needed to complete) of strokes based on the millimeters of the desired stroke length devided by the length gained in one revolution of the motor axle, times the time it takes to complete a revolution
  float startLength = ((float) startLengthMM / (float) mmPerRev) * (float) motorStepsPerRev;
  
  //sets position of motors
  motorA.setCurrentPosition(startLength);
  motorB.setCurrentPosition(startLength);
  
  //clears all previously given commands to start fresh
  for (int i = 0; i<INLENGTH; i++) {
    lastCommand[i] = 0;
  }    
  
  //turns on communication with the machine
  comms_ready();
//if using servos to lift the pen (we are not) the pen will start up
#ifdef PENLIFT
  penlift_penUp();
#endif

//delay 500 miliseconds at end of loop
  delay(500);

}

//starts main loop that run programs
void loop()
{
  //tests if on the last commands
  if (comms_waitForNextCommand(lastCommand)) 
  {
    //if debugging, prints info abour last command
#ifdef DEBUG_COMMS    
    Serial.print(F("Last comm: "));
    Serial.print(lastCommand);
    Serial.println(F("..."));
#endif
    //parses and executes given command
    comms_parseAndExecuteCommand(lastCommand);
  }
}




