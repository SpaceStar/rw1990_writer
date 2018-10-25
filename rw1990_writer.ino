#include <stdio.h>
#include <OneWire.h>

#define pin 12

static int serial_fputchar(const char ch, FILE *stream) { Serial.write(ch); return ch; }
static FILE *serial_stream = fdevopen(serial_fputchar, NULL);

OneWire ibutton(pin); // ibutton connected on PIN 10.

byte newID[8] = {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F};
byte addr[8], addr_for_copy[8]; // array to store the Ibutton ID.
char addr_out[24]; // addr as string

void setup() {
  stdout = serial_stream;
  Serial.begin(9600);
}

void loop() {
  if (!ibutton.reset()) { // read attached ibutton and asign value to buffer
    delay(200);
    return;
  }

  if (Serial.available()) {
    switch (Serial.read()) {
      case 'w':
        if (ibutton.crc8(newID, 8)) {
          Serial.println("Incorrect crc.");
          printf("Correct crc for writable key is %02X.\n", ibutton.crc8(newID, 7));
          wait();
          return;
        }
        write_id(newID);
        break;
      case 'c':
        write_id(addr_for_copy);
        break;
      default:
        return;
    }
    wait();
    return;
  }

  ibutton.write(0x33);
  ibutton.read_bytes(addr, 8);
  for (byte i = 0; i < 8; i++) {
    sprintf(addr_out + i*3, "%02X ", addr[i]);
  }
  addr_out[23] = 0;

  if (!ibutton.crc8(addr, 8)) {
    Serial.println(addr_out);
    for (byte i = 0; i < 8; i++) {
      addr_for_copy[i] = addr[i];
    }
    wait();
  }
}

void write_id(byte *id) {
  Serial.print("  ID before write: ");
  Serial.println(addr_out);
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
    writeByte(id[i]);
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
  ibutton.read_bytes(addr, 8); // writed ID
  if (cmp(id, addr, 8)) {
    Serial.println("  Successfully writed!");
  } else {
    Serial.println("  ERROR");
  }
}

void wait(void) {
  while (ibutton.reset()) {
    delay(200);
  }
}

bool cmp(byte *arr1, byte *arr2, int count) {
  bool equals = true;
  for (byte i = 0; i < count; i++) {
    if (arr1[i] != arr2[i]) {
      equals = false;
      break;
    }
  }
  return equals;
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
