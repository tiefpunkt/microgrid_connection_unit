#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED SCREEN
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// PINS
#define PIN_VOLTAGE_LOCAL A0
#define PIN_VOLTAGE_GRID A1
#define PIN_CURRENT A2
#define PIN_MOSFET 2

// CONFIGURATION
#define MAX_CURRENT 0.3
#define MIN_VOLTAGE_LOCAL 5.0
#define MAX_VOLTAGE 14.4
#define MIN_VOLTAGE_GRID 0
#define OVERSAMPLING 16
#define CURRENT_SENSOR_RANGE 20.0
#define VOLTAGE_DIVIDER_FACTOR 11.0
// VARIABLES
float voltage_local;
float voltage_grid;
float current;
uint8_t dutycycle;
uint16_t sample1, sample2, sample3;
uint8_t serial_cycle_cnt;
String alert = "";

float current_reference;

// 'microgird_oled02', 102x64px
#define LOGO_HEIGHT   64
#define LOGO_WIDTH    102
// 'microgird_oled02', 102x64px
const unsigned char logo [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x03, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x08, 
  0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x0c, 0x08, 0x10, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x18, 
  0x30, 0x20, 0x1f, 0xe3, 0xf0, 0x20, 0x7e, 0x2f, 0x83, 0xf0, 0x00, 0x7f, 0xff, 0xfe, 0x40, 0x1c, 
  0x36, 0x38, 0x20, 0xc0, 0x78, 0x87, 0x38, 0x00, 0x81, 0x83, 0x02, 0x40, 0x18, 0x1c, 0x18, 0x20, 
  0x80, 0x30, 0x0c, 0x08, 0x00, 0x81, 0x02, 0x07, 0xc0, 0x18, 0x08, 0x08, 0x21, 0x80, 0x20, 0x0c, 
  0x0c, 0x00, 0x81, 0x02, 0x06, 0xc0, 0x18, 0x08, 0x08, 0x21, 0x80, 0x20, 0x08, 0x0c, 0x01, 0x83, 
  0x02, 0x06, 0x80, 0x18, 0x08, 0x08, 0x21, 0x00, 0x20, 0x08, 0x0c, 0x01, 0x02, 0x06, 0x0e, 0x80, 
  0x18, 0x08, 0x08, 0x21, 0x00, 0x20, 0x08, 0x0c, 0x01, 0x02, 0x04, 0x0b, 0x80, 0x18, 0x08, 0x08, 
  0x21, 0x00, 0x20, 0x08, 0x0c, 0x01, 0x06, 0x04, 0x0b, 0x00, 0x18, 0x08, 0x08, 0x21, 0x00, 0x20, 
  0x08, 0x0c, 0x02, 0x06, 0x0c, 0x0b, 0x00, 0x18, 0x08, 0x08, 0x21, 0x00, 0x20, 0x08, 0x0c, 0x03, 
  0xff, 0xff, 0xfb, 0x00, 0x18, 0x08, 0x08, 0x21, 0x00, 0x20, 0x08, 0x0c, 0x02, 0x0c, 0x08, 0x12, 
  0x00, 0x18, 0x08, 0x08, 0x21, 0x80, 0x20, 0x08, 0x0c, 0x04, 0x0c, 0x08, 0x12, 0x00, 0x18, 0x08, 
  0x08, 0x21, 0x80, 0x20, 0x0c, 0x0c, 0x04, 0x08, 0x18, 0x3e, 0x00, 0x18, 0x08, 0x08, 0x20, 0x80, 
  0x20, 0x0c, 0x08, 0x04, 0x08, 0x10, 0x22, 0x00, 0x18, 0x08, 0x08, 0x20, 0xe2, 0x20, 0x07, 0x38, 
  0x0c, 0x18, 0x10, 0x22, 0x00, 0x18, 0x08, 0x08, 0x20, 0x7e, 0x20, 0x03, 0xf0, 0x08, 0x18, 0x30, 
  0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x20, 0x42, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x20, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xc2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x03, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x04, 0x90, 
  0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x09, 0x90, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x30, 0x00, 0x00, 0x00, 0x00, 0x08, 0xe8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x13, 
  0x00, 0x00, 0x01, 0xe4, 0x2f, 0x84, 0x07, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x11, 0xc0, 0x00, 0x07, 
  0x7c, 0x3c, 0xc4, 0x1d, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x11, 0x80, 0x00, 0x06, 0x0c, 0x30, 0x04, 
  0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x0c, 0x0c, 0x30, 0x04, 0x10, 0x30, 0x00, 
  0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x0c, 0x0c, 0x30, 0x04, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 
  0x11, 0x00, 0x00, 0x0c, 0x0c, 0x30, 0x04, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 0x01, 0x08, 0x00, 
  0x0c, 0x0c, 0x30, 0x04, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 0x09, 0x08, 0x00, 0x0c, 0x0c, 0x30, 
  0x04, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 0x09, 0x08, 0x00, 0x0c, 0x0c, 0x30, 0x04, 0x10, 0x30, 
  0x00, 0x00, 0x00, 0x00, 0x08, 0x88, 0x00, 0x0c, 0x0c, 0x30, 0x04, 0x10, 0x30, 0x00, 0x00, 0x00, 
  0x00, 0x18, 0x88, 0x00, 0x0c, 0x0c, 0x30, 0x04, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 0x18, 0x98, 
  0x00, 0x0c, 0x0c, 0x30, 0x04, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 0x04, 0x98, 0x00, 0x0c, 0x0c, 
  0x30, 0x04, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x94, 0x00, 0x04, 0x0c, 0x30, 0x04, 0x10, 
  0x30, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x94, 0x00, 0x06, 0x1c, 0x30, 0x04, 0x18, 0x70, 0x00, 0x00, 
  0x00, 0x00, 0x2c, 0x30, 0x00, 0x03, 0xfc, 0x30, 0x04, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x12, 
  0x2a, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x56, 0x26, 0x00, 0x00, 
  0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x52, 0x00, 0x00, 0x0c, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0x48, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x61, 0x85, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x40, 0x83, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 
  0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
void setup() {
  pinMode(PIN_MOSFET, OUTPUT);
  analogWrite(PIN_MOSFET, 0);
  Serial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  delay(10);
  display.clearDisplay();

  display.drawBitmap(10, 0, logo, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();


  analogReference(EXTERNAL);
  delay(1000);
  for (int i = 0; i< OVERSAMPLING; i++) {
    sample3 += analogRead(PIN_CURRENT);
    delay(10);
  }
  current_reference =(sample3 / OVERSAMPLING);
  delay(1000);
  display.clearDisplay();
}

void getVoltages() {
  sample1 = 0;
  sample2 = 0;
  sample3 = 0;
  for (int i = 0; i< OVERSAMPLING; i++) {
    sample1 += analogRead(PIN_VOLTAGE_LOCAL);
    sample2 += analogRead(PIN_VOLTAGE_GRID);
    sample3 += analogRead(PIN_CURRENT);
  }
  voltage_local = sample1 / OVERSAMPLING * 3.3 / 1024.0 * 11.0;
  voltage_grid = sample2 / OVERSAMPLING * 3.3 / 1024.0 * 11.0;
  current = ((sample3 / OVERSAMPLING) - current_reference) * (5.0 / 1024.0)*10.0;
}

void calculateDutyCycle() {
  alert = "";
  if ((voltage_local < MIN_VOLTAGE_LOCAL) || (voltage_grid < MIN_VOLTAGE_GRID)) {
    // Voltage is too low
    alert = "VMin";
    dutycycle = 0;
  } else if ((voltage_local > MAX_VOLTAGE) || (voltage_grid > MAX_VOLTAGE)) {
    // voltage is too high
    alert = "VMax";
    dutycycle = 0;
  } else if (abs(current) > MAX_CURRENT*1.3) {
    // overcurrent emergency brake
    alert = "I EMERG";
    Serial.print((sample3 / OVERSAMPLING));
    Serial.print(" - ");
    Serial.println(current);
    dutycycle = 0;
  } else if (abs(current) > MAX_CURRENT) {
    // current only a bit too high
    alert = "I Max";
    dutycycle--;
  } else if (dutycycle < 255) {
    dutycycle++;
  }
}

void serialDebug() {
  Serial.println(millis());
  Serial.print("Voltage Local: ");
  Serial.print(voltage_local);
  Serial.println("V");
  Serial.print("Voltage Grid: ");
  Serial.print(voltage_grid);
  Serial.println("V");
  Serial.print("Current: ");
  Serial.print(current);
  Serial.println("A");
  Serial.print("dutycycle: ");
  Serial.print(dutycycle);
  Serial.println();
  Serial.println();
}

void displayMain() {
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F(" MicroGrid Connector "));

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,12); 
  display.print("V Local: ");
  display.print(voltage_local);
  display.println("V");
  display.print("V Grid:  ");
  display.print(voltage_grid);
  display.println("V");
  display.print("Current: ");
  display.print(current);
  display.println("A");
  display.print("dutycycle: ");
  display.println(dutycycle);

  if (alert.length() > 0) {
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.print(alert);
  }
  
  display.display();
}

void loop() {
  getVoltages();
  calculateDutyCycle();

  analogWrite(PIN_MOSFET, dutycycle);

  if (serial_cycle_cnt > 50) {
    serialDebug();
    serial_cycle_cnt = 0;
  } else {
    serial_cycle_cnt++;
  }
  displayMain();
  delay(10);
}
