#define TACHOMETER_PIN 2

#define DIRKI 12

#define TACHOMETER_TIME_INTERVAL 300

#define FIRST_GROUP_RELAY_MODE 1 // 1 - Полное включение, 2 - пульсация
#define FIRST_GROUP_RELAY_MODE_PIN 2 // 1 - прямое реле, 2 - обратное реле

#define SECOND_GROUP_RELAY_MODE 1 // 1 - Полное включение, 2 - пульсация
#define SECOND_GROUP_RELAY_MODE_PIN 2 // 1 - прямое реле, 2 - обратное реле

#define BTN_PIN_START 3 // Кнопка СТАРТ/СТОП

#define FIRST_GROUP_RELAY_PIN_1 11
#define FIRST_GROUP_RELAY_PIN_2 12

#define SECOND_GROUP_RELAY_PIN_1 6
#define SECOND_GROUP_RELAY_PIN_2 7

#define BEEP_PIN 9 // Пин пьезо (при выборе 9 пина, 10 - недоступен из-за шим)

#include <Wire.h>
#include <GyverButton.h>
#include <DTM1650.h>

DTM1650 display;

GButton button_start(BTN_PIN_START);

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

#if SECOND_GROUP_RELAY_MODE == 1
#if SECOND_GROUP_RELAY_MODE_PIN == 1
    digitalWrite(SECOND_GROUP_RELAY_PIN_1, HIGH);
    digitalWrite(SECOND_GROUP_RELAY_PIN_2, HIGH);
#else
    digitalWrite(SECOND_GROUP_RELAY_PIN_1, LOW);
    digitalWrite(SECOND_GROUP_RELAY_PIN_2, LOW);
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

#if SECOND_GROUP_RELAY_MODE_PIN == 1
    digitalWrite(SECOND_GROUP_RELAY_PIN_1, LOW);
    digitalWrite(SECOND_GROUP_RELAY_PIN_2, LOW);
#else
    digitalWrite(SECOND_GROUP_RELAY_PIN_1, HIGH);
    digitalWrite(SECOND_GROUP_RELAY_PIN_2, HIGH);
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

#if SECOND_GROUP_RELAY_MODE == 2
#if SECOND_GROUP_RELAY_MODE_PIN == 1
            digitalWrite(SECOND_GROUP_RELAY_PIN_1, LOW);
            digitalWrite(SECOND_GROUP_RELAY_PIN_2, LOW);
#else
            digitalWrite(SECOND_GROUP_RELAY_PIN_1, HIGH);
            digitalWrite(SECOND_GROUP_RELAY_PIN_2, HIGH);
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

#if SECOND_GROUP_RELAY_MODE == 2
#if SECOND_GROUP_RELAY_MODE_PIN == 1
        digitalWrite(SECOND_GROUP_RELAY_PIN_1, HIGH);
        digitalWrite(SECOND_GROUP_RELAY_PIN_2, HIGH);
#else
        digitalWrite(SECOND_GROUP_RELAY_PIN_1, LOW);
        digitalWrite(SECOND_GROUP_RELAY_PIN_2, LOW);
#endif
#endif

        is_relay_pulse = true;
        timer_relay_pulse = millis();
    }
}

void setup()
{
    pinMode(FIRST_GROUP_RELAY_PIN_1, OUTPUT);
    pinMode(FIRST_GROUP_RELAY_PIN_2, OUTPUT);
    pinMode(SECOND_GROUP_RELAY_PIN_1, OUTPUT);
    pinMode(SECOND_GROUP_RELAY_PIN_2, OUTPUT);
    relay_off();
	
    button_start.setClickTimeout(50);
	
    Wire.begin();
    display.init();
    display.set_brightness(DTM1650_BRIGHTNESS_MAX);
	
    attachInterrupt(digitalPinToInterrupt(TACHOMETER_PIN), t_interruption, RISING);
}

void loop()
{
    button_start.tick();

    if (button_start.isSingle())
    {
    	if (relay)
    	{
            relay = false;
            relay_off();
            relay_pulse(true);
            tone(BEEP_PIN, 2000, 100);
    	}
        else
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
        t_currentTime = t_interruptionTime;
        t_interruptionCounter = 0;
		interrupts();
        t_RPM = 60000 / ((t_currentTime - t_previousTime) * DIRKI / t_counter);
        t_previousTime = t_currentTime;
        //i++;
        display.write_num(t_RPM);
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

void t_interruption()
{
	t_interruptionTime = millis();
	t_interruptionCounter++;
}

