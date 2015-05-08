/**
*  Polargraph Server. - CORE
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  https://github.com/euphy/polargraph_server_a1

Comms.

This is one of the core files for the polargraph server program.  
Comms can mean "communications" or "commands", either will do, since
it contains methods for reading commands from the serial port.

*/

//runs method that accepts a new comand
boolean comms_waitForNextCommand(char *buf)
{
  // send ready
  // wait for instruction
  //keeps track of time idled
  long idleTime = millis();
  int bufPos = 0;
  //clears the array of chars that represent the command about to be given in order to make space for new command
  for (int i = 0; i<INLENGTH; i++) {
    buf[i] = 0;
  }  
  
  //last time command was executed
  long lastRxTime = 0L;

  // loop while there's there isn't a terminated command.
  // (Note this might mean characters ARE arriving, but just
  //  that the command hasn't been finished yet.)
  boolean terminated = false;
  while (!terminated)
  {
    //if debugging, prins information about command being given
#ifdef DEBUG_COMMS    
    Serial.print(F("."));
#endif    
    //calculates the amount of time it has been since the last command was issued
    long timeSince = millis() - lastRxTime;
    
    // If the buffer is being filled, but hasn't received a new char in less than 100ms,
    // just cancel it. It's probably just junk. (i.e, this means there is a communication error)
    if (bufPos != 0 && timeSince > 100)
    {
      //if debugging, print information about error generated
#ifdef DEBUG_COMMS
      Serial.print(F("Timed out:"));
      Serial.println(timeSince);
#endif
      // Clear the buffer and reset the position if it took too long
      for (int i = 0; i<INLENGTH; i++) {
        buf[i] = 0;
      }
      bufPos = 0;
    }
    
    // idle time is mostly spent in this loop.  This loop is running background processes that set up the machine and prepare it to recieve commands
    impl_runBackgroundProcesses();
    timeSince = millis() - idleTime;
    if (timeSince > rebroadcastReadyInterval)
    {
      // issue a READY every 5000ms of idling
      //prints onto a new line if debugging
#ifdef DEBUG_COMMS      
      Serial.println("");
#endif
      comms_ready();
      idleTime = millis();
    }
    
    // And now read the command if one exists.
    if (Serial.available() > 0)
    {
      // Get the char
      char ch = Serial.read();
      
      //if debugging, prints the character recieved
#ifdef DEBUG_COMMS
      Serial.print(F("ch: "));
      Serial.println(ch);
#endif
      
      // look at it, if it's a terminator, then lets terminate the string
      if (ch == INTERMINATOR) {
        buf[bufPos] = 0; // null terminate the string
        terminated = true;
        
        //if debugging, print that the string was terminated
#ifdef DEBUG_COMMS
        Serial.println(F("Term'd"));
#endif
        //clears command char array in preparation for the next command
        for (int i = bufPos; i<INLENGTH-1; i++) {
          buf[i] = 0;
        }
        
      } else {
        // otherwise, just add it into the buffer -- this is the queue that keeps track of commands
        buf[bufPos] = ch;
        bufPos++;
      }
      
      //if debugging, prints what is happening
#ifdef DEBUG_COMMS
      Serial.print(F("buf: "));
      Serial.println(buf);
      Serial.print(F("Bufpos: "));
      Serial.println(bufPos);
#endif

      //keeps track of last time a command was issued
      lastRxTime = millis();
    }
  }
  
  //keeps track of idle time, the last time a command was issued, etc.
  idleTime = millis();
  lastOperationTime = millis();
  lastInteractionTime = lastOperationTime;
  
  //if debugging, print more info
#ifdef DEBUG_COMMS
  Serial.print(F("xbuf: "));
  Serial.println(buf);
#endif

  //return true because a command was successfully recieved
  return true;
}

//method to parse and execute command
void comms_parseAndExecuteCommand(char *inS)
{
  //if debugging, print more info
#ifdef DEBUG_COMMS
  Serial.print("3inS: ");
  Serial.println(inS);
#endif
 
 //boolean to decide if the command was able to be parsed
  boolean commandParsed = comms_parseCommand(inS);
  
  //if so, process and execute command
  if (commandParsed)
  {
    impl_processCommand(lastCommand);
    for (int i = 0; i<INLENGTH; i++) { inS[i] = 0; }  
    commandConfirmed = false;
    comms_ready();
  }
  
  //if not, print error message
  else
  {
    Serial.print(MSG_E_STR);
    Serial.print(F("Comm ("));
    Serial.print(inS);
    Serial.println(F(") not parsed."));
  }
  
  inNoOfParams = 0;
  
}

//decides whether the command is being parsed
boolean comms_parseCommand(char *inS)
{
  //if debugging, print more info
#ifdef DEBUG_COMMS
  Serial.print(F("1inS: "));
  Serial.println(inS);
#endif

  // strstr returns a pointer to the location of ",END" in the incoming string (inS). ("END" terminates the command; this finds where that is in the string)
  char* sub = strstr(inS, CMD_END);
  
  //if debugging, print more info
#ifdef DEBUG_COMMS
  Serial.print(F("2inS: "));
  Serial.println(inS);
#endif
  
  //finds length of command, puts zero to terminate command at end
  sub[strlen(CMD_END)] = 0; // null terminate it directly after the ",END"
  
  //if debugging, print more info
#ifdef DEBUG_COMMS
  Serial.print(F("4inS: "));
  Serial.println(inS);
  Serial.print(F("2Sub: "));
  Serial.println(sub);
  Serial.println(strcmp(sub, CMD_END));
#endif
  
  //if at end, stop parsing. Else, continue
  if (strcmp(sub, CMD_END) == 0) 
  {
    comms_extractParams(inS);
    return true;
  }
  else
    return false;
}  

//method to extract parameters from a command based on the inputted char array, inS
void comms_extractParams(char* inS) 
{ 
  //creates an empty char array the length of the command inS, makes a copy of the array inS and stores it in in
  char in[strlen(inS)];
  strcpy(in, inS);
  char * param;
  
  //if debugging, print more info
#ifdef DEBUG_COMMS
  Serial.print(F("In: "));
  Serial.print(in);
  Serial.println("...");
#endif  
 
  //controls which parameter you are working with
  byte paramNumber = 0;
  
  param = strtok(in, COMMA);
  
  //creates empty arrays to hold parameters
  inParam1[0] = 0;
  inParam2[0] = 0;
  inParam3[0] = 0;
  inParam4[0] = 0;
  
  //starts going through parameters.
  for (byte i=0; i<6; i++) {
      if (i == 0) {
        //if at te end of the parameter, copy it and save in inCmd
        strcpy(inCmd, param);
      }
      else {
        //if not, parse parameter
        param = strtok(NULL, COMMA);
        if (param != NULL) {
          if (strstr(CMD_END, param) == NULL) {
            // It's not null AND it wasn't 'END' either
            paramNumber++;
          }
        }
        
        //contols parameter based on various cases. If it is not null, copy command to desired array
        switch(i)
        {
          case 1:
            if (param != NULL) strcpy(inParam1, param);
            break;
          case 2:
            if (param != NULL) strcpy(inParam2, param);
            break;
          case 3:
            if (param != NULL) strcpy(inParam3, param);
            break;
          case 4:
            if (param != NULL) strcpy(inParam4, param);
            break;
          default:
            break;
        }
      }
      //if debugging, print more info
#ifdef DEBUG_COMMS
      Serial.print(F("P: "));
      Serial.print(i);
      Serial.print(F("-"));
      Serial.print(paramNumber);
      Serial.print(F(":"));
      Serial.println(param);
#endif
  }
  
  //sets how many parameters you are giving
  inNoOfParams = paramNumber;

//if debugging, print more info
#ifdef DEBUG_COMMS
    Serial.print(F("Command:"));
    Serial.print(inCmd);
    Serial.print(F(", p1:"));
    Serial.print(inParam1);
    Serial.print(F(", p2:"));
    Serial.print(inParam2);
    Serial.print(F(", p3:"));
    Serial.print(inParam3);
    Serial.print(F(", p4:"));
    Serial.println(inParam4);
    Serial.print(F("Params:"));
    Serial.println(inNoOfParams);  
#endif
}

//prints if the comms are ready to the terminal window
void comms_ready()
{
  Serial.println(F(READY_STR));
}

//prints if the comms are drawing to the terminal window
void comms_drawing()
{
  Serial.println(F(DRAWING_STR));
}

//prints if a resend of information is needed to the terminal window
void comms_requestResend()
{
  Serial.println(F(RESEND_STR));
}

//prints an error message if an unrecognized command was given
void comms_unrecognisedCommand(String &com)
{
  Serial.print(MSG_E_STR);
  Serial.print(com);
  Serial.println(F(" not recognised."));
}  


