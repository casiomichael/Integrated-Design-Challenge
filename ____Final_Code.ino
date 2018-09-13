#include <SoftwareSerial.h>
#define Rx 10 // DOUT to pin 10
#define Tx 11 // DIN to pin 11
#define LCDPin 53
SoftwareSerial Xbee(Rx, Tx); // Xbee initialization
#include <Servo.h>                           // Include servo library
Servo servoLeft;                             // Declare left and right servos
Servo servoRight;
SoftwareSerial mySerial = SoftwareSerial(255, LCDPin); // initializes LCD display
int hashCount = 0; // keeps track of hash marks
int RFIDcount = 0; // keeps track of number of RFID
int turretCount = 0; // keeps track of yellow squad info
int deathCount = 0; // keeps track of blue squad info
int TIEcount = 0; // keeep track of red squad info
int blockCount = 0; // keep track of green squad info
int newHash = 0; // keeps track of second track's hash count
int bigD = 0; // keeps track of whether D hs been received
float T = 0; // initializes delay for jump
#include <SoftwareSerial.h>

void setup() {
  Serial.begin(9600); // Set to no line ending
  Xbee.begin(9600); // types a char, then hits enter
  delay(500); // waits 0.5 seconds
  pinMode(3, OUTPUT); // initializes red outgoing LED
  pinMode(8, OUTPUT); // initializes green receiving LED
  pinMode(53, OUTPUT); // initializes LCD output
  pinMode(9, OUTPUT); // initializes yellow LED for sensing
  servoLeft.attach(13); // Attach left signal to pin 13
  servoRight.attach(12);  // attach pin 12 to right servo
  digitalWrite(9, LOW); // sets yellow LED to low
  digitalWrite(LCDPin, HIGH); // turns on LCD display
  mySerial.begin(9600); // initializes LCD display
  delay(100);
  clearLCD(); // clears LCD
  delay(5);
  digitalWrite(LCDPin, LOW); // sets LCD display to low

}

void loop() {

  long qti1 = rcTime(7); // finds reading of left QTI using rcTime function
  long qti2 = rcTime(6); // finds reading of left-mid QTI
  long qti3 = rcTime(5); // finds reading of right-mid QTI
  long qti4 = rcTime(4); // finds reading of right QTI
  long qtiRFID = rcTime(2); // finds reading of RFID QTI

  if (qti2 > 500 && qti3 > 500) { // if two mid QTIs see black
    servoLeft.writeMicroseconds(1470); // go forward
    servoRight.writeMicroseconds(1530);
    delay(50);
  }
  else {
    if (qti2 > 500 && qti3 < 500) { // if left-mid QTI sees black, but not right-mid QTI
      servoLeft.writeMicroseconds(1440); // self-correct by going to the left a little
      servoRight.writeMicroseconds(1440);
    }

    if (qti2 < 500 && qti3 > 500) { // if right-mid QTI sees black, but not left-mid QTI
      servoLeft.writeMicroseconds(1560); // self-correct by going to the right a little
      servoRight.writeMicroseconds(1560);
    }
  }

  if (qti1 > 500 && qti4 > 500) { // if the outer QTIs see black, means at a hash mark
    servoLeft.writeMicroseconds(1500); // pause for a second
    servoRight.writeMicroseconds(1500);
    hashCount++;

    if (hashCount == 6) { // if at end of first track, which is hashmark 6
      servoLeft.writeMicroseconds(1500); // stops
      servoRight.writeMicroseconds(1500);
      delay(1000);
      for (int s = 0; s < RFIDcount; s++) { // send appropriate number of o's
        xbeeSend('o');
      }
      xbeeSend('d'); // send one d
      delay(50);
      detachServos(); // detach servos to prevent bot from going crazy
      int v = 0;
      while (v < 50000) { // enter while loop
        xbeeReceive(); // receive
        mySerial.print("Y:"); // display values as they come in on the LCD display
        mySerial.print(turretCount);
        mySerial.print(", B:");
        mySerial.print(deathCount);
        mySerial.print(", O:");
        mySerial.print(RFIDcount);
        newLine(); // starts a new line the LCD
        mySerial.print("R:");
        mySerial.print(TIEcount);
        mySerial.print(", G:");
        mySerial.print(blockCount);
        newLine();
        v++; // increment counter
        delay(100);
        if (bigD > 0) { // if D received, break out of this loop
          break;
        }
      }
      T = 30000 * float(RFIDcount - 1); // sets delay for jump
      delay(T);

      attachServos(); // reattach servos
      servoLeft.writeMicroseconds(1700); // makes 45 degree clockwise turn
      servoRight.writeMicroseconds(1510);
      delay(1200);
      servoLeft.writeMicroseconds(1440); // go straight for 1875 ms
      servoRight.writeMicroseconds(1560);
      delay(1875);

      while (1) { // second line-following
        long qti1 = rcTime(7); // finds reading of left QTI using rcTime function
        long qti2 = rcTime(6); // finds reading of left-mid QTI
        long qti3 = rcTime(5); // finds reading of right-mid QTI
        long qti4 = rcTime(4); // finds reading of right QTI

        if (qti2 > 500 && qti3 > 500 && qti1 < 500 && qti4 < 500) { // if mid QTIs black
          servoLeft.writeMicroseconds(1420); // go forward
          servoRight.writeMicroseconds(1580);
          delay(50);
        }
        if (qti1 > 500 && qti4 > 500 && qti2 > 500 && qti3 > 500) { // if sense black hash
          digitalWrite(9, HIGH); // light up LED
          delay(100);
          digitalWrite(9, LOW);
          newHash++; // increment new hash count
          if (newHash == (6 - RFIDcount)) { // if you find the place to stop
            detachServos(); // stop moving
            mySerial.print("Y:"); // display all squad info
            mySerial.print(turretCount);
            mySerial.print(", B:");
            mySerial.print(deathCount);
            mySerial.print(", O:");
            mySerial.print(RFIDcount);
            newLine();
            mySerial.print("R:");
            mySerial.print(TIEcount);
            mySerial.print(", G:");
            mySerial.print(blockCount);
            newLine();
            delay(5000000); // delay so no possibility of looping around
          }

          servoLeft.writeMicroseconds(1450); // keep moving past hash if not stopping
          servoRight.writeMicroseconds(1550);
          delay(1000);
        }

        if (qti2 < 500 && qti3 < 500) { // if mid QTIs see white
          servoLeft.writeMicroseconds(1460); // go forward
          servoRight.writeMicroseconds(1540);
        }

        if (qti2 > 500 && qti3 < 500) { // if left-mid QTI sees black, but not right-mid QTI
          servoLeft.writeMicroseconds(1480); // self-correct by going to the left a little
          servoRight.writeMicroseconds(1480);
        }

        if (qti2 < 500 && qti3 > 500) { // if right-mid QTI sees black, but not left-mid QTI
          servoLeft.writeMicroseconds(1520); // self-correct by going to the right a little
          servoRight.writeMicroseconds(1520);
        }
      }
    }
    else {
      delay(1000);
      qtiRFID = rcTime(2); // sets up the rfid on the side; logic is that if counter isn't 6, then scan the rfid on the side
      qtiRFID = rcTime(2); // reads again for good measure
      if (qtiRFID > 500) {
        RFIDcount++; // increase RFID count
        digitalWrite(9, HIGH); // if the rfid is black, then light up the yellow led, if not then don't turn it on
        delay(1000);
        digitalWrite(9, LOW); // turn the yellow led back off
      }
      servoLeft.writeMicroseconds(1450);         // keep going after reading the rfid
      servoRight.writeMicroseconds(1550);
      delay(500);
    }
  }
}

void xbeeReceive() {
  if (Xbee.available()) { // Is data available from XBee?
    char incoming = Xbee.read() ; // Read incoming character
    if (incoming == 'Y') { // if receive Y, add one to turret count
      turretCount++;
    }
    if (incoming == 'B') { // if receive B, add one to death star count
      deathCount++; 
    }
    if (incoming == 'R') { // if receive R, add one to tie fighter count
      TIEcount++;
    }
    if (incoming == 'G') { // if receive G, add one to block count
      blockCount++;
    }
    if (incoming == 'D') { // if receive D, add one to bigD
      bigD++;
    }
    digitalWrite(8, HIGH); // turn on green receiving LED
    delay(100);
    digitalWrite(8, LOW); // turn off LED
  }
  delay (5) ; // reset
}

void xbeeSend(char outgoing) {
  Xbee.print(outgoing); // print to serial
  digitalWrite(3, HIGH); // turn on red outgoing LED
  delay(1000);
  digitalWrite(3, LOW); // turn off LED
  delay(500);
}

long rcTime(int pin)
{
  pinMode(pin, OUTPUT);                      // charge capacitor
  digitalWrite(pin, HIGH);                   // ..by setting pin ouput-high
  delay(1);                                  // ..for 5 ms
  pinMode(pin, INPUT);                       // Set pin to input
  digitalWrite(pin, LOW);                    // ..with no pullup
  long time  = micros();                     // Mark the time
  while (digitalRead(pin));                  // Wait for voltage < threshold
  time = micros() - time;                    // Calculate decay time
  return time;                               // Return decay time
}

void detachServos() {
  servoLeft.detach(); // detach left servo
  servoRight.detach(); // detach right servo
}

void attachServos() {
  servoLeft.attach(13); // reattach left servo
  servoRight.attach(12); // reattach right servo
}

void clearLCD() {
  mySerial.write(12); // clear LCD
}

void newLine() {
  mySerial.write(13); // create new line in LCD display
}

