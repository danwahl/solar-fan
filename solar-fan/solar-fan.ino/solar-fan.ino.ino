#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

// hardware definitions
#define VBAT_PIN      A0              // also reset pin
#define VBAT_SLOPE    3.24889         // x/1e3 = (y - x)/2.2e3 + (5 - x)/45e3
#define VBAT_OFFSET   -0.244444
#define VBAT_MIN      10.5f
#define VBAT_NOM      12.0f           // should be greater than VBAT_MIN
#define FAN_IN_PIN    A3
#define FAN_OUT_PIN   0
#define LED_IN_PIN    A2
#define LED_OUT_PIN   1
#define USB_OUT_PIN   2

// system states
enum {RUN, LOWBAT};

// global vars
float vbat;
unsigned fan_in, led_in, fan_out, led_out, usb_out, state;

/***************************************************************************
 * watchdog timer isr
 **************************************************************************/
ISR(WDT_vect) {
}

/***************************************************************************
 * setup
 **************************************************************************/
void setup() {
  // set tmr0 pwm frequency to 1e6/2^8/psc
  TCCR0B &= ~(_BV(CS01) | _BV(CS02));
  TCCR0B |= _BV(CS00);

  // usb out pin setup
  pinMode(USB_OUT_PIN, OUTPUT);

  // disable outputs
  analogWrite(FAN_OUT_PIN, 0);
  analogWrite(LED_OUT_PIN, 0);
  digitalWrite(USB_OUT_PIN, LOW);

  // go to run
  state = RUN;
}

/***************************************************************************u
 * loop
 **************************************************************************/
void loop() {
  // read inputs
  vbat = ((float)analogRead(VBAT_PIN)*5.0f/1023.0f)*VBAT_SLOPE + VBAT_OFFSET;
  fan_in = analogRead(FAN_IN_PIN);
  led_in = analogRead(LED_IN_PIN);

  /*// map inputs to outputs
  fan_out = map(fan_in, 0, 1023, 0, 255);
  led_out = map(led_in, 0, 1023, 0, 255);
  usb_out = HIGH;

  // write outputs
  analogWrite(FAN_OUT_PIN, fan_out);
  analogWrite(LED_OUT_PIN, led_out);
  digitalWrite(USB_OUT_PIN, usb_out);

  // idle for a bit
  sleep_system(SLEEP_MODE_IDLE, WDTO_1S);
  //delay(1000);*/

  switch (state) {
    case (RUN):
      // check if vbat below min voltage threshold
      if (vbat < VBAT_MIN) {
        // set outputs
        fan_out = led_out = 0;
        usb_out = LOW;

        // disable outputs
        analogWrite(FAN_OUT_PIN, fan_out);
        analogWrite(LED_OUT_PIN, led_out);
        digitalWrite(USB_OUT_PIN, usb_out);

        // set state
        state = LOWBAT;

        // sleep for a while
        sleep_system(SLEEP_MODE_PWR_DOWN, WDTO_8S);
      }
      else {
        // map inputs to outputs
        fan_out = map(fan_in, 0, 1023, 0, 255);
        led_out = map(led_in, 0, 1023, 0, 255);
        usb_out = HIGH;

        // write outputs
        analogWrite(FAN_OUT_PIN, fan_out);
        analogWrite(LED_OUT_PIN, led_out);
        digitalWrite(USB_OUT_PIN, usb_out);

        // idle for a bit
        sleep_system(SLEEP_MODE_IDLE, WDTO_4S);
      }
      break;
    case (LOWBAT):
      // back to run if vbat nominal
      if (vbat >= VBAT_NOM) {
        state = RUN;

        // idle for a bit
        sleep_system(SLEEP_MODE_IDLE, WDTO_15MS);
      }
      // otherwise sleep again
      else sleep_system(SLEEP_MODE_PWR_DOWN, WDTO_8S);
      break;
    default:
      state = RUN;
      break;
  }
}

/***************************************************************************
 * sleep system
 **************************************************************************/
void sleep_system(unsigned mode, unsigned wdto) {
  // disable peripherals
  power_timer1_disable();
  power_adc_disable();

  // set sleep mode
  set_sleep_mode(mode);
  sleep_enable();

  // set watchdog and sleep
  setup_watchdog(wdto);
  sleep_mode();

  // zzzzz... (sleeping here)

  // disable sleep
  sleep_disable();

  // enable peripherals
  power_timer1_enable();
  power_adc_enable();
}

/***************************************************************************
 * watchdog setup
 **************************************************************************/
void setup_watchdog(unsigned wdto) {
  // Limit incoming amount to legal settings
  if (wdto > WDTO_8S) wdto = WDTO_8S;

  // Set the special 5th bit if necessary
  byte bb = wdto & 0x07;
  if (wdto > 7) bb |= (1 << 5);

  MCUSR &= ~(1 << WDRF);              // Clear the watchdog reset
  WDTCR |= (1 << WDCE) | (1 << WDE);  // Set WD_change enable, set WD enable
  WDTCR = bb;                         // Set new watchdog timeout value
  WDTCR |= _BV(WDIE);                 // Set the interrupt enable, this will keep unit from resetting after each int
}

