// Arduino Nano program to read input signals from tactile robot arm sensors
// Research project by Stephanie Bruce
// Two mulpitplexors are used to read up to 16 analogue input signals
// This program polls the multiplexors to read the 16 analogue input signals through 2 Arduino analogue input pins
// Additional input signals could easily be added by increasing the number of multiplexors.
// Each muliplexor reads up to 8 analgue signals, and requires 1 analogue input on the Arduino board.
// Three dignial outputs are needed to control the muliplexor, but can be shared across multiple multiplexors.
// By selecting a multiplexor channel, that channel on each multiplexor is set as the control channels are wired in parallel.
#include <stdarg.h>
// Set up variables
int Avg_count = 5; // Set the number of times to take a reading for calcualting an average (filtered) value
float Sensor_array[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Create an array for holding the analgue input reading for each sensors. Mapped to muliplexor channel.
float Results[5][16]; //Create a two dimentional array for 5 sets of readings from 16 sensors. 5 is the number used to average the readings. This can be changed.
float Calibration_offset[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Create an array for the offset calculated to calibrate each sensor.
float MUX_results[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Create an array for reading a signal from each multiplexor channel. Set each initial value to be zero.
float Sensor_average[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Create and array for filtering the sensor signals.
float Sensor_percent[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Create an array for the scaled sensor reading.
float Scaled_result[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Create an array allowing the results to be scalled for improved sensitivity
float Scale_factor = 1.8; // Set a factor to scale the results to increase sensitivity. Increased from 1.8
int count = 0; // Set a counter to track the number of times that readings have been taken for averaging (filtering)
int a = 0; // Set a variable for looping Avg_count times.
int i = 0; //Set a variable for reading each sensor
int j = 0; //Set a variable for reading from the second multiplexor
int k = 19; //To count the number of readings for printing headings
int l = 0; //Set a variable for looping through the 16 readings without affecting the value of i
int integer_result = 0; //Set a variable for converting a floating result to an integer
bool bit_A = LOW; // Mulitplexor control bit A - initialise mulitplexor control variables as boolean and set to LOW
bool bit_B = LOW; // Mulitplexor control bit A - initialise mulitplexor control variables as boolean and set to LOW
bool bit_C = LOW; // Mulitplexor control bit A - initialise mulitplexor control variables as boolean and set to LOW
char buff[128]; //Set a character variable for printing results

void setup() {
  Serial.begin(9600); // Set the baud rate for communicating with the Arduino
  
  // Write an initialisation heading
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("Tactile Sensor Robot Project - Stepahnie Bruce");
  Serial.println("Reading the sensors");
  
  // Set digital output pins to control the muliplxor channel
  pinMode(3,OUTPUT);  // Set pin 3 as a digital output. Mulitplexor channel control bit A
  pinMode(4,OUTPUT);  // Set pin 4 as a digital output. Multiplexor channel control bit B
  pinMode(6,OUTPUT);  // Set pin 6 as a digital output. Multiplexor channel control bit C

  Serial.println("Starting.....");
  Serial.println();
  Serial.println("About to perform calibration - do not touch sensors...");
  delay(2000); //Wait 2 seconds to allow the sensors to be in a passive state
  Serial.println("Performing calibration...");
  take_5_readings(); // Call the routine to take a series of readings from each sensor and store them into a 2 dimensional array for averaging.
  calibrate(); //Call the routine to use the readings to calibrate each sensor in a passive state.
  Serial.println();
  Serial.println("Calibration complete. Please use the sensors.");
}

void loop(){
  take_5_readings();
  calculate_average();
  scale_readings(); 
  print_results();
  delay(10); 
}

void take_5_readings(){ // A routine to take 5 consecutive readings for each of 16 sensors and store the results in a two dimensional array.
  for(a==0;a<=(Avg_count-1);a++){ // Increament from 1 to 5 for each set of readings.
    read_sensors(); // Call the routine to take a set of readings
    for (i=0;i<=15;i++){ // Store the readings from one cycle through the muliplexors into the array. Do this for each value of a.
      Results[a][i]=Sensor_array[i];
    }
    i=0; //Ensure that variable i is zero for its next use.
  }
  a=0;  //Ensure that variable a it zero for its next use
}

void read_sensors() {
  // This is a routine to read each of the sensors and put the result into an array called MUX_results
  // Read each of 8 input signals from each multiplexor
  for (i==0;i<=7;i++) { // Run a loop 8 times, with i starting at 0
    j=i+8;       // Set the index for the 2nd multiplexor for an 8 channel multiplexor. Increase the offset to 16 if 16 channel.
    // Set the control bits by calculating the binary number corresponding to each value of i (mulitplexor channel)

    // Multiplexor control bit A
    // If i is odd, set control bit_A to HIGH
    int A=i % 2; // Find the remainder of i/2. If the remainder is 1, i is odd.
    if (A==1) bit_A=HIGH;
    else bit_A=LOW;
    
    // Multiplexor control bit B
    int B=i % 4; // Find the remainder of i/4.
    if (B>1) bit_B=HIGH;
    else bit_B=LOW;
    
    // Multiplexor control bit C
    if (i>3) bit_C=HIGH; // If i>3, set control bit_C to HIGH
    else bit_C=LOW;

    // Set mulitplexors to read channel i
    digitalWrite(3,bit_A);
    digitalWrite(4,bit_B);
    digitalWrite(6,bit_C);

    // Read the muliplexor analogue signals
    delay(30);
    MUX_results[i]=analogRead(A4); // Multiplexor 1 analogue signal
    MUX_results[j]=analogRead(A5); // Multiplexor 2 analogue signal
  }
  // We now have a reading from each mulitplex channel in the array MUX_results[*]
  // The signal numbers do not match the mulitplexor channel numbers so we need to re-map to get the right reading for the right sensor
  // Sensor 1 = MUX 1 channel 5 = Sensor_array[0}
  // Sensor 2 = MUX 1 channel 7 = Sensor_array[1]
  // Sensor 3 = MUX 1 channel 6 = Sensor_array[2]
  // Sensor 4 = MUX 1 channel 4 = Sensor_array[3]
  // Sensor 5 = MUX 1 channel 3 = Sensor_array[4]
  // Sensor 6 = MUX 1 channel 0 = Sensor_array[5]
  // Sensor 7 = MUX 1 channel 1 = Sensor_array[6]
  // Sensor 8 = MUX 1 channel 2 = Sensor_array[7]
  // Sensor 9 = MUX 2 channel 5 = Sensor_array[8]
  // Sensor 10= MUX 2 channel 7 = Sensor_array[9]
  // Sensor 11= MUX 2 channel 6 = Sensor_array[10]
  // Sensor 12= MUX 2 channel 4 = Sensor_array[11]
  // Sensor 13= MUX 2 channel 3 = Sensor_array[12]
  // Sensor 14= MUX 2 channel 0 = Sensor_array[13]
  // Sensor 15= MUX 2 channel 1 = Sensor_array[14]
  // Sensor 16= MUX 2 channel 2 = Sensor_array[15
  // Run a loop to map each MUX channel to the relevant sensor number
  for (l==0;l<=15;l++) { // Run a loop 16 times, with l starting at 0
    if (l==0) Sensor_array[5] = MUX_results[l];
    if (l==1) Sensor_array[6] = MUX_results[l];
    if (l==2) Sensor_array[7] = MUX_results[l];
    if (l==3) Sensor_array[4] = MUX_results[l];
    if (l==4) Sensor_array[3] = MUX_results[l];
    if (l==5) Sensor_array[0] = MUX_results[l];
    if (l==6) Sensor_array[2] = MUX_results[l];
    if (l==7) Sensor_array[1] = MUX_results[l];
    if (l==8) Sensor_array[13]= MUX_results[l];
    if (l==9) Sensor_array[14]= MUX_results[l];
    if (l==10)Sensor_array[15]= MUX_results[l];
    if (l==11)Sensor_array[12]= MUX_results[l];
    if (l==12)Sensor_array[11]= MUX_results[l];
    if (l==13)Sensor_array[8] = MUX_results[l];
    if (l==14)Sensor_array[10]= MUX_results[l];
    if (l==15)Sensor_array[9] = MUX_results[l];
  }
  l=0; // Reset the integer variable to 0 for future use. 
} 

void calibrate(){
  a=0;
  i=0;
  for (a=0;a<=(Avg_count-1);a++){
    for (i=0;i<=15;i++){
      Calibration_offset[i]=Calibration_offset[i]+Results[a][i]; 
    }
  }
  a=0;
  i=0;
  Serial.println();
  Serial.println("Calibration offsets are:");
  Serial.print("Sensor ");
  for (i=0;i<=15;i++){
    Calibration_offset[i]=(1023-Calibration_offset[i]/Avg_count);
    Serial.print("#");
    Serial.print(i+1);
    Serial.print(":");
    Serial.print(Calibration_offset[i]);
    Serial.print(" ");
  }
  delay(1500);
}

void calculate_average(){
  a=0;
  i=0;
  for(i=0;i<=15;i++){
    Sensor_average[i]=0;
  }
  i=0;
  for (a=0;a<=(Avg_count-1);a++){
    for (i=0;i<=15;i++){
      Sensor_average[i]=Sensor_average[i]+Results[a][i]; 
    }
  }
  a=0;
  i=0;  
  for (i=0;i<=15;i++){
    Sensor_average[i]=Sensor_average[i]/Avg_count;
  }
}

void scale_readings(){
  i=0;  
  for (i=0;i<=15;i++){
    Scaled_result[i]=Sensor_average[i]+Calibration_offset[i];
    Scaled_result[i]=(100-Scaled_result[i]*100/1024); // Convert to percentage 
    Scaled_result[i]=Scaled_result[i]*Scale_factor;  
  }  
}

void print_results(){
  //Print the results
  //Each 20 readings print a heading row
  i=0;
  k=k+1;
  if (k==20) { //First time through, and each 20 times after that, print a heading
    Serial.println("            _________________________________________________________________________________________________________________________________");    
    Serial.println("Channel number: 1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16   |");
    k=0; //Reset counting after printing a heading
  }
  //Print the sensor readings
  Serial.print("            |");
  for (i==0;i<=15;i++) {
    integer_result=int(Scaled_result[i]);
    sprintf(buff, " %3d%%  |",integer_result);
    Serial.print(buff);
    //Serial.print(integer_result);
    //Serial.print("%      ");
  }
  Serial.println(); 
  i=0; // Reset the integer variable to 0 for future use.
}
