#include "Initialization.hpp"

uint32_t lastinput = 0;

static void isr_button_A()
{
  if (millis() - lastinput < 20)
    return;
  lastinput = millis();
  global_msg_queue->push(OLED_NEXT_MENU);
}

static void isr_button_B()
{
  if (millis() - lastinput < 20)
    return;
  lastinput = millis();
  global_msg_queue->push(OLED_PREV_MENU);
}

static void isr_rotary_btn()
{
  if (millis() - lastinput < 20)
    return;
  lastinput = millis();
  global_msg_queue->push(OLED_EDIT_MENU);
}

static void isr_rotary()
{
  if (millis() - lastinput < 20)
    return;
  lastinput = millis();
  PinStatus rotary_A_state = digitalRead(ROTARY_A_PIN);
  PinStatus rotary_B_state = digitalRead(ROTARY_B_PIN);

  if (rotary_A_state && rotary_B_state)
  { // CCW
    global_msg_queue->push(OLED_ROTARY_CCW);
  }
  else if (rotary_A_state && !rotary_B_state)
  { // CW
    global_msg_queue->push(OLED_ROTARY_CW);
  }
}

static void isr_motion()
{
  global_msg_queue->push(MOTION_DETECTED);
}

static void isr_rtc_alarm()
{
  global_msg_queue->push(RTC_UPDATE);
}

void initI2C()
{
  Wire.setClock(I2C_FREQ);
  Wire.begin();
}

void initGPIO()
{
  // Buttons
  pinMode(BUTTON_A_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_A_PIN), isr_button_A, FALLING);
  pinMode(BUTTON_B_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_B_PIN), isr_button_B, FALLING);
  // Rotary Enocder
  pinMode(ROTARY_A_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ROTARY_A_PIN), isr_rotary, RISING);
  pinMode(ROTARY_B_PIN, INPUT_PULLUP);

  pinMode(ROTARY_BTN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ROTARY_BTN_PIN), isr_rotary_btn, FALLING);
  // Motion Sensor
  pinMode(MOTION_SENSOR_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR_PIN), isr_motion, RISING);
  // // Furnace Relay
  pinMode(FURNACE_RELAY_PIN, OUTPUT);
  digitalWrite(FURNACE_RELAY_PIN, LOW);
  digitalWrite(STATUS_LED, LOW);
}

void initTimers()
{
  TC4_configure(30, 3);
  TC4_start_timer();

  TC3_configure(3);
  TC3_start_timer();
}

void initWiFi(Messenger& messenger)
{
  WiFi.setHostname(DEVICE_NAME);
  int r = messenger.connect_to_wifi(3);
  Serial.print("WiFi Connection Status: ");
  Serial.println(r);
  WiFi.lowPowerMode();
}

void initRTC(RTCZero &rtc)
{
  rtc.begin();
  if (WiFi.status() == WL_CONNECTED) {
    unsigned long epoch = WiFi.getTime();
    Serial.print('Epoch from WiFi: ');
    Serial.println(epoch);
    rtc.setEpoch(epoch);
  }
  else {
    rtc.setTime(12, 30, 0);
    rtc.setDate(10, 13, 22);
  }

  REG_RTC_FREQCORR = 127;
  REG_RTC_FREQCORR |= RTC_FREQCORR_SIGN;

  rtc.setAlarmTime(1, 0, 0);
  rtc.enableAlarm(rtc.MATCH_SS);
  rtc.attachInterrupt(isr_rtc_alarm);
}