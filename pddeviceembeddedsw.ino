/**************************************************************************/
/*!
    @file     Adafruit_MMA8451.h
    @author   G Tidanian

*/
/**************************************************************************/

#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#define COMMON_ANNODE

Adafruit_MMA8451 mma = Adafruit_MMA8451();

bool isHit = false;
bool ledOff;
unsigned long hitStart = 0;
unsigned long ledOntime = 0;
unsigned long ledOfftime = 0;
unsigned long fetchTimer = 0;
float sum = 0;
float MS = 0; //mean square
float patternList[50] = {0};
float a_y=0;
float a_z;
float break_rms=0.341; //value of RMS to stop cueing
float mod_rms=0.511;//value of RMS to correspond with moderate gait
int count = 0;
int redPin = 11;
int greenPin = 10;
int bluePin = 9;
int inPin = 7;
int buzzer = 8;
int T = 1; //Time period measurement
int n = 50; //number of values in RMS calculation


void setup(void) {
  Serial.begin(9600);
  
  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }

  mma.setRange(MMA8451_RANGE_8_G);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(inPin, INPUT);
  pinMode(buzzer, OUTPUT);

}

//Function to set colour of RGB LED
void setColour(int red, int green, int blue) {
  
  
 #ifdef COMMON_ANNODE
  
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
 #endif
 analogWrite(redPin, red);
 analogWrite(greenPin, green);
 analogWrite(bluePin, blue);
}
  



void loop() {

  MS = 0;
  sum = 0;
  
  if(millis() - fetchTimer >= 20){
    mma.read();
    sensors_event_t event; 
    mma.getEvent(&event);
    if(event.acceleration.y-10.01<5){
      a_y = event.acceleration.y-10.01;
    }
    else{
      a_y = a_y; //ignore values of acceleration above the threshold of 5 m/s^2
    }
    
    a_y = event.acceleration.y - 10.01; //remove the component of gravity from the vertical acceleration
    a_z = event.acceleration.z;
    Serial.println(a_y);
    patternList[count] = a_y;
    count = count + 1;
    fetchTimer = millis();
  }
  
  if(count == n-1){
    for (int i=0; i<n-1; i++){
      sum += patternList[i]; //calculate sum of values
    }
    float avg = sum / n; //calculate mean acceleration
    for (int i=0; i<n-1; i++) {
      MS += sq(patternList[i] - avg)*0.02; //calculate mean square
    }
    float rms = sqrt(MS/T);
    count = count - 1;
    memmove(&patternList[0], &patternList[1], (n)*sizeof(patternList[0])); //shift the array one value to the left
  }
  
  if(abs(a_z) > 7 and isHit == false){ //if device is hit
    isHit = true;
    hitStart = millis();
    ledOff = true;
    ledOfftime = millis();
  }
  
  if(isHit == true and millis() - hitStart <= 10000 and rms < break_rms and digitalRead(inPin)==LOW){ //if switch is in visual cue mode
    if(ledOff == true and millis() - ledOntime > 500){
      setColour(0, 225, 255);
      ledOfftime = millis();
      ledOff = false;
      }
    if(ledOff == false and millis()-ledOfftime >500){
      setColour(0, 0, 0);
      ledOntime = millis();
      ledOff = true;
      }
  }
  if(isHit == true and millis() - hitStart <= 10000 and rms < break_rms and digitalRead(inPin)==HIGH){ //if switch is in auditory cue mode
    if(ledOff == true and millis() - ledOntime > 500){
      tone(buzzer, 1000); // Send 1KHz sound signal...
      ledOfftime = millis();
      ledOff = false;
      //Serial.println("on");
    }
    if(ledOff == false and millis()-ledOfftime >500){
      noTone(buzzer);
      ledOntime = millis();
      ledOff = true;
    }
   }

  if(millis() - hitStart > 10000 or rms >= break_rms and digitalRead(inPin == LOW)){ //stop cue if walking commences
    isHit = false;
    setColour(0, 0, 255);
  }
  if(millis() - hitStart > 10000 or rms >= break_rms and digitalRead(inPin == HIGH)){
    isHit = false;
     noTone(buzzer);
  }
  
  if(isHit == false){
    if (rms < break_rms) {
      setColour(0, 0, 255); //set RGB LED red if stationary movement is identified
    }
    
    else if (rms >= mod_rms){
      setColour(0, 225, 0); //set RGB LED green if moderate walking is identified
    }
    
    else {
      setColour(0, 225, 255); //set RGB LED amber if slow walking is identified
    }   
  }
  }
  





  


