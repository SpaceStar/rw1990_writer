#include <stdio.h>
#include <OneWire.h>

#define pin 12 // iButton connected on PIN 12

static int serial_fputchar(const char ch, FILE *stream) { Serial.write(ch); return ch; }
static FILE *serial_stream = fdevopen(serial_fputchar, NULL);

OneWire ibutton(pin);

byte newID[8] = {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F}; // new ID for writing to the iButton
byte addr[8], addr_for_copy[8]; // arrays to read and save the iButton ID
char addr_out[24]; // addr as string

void setup() {
  stdout = serial_stream;
  Serial.begin(9600);
}

void loop() {
  if (!ibutton.reset()) { // check if iButton was connected
    delay(200);
    return;
  }

  // read ID
  ibutton.write(0x33);
  ibutton.read_bytes(addr, 8);
  for (byte i = 0; i < 8; i++) {
    sprintf(addr_out + i*3, "%02X ", addr[i]);
  }
  addr_out[23] = 0;

  if (Serial.available()) {
    switch (Serial.read()) {
      case 'w': // write new ID
        if (ibutton.crc8(newID, 8)) {
          Serial.println("Incorrect crc.");
          printf("Correct crc for writable key is %02X.\n", ibutton.crc8(newID, 7));
          wait();
          return;
        }
        write_id(newID);
        break;
      case 'c': // write saved ID
        write_id(addr_for_copy);
        break;
      default:
        return;
    }
    wait();
    return;
  }

  // if crc is correct then print and save ID
  if (!ibutton.crc8(addr, 8)) {
    Serial.println(addr_out);
    for (byte i = 0; i < 8; i++) {
      addr_for_copy[i] = addr[i];
    }
    wait();
  }
}

void write_id(byte *id) {
  Serial.print("Waiting: ");
  for (byte i = 0; i < 8; i++) {
    delay(50);
    Serial.print('*');
  }
  Serial.println();
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

// wait until iButton is disconnected
void wait(void) {
  while (ibutton.reset()) {
    delay(200);
  }
}

// compare 2 arrays with count bytes
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
