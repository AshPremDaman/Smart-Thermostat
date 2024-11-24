/*Uses FreeRTOS to control tasks such as blinking an LED, displaying a temperature 
reading on a seven-segment display, and managing a fan based on sensor data and user input.
*/

#include <Arduino_FreeRTOS.h>

void setup() {
  Serial.begin(19200);

  //          func               name             stack  args  priority  task
  xTaskCreate(TaskBlinkExternal, "BlinkExternal", 1024,  NULL, 2,        NULL);
  xTaskCreate(TaskDisplayNumber, "DisplayNumber", 1024,  NULL, 2,        NULL);
  xTaskCreate(TaskReadSensor,    "ReadSensor",    1024,  NULL, 2,        NULL);
  xTaskCreate(TaskTurnOnFan,     "TurnOnFan",     1024,  NULL, 2,        NULL);
  xTaskCreate(TaskSetFanTemp,    "SetFanTemp",    1024,  NULL, 2,        NULL);

  vTaskStartScheduler();
}

void loop() {}

// /*--------------------------------------------------*/
// /*---------------------- Tasks ---------------------*/
// /*--------------------------------------------------*/

/***************************************************
* void TaskBlinkExternal(void *pvParameters);
* pvParameters - unused
*
* Blinks an external LED according to the lab spec.

*/
#define LED_EXTERNAL 7
void TaskBlinkExternal(void *pvParameters) {
  (void) pvParameters; // suppress unused warning

  pinMode(LED_EXTERNAL, OUTPUT);

  while (true) {
    digitalWrite(LED_EXTERNAL, HIGH);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    digitalWrite(LED_EXTERNAL, LOW);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

// Interrupt
#define CLOCK_FREQUENCY 16000000
#define PRESCALER 64
#define INTERRUPT_FREQUENCY 1000
volatile float num_to_display = 0;

// Pin numbers for each segment
int segment_pins[8] = {41, 49, 44, 40, 38, 43, 46, 42};
// Pin numbers for each digit
int digit_pins[4] = {48, 47, 45, 39};
// Encodings for 0-9
byte encodings[10] = {
  0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,
  0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11100110
};

/***************************************************
* ISR (TIMER1_COMPA_vect);
*
* Displays `num_to_display` on the seven-segment display.
* Displays two integral places, one decimal place, and also an F.

*/
ISR (TIMER1_COMPA_vect) {
  float n = num_to_display * 10.0;

  int divisor = 1;
  for (int i = 1; i < 4; i++) {
    // Get ith digit in base 10
    int digit = ((int) (n / divisor)) % 10;
    divisor *= 10;
    // Display it
    int encoding = encodings[digit];
    if (i == 2) encoding |= 1 << 0; // decimal point
    display_digit(i, encoding);
  }

  display_digit(0, 0b10001110); // show an F
}

/***************************************************
* void display_digit(int digit, byte encoding);
* digit    - which of the four digits to set
* encoding - which segments of that digit to turn on
*
* Sets one digit of the four-digit seven-segment display

*/
void display_digit(int digit, byte encoding) {
  // Set up the abcdefg and dp wires ahead of time
  for (int j = 0; j < 8; j++) {
    // For each segment, decode the signal and send it
    int signal = ((encoding >> (7 - j)) & 1);
    digitalWrite(segment_pins[j], signal);
  }
  // Pulse the wire for the specified digit LOW
  digitalWrite(digit_pins[digit], LOW);
  delayMicroseconds(50);
  digitalWrite(digit_pins[digit], HIGH);
}

/***************************************************
* void TaskDisplayNumber(void *pvParameters);
* pvParameters - unused
*
* Sets up the timer for the interrupt that displays
* the current temperature.

*/
void TaskDisplayNumber(void *pvParameters) {
  (void) pvParameters; // suppress unused warning

  // Set output pins
  for (int i = 0; i < 8; i++) pinMode(segment_pins[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(digit_pins[i], OUTPUT);

  // Set up interrupt timer
  cli();
  TCCR1A = 0b00000000;
  TCCR1B = 0b00001011;
  TCNT1 = 0;
  OCR1A = (CLOCK_FREQUENCY / PRESCALER) / INTERRUPT_FREQUENCY;
  TIMSK1 |= (1 << OCIE1A);
  sei();

  // End this task now that the interrupt is running
  vTaskDelete(NULL);
}

/***************************************************
* void TaskReadSensor(void *pvParameters);
* pvParameters - unused
*
* Uses a GPIO pin to read temperature data from a sensor.
* Sets `num_to_display` to the current temperature in Fahrenheit.

* Acknowledgments: https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf
*/
#define SENSOR_PIN 3
void TaskReadSensor(void *pvParameters) {
  (void) pvParameters; // suppress unused warning

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  while (true) {
    // Send request for data
    pinMode(SENSOR_PIN, OUTPUT);
    digitalWrite(SENSOR_PIN, LOW);
    vTaskDelay(50 / portTICK_PERIOD_MS); // wait for at least 18ms

    cli(); // interrupts mess up the timing
    digitalWrite(SENSOR_PIN, HIGH);
    pinMode(SENSOR_PIN, INPUT);

    // Wait for response
    wait_for_sensor(LOW);
    wait_for_sensor(HIGH);
    wait_for_sensor(LOW);
    delayMicroseconds(50);
    
    // Read data
    int humidity_integral    = read_sensor_byte();
    int humidity_decimal     = read_sensor_byte();
    int temperature_integral = read_sensor_byte();
    int temperature_decimal  = read_sensor_byte();
    int checksum             = read_sensor_byte();
    sei();

    // Compute checksum
    int is_checksum_correct = checksum == (0xFF &
      (humidity_integral + humidity_decimal +
      temperature_integral + temperature_decimal));

    if (!is_checksum_correct) {
      Serial.println("Error reading temperature!");
    } else {
      // Compute degrees Fahrenheit from raw data
      float celsius = temperature_integral + temperature_decimal * 0.1;
      float fahrenheit = (celsius * (9 / 5)) + 32;
      num_to_display = fahrenheit;
    }

    // Sensor can only be polled once per second
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

/***************************************************
* int read_sensor_byte();
* Returns - one byte from the sensor
*
* Reads a byte by reading 8 bits.

*/
int read_sensor_byte() {
  int byte = 0;
  for (int i = 0; i < 8; i++) {
    int bit = read_sensor_bit();
    byte |= bit << (7 - i);
  }
  return byte;
}

/***************************************************
* int read_sensor_bit();
* Returns - one bit from the sensor
*
* Reads a single bit of data from the sensor.
* Must be called immediately after the rising edge that starts the bit.

*/
int read_sensor_bit() {
  // 35us in is where a 1 looks different than a 0
  delayMicroseconds(35);
  int signal = digitalRead(SENSOR_PIN);

  // wait for the next bit
  wait_for_sensor(LOW);
  wait_for_sensor(HIGH);

  return signal;
}

/***************************************************
* void wait_for_sensor(int state);
* state - which signal to wait for
*
* Busy waits until the sensor is sending a specific value.

*/
void wait_for_sensor(int state) {
  while (digitalRead(SENSOR_PIN) != state) {}
}

/***************************************************
* void TaskTurnOnFan(void *pvParameters);
* pvParameters - unused
*
* Turns on the fan when the temperature exceeds `fan_temp`.

*/
#define FAN_PIN 4
volatile float fan_temp = 58.0;
void TaskTurnOnFan(void *pvParameters) {
  (void) pvParameters; // suppress unused warning

  pinMode(FAN_PIN, OUTPUT);

  while (true) {
    digitalWrite(FAN_PIN, num_to_display > fan_temp);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/***************************************************
* void TaskSetFanTemp(void *pvParameters);
* pvParameters - unused
*
* Reads a potentiometer signal and sets the `fan_temp`
* to some fraction between `min_temp` and `max_temp`.

*/
#define POT_PIN A6
float min_temp = 40;
float max_temp = 80;
void TaskSetFanTemp(void *pvParameters) {
  (void) pvParameters; // suppress unused warning

  pinMode(POT_PIN, INPUT);

  while (true) {
    float data = ((float) analogRead(POT_PIN)) / 1024.0;
    fan_temp = min_temp + data * (max_temp - min_temp);
    Serial.println(fan_temp);

    vTaskDelay(1); // 1 tick, not 1 ms
  }
}

