//#include <EEPROM.h>
//#include <Arduino.h>  // for type definitions


//this class converts things into eeprom values so they can be overwritten by users in the driver GUI
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{   
  //takes in any value, works through it character by character, writing each as eeprom
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

//reads eeprom values 
template <class T> int EEPROM_readAnything(int ee, T& value)
{
   //takes an eeprom value, works through it character by character, reading each one
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}
