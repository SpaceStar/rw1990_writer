#include <stdio.h>
#include <OneWire.h>

#define pin 12

static int serial_fputchar(const char ch, FILE *stream) { Serial.write(ch); return ch; }
static FILE *serial_stream = fdevopen(serial_fputchar, NULL);

OneWire ibutton(pin); // ibutton connected on PIN 10.

byte addr[8]; // array to store the Ibutton ID.

byte newID[8] = {0x01, 0xBE, 0x40, 0x11, 0x5A, 0x36, 0x00, 0xE1};
byte writedID[8];

void setup() {
  stdout = serial_stream;
  Serial.begin(115200);
}

void loop() {
  if (!ibutton.reset()) { // read attached ibutton and asign value to buffer
    delay(200);
    return;
  }

  ibutton.write(0x33);
  ibutton.read_bytes(addr, 8);
  char addr_out[24] = "";
  for (byte i = 0; i < 8; i++) {
    sprintf(addr_out + i*3, "%02X ", addr[i]);
  }
  addr_out[23] = 0;

  if (!ibutton.crc8(addr, 8)) {
    Serial.println(addr_out);
  }

  if (Serial.read() == 'w') {
    if (ibutton.crc8(newID, 8)) {
      Serial.println("Incorrect crc.");
      printf("Correct crc for this key is %02X.\n", ibutton.crc8(newID, 7));
      wait();
      return;
    }

    Serial.print("  ID before write: ");
    Serial.println(addr_out);
    // send reset
    ibutton.reset();
    // send 0xD1
    ibutton.write(0xD1);
    // send logical 0
    digitalWrite(pin, LOW); pinMode(pin, OUTPUT); delayMicroseconds(60);
    pinMode(pin, INPUT); digitalWrite(pin, HIGH); delay(10);

    Serial.print("  Writing iButton ID:\n    ");

    ibutton.reset();
    ibutton.write(0xD5);
    for (byte i = 0; i < 8; i++) {
      writeByte(newID[i]);
      Serial.print('*');
    }
    Serial.println();
    ibutton.reset();
    // send 0xD1
    ibutton.write(0xD1);
    // send logical 1
    digitalWrite(pin, LOW); pinMode(pin, OUTPUT); delayMicroseconds(10);
    pinMode(pin, INPUT); digitalWrite(pin, HIGH); delay(10);

    // check if write is correct
    ibutton.reset();
    ibutton.write(0x33);
    ibutton.read_bytes(writedID, 8);
    byte correct = 1;
    for (byte i = 0; i < 8; i++) {
      if (newID[i] != writedID[i]) {
        correct = 0;
        break;
      }
    }
    if (correct) {
      Serial.println("  Successfully writed!");
    } else {
      Serial.println("  ERROR");
    }
  }

  if (!ibutton.crc8(addr, 8)) {
    wait();
  }
}

void wait(void) {
  while (ibutton.reset()) {
    delay(200);
  }
}

void writeByte(byte data) {
  for(byte i = 0; i < 8; i++) {
    if (data & 1) {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      delayMicroseconds(60);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    } else {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      delayMicroseconds(10);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    }
    data = data >> 1;
  }
}
