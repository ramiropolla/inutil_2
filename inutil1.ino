#include <Servo.h>

#define MOTOR_EN 3
#define MOTOR_1  4
#define MOTOR_2  8
#define FIMDECUR 10
#define SENSOR1  2
#define SENSOR2  6

#define FINGER_EN  5
#define FINGER_1   6
#define FINGER_2   7
#define FIMDODEDO  9

#define POS_B1 150
#define POS_B2 1280
#define POS_B3 2400

#define BOTAO_1 11
#define BOTAO_2 12
#define BOTAO_3 13

#define POS_MIN 0
#define POS_MAX 2800

static boolean direction = false;
static boolean do_push = false;
static volatile long count = 0;
static int stop_at = -1;
static boolean push_button = false;

void setup()
{
    unsigned long start_time;

    pinMode(SENSOR1 , INPUT);
    pinMode(SENSOR2 , INPUT);

    pinMode(BOTAO_1, INPUT);
    pinMode(BOTAO_2, INPUT);
    pinMode(BOTAO_3, INPUT);

    pinMode(FIMDECUR, INPUT);
    pinMode(MOTOR_EN, OUTPUT);
    pinMode(MOTOR_1 , OUTPUT);
    pinMode(MOTOR_2 , OUTPUT);
    pinMode(FINGER_EN, OUTPUT);
    pinMode(FINGER_1 , OUTPUT);
    pinMode(FINGER_2 , OUTPUT);

    digitalWrite(MOTOR_EN, HIGH);
    digitalWrite(FINGER_EN, LOW);

    attachInterrupt(0, pulso, CHANGE);

    Serial.begin(9600);
    Serial.println("STARTING");

    // move pro comeco
    motor_up();
    delay(100);
    motor_down();
    while (!digitalRead(FIMDECUR)) {};

    // desce dedo
    finger_down();
    start_time = millis();
    while (!digitalRead(FIMDODEDO) && (millis() - start_time < 300));
    finger_stall();

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

static void finger_up()
{
    Serial.print("finger up ");
    Serial.println(count);
    digitalWrite(FINGER_1, HIGH);
    digitalWrite(FINGER_2, LOW );
    digitalWrite(FINGER_EN, HIGH);
}
static void finger_down()
{
    Serial.print("finger down ");
    Serial.println(count);
    digitalWrite(FINGER_1, LOW );
    digitalWrite(FINGER_2, HIGH);
    analogWrite(FINGER_EN,  200);
}
static void finger_stall()
{
    Serial.print("finger stall ");
    Serial.println(count);
    digitalWrite(FINGER_1, LOW);
    digitalWrite(FINGER_2, LOW);
}

static void go_to_pos(int pos)
{
    if (pos >= POS_MIN && pos <= POS_MAX && pos != stop_at) {
        Serial.print("go to pos ");
        Serial.println(pos);
        stop_at = pos;
        if        (count > stop_at) {
            motor_down();
        } else if (count < stop_at) {
            motor_up();
        } else {
            motor_stall();
        }
    }
}

static void bater_botao()
{
    unsigned long start_time;
    finger_up();
    delay(200);
    finger_stall();
    delay(500);
    finger_down();
    start_time = millis();
    while (!digitalRead(FIMDODEDO) && (millis() - start_time < 200));
    finger_stall();
    push_button = false;
    do_push = false;
}

static int distancia(int pos)
{
    int res = pos - count;
    if (res < 0)
        res = -res;
    return res;
}

void loop()
{
    byte b1, b2, b3;
    byte fc = digitalRead(FIMDECUR);

    if (push_button) {
        bater_botao();
    }

    b1 = digitalRead(BOTAO_1);
    b2 = digitalRead(BOTAO_2);
    b3 = digitalRead(BOTAO_3);

    if (b1 && !b2 && !b3) {
        do_push = true;
        go_to_pos(POS_B1);
        delay(100);
    } else if (!b1 && b2 && !b3) {
        do_push = true;
        go_to_pos(POS_B2);
        delay(100);
    } else if (!b1 && !b2 && b3) {
        do_push = true;
        go_to_pos(POS_B3);
        delay(100);
    } else if (b1 && b2 && !b3) {
        do_push = true;
        go_to_pos(POS_B1);
        delay(100);
        go_to_pos(POS_B2);
        delay(100);
    } else if (!b1 && b2 && b3) {
        do_push = true;
        go_to_pos(POS_B1);
        delay(100);
        go_to_pos(POS_B2);
        delay(100);
        go_to_pos(POS_B3);
        delay(100);
    } else {
        if (count > 0)
            go_to_pos(0);
    }
    do_push = false;

    if (Serial.available()) {
        int pos = Serial.parseInt();
        Serial.print(pos);
        Serial.print(" ");
        Serial.print(count);
        Serial.print(" ");
        Serial.println(stop_at);
        if (pos)
            do_push = true;
        go_to_pos(pos);
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

    if (count >= POS_MAX || ((count >= 0) && (stop_at >= 0) && (stop_at == count) && do_push)) {
        motor_stall();
        push_button = true;
    }
}

