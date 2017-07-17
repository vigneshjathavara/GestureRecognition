
/* How to use a flex sensor/resistro - Arduino Tutorial
   Fade an LED with a flex sensor
   More info: http://www.ardumotive.com/how-to-use-a-flex-sensor-en.html
   Dev: Michalis Vasilakis // Date: 9/7/2015 // www.ardumotive.com  */                                                                                                                                                                                                                  
   

//Constants:
const int flexPin0 = A0; //pin A0 to read analog input
const int flexPin1 = A1;

//Variables:
int value0;//save analog value
int value1;//save analog value


void setup(){
  
  Serial.begin(9600);       //Begin serial communication

}
bool back = false;
bool back1 = false;
bool hold = false;
bool clicks = false;
bool clicks1 = false;
unsigned long time1;
void loop(){
  
  value0 = analogRead(flexPin0); //Read and save analog value from potentiometer
  value1 = analogRead(flexPin1);
/*  if (value0 >= 1000 && value1 >= 850)
{
  Serial.print("Zoom_out");
}
*/
 if(value0 > 1000 && value1 > 890)
  {
    if(clicks == false && millis()-time1 >200)
    {
      clicks=true;
      time1 = millis();
  
    }
  }
 else  
 if(value0 <890 && value1 <780)
 {
  if(clicks1 == false && millis()-time1 >200)
  { clicks1 = true;
    time1 = millis();   
 }
 }



  if(clicks && value0 <950 && value0>890 && value1 <810 && value1>780)
  {
      back =true;
      int a = value0;
      int b = value1;
  }

  if (clicks1 && value0 >=980 && value1 >= 840)
  {
    back1 =true;
    
  }



  if(clicks && millis()-time1>400 && back )    
      {
        clicks=false;
        back = false;
        Serial.println("ZOOM_IN");
      }     
  if (clicks1 && millis()-time1>500 && back1)
   {
    clicks1 = false;
    back1 = false;
    Serial.println("ZOOM_OUT");   
   }
   

  

  
  
 // Serial.println(value1);               //Print value
  //Serial.println(value0); 

  delay(100);                          //Small delay
  
}



