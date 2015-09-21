#include <AccelStepper.h>
#include <Makeblock.h>
#include <SoftwareSerial.h>
#include <Wire.h>

//Me_LimitSwitch module can only be connected to PORT_3, PORT_4, PORT_6,PORT_7,PORT_8 of base shield or from PORT_3 to PORT_8 of baseboard.
MeLimitSwitch limitSwitch1(PORT_4, SLOT2); 
MeLimitSwitch limitSwitch2(PORT_4, SLOT1); 
MeLineFollower lineFinder(PORT_6);
MeDCMotor vaccum(M1);
MeDCMotor valve(M2);

//Rotating axel
int dir_rot_Pin = mePort[PORT_2].s1;//the direction pin connect to Base Board PORT1 SLOT1
int stp_rot_Pin = mePort[PORT_2].s2;//the Step pin connect to Base Board PORT1 SLOT2
AccelStepper rotStepper(AccelStepper::DRIVER,stp_rot_Pin,dir_rot_Pin); 

//Rotating axel
int dir_arm_Pin = mePort[PORT_1].s1;//the direction pin connect to Base Board PORT1 SLOT1
int stp_arm_Pin = mePort[PORT_1].s2;//the Step pin connect to Base Board PORT1 SLOT2
AccelStepper armStepper(AccelStepper::DRIVER,stp_arm_Pin,dir_arm_Pin); 

void setup()
{
    Serial.begin(9600);
    Serial.println("Start.");
    rotStepper.setMaxSpeed(10000);
    rotStepper.setCurrentPosition(0);
    armStepper.setMaxSpeed(10000);
    armStepper.setCurrentPosition(0);
}
void loop()
{
   static int seq = 0;
   static bool once = false;
   int sensorState = lineFinder.readSensors();
   int color = 0;

   Serial.print("Sensor: ");
   Serial.println(sensorState);
  
   if(limitSwitch1.touched()) //If the limit switch is touched, the  return value is true.
   {
     Serial.println("State1: DOWN.");
     delay(1);
   }
   if(!limitSwitch1.touched()){
     Serial.println("State1: UP.");
     delay(1);
   }

   if(limitSwitch2.touched()) //If the limit switch is touched, the  return value is true.
   {
     Serial.println("State2: DOWN.");
     delay(1);
   }
   if(!limitSwitch2.touched()){
     Serial.println("State2: UP.");
     delay(1);
   }
  
  if(!once){
    rotStepper.setSpeed(500);
    rotStepper.setAcceleration(100);
    armStepper.setSpeed(500);
    armStepper.setAcceleration(100);

    armStepper.moveTo(-10000);
    
    while (armStepper.currentPosition() != -10000 && !limitSwitch2.touched()){ // Full speed basck to 0
      armStepper.run();
    }
    armStepper.stop();
    armStepper.setCurrentPosition(0);
    delay(10);
    armStepper.moveTo(100);
    
    while (armStepper.currentPosition() != 100){ // Full speed basck to 0
      armStepper.run();
    }
    
    rotStepper.moveTo(10000);
    
    while (rotStepper.currentPosition() != 10000 && !limitSwitch1.touched()){ // Full speed basck to 0
      rotStepper.run();
    }
    rotStepper.stop();
    rotStepper.setCurrentPosition(0);
    delay(10);
    rotStepper.moveTo(-100);
    
    while (rotStepper.currentPosition() != -100){ // Full speed basck to 0
      rotStepper.run();
    }
  }

 /*
  if(once){

    // Go to homepose
    allAxisToPosition(-2350, 800, 2500, 1000);

    //Pick new part
    suction(true);
    
    //Check that part i suction cup
    
    //Move to color sensor

    //Check color

    //Drop part at right color
    if(color > 11){
      // Yellow part
      suction(false);
    }else if(color > 22){
      // red part
      suction(false);
    }else{
      // bad part
      suction(false);
    }

    
    allAxisToPosition(-2350, 2700, 2500, 1000);
    suction(true);
    allAxisToPosition(-2350, 800, 2500, 1000);
    allAxisToPosition(-4400, 800, 2500, 1000);
    allAxisToPosition(-4400, 1870, 2500, 1000);
    suction(false);
    allAxisToPosition(-4400, 800, 2500, 1000);
    allAxisToPosition(-2350, 800, 2500, 1000);
   
  }
  */

  //255
  delay(3000);

  once = true;
}

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

void suction(bool state){
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





