
/* How to use a flex sensor/resistro - Arduino Tutorial
   Fade an LED with a flex sensor
   More info: http://www.ardumotive.com/how-to-use-a-flex-sensor-en.html
   Dev: Michalis Vasilakis // Date: 9/7/2015 // www.ardumotive.com  */

//#include "CurieTimerOne.h"

//Constants:
//const int ledPin = 3;   //pin 3 has PWM funtion
const int flexPin = A0; //pin A0 to read analog input
//unsigned long interval=200;
unsigned long time1;
bool clicks = false;
bool back = false;


//Variables:
int value; //save analog value


void setup(){
  
  //pinMode(ledPin, OUTPUT);  //Set pin 3 as 'output'
  Serial.begin(9600);       //Begin serial communication

}
value = analogRead(flexPin); //Read and save analog value from potentiometer
   //Serial.println(value);
  if(value>1000)
  {
    if(clicks == false && millis()-time1 >200)
    {
      clicks=true;
      time1 = millis();
    }

    else
    {
      if( back && millis()-time1<700)
      {
          clicks = false;
          back = false;
          Serial.println("Double_Click"); 
          time1 = millis();
      }

      if( !back && millis()-time1>700 && hold==false)
      {
          //clicks = false;
          hold=true;
          Serial.println("Click_Hold");             
      }
    }
  }

  if(clicks && value <940 && hold==false)
  {
      back =true;
  }



  if(clicks && millis()-time1>700 && back )    
      {
        clicks=false;
        back = false;
        Serial.println("Single_Click");
      }

  if(hold == true && value<940)   
      {
          hold = false;
          clicks= false;
          Serial.println("Hold_Release");
      }
  
  //Serial.println(value);               //Print value
  //value = map(value, 700, 900, 0, 255);//Map value 0-1023 to 0-255 (PWM)
  //analogWrite(ledPin, value);          //Send PWM value to led
  delay(100);                          //Small delay
  
  
}


