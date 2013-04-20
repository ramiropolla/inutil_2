#define BOTAO1   7
#define BOTAO2   8
#define SENSOR1  2
#define SENSOR2  3
#define MOTOR_EN 4
#define MOTOR_1  5
#define MOTOR_2  6
#define FIMDECUR 9

#define POS_MIN 0
#define POS_MAX 3700

static boolean direction = false;
static long count = 0;
static int stop_at = -1;

void setup()
{
    pinMode(BOTAO1  , INPUT );
    pinMode(BOTAO2  , INPUT );
    pinMode(SENSOR1 , INPUT );
    pinMode(SENSOR2 , INPUT );
    pinMode(FIMDECUR, INPUT );
    pinMode(MOTOR_EN, OUTPUT);
    pinMode(MOTOR_1 , OUTPUT);
    pinMode(MOTOR_2 , OUTPUT);

    digitalWrite(MOTOR_EN, HIGH);

    attachInterrupt(1, pulso, CHANGE);

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
    static int ob1 = -1;
    static int ob2 = -1;
    byte b1 = digitalRead(BOTAO1  );
    byte b2 = digitalRead(BOTAO2  );
    byte fc = digitalRead(FIMDECUR);

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

    if (fc) {
        count = 0;
    }

    if (ob1 != b1 || ob2 != b2) {
        if        (b1) {
            motor_up();
        } else if (b2) {
            motor_down();
        } else {
            motor_stall();
        }
        ob1 = b1;
        ob2 = b2;
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
    }
}

