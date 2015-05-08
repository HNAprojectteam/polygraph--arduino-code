/**
*  Polargraph Server. - IMPLEMENTATION
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  https://github.com/euphy/polargraph_server_a1

Implementation of executeCommand for UNO-sized boards 
without "store" features. Doesn't actually do anything except
pass the command to the real executeCommand.  */

//process command given
void impl_processCommand(String com)
{
  //call the executeCommand method
  impl_executeCommand(com);
}

//run background processes
void impl_runBackgroundProcesses()
{
  //finds the time it has been since the motors last operated
  long motorCutoffTime = millis() - lastOperationTime;
  
  //if the machine can automatically power down, if it is on, and if the motors have idled too long, power down and print message to terminal window
  if ((automaticPowerDown) && (powerIsOn) && (motorCutoffTime > motorIdleTimeBeforePowerDown))
  {
    Serial.println(F("MSG_INFO_STRPowering down."));
    impl_releaseMotors();
  }
}

//loads machine specs from eeprom
void impl_loadMachineSpecFromEeprom()
{}

//executes command by sending it to the real method in exec
void impl_executeCommand(String &com)
{
  //if the commang was recognized, nothing. if not, turn on comms and run method to print error
  if (exec_executeBasicCommand(com))
  {
    // that's nice, it worked
  }
  else
  {
    comms_unrecognisedCommand(com);
    comms_ready();
  }
}

//turns on motors
void impl_engageMotors()
{
  //powers on motors, runs them for a short time to test them
  motorA.enableOutputs();
  motorB.enableOutputs();
  powerIsOn = true;
  motorA.runToNewPosition(motorA.currentPosition()+4);
  motorB.runToNewPosition(motorB.currentPosition()+4);
  motorA.runToNewPosition(motorA.currentPosition()-4);
  motorB.runToNewPosition(motorB.currentPosition()-4);
}

//turns of motors and releases them from holding (they can now spin freely)
void impl_releaseMotors()
{
  //disables all outputs, turns power off, if using servos picks pen up
  motorA.disableOutputs();
  motorB.disableOutputs();  
#ifdef PENLIFT 
  penlift_penUp();
#endif
  powerIsOn = false;  
}
//
//void impl_transform(float &tA, float &tB)
//{ 
//}
