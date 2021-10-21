#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal.h>
#include <Wire.h>

//Create LiquidCrystal Object. Pin parameters are (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd = LiquidCrystal(4, 6, 10, 11, 12, 13);

//I/0 Setup + Global Variables
const int stepUpPin = 7;
const int stepDownPin = 8;
const int pausePin = 9;
const int accelAddr = 0x68;
int stepCt = 1;
float distance = 0;
float paceCount = 0;
float distPerStep = 0;



void setup() {

  //initialize serial monitor
  Serial.begin(74880);

  //declare pin states
  pinMode(stepUpPin, INPUT);
  pinMode(stepDownPin, INPUT);
  pinMode(pausePin, INPUT);
  
  //wakeup IMU
  Wire.begin();
  Wire.beginTransmission(accelAddr);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  //configure accelerometer sensitivity
  Wire.beginTransmission(accelAddr);
  Wire.write(0x1C);
  Wire.write(0x01);
  Wire.endTransmission(true);

  delay(20);
 
  //initialize LCD
  lcd.begin(16,2);
  
  //declare variables
  bool prevStateUp = LOW;    // Experimental button press detection in case simple method breaks
  bool prevStateDown = LOW;
  bool prevStatePause = LOW;

  //logic for finding paceCount and distPerStep
   
    //Base LCD stuff
    lcd.setCursor(1,0);
    lcd.print("Enter 10 Meter ");
    lcd.setCursor(1,1);
    lcd.print("Pace Count:");
    

    //paceCount stuff
    while (digitalRead(pausePin) == LOW){     //Pause Code until all data is input and button pressed to continue


        //Method for detecting button presses
        bool currStateUp = digitalRead(stepUpPin);
        if(currStateUp != prevStateUp){
           //transition has happened
           if(currStateUp == HIGH){
              //button is pressed
              paceCount ++;
              //update LCD
              lcd.setCursor(13,1);
              lcd.print("   ");
              lcd.setCursor(13,1);
              lcd.print((int) paceCount);
              delay(200); 
           }
           prevStateUp = currStateUp; 
        }
            bool currStateDown = digitalRead(stepDownPin);
            if(currStateDown != prevStateDown){
              //transition has happened
              if(currStateDown == HIGH){
                 //button is pressed
                 if(paceCount == 0){
                 }
                 else{
                  paceCount --;
                 }
                  //update LCD
                  lcd.setCursor(13,1);
                  lcd.print("   ");
                  lcd.setCursor(13,1);
                  lcd.print((int) paceCount);
                  delay(200); 
           }
           prevStateDown = currStateDown; 
        }         

    }

    distPerStep = paceCount/10;
    
    // need to check if this number is negative and if so return to start of protocols
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Steps: 0");
    lcd.setCursor(0,1);
    lcd.print("Dist(m): 0");
}

void loop() {

//  Serial.print("Distance: ");
//  Serial.println(distance);
//  Serial.print("Step Count: ");
//  Serial.println(stepCt);
  
  //declare variables
  int loopCt = 1;  
  float accelX = 0;
  float accelY = 0;
  float accelZ = 0;
  float accelTot = 0;
  float accelAvg = 0;
  
  //get accelerations 
  Wire.beginTransmission(accelAddr); // Begin communication with accelerometer
  Wire.write(0x3B);                // This starts our transmission at the first register for the accelerometer data <<X1>>
  Wire.endTransmission(false);     // This makes the slave device (accelerometer) restart after every data transfer keeping under the arduino's control
  Wire.requestFrom(accelAddr, 6, true); //This pulls all of our data from each of the registers that the accelerometer data is stored in, then releases control of the accelerometer

  /*manipulate the acceleration values to get them to work
  Here, because the I2C protocol limits the data transfer by byte, and because the accelerometer data is comprised of a 16-bit value, I have to pull two seperate
  data packets and combine them to get the full value properly (this is why there are six registers for only three directional measurements of acceleration). 
  I combine the data by first taking the first half and making it a 16-bit number with all zeros in the rightmost byte (<< 8 command), then I pass that into an OR condition
  with the second half of the 16-bit number to effectively "splice" the data together. The division at the end is a scaling value provided by the accelerometer datasheet 
  for configuring the sensitivity of the accelerometer. The datasheet can be found here: https://cdn.sparkfun.com/datasheets/Sensors/Accelerometers/RM-MPU-6000A.pdf
  */
  
  accelX = (Wire.read() << 8 | Wire.read()) / 4096.0;
  accelY = (Wire.read() << 8 | Wire.read()) / 4096.0;   //8192.0
  accelZ = (Wire.read() << 8 | Wire.read()) / 4096.0;

//  Serial.print("AccelX: ");
//  Serial.println(accelX);
//  Serial.print("AccelY: ");
//  Serial.println(accelY);
//  Serial.print("AccelZ: ");
//  Serial.println(accelZ);

  
  //logic for pedometer and ranger
  accelTot = sqrt((accelX*accelX)+(accelY*accelY)+(accelZ*accelZ));
  accelAvg = (accelAvg + accelTot)/loopCt;
  
//  if (stepCt == 1){
//    if (accelTot > 1.5*accelAvg){
//      stepCt ++;
//      distance = distance + (stepCt * distPerStep);
//    }
//  }

//  Serial.print("AccelTot: ");
  Serial.println(accelTot);
  Serial.print("\t");
//  Serial.print("AccelAvg: ");
//  Serial.println(accelAvg);

  if (accelTot > 10){
    stepCt ++;
    distance = (stepCt * distPerStep);
    delay(200);
  }

 //LCD Stuff
    lcd.setCursor(7,0);
    lcd.print(stepCt);
    lcd.setCursor(9,1);
    lcd.print(distance);

 //Increment Loop Count
    loopCt++;

}
