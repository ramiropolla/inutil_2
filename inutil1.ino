#include <Servo.h>

Servo sv;

#define MOTOR_EN 3
#define MOTOR_1  4
#define MOTOR_2  8
#define FIMDECUR 10
#define SERVO    9
#define SENSOR1  2
#define SENSOR2  6

#define POS_MIN 0
#define POS_MAX 3500

static boolean direction = false;
static long count = 0;
static int stop_at = -1;
static boolean push_button = false;

void setup()
{
    pinMode(SENSOR1 , INPUT );
    pinMode(SENSOR2 , INPUT );
    pinMode(FIMDECUR, INPUT );
    pinMode(MOTOR_EN, OUTPUT);
    pinMode(MOTOR_1 , OUTPUT);
    pinMode(MOTOR_2 , OUTPUT);

    digitalWrite(MOTOR_EN, HIGH);

    attachInterrupt(0, pulso, CHANGE);

    sv.attach(SERVO);
    sv.write(0);

    Serial.begin(9600);
    Serial.println("STARTING");

    // move pro comeco
    motor_up();
    delay(100);
    motor_down();
    while (!digitalRead(FIMDECUR)) {};

    while (Serial.available()) { Serial.read(); }
}

static void motor_up()
{
    Serial.print("up ");
    Serial.println(count);
    direction = true;
    digitalWrite(MOTOR_1, LOW );
    digitalWrite(MOTOR_2, HIGH);
}
static void motor_down()
{
    Serial.print("down ");
    Serial.println(count);
    direction = false;
    digitalWrite(MOTOR_1, HIGH);
    digitalWrite(MOTOR_2, LOW );
}
static void motor_stall()
{
    Serial.print("stall ");
    Serial.println(count);
    digitalWrite(MOTOR_1, LOW);
    digitalWrite(MOTOR_2, LOW);
    stop_at = -1;
}

void loop()
{
    byte fc = digitalRead(FIMDECUR);

    if (push_button) {
        byte pos;
        for(pos = 0; pos < 150; pos++) {
            sv.write(pos);
            delay(4);
        }
        delay(1000);
        sv.write(0);
        push_button = false;
        Serial.print("finger ");
        Serial.println(count);
    }

    if (Serial.available()) {
        int pos = Serial.parseInt();
        Serial.print(pos);
        Serial.print(" ");
        Serial.print(count);
        Serial.print(" ");
        Serial.println(stop_at);
        if (pos >= POS_MIN && pos <= POS_MAX) {
            stop_at = pos;
            if        (count > stop_at) {
                motor_down();
            } else if (count < stop_at) {
                motor_up();
            } else {
                motor_stall();
            }
        }
        while (Serial.available()) { Serial.read(); }
    }

    if (fc && count) {
        int cc = count;
        Serial.print("fim de curso ");
        Serial.println(cc);
        count = 0;
    }
}

void pulso()
{
    if (direction) {
        count++;
    } else {
        count--;
    }

    if ((count >= 0) && (stop_at >= 0) && (stop_at == count)) {
        motor_stall();
        push_button = true;
    }
}

