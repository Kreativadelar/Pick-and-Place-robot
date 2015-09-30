/*
**   _   __               _   _                _      _            
**  | | / /              | | (_)              | |    | |           
**  | |/ / _ __ ___  __ _| |_ ___   ____ _  __| | ___| | __ _ _ __ 
**  |    \| '__/ _ \/ _` | __| \ \ / / _` |/ _` |/ _ \ |/ _` | '__|
**  | |\  \ | |  __/ (_| | |_| |\ V / (_| | (_| |  __/ | (_| | |   
**  \_| \_/_|  \___|\__,_|\__|_| \_/ \__,_|\__,_|\___|_|\__,_|_|   
**                                                                 
**
**  Pick and place robot using Makeblock modules
**  from Kreativadelar.se, Sweden
**
**  This code is written "quick & dirty" and should not be as a guide
**  in how to program an Arduino. Feel free to change code as you like
**  and share with your friends.
**
**  If you want to share your code changes, please e-mail them to
**  info@kreativadelar.se and we will put them on our web for other
**  customers to download.
**
**  (C) Kreativadelar.se 2015, Sweden, Patrik
**  http://www.kreativadelar.se
**
**  To use this code you need the following libraries: 
**  
**  AccelStepper which can be
**  downloaded free from http://www.airspayce.com/mikem/arduino/AccelStepper/
**  
**  Makeblock Library which can be  
**  downloaded free from https://github.com/Makeblock-official/Makeblock-Library/archive/master.zip
**
**  Version 1.0, Initial public release, July 2015
**
**  This example code is in the public domain.
**
*/

#include <AccelStepper.h>
#include <Makeblock.h>
#include <SoftwareSerial.h>
#include <Wire.h>

MeLimitSwitch limitSwitch1(PORT_4, SLOT2); 
MeLimitSwitch limitSwitch2(PORT_4, SLOT1); 
MeLineFollower lineFinder(PORT_3);
MeLightSensor lightSensor(PORT_6);
MeDCMotor vaccum(M1);
MeDCMotor valve(M2);

//Rotating axis
int dir_rot_Pin = mePort[PORT_2].s1;    //the direction pin connect to Base Board PORT2 SLOT1
int stp_rot_Pin = mePort[PORT_2].s2;    //the Step pin connect to Base Board PORT2 SLOT2
AccelStepper rotStepper(AccelStepper::DRIVER,stp_rot_Pin,dir_rot_Pin); 

//Arm axis
int dir_arm_Pin = mePort[PORT_1].s1;    //the direction pin connect to Base Board PORT1 SLOT1
int stp_arm_Pin = mePort[PORT_1].s2;    //the Step pin connect to Base Board PORT1 SLOT2
AccelStepper armStepper(AccelStepper::DRIVER,stp_arm_Pin,dir_arm_Pin); 

/********************************************
 * Setup 
 ********************************************/
void setup()
{
  delay(1000);
  
  Serial.begin(9600);
  Serial.println("Starting up...");
  
  rotStepper.setMaxSpeed(10000);
  rotStepper.setCurrentPosition(0);
  armStepper.setMaxSpeed(10000);
  armStepper.setCurrentPosition(0);

  Serial.println("System is started");
}


/********************************************
 * Loop
 ********************************************/
void loop()
{
   static int seq = 0;
   static bool once = false;
   int sensorState = lineFinder.readSensors();
   int color = 0;
   int count = 0;
   const int limit = 2;

   Serial.print("Sensor: ");
   Serial.println(sensorState);
   
  if(!once){
    syncingAllAxis();
    once = true;
  }

  if(once){

    // Go to homepose
    Serial.println("Going to home position"); 
    allAxisToPosition(-2300, 800, 2500, 1000);

    //Pick new part
    Serial.println("Picking up new part");
  
    for(count = 0; count < limit; count++){
      allAxisToPosition(-2300, 1890, 2500, 1000);
      suction(true);

      //Move to pick up sensor
      Serial.println("Moveing to pick up sensor");
      allAxisToPosition(-2300, 1600, 2500, 1000);
      delay(500);
      
      //Check that part i suction cup
      Serial.println("Check that we got a part");
      sensorState = lineFinder.readSensors();
      Serial.print("Value");
      Serial.println(sensorState);

      if(sensorState>0){
        allAxisToPosition(-2300, 800, 2500, 1000);
        count = 0;
        break;
      }else{
        allAxisToPosition(-2300, 1600, 2500, 1000);
      }
    }

    // Check for error
    if(count > 0){
      bool showError = false;
      while(true){
        if(!showError){
          Serial.println("------------ ERROR ------------");
          Serial.println("Error no parts in storage!");
          Serial.println("-------------------------------");
          showError = true;
          suction(false);
        }
      }
      delay(200);
    }
    
    //Move to color sensor
    Serial.println("Going to color sensor");
    allAxisToPosition(-5200, 800, 2500, 1000);
    allAxisToPosition(-5200, 1800, 2500, 1000);

    //Check color
    Serial.println("Checking color");
    for(int i=0; i<1; i++){
      color = checkColor();
    }
    allAxisToPosition(-5200, 800, 2500, 1000);
    
    //Drop part at right color
    if(color > 900 && color < 1000){
      // Yellow part
      Serial.println("Placing yellow part");
      allAxisToPosition(-4500, 800, 2500, 1000);
      allAxisToPosition(-4500, 2200, 2500, 1000);
      suction(false);
      allAxisToPosition(-4500, 800, 2500, 1000);
    }else if(color > 300 && color < 610){
      // Red part
      Serial.println("Placing red part");
      allAxisToPosition(-4000, 800, 2500, 1000);
      allAxisToPosition(-4000, 2200, 2500, 1000);
      suction(false);
      allAxisToPosition(-4000, 800, 2500, 1000);
    }else{
      // Bad part
      Serial.println("----------- WARNING -----------");
      Serial.println("Placing bad part!");
      Serial.println("-------------------------------");
      allAxisToPosition(-1600, 800, 2500, 1000);
      allAxisToPosition(-1600, 1600, 2500, 1000);
      suction(false);
      allAxisToPosition(-1600, 800, 2500, 1000);
    }
  }
}

/********************************************
 * Function to move all axis
 ********************************************
 * Set new position for both base and arm 
 * axis. 
 * Also set speed and accel.
 ********************************************/
void allAxisToPosition(long _position1, long _position2, float _speed, float _acc){
  rotStepper.setSpeed(_speed);
  armStepper.setSpeed(_speed);
  rotStepper.setAcceleration(_acc);
  armStepper.setAcceleration(_acc);

  rotStepper.moveTo(_position1);
  armStepper.moveTo(_position2);
    
  while (rotStepper.currentPosition() != _position1 || armStepper.currentPosition() != _position2){ // Full speed basck to 0
    
    if(rotStepper.currentPosition() != _position1)
      rotStepper.run();
      
    if(armStepper.currentPosition() != _position2)
      armStepper.run();
  }

}

/********************************************
 * Function to sync all axis
 ********************************************/
void syncingAllAxis(){
  syncingAxis(2);
  syncingAxis(1);
}

/********************************************
 * Function to sync a specific axis
 ********************************************/
void syncingAxis(int axis){
  rotStepper.setSpeed(200);
  rotStepper.setAcceleration(100);
  armStepper.setSpeed(200);
  armStepper.setAcceleration(100);

  
  if(axis == 1){
    Serial.println("----------- WARNING -----------");
    Serial.println("Syncing axis 1");
    Serial.println("-------------------------------");
    rotStepper.moveTo(10000);
  
    while (rotStepper.currentPosition() != 10000 && !limitSwitch1.touched()){ 
      rotStepper.run();
    }
    rotStepper.stop();
    rotStepper.setCurrentPosition(0);
    delay(10);
    rotStepper.moveTo(-100);
    
    while (rotStepper.currentPosition() != -100){
      rotStepper.run();
    }

    Serial.println("Syncing for axis 1 is done");
  }else if(axis == 2){
    Serial.println("----------- WARNING -----------");
    Serial.println("Syncing axis 2");
    Serial.println("-------------------------------");
    armStepper.moveTo(-10000);
  
    while (armStepper.currentPosition() != -10000 && !limitSwitch2.touched()){
      armStepper.run();
    }
    armStepper.stop();
    armStepper.setCurrentPosition(0);
    delay(10);
    armStepper.moveTo(100);
    
    while (armStepper.currentPosition() != 100){
      armStepper.run();
    }
    
    Serial.println("Syncing for axis 2 is done");
  }
}

/********************************************
 * Function to activate suction
 ********************************************/
void suction(bool state){

  delay(100);
  
  if(state){
    valve.stop();
    vaccum.run(255);
    delay(500);
    
  }else{
    vaccum.stop();
    valve.run(255);
    delay(500);
    valve.stop();
  }
}

/********************************************
 * Function to get color value from sensor
 ********************************************/
int checkColor(){

  lightSensor.lightOn();

  delay(500);
  
  int value = lightSensor.read();  

  // print the results to the serial monitor:
  Serial.print("value = " );                                            
  Serial.println(value); 

  lightSensor.lightOff();

  return value;
}





