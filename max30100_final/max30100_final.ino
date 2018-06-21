/*
Arduino-MAX30100 oximetry / heart rate integrated sensor library
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// The example shows how to retrieve raw values from the sensor
// experimenting with the most relevant configuration parameters.
// Use the "Serial Plotter" app from arduino IDE 1.6.7+ to plot the output
#define NEO_NUM 0
#include <Wire.h>
#include "MAX30100.h"
#include "MAX30100_PulseOximeter.h"
#include <SoftwareSerial.h>

// Sampling is tightly related to the dynamic range of the ADC.
// refer to the datasheet for further info
#define SAMPLING_RATE                       MAX30100_SAMPRATE_100HZ

// The LEDs currents must be set to a level that avoids clipping and maximises the
// dynamic range
#define IR_LED_CURRENT                      MAX30100_LED_CURR_50MA
#define RED_LED_CURRENT                     MAX30100_LED_CURR_27_1MA

// The pulse width of the LEDs driving determines the resolution of
// the ADC (which is a Sigma-Delta).
// set HIGHRES_MODE to true only when setting PULSE_WIDTH to MAX30100_SPC_PW_1600US_16BITS
#define PULSE_WIDTH                         MAX30100_SPC_PW_1600US_16BITS
#define HIGHRES_MODE                        true
#define REPORTING_PERIOD_MS     1000
#define SYST_B1                 0.0020795780879109033


// Instantiate a MAX30100 sensor class
MAX30100 sensor;
PulseOximeter pulse_oxim;

const int SYST_DIAS_SIZE = 10;
float input[3]; ////last 3 readings from sensors
int input_index;
bool input_init; //whether input[] has only valid values
float diastolic[SYST_DIAS_SIZE + 1], systolic[SYST_DIAS_SIZE + 1], std_dev;
int dstlc_index, systlc_index;
bool doAnalysis = true;

float global_min = 1000.0;
int global_min_count = -1;
float global_max = 0.0;
int global_max_count = -1;
float global_min_vector[5];

uint32_t tsLastReport = 0;

SoftwareSerial sw(2, 3); // RX, TX

//pressure = b1 * value + b0
//const float SYST_B1 = 0.0020795780879109033;
const float SYST_B0 = 31.818555188385247;
const float DIAS_B1 = 0.0011065852704391192;
const float DIAS_B0 = 25.293501614815213;


float getSystolic() {
  return (systolic[SYST_DIAS_SIZE] * SYST_B1 + SYST_B0) / 10;
}

float getDiastolic() {
  return (diastolic[SYST_DIAS_SIZE] * DIAS_B1 + DIAS_B0) / 10;
}

void analyseData(float val, int index) {
  
  input[input_index] = val;
  input_index = (input_index + 1) % 3;
  
  if (isLocalMax()) {
    if (isGlobalMax()) {
      //Serial.print("MAX: ");
      //Serial.println(global_max);
      addToSystolic(input[(input_index + 1) % 3]);
    }
  }
  else if (isLocalMin()) {
    if (isGlobalMin()) {
      //Serial.print("MIN: ");
      //Serial.println(global_min);
      addToDiastolic(input[(input_index + 1) % 3]);
    }
  }
}

bool isLocalMax() {
  // function may have one of the following forms:
  // 1 - /\ or ,-
  //int previous = input_index;
  int candidate = (input_index + 1) % 3;
  //int post = (candidate + 1) % 3;
  return (input[input_index] < input[candidate] &&
          input[candidate] >= input[(candidate + 1) % 3]);
}

bool isLocalMin() {
  // function may have one of the following forms:
  // 1 - \/ or \_
  //int previous = input_index;
  int candidate = (input_index + 1) % 3;
  //int post = (candidate + 1) % 3;
  return (input[input_index] > input[candidate] &&
          input[candidate] <= input[(candidate + 1) % 3]);
}

boolean isGlobalMax() {
  if (global_max_count == -1) {
    global_max = input[(input_index + 1) % 3];
    global_max_count++;
  }
  else if (global_max >= input[(input_index + 1) % 3]) {
    global_max_count++;
    if (global_max_count == 5) {
      global_max_count = -1;
      return true;
    }
  }
  else { // input > global_max
    global_max = input[(input_index + 1) % 3];
    global_max_count = 0;
  }
  return false;
}

boolean isGlobalMin() {
  if (global_min_count != -1) {
    global_min = input[(input_index + 1) % 3];
    int i = 0;

    for (; i < 5; i++) {
      if (global_min >= global_min_vector[i]) {
        break;
      }
    }

    global_min_vector[global_min_count] = global_min;
    global_min_count = (global_min_count + 1) % 5;

    if (i == 5) {
      //then it is a global minimum
      return true;
    }
    
  }
  else {
    for (int i = 0; i < 5; i++) {
      if (global_min_vector[i] == 0.0) {
        global_min_vector[i] = input[(input_index + 1) % 3];
        if (i == 4) {
          global_min_count = 0;
        }
        break;
      }
    }
  }
  
  return false;
}

void addToSystolic(float val) {
  //computes mean
  systolic[SYST_DIAS_SIZE] -= systolic[systlc_index] / SYST_DIAS_SIZE;
  systolic[SYST_DIAS_SIZE] += val / SYST_DIAS_SIZE;

  //adds to list
  systolic[systlc_index] = val;

  //updates index
  systlc_index = (systlc_index + 1) % SYST_DIAS_SIZE;

  //Serial.print("Syst: ");
  //Serial.println(systolic[SYST_DIAS_SIZE] * SYST_B1 + SYST_B0);
}

void addToDiastolic(float val) {
    //computes mean
    diastolic[SYST_DIAS_SIZE] -= diastolic[dstlc_index] / SYST_DIAS_SIZE;
    diastolic[SYST_DIAS_SIZE] += val / SYST_DIAS_SIZE;
    
    //adds to list
    diastolic[dstlc_index] = val;
    
    //updates index
    dstlc_index = (dstlc_index + 1) % SYST_DIAS_SIZE;

    //Serial.print("Dias: ");
    //Serial.println(diastolic[SYST_DIAS_SIZE] * DIAS_B1 + DIAS_B0);
}

/*void updateStdDev() {
  if (systlc_init && dstlc_init) {
    std_dev = systolic[10] - diastolic[10];
    std_dev /= 1.4142; // approx. sqrt(2)
  }
}*/

void setup()
{
    Serial.begin(115200);

    Serial.print("Initializing MAX30100..");

    // Initialize the sensor
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!sensor.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

    // Set up the wanted parameters
    sensor.setMode(MAX30100_MODE_SPO2_HR);
    sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);
    sensor.setLedsPulseWidth(PULSE_WIDTH);
    sensor.setSamplingRate(SAMPLING_RATE);
    sensor.setHighresModeEnabled(HIGHRES_MODE);


    Serial.print("Initializing pulse oximeter..");

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pulse_oxim.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

     Serial.println("Interfacfing arduino with nodemcu");
     sw.begin(115200);

    reset();
}

void reset() {
  for (int i = 0; i < 5; i++) {
    global_min_vector[i] = 0.0;
  }
  
  for (int i = 0; i < SYST_DIAS_SIZE + 1; i++) {
    //stores the last 10 pulse beats/pressure
    diastolic[i] = 0.0;
    systolic[i] = 1023.0;
    //last value (11th) is the mean
  }
  dstlc_index = 0;
  systlc_index = 0;
  std_dev = 0; //standard deviation
  
  for (int i = 0; i < 3; i++) {
    input[i] = -1.0; //last 3 readings from sensors
  }
  input_index = 0;
  input_init = false;
  
}

void readNoise(int& signalMax, int& signalMin){
    int sample = analogRead(A0);
    
    if (sample > signalMax)
    {
      signalMax = sample;  // save just the max levels
    }
    else if (sample < signalMin)
    {
      signalMin = sample;  // save just the min levels
    }
}

 int signalMin = 1024;
    int signalMax = 0;

//double readNoise(){
//  unsigned long startMillis= millis();  // Start of sample window
//  
//  int sample;
//  int signalMax = 0;
//  int signalMin = 1024;
//  
//  // collect data for 50 mS
//  while (millis() - startMillis < 1000){
//     sample = analogRead(A0);
//     if (sample > signalMax)
//        signalMax = sample;
//     else if (sample < signalMin)
//        signalMin = sample;
//  }
//  Serial.print("MAX: "+String(signalMax)+"\tMIN: "+String(signalMin)+"\tDB:");
//  return 20.0*log10(signalMax - signalMin);  // max - min = peak-peak amplitude
//}
void loop()
{
    uint16_t ir, red;

    sensor.update();
    pulse_oxim.update();

    while (sensor.getRawValues(&ir, &red)) {
        /*Serial.print(ir);
        Serial.print('\t');
        Serial.println(red);*/
        analyseData(ir, 0);
        //analyseData(red, 1);
    }

   readNoise(signalMax,signalMin);
    
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        
        Serial.print("Blood pressure: ");
        Serial.print(getSystolic());
        Serial.print("/");
        Serial.print(getDiastolic());
        Serial.print("\tHeart rate: ");
        Serial.print(pulse_oxim.getHeartRate());
        Serial.print("bpm \t SpO2: ");
        Serial.print(pulse_oxim.getSpO2());
        Serial.println("%");

        sw.print("alarm=");
        sw.print(0);
        sw.print("&num=");
        sw.print(NEO_NUM);
        sw.print("&syst=");
        sw.print(getSystolic());
        sw.print("&dias=");
        sw.print(getDiastolic());
        sw.print("&bpm=");
        sw.print(pulse_oxim.getHeartRate());
        sw.print("&spo2=");
        sw.print(pulse_oxim.getSpO2());
        sw.print("&noise=");
        sw.print(20.0*log10(signalMax - signalMin));
        sw.print("&temp=");
        sw.print((analogRead(A1)*500.0)/1024);
        sw.print("\r");
        
        //double ppp=signalMax - signalMin;
        //Serial.println("WTF:"+String(analogRead(A0))+"\tMAX: "+String(signalMax)+"\tMIN: "+String(signalMin)+"\tDB: "+String(20.0*log10(ppp)));
        signalMin = 1024;
        signalMax = 0;
        
        tsLastReport = millis();
    }
    
    
}
