
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
bool flag = true;
void loop(){
 
  value = analogRead(flexPin); //Read and save analog value from potentiometer
  if (value>1000)
  {
    //time1=millis();
    if (clicks == false)
    {
      //Serial.println("one_click");
      clicks = true;
   
     time1=millis();
     //Serial.println(time1);
     //Serial.println(millis()-time1);
   }
    else if(millis()-time1 > 300)
    { 
       // Serial.print("else:");
        //Serial.println(millis());
        
        if(millis()-time1<500 && flag == true) 
        {
         // Serial.print("check:"); Serial.println(value);
          Serial.println("Double_click");
          clicks = false;
          flag = false;
        }
        else 
        {
          //Serial.println("SingleClick");
          clicks = false;
          //flag = true;
          
        }
    
    }
  }

  if(millis()-time1 > 600 && flag == false)
{                                                                                                                                                                                                                                                                                                                                                                                       
  flag =true;
}
  if(millis()-time1 > 500 &&clicks==true)
  {
 // flag =true;
    clicks = false;
    Serial.println("SingleClick");
  }
  /*if (value<1000 && clicks == true)
  {
    back=true;
  }
 */

  
  //Serial.println(value);               //Print value
  //value = map(value, 700, 900, 0, 255);//Map value 0-1023 to 0-255 (PWM)
  //analogWrite(ledPin, value);          //Send PWM value to led
  delay(100);                          //Small delay
  
  
}


