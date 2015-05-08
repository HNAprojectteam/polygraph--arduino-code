/**
*  Polargraph Server. - CORE
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  https://github.com/euphy/polargraph_server_a1

Penlift.

This is one of the core files for the polargraph server program.  
This file contains the servo calls that raise or lower the pen from
the page.

The behaviour of the pen lift is this:

If a simple "pen up", or "pen lift" command is received ("C14,END"), then the machine will
not try to lift the pen if it thinks it is already up.  It checks the value of the 
global boolean variable "isPenUp" to decide this.

If a qualified "pen up" is received, that is one that includes a pen position (eg "C14,150,END"),
then the global "up" position variable is updated, and the servo is moved to that position,
even if it already is "up".  Because naturally, if the up position has changed, even if it
is already up, there's a good chance it won't be up enough.

The same goes for the 

*/

//if using a servo to control pen height (we don't)...
#ifdef PENLIFT
//to move the pen
void penlift_movePen(int start, int end, int delay_ms)
{ 
  //turn on the servo's pin on the arduino
  penHeight.attach(PEN_HEIGHT_SERVO_PIN);
  
  //if the position you are in is numerically less then where you want the pen to be (i.e, you are moving the pen up and have not finished)
  if(start < end)
  { 
    //change pen height by lifting it up
    for (int i=start; i<=end; i++) 
    {
      penHeight.write(i);
      delay(delay_ms);
      
      //if debugging, print more info
#ifdef DEBUG_PENLIFT
      Serial.println(i);
#endif
    }
  }
  
  //else if you are moving the pen down and have not finished
  else
  {
    for (int i=start; i>=end; i--) 
    {
      //change pen height by moving it down
      penHeight.write(i);
      delay(delay_ms);
      
      //if debugging, print more info
#ifdef DEBUG_PENLIFT
      Serial.println(i);
#endif
    }
  }
  
  //turn off the servo's pin to avoid wasting power
  penHeight.detach();
}

//lift pen
void penlift_penUp()
{
  //if there are multiple actions, the pen is not at default position and the method must also decide where the pen currently is so as not to burn out the motor
  if (inNoOfParams > 1)
  {
    //if debugging, print more info
#ifdef DEBUG_PENLIFT
    Serial.print("Penup with params");
#endif
    //find position you are moving from by finding if pen is up or not
    int positionToMoveFrom = isPenUp ? upPosition : downPosition;
    
    //get desired position based on parameters
    upPosition = atoi(inParam1);
    
    //move pen from current position to desired position at specified speed
    penlift_movePen(positionToMoveFrom, upPosition, penLiftSpeed);
  }
  
  //otherwise, pen is down
  else
  {
    
    //move from down position to desired position at desired speed
    if (isPenUp == false)
    {
      penlift_movePen(downPosition, upPosition, penLiftSpeed);
    }
  }
  
  //set isPenUp as true
  isPenUp = true;
}


//move pen down
void penlift_penDown()
{
  // check to see if this is a multi-action command (if there's a
  // parameter then this sets the "down" motor position too).
  if (inNoOfParams > 1)
  { 
    //find position to move from
    int positionToMoveFrom = isPenUp ? upPosition : downPosition;
    
    //get down position from parameters
    downPosition = atoi(inParam1);
    
    //move pen down from desired position 
    penlift_movePen(positionToMoveFrom, downPosition, penLiftSpeed);
  }
  
  //otherwise, don't have to worry about controlling motors, just move from default up position
  else
  {
    if (isPenUp == true)
    {
      penlift_movePen(upPosition, downPosition, penLiftSpeed);
    }
  }
  isPenUp = false;
}
#endif
