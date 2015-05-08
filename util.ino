/**
*  Polargraph Server. - CORE
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  https://github.com/euphy/polargraph_server_a1

Util.

This is one of the core files for the polargraph server program.  

This has all the methods that let the rest actually work, including
the geometry routines that convert from the different coordinates
systems, and do transformations.

*/

//these multiply and divide values by the standard stepMultiplier of the machine to calculate multipliers and dividers
long multiplier(int in)
{
  return multiplier((long) in);
}
long multiplier(long in)
{
  return in * stepMultiplier;
}
float multiplier(float in)
{
  return in * stepMultiplier;
}
long divider(long in)
{
  return in / stepMultiplier;
}






//this changes length
void changeLength(long tAl, long tBl)
{
  //copies parameters
  float tA = float(tAl);
  float tB = float(tBl);

  //finds last time motors operated
  lastOperationTime = millis();

  //finds current motor speed
  float currSpeedA = motorA.speed();
  float currSpeedB = motorB.speed();
  

  //sets motor speed as 0 and attempts to move motor
  motorA.setSpeed(0.0);
  motorB.setSpeed(0.0);
  motorA.moveTo(tA);
  motorB.moveTo(tB);
  
  //if you are not using acceleration
  if (!usingAcceleration)
  {
    // The moveTo() function changes the speed in order to do a proper
    // acceleration. This counteracts it. Ha.
     
    //manually accelerate anticlockwise 
    if (motorA.speed() < 0)
      currSpeedA = -currSpeedA;
    if (motorB.speed() < 0)
      currSpeedB = -currSpeedB;

    //set motor speed as the current spped
    motorA.setSpeed(currSpeedA);
    motorB.setSpeed(currSpeedB);
  }
  
  //while you still have distance left to move...
  while (motorA.distanceToGo() != 0 || motorB.distanceToGo() != 0)
  {
    //this is running background processes
    impl_runBackgroundProcesses();
    
    //if your machine is running and you are using acceleration, run the motors. Otherwise, move them without acceleration
    if (currentlyRunning)
    {
      if (usingAcceleration)
      {
        motorA.run();
        motorB.run();
      }
      else
      {

        motorA.runSpeedToPosition();
        motorB.runSpeedToPosition();
      }
    }
  }
  //report current position to machine
  reportPosition();
}

//changes relative length if given floats
void changeLengthRelative(float tA, float tB)
{
  changeLengthRelative((long) tA, (long)tB);
}

//changes relative length if given longs
void changeLengthRelative(long tA, long tB)
{
  //calculates last operation time and moves motors to specified position
  lastOperationTime = millis();
  motorA.move(tA);
  motorB.move(tB);
  
  //while the motors still have distance to go, run them if possible.  If using acceleration, just run standardly, otherwise, run without acceperation.
  while (motorA.distanceToGo() != 0 || motorB.distanceToGo() != 0)
  {
    //impl_runBackgroundProcesses();
    if (currentlyRunning)
    {
      if (usingAcceleration)
      {
        motorA.run();
        motorB.run();
      }
      else
      {
        motorA.runSpeedToPosition();
        motorB.runSpeedToPosition();
      }
    }
  }
  
  //report position of motors to machine
  reportPosition();
}

//get max length of a drawing
long getMaxLength()
{
  if (maxLength == 0)
  {
//    float length = getMachineA(pageWidth, pageHeight);

    //max length is the diagonal of the paper plus 0.5. Print to the console and return value
    maxLength = long(getMachineA(pageWidth, pageHeight)+0.5);
    Serial.print(F("maxLength: "));
    Serial.println(maxLength);
  }
  return maxLength;
}

//gets width using coordinates of far corners
float getMachineA(float cX, float cY)
{
  //pythag theorem to calculate
  float a = sqrt(sq(cX)+sq(cY));
  return a;
}

//gets height of machine using coordinates of far corners
float getMachineB(float cX, float cY)
{
  //pythag theorem to calculate
  float b = sqrt(sq((pageWidth)-cX)+sq(cY));
  return b;
}

//find acces of movement
void moveAxis(AccelStepper &m, int dist)
{
  //move the motor specified distance
  m.move(dist);
  
  //while moving, run background processes
  while (m.distanceToGo() != 0)
  {
    //if the background processes do not cause the machine to shut down, allow it to continue
    impl_runBackgroundProcesses();
    if (currentlyRunning)
      m.run();
  }
  //set time of last operaiton of motors
  lastOperationTime = millis();
}

//prints position of motors to terminal window
void reportPosition()
{
  if (reportingPosition)
  {
    Serial.print(OUT_CMD_SYNC_STR);
    Serial.print(divider(motorA.currentPosition()));
    Serial.print(COMMA);
    Serial.print(divider(motorB.currentPosition()));
    Serial.println(CMD_END);
  }
}

//gets cartesian coordinates
float getCartesianXFP(float aPos, float bPos)
{
  //uses pythag theorem to calculate x-coordinate
  float calcX = (sq((float)pageWidth) - sq((float)bPos) + sq((float)aPos)) / ((float)pageWidth*2.0);
  return calcX;  
}

//uses pythag theorem to calculate y-coordinate
float getCartesianYFP(float cX, float aPos) 
{
  float calcY = sqrt(sq(aPos)-sq(cX));
  return calcY;
}

//finds x-coordinate using input positions and the pythag theorem
long getCartesianX(float aPos, float bPos)
{
  long calcX = long((pow(pageWidth, 2) - pow(bPos, 2) + pow(aPos, 2)) / (pageWidth*2));
  return calcX;  
}

//finds the x-coordinate using motor's current positions
long getCartesianX() {
  long calcX = getCartesianX(motorA.currentPosition(), motorB.currentPosition());
  return calcX;  
}

//finds y-coordinate with the getCartesianX() method and current motor positions
long getCartesianY() {
  return getCartesianY(getCartesianX(), motorA.currentPosition());
}
//find y-coordinate with the pythag theorem and input positions
long getCartesianY(long cX, float aPos) {
  long calcY = long(sqrt(pow(aPos,2)-pow(cX,2)));
  return calcY;
}

