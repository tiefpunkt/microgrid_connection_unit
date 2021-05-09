
// PINS
#define PIN_VOLTAGE_LOCAL A0
#define PIN_VOLTAGE_GRID A1
#define PIN_CURRENT A2
#define PIN_MOSFET 2

// CONFIGURATION
#define MAX_CURRENT 0.18
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

void setup() {
  pinMode(PIN_MOSFET, OUTPUT);
  analogWrite(PIN_MOSFET, 0);
  Serial.begin(9600);
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
  voltage_local = sample1 / OVERSAMPLING * 5.0 / 1024.0 * 11.0;
  voltage_grid = sample2 / OVERSAMPLING * 5.0 / 1024.0 * 11.0;
  current = ((sample3 / OVERSAMPLING) - 510.0) * (5.0 / 1024.0)*10.0;
}

void calculateDutyCycle() {
  if ((voltage_local < MIN_VOLTAGE_LOCAL) || (voltage_grid < MIN_VOLTAGE_GRID)) {
    // Voltage is too low
    Serial.println("VMin");
    dutycycle = 0;
  } else if ((voltage_local > MAX_VOLTAGE) || (voltage_grid > MAX_VOLTAGE)) {
    // voltage is too high
        Serial.println("VMax");
    dutycycle = 0;
  } else if (abs(current) > MAX_CURRENT*1.3) {
    // overcurrent emergency brake
        Serial.println("I EMERG");

    dutycycle = 0;
  } else if (abs(current) > MAX_CURRENT) {
    // current only a bit too high
    Serial.println("I Max");
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

  delay(10);
}
