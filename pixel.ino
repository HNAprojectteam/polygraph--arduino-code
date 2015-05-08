/**
*  Polargraph Server. - CORE
*  Written by Sandy Noble
*  Released under GNU License version 3.
*  http://www.polargraph.co.uk
*  https://github.com/euphy/polargraph_server_a1

Pixel.

This is one of the core files for the polargraph server program.  

This is a biggie, and has the routines necessary for generating and drawing
the squarewave and scribble pixel styles.

*/

//if drawing from pixels
#ifdef PIXEL_DRAWING

//change direction drawing in
void pixel_changeDrawingDirection() 
{
  //get x and y coordinates based on generated parameters
  globalDrawDirectionMode = atoi(inParam1);
  globalDrawDirection = atoi(inParam2);
//  Serial.print(F("Changed draw direction mode to be "));
//  Serial.print(globalDrawDirectionMode);
//  Serial.print(F(" and direction is "));
//  Serial.println(globalDrawDirection);
}

//draw a standard ("square") pixel
void pixel_drawSquarePixel() 
{  
    //get origin point, size of pixel, and density of pixel based on input paramters
    long originA = multiplier(atol(inParam1));
    long originB = multiplier(atol(inParam2));
    int size = multiplier(atoi(inParam3));
    int density = atoi(inParam4);
    
    /*  Here density is accepted as a recording of BRIGHTNESS, where 0 is black and 255 is white.
        Later on, density gets scaled to the range that is available for this particular 
        pixel+pentip combination, and also inverted so that it becomes a recording of DARKNESS,
        where 0 is white and the higher values are darker.
        
        (Using the same variable to save on space, really.)
        
        This is because paper is white, and ink is black, and this density value is used to 
        control how many waves are drawn. 
        
        O waves means no ink, so a very light pixel.
        50 waves means lots of ink, so a much darker pixel.
    */
    
    //find half the size of pixel (as pixels are a combo of 2 lines
    int halfSize = size / 2;
    
    //find start and end points of drawn lines
    long startPointA;
    long startPointB;
    long endPointA;
    long endPointB;

    int calcFullSize = halfSize * 2; // see if there's any rounding errors when you end up drawing full pixel
    
    //record offset if there are rounding errors
    int offsetStart = size - calcFullSize;
    
    //If you're in the standard drawing mode, drawing top right to bottom left
    if (globalDrawDirectionMode == DIR_MODE_AUTO)
      //direction you're drawing in is standard
      globalDrawDirection = pixel_getAutoDrawDirection(originA, originB, motorA.currentPosition(), motorB.currentPosition());
      
    //if you're drawing from the bottom right to the top left...
    if (globalDrawDirection == DIR_SE) 
    {
      //Serial.println(F("d: SE"));

      //Start X coordinate adjusted by half a stroke while Y coordinate remains constant for both origin and end
      startPointA = originA - halfSize;
      startPointA += offsetStart;
      startPointB = originB;
      endPointA = originA + halfSize;
      endPointB = originB;
    }
    
    //If drawing from the bottom left to the top right...
    else if (globalDrawDirection == DIR_SW)
    {
//      Serial.println(F("d: SW"));
      //Y coordinate gets adjusted back by a half-stroke while X stays the same for both origin and end
      startPointA = originA;
      startPointB = originB - halfSize;
      startPointB += offsetStart;
      endPointA = originA;
      endPointB = originB + halfSize;
    }
    
    //If drawing from the top left to the bottom right...
    else if (globalDrawDirection == DIR_NW)
    {
//      Serial.println(F("d: NW"));

      //X coordinate grows by a half-step while Y-coordinate is decreased by a half-step for both origin and end
      startPointA = originA + halfSize;
      startPointA -= offsetStart;
      startPointB = originB;
      endPointA = originA - halfSize;
      endPointB = originB;
    }
    
    //otherwise, just set as standard NE
    else //(drawDirection == DIR_NE)
    {
//      Serial.println(F("d: NE"));
      startPointA = originA;
      startPointB = originB + halfSize;
      startPointB -= offsetStart;
      endPointA = originA;
      endPointB = originB - halfSize;
    }

    /* pixel_scaleDensity takes it's input value as a BRIGHTNESS value (ie 255 = white),
       but returns a DARKNESS value (ie 0 = white). 
       Here I'm using the same variable to hold both, save space in memory. */
    density = pixel_scaleDensity(density, 255, pixel_maxDensity(penWidth, size));
//    Serial.print(F("Start point: "));
//    Serial.print(startPointA);
//    Serial.print(COMMA);
//    Serial.print(startPointB);
//    Serial.print(F(". end point: "));
//    Serial.print(endPointA);
//    Serial.print(COMMA);
//    Serial.print(endPointB);
//    Serial.println(F("."));
    
    //changes length of stroke to be drawing from the starting points of the pixel
    changeLength(startPointA, startPointB);
    
    //if the stroke density is large enough to leave a mark, draw in standard direction
    if (density > 1)
    {
      pixel_drawSquarePixel(size, size, density, globalDrawDirection);
    }
    
    //now drawing from endpoints of this pixel, so change to reflect
    changeLength(endPointA, endPointB);
    
    //outputAvailableMemory(); 
}

  //get a random directio for when using the scribble function
  byte pixel_getRandomDrawDirection()
  {  
    return random(1, 5);
  }
  
  //get an automatically calculated direction based on distance between source and target points
  byte pixel_getAutoDrawDirection(long targetA, long targetB, long sourceA, long sourceB)
  {
    //get byte for random direction
    byte dir = DIR_SE;
    
      //calculate direction based on where the X and Y positions are relative to each other
      if (targetA<sourceA && targetB<sourceA)
        dir = DIR_NW;
      else if (targetA>sourceA && targetB>sourceB)
        dir = DIR_SE;
      else if (targetA<sourceA && targetB>sourceB)
        dir = DIR_SW;
      else if (targetA>sourceA && targetB<sourceB)
        dir = DIR_NE;
      else if (targetA==sourceA && targetB<sourceB)
        dir = DIR_NE;
      else if (targetA==sourceA && targetB>sourceB)
        dir = DIR_SW;
      else if (targetA<sourceA && targetB==sourceB)
        dir = DIR_NW;
      else if (targetA>sourceA && targetB==sourceB)
        dir = DIR_SE;
      else
      {
//    Serial.println("Not calculated - default SE");
  }
  
  //returns calculated direction
  return dir;
}

//draws a scribble pixel 
void pixel_drawScribblePixel() 
{  
    //get origin, size, and density of pixel from parameters in command
    long originA = multiplier(atol(inParam1));
    long originB = multiplier(atol(inParam2));
    int size = multiplier(atoi(inParam3));
    int density = atoi(inParam4);
    
    //get max density from width of pen and size of pixel
    int maxDens = pixel_maxDensity(penWidth, size);

    //scale density based on brightness of current location in image
    density = pixel_scaleDensity(density, 255, maxDens);
    
    //draw random scribble pixel
    pixel_drawScribblePixel(originA, originB, size*1.1, density);
    
//    outputAvailableMemory(); 
}

//draw a scribble picture from user parameters
void pixel_drawScribblePixel(long originA, long originB, int size, int density) 
{

//  int originA = motorA.currentPosition();
//  int originB = motorB.currentPosition();
  
  //calculate low and high perimeters to find max area scribble can take up
  long lowLimitA = originA-(size/2);
  long highLimitA = lowLimitA+size;
  long lowLimitB = originB-(size/2);
//  long highLimitB = lowLimitB+size;

  //generate random X and Y values
  int randA;
  int randB;
  
  //find size of pixel being drawn
  int inc = 0;
  int currSize = size;
  
  for (int i = 0; i <= density; i++)
  {
    //fill pixels with random values
    randA = random(0, currSize);
    randB = random(0, currSize);
    changeLength(lowLimitA+randA, lowLimitB+randB);
    
    lowLimitA-=inc;
    highLimitA+=inc;
    currSize+=inc*2;
  }
}

//find minimum size of pixel based on pen size
int pixel_minSegmentSizeForPen(float penSize)
{
  //find number of steps a specific pen has by multiplying the pen size by the number of steps it takes the motor to move the pen 1mm
  float penSizeInSteps = penSize * stepsPerMM;
  
  //Automatically set the min as 1. If the pen size in motor steps is large, set the minimum as that
  int minSegSize = 1;
  if (penSizeInSteps >= 2.0)
    minSegSize = int(penSizeInSteps);
    
//  Serial.print(F("Min segment size for penSize "));
//  Serial.print(penSize);
//  Serial.print(F(": "));
//  Serial.print(minSegSize);
//  Serial.print(F(" steps."));
//  Serial.println();
  
  //return value
  return minSegSize;
 }

  //find maximum density based on pen size and size of a row of pixels
  int pixel_maxDensity(float penSize, int rowSize)
  {
  //the find row size in mm by multiplying the row size in steps by the mm per step
  float rowSizeInMM = mmPerStep * rowSize;
  
  //if debugging, print more info
  #ifdef DEBUG_PIXEL
    Serial.print(F("MSG,D,rowsize in mm: "));
    Serial.print(rowSizeInMM);
    Serial.print(F(", mmPerStep: "));
    Serial.print(mmPerStep);
    Serial.print(F(", rowsize: "));
    Serial.println(rowSize);
  #endif
 
  //find number of segments in row by dividing row size in mm by pen size in mm
  float numberOfSegments = rowSizeInMM / penSize;
  
  //max density automatically equals one, but if the number of segments is much larger, than set the max density as that
  int maxDens = 1;
  if (numberOfSegments >= 2.0)
    maxDens = int(numberOfSegments);
    
   //if the max segments is very small, print off error info 
  if (maxDens <= 2)
  {
    Serial.print("num of segments float:");
    Serial.println(numberOfSegments);
    Serial.print(F("MSG,I,Max density for penSize: "));
    Serial.print(penSize);
    Serial.print(F(", rowSize: "));
    Serial.print(rowSize);
    Serial.print(F(" is "));
    Serial.println(maxDens);
    Serial.println(F("MSG,I,You probably won't get any detail in this."));
  }
    

  //return max density
  return maxDens;
  }

  //calculate pixel scale density
  int pixel_scaleDensity(int inDens, int inMax, int outMax)
  {
    //calculate the reduced density by dividing the in density by the maximum input and multiplying it by the maximum density
    float reducedDens = (float(inDens) / float(inMax)) * float(outMax);
    
    //the reduced density is the difference between the out maximum minus the reduced density
    reducedDens = outMax-reducedDens;
    //  Serial.print(F("inDens:"));
    //  Serial.print(inDens);
    //  Serial.print(F(", inMax:"));
    //  Serial.print(inMax);
    //  Serial.print(F(", outMax:"));
    //  Serial.print(outMax);
    //  Serial.print(F(", reduced:"));
    //  Serial.println(reducedDens);
  
    // round up if bigger than .5
    int result = int(reducedDens);
    if (reducedDens - (result) > 0.5)
      result ++;
  
    return result;
   }

   //draw a suare pixel
   void pixel_drawSquarePixel(int length, int width, int density, byte drawDirection) 
   {
     // work out how wide each segment should be
     int segmentLength = 0;
     
     //if the density is large enough to actually draw a stroke
     if (density > 0)
     {
      // work out some segment widths--the width of a segment and the remainder per pixel (the white space left with each pen stroke)
      //length of a basic segment
      int basicSegLength = length / density;
      //remainder of a basic segment
      int basicSegRemainder = length % density;
      //exact remainder per segment
      float remainderPerSegment = float(basicSegRemainder) / float(density);
      float totalRemainder = 0.0;
      int lengthSoFar = 0;
    
    //while there is space left to draw in
    for (int i = 0; i <= density; i++) 
    {
      //ad up total remainder so you know how far you've gotten
      totalRemainder += remainderPerSegment;

      //if you've gone far enough that your remainder is 1, add that to segment length and keep going
      if (totalRemainder >= 1.0)
      {
        
        totalRemainder -= 1.0;
        segmentLength = basicSegLength+1;
      }
      //ptherwise, the segment length just equals the basic segment length
      else
      {
        segmentLength = basicSegLength;
      }
      
      //draw pixel in specified direction
      if (drawDirection == DIR_SE) {
        pixel_drawSquareWaveAlongAxis(motorA, motorB, width, segmentLength, density, i);
      }
      else if (drawDirection == DIR_SW) {
        pixel_drawSquareWaveAlongAxis(motorB, motorA, width, segmentLength, density, i);
      }
      else if (drawDirection == DIR_NW) {
        segmentLength = 0 - segmentLength; // reverse
        pixel_drawSquareWaveAlongAxis(motorA, motorB, width, segmentLength, density, i);
      }
      else if (drawDirection == DIR_NE) {
        segmentLength = 0 - segmentLength; // reverse
        pixel_drawSquareWaveAlongAxis(motorB, motorA, width, segmentLength, density, i);
      }
      
      //distance so far is upped so the machine knows how much farther to go
      lengthSoFar += segmentLength;
    //      Serial.print("distance so far:");
    //      Serial.print(distanceSoFar);
      
      
      reportPosition();
    } // end of loop
  }
}

//draws a square wave
void pixel_drawSquareWaveAlongAxis(AccelStepper &longAxis, AccelStepper &wideAxis, int waveAmplitude, int waveLength, int totalWaves, int waveNo)
{
  //if this is the first wave
  if (waveNo == 0) 
  { 
    // first one, if the last wave was a top just draw half a wave
    Serial.println(F("First wave half"));
    if (lastWaveWasTop) {
      moveAxis(wideAxis, waveAmplitude/2);
      moveAxis(longAxis, waveLength);
    }
    
    //otherwise start from standard position
    else {
      moveAxis(wideAxis, 0-(waveAmplitude/2));
      moveAxis(longAxis, waveLength);
    }
    //flip direction so you get both sides of the wave
    pixel_flipWaveDirection();
  }
  
  //If this is the last wave
  else if (waveNo == totalWaves) 
  { 
    // last one, half a line if the last wave was a top one
    if (lastWaveWasTop) {
      moveAxis(wideAxis, waveAmplitude/2);
    }
    //otherwise draw from the bottom of the wave
    else {
      moveAxis(wideAxis, 0-(waveAmplitude/2));
    }
  }
  
  //you're in the middle so just draw a standard wave. If the last wave was a top one, draw a bottom one, otherwise draw a top
  else 
  { 
    // intervening lines - full lines, and an along
    if (lastWaveWasTop) {
      moveAxis(wideAxis, waveAmplitude);
      moveAxis(longAxis, waveLength);
    }
    else {
      moveAxis(wideAxis, 0-waveAmplitude);
      moveAxis(longAxis, waveLength);
    }
    pixel_flipWaveDirection();
  }
}

//flips the wave direciton so you draw both sides of the waves
void pixel_flipWaveDirection()
{
  if (lastWaveWasTop)
    lastWaveWasTop = false;
  else
    lastWaveWasTop = true;
}

//tests pen width
  void pixel_testPenWidth()
  {
    //gets row width, start width, end width, and increment size from parameters
    int rowWidth = multiplier(atoi(inParam1));
    float startWidth = atof(inParam2);
    float endWidth = atof(inParam3); 
    float incSize = atof(inParam4);

    //sets a temporary direcition from the standard
    int tempDirectionMode = globalDrawDirectionMode;
    globalDrawDirectionMode = DIR_MODE_PRESET;
    
    //finds old pen width
    float oldPenWidth = penWidth;
    int iterations = 0;
    
    //tests pen width by drawing
    for (float pw = startWidth; pw <= endWidth; pw+=incSize)
    {
      iterations++;
      penWidth = pw;
      int maxDens = pixel_maxDensity(penWidth, rowWidth);
      
      //draws pixel to test width
      pixel_drawSquarePixel(rowWidth, rowWidth, maxDens, DIR_SE);
    }
    //sets pen width as the old pen width
    penWidth = oldPenWidth;
    
    //moves the axis of the line based on the motor positions
    moveAxis(motorB, 0-rowWidth);
    
    //rotates through motor moevemts to test pen width
    for (int i = 1; i <= iterations; i++)
    {
      moveAxis(motorB, 0-(rowWidth/2));
      moveAxis(motorA, 0-rowWidth);
      moveAxis(motorB, rowWidth/2);
    }
    
    //sets values based on test
    penWidth = oldPenWidth;
    globalDrawDirectionMode = tempDirectionMode;
  }
#endif

