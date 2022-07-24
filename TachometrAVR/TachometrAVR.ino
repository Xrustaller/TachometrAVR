#define TACHOMETER_PIN 2

#define DIRKI 4 //

#define TACHOMETER_TIME_INTERVAL 800

#define FIRST_GROUP_RELAY_MODE 1 //  1 - Полное включение, 2 - пульсация
#define FIRST_GROUP_RELAY_MODE_PIN 1 //  1 - прямое реле, 2 - обратное реле

#define BTN_PIN_START 3 // старт
#define BTN_PIN_STOP 4 // стоп

#define FIRST_GROUP_RELAY_PIN_1 6
#define FIRST_GROUP_RELAY_PIN_2 7

#define BEEP_PIN 9 // Зуммер

#define BUS_ID 6
#define PIN_REDE A2

#include <Wire.h>
#include <GyverButton.h>
#include <DTM1650.h>
#include <ModbusRtu.h>

DTM1650 display;

GButton button_start(BTN_PIN_START);
GButton button_stop(BTN_PIN_STOP);

Modbus bus(BUS_ID, 0, PIN_REDE);
int8_t state = 0;

uint16_t temp[1] = { 0 };

volatile unsigned long t_interruptionTime;

volatile byte t_interruptionCounter;//, i = 0;

long t_previousTime;

uint16_t t_RPM;
//unsigned int buffer_arr[5], t_RPM;


unsigned long timer_relay_pulse;
bool is_relay_pulse = false;

bool relay = false;

void relay_on()
{
#if FIRST_GROUP_RELAY_MODE == 1
#if FIRST_GROUP_RELAY_MODE_PIN == 1
    digitalWrite(FIRST_GROUP_RELAY_PIN_1, HIGH);
    digitalWrite(FIRST_GROUP_RELAY_PIN_2, HIGH);
#else
    digitalWrite(FIRST_GROUP_RELAY_PIN_1, LOW);
    digitalWrite(FIRST_GROUP_RELAY_PIN_2, LOW);
#endif
#endif
}

void relay_off()
{
#if FIRST_GROUP_RELAY_MODE_PIN == 1
    digitalWrite(FIRST_GROUP_RELAY_PIN_1, LOW);
    digitalWrite(FIRST_GROUP_RELAY_PIN_2, LOW);
#else
    digitalWrite(FIRST_GROUP_RELAY_PIN_1, HIGH);
    digitalWrite(FIRST_GROUP_RELAY_PIN_2, HIGH);
#endif
}

void relay_pulse(const bool pulse)
{
    if (is_relay_pulse)
    {
        if (millis() - timer_relay_pulse > 1000)
        {
#if FIRST_GROUP_RELAY_MODE == 2
#if FIRST_GROUP_RELAY_MODE_PIN == 1
            digitalWrite(FIRST_GROUP_RELAY_PIN_1, LOW);
            digitalWrite(FIRST_GROUP_RELAY_PIN_2, LOW);
#else
            digitalWrite(FIRST_GROUP_RELAY_PIN_1, HIGH);
            digitalWrite(FIRST_GROUP_RELAY_PIN_2, HIGH);
#endif
#endif
            is_relay_pulse = false;
        }
    }

    if (pulse)
    {
#if FIRST_GROUP_RELAY_MODE == 2
#if FIRST_GROUP_RELAY_MODE_PIN == 1
        digitalWrite(FIRST_GROUP_RELAY_PIN_1, HIGH);
        digitalWrite(FIRST_GROUP_RELAY_PIN_2, HIGH);
#else
        digitalWrite(FIRST_GROUP_RELAY_PIN_1, LOW);
        digitalWrite(FIRST_GROUP_RELAY_PIN_2, LOW);
#endif
#endif
        is_relay_pulse = true;
        timer_relay_pulse = millis();
    }
}

void t_interruption()
{
    t_interruptionTime = millis();
    t_interruptionCounter++;
}

void setup()
{
    pinMode(FIRST_GROUP_RELAY_PIN_1, OUTPUT);
    pinMode(FIRST_GROUP_RELAY_PIN_2, OUTPUT);
    relay_off();

    button_start.setClickTimeout(50);
    button_stop.setClickTimeout(50);

    bus.begin(19200);
    Wire.begin();
    display.init();
    display.set_brightness(DTM1650_BRIGHTNESS_MAX);

    attachInterrupt(digitalPinToInterrupt(TACHOMETER_PIN), t_interruption, RISING);
}

void loop()
{
    state = bus.poll(temp, 2);
    button_start.tick();
    button_stop.tick();

    if (relay)
    {
        if (button_stop.isSingle())
        {
            relay = false;
            relay_off();
            relay_pulse(true);
            tone(BEEP_PIN, 2000, 100);
        }
    }
    else
    {
        if (button_start.isSingle())
        {
            relay = true;
            relay_on();
            relay_pulse(true);
            tone(BEEP_PIN, 3000, 100);
        }
    }

    if (millis() - t_previousTime >= TACHOMETER_TIME_INTERVAL)
    {
        unsigned long t_currentTime;

        byte t_counter;
        noInterrupts();
        t_counter = t_interruptionCounter;
        t_interruptionCounter = 0;
        t_currentTime = t_interruptionTime;
        interrupts();
        t_RPM = 60000 / ((t_currentTime - t_previousTime) * DIRKI / t_counter);
        t_previousTime = t_currentTime;
        //i++;
        display.write_num(t_RPM);
        temp[0] = t_RPM;
    }

    relay_pulse(false);

    /*if (i >= 5)
    {
        for (byte i = 0; i < 5; i++)
        {
            t_RPM += buffer_arr[i];
        }
        t_RPM /= 5;
        display.write_num(t_RPM);
        t_RPM = 0;
        i = 0;
    }*/
}

