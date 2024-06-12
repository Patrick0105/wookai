#include <stdio.h>
#include <string.h>
#include <RtcDS1302.h>
#include "SevSeg.h"

SevSeg sevseg;
const int sw = 8;                      // 按键开关连接至数字接脚第8脚
const int sw2 = 9;                     // 按键开关2连接至数字接脚第9脚
const int debounceDelay = 50;          // 按键开关稳定所需的时间
int val;                               // 按键开关状态
int val2;                              // 按键开关2状态
int swValue = 0;                       // 按键开关数值
unsigned long lastDebounceTime = 0;    // 上一次消除抖动的时间
unsigned long lastDebounceTime2 = 0;   // 上一次消除抖动的时间2
bool displayMode = false;              // 显示模式，默认显示年月日
bool is24HourFormat = true;            // 默认24小时制
bool settingMode = false;              // 设定模式
int settingStep = 0;                   // 当前设定步骤
unsigned long previousMillis = 0;
const long interval = 1000;            // 設置更新間隔為1000毫秒（1秒）

ThreeWire myWire(3, 4, 2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
#define delay_time 6

int pinA = 14;
int pinB = 15;
int pinC = 16;
int pinD = 17;
int pinE = 18;
int pinF = 19;
int pinG = 7;
int D1 = 13;
int D2 = 12;
int D3 = 11;
int D4 = 10;

byte segs[7] = { pinA, pinB, pinC, pinD, pinE, pinF, pinG };

byte seven_seg_digits[10][7] = { { 1,1,1,1,1,1,0 },  // = 0
                                 { 0,1,1,0,0,0,0 },  // = 1
                                 { 1,1,0,1,1,0,1 },  // = 2
                                 { 1,1,1,1,0,0,1 },  // = 3
                                 { 0,1,1,0,0,1,1 },  // = 4
                                 { 1,0,1,1,0,1,1 },  // = 5
                                 { 1,0,1,1,1,1,1 },  // = 6
                                 { 1,1,1,0,0,0,0 },  // = 7
                                 { 1,1,1,1,1,1,1 },  // = 8
                                 { 1,1,1,1,0,1,1 }   // = 9
                             };

void setup() {
    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printYear(compiled);
    printDate(compiled);
    printTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) {
        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected()) {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    } else if (now > compiled) {
        Serial.println("RTC is newer than compile time. (this is expected)");
    } else if (now == compiled) {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    byte numDigits = 4; // 四位数显示器
    byte digitPins[] = {D1, D2, D3, D4};
    byte segmentPins[] = {14, 15, 16, 17, 18, 19, 7, 6};

    bool resistorsOnSegments = true;
    byte hardwareConfig = COMMON_CATHODE; // 共阴极
    bool updateWithDelays = false; // Default. Recommended
    bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
    sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
    sevseg.setBrightness(90);

    Serial.begin(9600);               // 设置串口传输速率为9600bps
    pinMode(sw, INPUT_PULLUP);         // 设置数字第8脚为输入模式(使用内部上拉电阻)
    pinMode(sw2, INPUT_PULLUP);        // 设置数字第9脚为输入模式(使用内部上拉电阻)

    pinMode(pinA, OUTPUT);
    pinMode(pinB, OUTPUT);
    pinMode(pinC, OUTPUT);
    pinMode(pinD, OUTPUT);
    pinMode(pinE, OUTPUT);
    pinMode(pinF, OUTPUT);
    pinMode(pinG, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);
    pinMode(D4, OUTPUT);
}

void loop() {
    val = digitalRead(sw);
    val2 = digitalRead(sw2);

    if (val == LOW && val2 == LOW && (millis() - lastDebounceTime) > debounceDelay) {
        settingMode = !settingMode;
        settingStep = 0;  // 重置设定步骤
        RtcDateTime now = Rtc.GetDateTime(); // 获取当前时间以便进行设置
        splitTime(now);
        lastDebounceTime = millis();
        while (digitalRead(sw) == LOW && digitalRead(sw2) == LOW);  // 等待放開按鍵
    }

    if (settingMode) {
        handleSettingMode();
    } else {
        handleDisplayMode();
    }
}

void handleDisplayMode() {
    static bool displayModeSwitch = false;  // 顯示模式切換
    val = digitalRead(sw);
    if (val == LOW && (millis() - lastDebounceTime) > debounceDelay) {
        displayModeSwitch = !displayModeSwitch;
        lastDebounceTime = millis();
        while (digitalRead(sw) == LOW);  // 等待放開按鍵
    }

    val2 = digitalRead(sw2);
    if (val2 == LOW && (millis() - lastDebounceTime2) > debounceDelay) {
        is24HourFormat = !is24HourFormat;
        lastDebounceTime2 = millis();
        while (digitalRead(sw2) == LOW);  // 等待放開按鍵
    }

    RtcDateTime now = Rtc.GetDateTime();
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        if (displayModeSwitch) {
            printTime(now);
            delayMicroseconds(1000); // 用很短的延遲來防止顯示器更新過快
            printMinuteSecond(now);
            delayMicroseconds(1000); // 用很短的延遲來防止顯示器更新過快
        } else {
            printYear(now);
            delayMicroseconds(1000); // 用很短的延遲來防止顯示器更新過快
            printDate(now);
            delayMicroseconds(1000); // 用很短的延遲來防止顯示器更新過快
        }
    }
}

void handleSettingMode() {
    static int timeParts[4] = {0}; // Array to hold hour and minute digits

    while (settingMode) {
        val = digitalRead(sw);
        val2 = digitalRead(sw2);

        if (val == LOW && (millis() - lastDebounceTime) > debounceDelay) {
            timeParts[settingStep] = (timeParts[settingStep] - 1 + 10) % 10;  // Decrement current setting
            lastDebounceTime = millis();
            while (digitalRead(sw) == LOW);  // 等待放開按鍵
        }

        if (val2 == LOW && (millis() - lastDebounceTime2) > debounceDelay) {
            timeParts[settingStep] = (timeParts[settingStep] + 1) % 10;  // Increment current setting
            lastDebounceTime2 = millis();
            while (digitalRead(sw2) == LOW);  // 等待放開按鍵
        }

        if (val == LOW && val2 == LOW && (millis() - lastDebounceTime) > debounceDelay) {
            // Move to the next setting step
            settingStep++;
            lastDebounceTime = millis();
            while (digitalRead(sw) == LOW && digitalRead(sw2) == LOW);  // 等待放開按鍵

            if (settingStep >= 4) {
                RtcDateTime now = Rtc.GetDateTime();
                RtcDateTime newTime(now.Year(), now.Month(), now.Day(),
                    timeParts[0] * 10 + timeParts[1],
                    timeParts[2] * 10 + timeParts[3],
                    0);  // 設定秒數為 0
                Rtc.SetDateTime(newTime);
                settingMode = false;
            }
        }

        // Display the current setting
        unsigned long startTime = millis();
        for (unsigned long elapsed = 0; elapsed < 1000; elapsed = millis() - startTime) {
            lightDigit1(timeParts[0]);
            delayMicroseconds(1000);
            lightDigit2(timeParts[1]);
            delayMicroseconds(1000);
            lightDigit3(timeParts[2]);
            delayMicroseconds(1000);
            lightDigit4(timeParts[3]);
            delayMicroseconds(1000);
        }
    }
}

void splitTime(const RtcDateTime& dt) {
    static int timeParts[6]; // Array to hold hour, minute, second, etc.
    timeParts[0] = dt.Hour() / 10;
    timeParts[1] = dt.Hour() % 10;
    timeParts[2] = dt.Minute() / 10;
    timeParts[3] = dt.Minute() % 10;
}

void pickDigit(int x) {
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    digitalWrite(D3, LOW);
    digitalWrite(D4, LOW);

    switch(x) {
        case 1: digitalWrite(D4, HIGH); break;
        case 2: digitalWrite(D3, HIGH); break;
        case 3: digitalWrite(D2, HIGH); break;
        case 4: digitalWrite(D1, HIGH); break;    
    }
}

void lightDigit1(byte number) {
    pickDigit(1);  
    lightSegments(number);  
}

void lightDigit2(byte number) {
    pickDigit(2);
    lightSegments(number);  
}

void lightDigit3(byte number) {
    pickDigit(3);
    lightSegments(number);  
}

void lightDigit4(byte number) {
    pickDigit(4);
    lightSegments(number);  
}

void lightSegments(byte number) {
    for (int i = 0; i < 7; i++) {
        digitalWrite(segs[i], seven_seg_digits[number][i]);
    }
}

char transChar(int number) {
    char value;
    switch (number)
    {
        case 0: value = '0'; break;
        case 1: value = '1'; break;
        case 2: value = '2'; break;
        case 3: value = '3'; break;
        case 4: value = '4'; break;
        case 5: value = '5'; break;
        case 6: value = '6'; break;
        case 7: value = '7'; break;
        case 8: value = '8'; break;
        case 9: value = '9'; break;
    }
    return value;
}

void printYear(const RtcDateTime& dt) {
    char datestring[20];
    snprintf_P(datestring, countof(datestring), PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               dt.Month(), dt.Day(), dt.Year(), dt.Hour(), dt.Minute(), dt.Second());
    Serial.print(datestring);

    int y1 = dt.Year() % 10;
    int y2 = (dt.Year() / 10) % 10;
    int y3 = (dt.Year() / 100) % 10;
    int y4 = (dt.Year() / 1000) % 10;

    unsigned long startTime = millis();
    for (unsigned long elapsed = 0; elapsed < 1000; elapsed = millis() - startTime) {
        lightDigit1(y1);
        delayMicroseconds(1000);
        lightDigit2(y2);
        delayMicroseconds(1000);
        lightDigit3(y3);
        delayMicroseconds(1000);
        lightDigit4(y4);
        delayMicroseconds(1000);
    }
}

void printDate(const RtcDateTime& dt) {
    char datestring[20];
    snprintf_P(datestring, countof(datestring), PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               dt.Month(), dt.Day(), dt.Year(), dt.Hour(), dt.Minute(), dt.Second());
    Serial.print(datestring);

    int m1 = dt.Month() % 10;
    int m2 = (dt.Month() / 10) % 10;
    int d1 = dt.Day() % 10;
    int d2 = (dt.Day() / 10) % 10;

    unsigned long startTime = millis();
    for (unsigned long elapsed = 0; elapsed < 1000; elapsed = millis() - startTime) {
        lightDigit1(d1);
        delayMicroseconds(1000);
        lightDigit2(d2);
        delayMicroseconds(1000);
        lightDigit3(m1);
        delayMicroseconds(1000);
        lightDigit4(m2);
        delayMicroseconds(1000);
    }    
}

void printTime(const RtcDateTime& dt) {
    char datestring[20];
    snprintf_P(datestring, countof(datestring), PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               dt.Month(), dt.Day(), dt.Year(), dt.Hour(), dt.Minute(), dt.Second());
    Serial.print(datestring);

    int hour = dt.Hour();
    if (!is24HourFormat) {
        if (hour == 0) {
            hour = 12; // Midnight hour
        } else if (hour > 12) {
            hour -= 12; // Convert to PM hours
        }
    }

    int t1 = hour % 10;
    int t2 = (hour / 10) % 10;
    int t3 = dt.Minute() % 10;
    int t4 = (dt.Minute() / 10) % 10;

    unsigned long startTime = millis();
    for (unsigned long elapsed = 0; elapsed < 1000; elapsed = millis() - startTime) {
        lightDigit1(t3);
        delayMicroseconds(1000);
        lightDigit2(t4);
        delayMicroseconds(1000);
        lightDigit3(t1);
        delayMicroseconds(1000);
        lightDigit4(t2);
        delayMicroseconds(1000);
    }
}

void printMinuteSecond(const RtcDateTime& dt) {
    char datestring[20];
    snprintf_P(datestring, countof(datestring), PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               dt.Month(), dt.Day(), dt.Year(), dt.Hour(), dt.Minute(), dt.Second());
    Serial.print(datestring);

    int s1 = dt.Second() % 10;
    int s2 = (dt.Second() / 10) % 10;
    int m1 = dt.Minute() % 10;
    int m2 = (dt.Minute() / 10) % 10;

    unsigned long startTime = millis();
    for (unsigned long elapsed = 0; elapsed < 1000; elapsed = millis() - startTime) {
        lightDigit1(s1);
        delayMicroseconds(1000);
        lightDigit2(s2);
        delayMicroseconds(1000);
        lightDigit3(m1);
        delayMicroseconds(1000);
        lightDigit4(m2);
        delayMicroseconds(1000);
    }
}
