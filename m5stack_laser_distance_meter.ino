#include <M5Stack.h>

HardwareSerial serial(2); // pin16:rx, pin17:tx

#define BUF_SIZE 64
char buf[BUF_SIZE];

void setup() 
{
  M5.begin();
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);

  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(0, 180);
  M5.Lcd.printf("Press A button to pause measuring.");
    
  M5.Lcd.setTextSize(10);

  serial.begin(19200);  // for Laser Distance Meter Module
  Serial.begin(115200); // for debug
}

bool enable_measure = true;

void loop() 
{
  M5.update();
  
  if (M5.BtnA.wasPressed()) {
    Serial.println("M5.BtnA.wasReleased()");
    enable_measure = !enable_measure;
  }

  // display
  if (enable_measure == false) {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("PAUSE    ");
  }
  else {
    float rv = measure();
    Serial.println(rv);
    if (rv < 0) {
      Serial.println("error");
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("error.. ");
    }
    else {
      Serial.println("success");
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("%.3fm   ", rv);
    }
  }
}

float measure() 
{
  // clear serial rx buffer
  while (true) {
    int c = serial.read();
    if (c == -1) break;
    delay(20);
  }

  // start measurement
  serial.print("D");

  if (readline(buf, BUF_SIZE) == false) return -1; // timeout

  // error check
  if (strlen(buf) < 5)   return -2; // format error
  if (buf[2] == 'E')     return -3; // error (ex.D:Er08)
  if (strlen(buf) != 14) return -4; // format error

  // parse result
  //
  // ex.
  // D: 1.234m,00120
  // D:Er08
  //
  char num_str[8];
  memset(num_str, 0, 8);
  for (int i = 0; i < 6; ++i) {
    num_str[i] = buf[i + 2];
  }

  float d = atof(num_str);

  return d;
}

bool readline(char *buf, size_t buf_size)
{
  memset(buf, 0, buf_size);
  int idx = 0;
  int error_count = 0;
  while (true) {
    int c = serial.read();
    if (c == 13) continue; // CR
    if (c == 10) break;    // LF
    if (c == -1) {
      delay(10);
      error_count ++;
      if (error_count > 100) return false; // timeout...
      continue;
    }
    buf[idx] = (char)c;
    idx ++;
    if (buf_size == idx) return false; // buffer overflow...
  }
  return true;
}
