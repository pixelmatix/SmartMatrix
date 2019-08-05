// Written from
// https://github.com/hzeller/rpi-rgb-led-matrix/issues/746
// and
// http://bobdavis321.blogspot.com/2019/03/teensy-31-and-smartmatrix-sd-running.html

// Temporary workaround to enable output on FM6126A chip driven RGBPanels
// Run this code first, and then your sketch.

// how many pixels wide if you chain panels
// 4 panels of 64x32 is 256 wie.
int MaxLed = 256; 

#define GPIO_PIN_CLK_TEENSY_PIN    14
#define GPIO_PIN_LATCH_TEENSY_PIN   3
#define GPIO_PIN_OE_TEENSY_PIN      4
#define GPIO_PIN_B0_TEENSY_PIN      6
#define GPIO_PIN_R0_TEENSY_PIN      2
#define GPIO_PIN_R1_TEENSY_PIN      21
#define GPIO_PIN_G0_TEENSY_PIN      5
#define GPIO_PIN_G1_TEENSY_PIN      8
#define GPIO_PIN_B1_TEENSY_PIN      20
#define ADDX_TEENSY_PIN_0   9
#define ADDX_TEENSY_PIN_1   10
#define ADDX_TEENSY_PIN_2   22
#define ADDX_TEENSY_PIN_3   23

int C12[16] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int C13[16] = {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0};

void setup() {
  // put your setup code here, to run once:
    pinMode(GPIO_PIN_CLK_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_LATCH_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_OE_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_B0_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_R0_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_R1_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_G0_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_G1_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_B1_TEENSY_PIN, OUTPUT);
    pinMode(ADDX_TEENSY_PIN_0, OUTPUT);
    pinMode(ADDX_TEENSY_PIN_1, OUTPUT);
    pinMode(ADDX_TEENSY_PIN_2, OUTPUT);
    pinMode(ADDX_TEENSY_PIN_3, OUTPUT);

    // Send Data to control register 11
    digitalWrite (GPIO_PIN_OE_TEENSY_PIN, HIGH); // Display reset
    digitalWrite (GPIO_PIN_LATCH_TEENSY_PIN, LOW);
    digitalWrite (GPIO_PIN_CLK_TEENSY_PIN, LOW);
    for (int l=0; l< MaxLed; l++) {    
      int y=l%16;
      digitalWrite (GPIO_PIN_R0_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_G0_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_B0_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_R1_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_G1_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_B1_TEENSY_PIN,LOW);
      if (C12[y]==1) {
          digitalWrite (GPIO_PIN_R0_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_G0_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_B0_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_R1_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_G1_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_B1_TEENSY_PIN,HIGH);
      }
      if (l>MaxLed-12){digitalWrite(GPIO_PIN_LATCH_TEENSY_PIN, HIGH);}
          else{digitalWrite(GPIO_PIN_LATCH_TEENSY_PIN, LOW);}
      digitalWrite(GPIO_PIN_CLK_TEENSY_PIN, HIGH);
      digitalWrite(GPIO_PIN_CLK_TEENSY_PIN, LOW);
    }
    digitalWrite (GPIO_PIN_LATCH_TEENSY_PIN, LOW);
    digitalWrite (GPIO_PIN_CLK_TEENSY_PIN, LOW);
    // Send Data to control register 12
    for (int l=0; l< MaxLed; l++) {    
      int y=l%16;
      digitalWrite (GPIO_PIN_R0_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_G0_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_B0_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_R1_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_G1_TEENSY_PIN,LOW);
      digitalWrite (GPIO_PIN_B1_TEENSY_PIN,LOW);
      if (C13[y]==1){
          digitalWrite (GPIO_PIN_R0_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_G0_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_B0_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_R1_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_G1_TEENSY_PIN,HIGH);
          digitalWrite (GPIO_PIN_B1_TEENSY_PIN,HIGH);
      }  
      if (l>MaxLed-13){digitalWrite(GPIO_PIN_LATCH_TEENSY_PIN, HIGH);}
          else{digitalWrite(GPIO_PIN_LATCH_TEENSY_PIN, LOW);}
      digitalWrite(GPIO_PIN_CLK_TEENSY_PIN, HIGH);
      digitalWrite(GPIO_PIN_CLK_TEENSY_PIN, LOW);
    }
    digitalWrite (GPIO_PIN_LATCH_TEENSY_PIN, LOW);
    digitalWrite (GPIO_PIN_CLK_TEENSY_PIN, LOW);
}

void loop() {
// Run regular SmartMatrix code here
}
