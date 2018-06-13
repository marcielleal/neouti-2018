/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/

const int sampleWindow = 10; // Sample window width in mS (50 mS = 20Hz)
int sample;
int lm35;

void setup() 
{
   Serial.begin(115200);
}

void loop() 
{
   long startMillis= millis();  // Start of sample window
   int peakToPeak = 0;   // peak-to-peak level

   int signalMax = 0;
   int signalMin = 1024;

   //sample = 0;

   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(A0);
      //if (sample < 1024 && sample >= 0)  // toss out spurious readings
      //{
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      //}
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * 5.0) / 4096;  // convert to volts
   double db = 20*log10(peakToPeak);
   Serial.println(String(sample) +"/v" + String(volts) + "/" + String(peakToPeak)+ "\t" +String(db));
   delay(100);
//   lm35 = analogRead(A0);
//   lm35 = 1000.0*lm35/1024.0;
//   lm35 = lm35/0.3125;
//
//   Serial.println(lm35);
}
