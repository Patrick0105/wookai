#include <stdio.h>
#include <string.h>
#include <RtcDS1302.h>
#include "SevSeg.h"
SevSeg sevseg;
const int sw=8;                         //按鍵開關連接至數字接腳第8腳。
const int debounceDelay=20;             //按鍵開關穩定所需的時間
int val;                                //按鍵開關狀態
int swValue=0;                          //按鍵開關數值
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


// 定義七個節段的腳位，將 A, B, C, D, E, F, G 依序放入陣列
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


void setup ()
{
  Rtc.Begin();


  //__DATE__，__TIME__，是程式碼編譯時的日期和時間
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printYear(compiled);
  printDate(compiled);
  printTime(compiled);
  Serial.println();


  //判斷DS1302是否接好
  if (!Rtc.IsDateTimeValid())
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing


    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }


  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }


  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }


  //判斷DS1302上紀綠的時間和編譯時的時間，哪個比較新
  //如果編譯時間比較新，就進行設定，把DS1302上的時間改成新的時間
  //now：DS1302上紀綠的時間，compiled：編譯時的時間
  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    //編譯時間比較新，把DS1302上的時間改成編譯的時間
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
     
  byte numDigits = 4; //四位數顯示器
  byte digitPins[] = {D1,D2,D3,D4};
  byte segmentPins[] = {14, 15, 16, 17, 18, 19, 7, 6};
     
  bool resistorsOnSegments = true;
  byte hardwareConfig = COMMON_CATHODE; //共陰極
  bool updateWithDelays = false; // Default. Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(90);
 
  Serial. begin ( 115200 ) ;               //設定序列埠傳輸速率為9600bps
  pinMode ( sw,INPUT_PULLUP ) ;         //設定數字第8 腳為輸入模式( 使用內部上拉電阻)。


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
void loop ()
{
  RtcDateTime now = Rtc.GetDateTime();


  //printDateTime(now);
  //Serial.println();
  //Serial.println(swValue);
 
  val= digitalRead ( sw ) ;               //讀取按鍵狀態。
  if ( val==LOW ) //按鍵開關被按下？                      
  {
     swValue++;                      //按鍵開關數值加一
     delay ( debounceDelay ) ;           //消除按鍵開關的不穩定狀態( 機械彈跳)。
     while ( digitalRead ( sw ) ==LOW ) //按鍵開關已放開？    
           ;                         //等待放開按鍵開關。
  }
  switch ( swValue )
  {
      case 0:                     //七段顯示器顯示日期
          printYear(now);
          Serial.println();
          break ;
      case 1:                     //七段顯示器顯示時間
          printDate(now);
          Serial.println();
          break ;        
       case 2:
          printTime(now);
          Serial.println();
          break ;
       case 3:
       //進入日期時間設定模式
                    //mode_Setup () ;
                    //for (int i=7;i >= 0;i--)
                        //led_Data [ i ] = '-' ;
                    //print_time () ;       //序列埠監控視窗顯示日期時間
                    break ;            
       case 4: swValue=0; break ;
     }
     //delay ( 1000 ) ;
}


// 選擇顯示的位數 (4:千、3:百、2:十、或 1:個位數)
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


// 點亮個位數
void lightDigit1(byte number) {
  pickDigit(1);  
  lightSegments(number);  
}


// 點亮十位數
void lightDigit2(byte number) {
  pickDigit(2);
  lightSegments(number);  
}


// 點亮百位數
void lightDigit3(byte number) {
  pickDigit(3);
  lightSegments(number);  
}


// 點亮千位數
void lightDigit4(byte number) {
  pickDigit(4);
  lightSegments(number);  
}


// 點亮七段顯示器
void lightSegments(byte number) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segs[i], seven_seg_digits[number][i]);
  }
}


/*
void mode_Setup () //設定模式，由USB 監控視窗設定及顯示日期時間                        
{
    while ( Serial. available () > 0 ) // 當序列埠有資料的時候，將資料拼接到變數comdata        
    {
           comdata += char ( Serial. read ()) ;
           delay ( 2 ) ;
           mark = 1;
    }
    // 以逗號分隔分解comdata 的字串，分解結果變成轉換成數字到numdata[] 陣列
    if ( mark == 1 )
    {
        Serial. print ( "You inputed : " ) ;
        //Serial. println ( comdata ) ;
        for ( int i = 0; i < comdata. length () ; i++ )
        {
            if ( comdata [ i ] == ',' || comdata [ i ] == 0x10 || comdata [ i ] == 0x13 )
            {
                j++;
            }
            else
            {
                numdata [ j ] = numdata [ j ] * 10 + ( comdata [ i ] - '0' ) ;
            }
        }
        // 將轉換好的numdata湊成時間格式，寫入DS1302
        //Time t ( numdata [ 0 ] , numdata [ 1 ] , numdata [ 2 ] , numdata [ 3 ] , numdata [ 4 ] , numdata [ 5 ] , numdata [ 6 ]) ;
        mark = 0;j=0;
        //comdata = String ( "" ) ;            // 清空comdata 變量，以便等待下次輸入
         清空numdata
        for ( int i = 0; i < 7 ; i++ ) numdata [ i ] =0;
    }
}*/


/*void print_time ()
{
                     // 從DS1302 取得當前時間
    memset ( day, 0, sizeof ( day )) ;         // 將星期從數字轉換為名稱
    switch ( t. day )
    {
            case 1: strcpy ( day, "日後" ) ; break ;
            case 2: strcpy ( day, "Monday" ) ; break ;
            case 3: strcpy ( day, "Tuesday" ) ; break ;
            case 4: strcpy ( day, "Wednesday" ) ; break ;
            case 5: strcpy ( day, "Thursday" ) ; break ;
            case 6: strcpy ( day, "Friday" ) ; break ;
            case 7: strcpy ( day, "Saturday" ) ; break ;
    }
    // 將日期代碼格式化湊成buf等待輸出
    snprintf ( buf, sizeof ( buf ) , "%s %04d-%02d-%02d %02d:%02d:%02d" , day, t. yr , t. mon , t. date , t. hr , t. min , t. sec ) ;
    Serial. println ( buf ) ;            // 輸出日期到序列埠
}*/


char transChar ( int number ) // 值轉換成字元函數        
{
     char value;
     switch ( number )
     {
            case 0: value= '0' ; break ;
            case 1: value= '1' ; break ;
            case 2: value= '2' ; break ;
            case 3: value= '3' ; break ;
            case 4: value= '4' ; break ;
            case 5: value= '5' ; break ;
            case 6: value= '6' ; break ;
            case 7: value= '7' ; break ;
            case 8: value= '8' ; break ;
            case 9: value= '9' ; break ;
     }
     return value;
}


//顯示完整年月日時間的副程式  
void printYear(const RtcDateTime& dt)
{
  char datestring[20];
  int yyyy = dt.Year();


  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);


  int y1 = dt.Year()%10;
  int y2 = (dt.Year()/10)%10;
  int y3 = (dt.Year()/100)%10;
  int y4 = (dt.Year()/1000)%10;
 
  unsigned long startTime = millis();
  for (unsigned long elapsed=0; elapsed < 1000; elapsed = millis() - startTime)
  {
    // 多工掃瞄，輪流點亮個、十、百、以及千位數的七段顯示器
     
    // 顯示年分
    // 顯示個位數
    lightDigit1(y1);
    delay(delay_time);
    // 顯示十位數
    lightDigit2(y2);
    delay(delay_time);
    // 顯示百位數
    lightDigit3(y3);            
    delay(delay_time);
    // 顯示千位數
    lightDigit4(y4);
    delay(delay_time);
   }
}


//顯示完整年月日時間的副程式  
void printDate(const RtcDateTime& dt)
{
  char datestring[20];
  int yyyy = dt.Year();


  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);
 
  int m1 = dt.Month()%10;
  int m2 = (dt.Month()/10)%10;
  int d1 = dt.Day()%10;
  int d2 = (dt.Day()/10)%10;


  unsigned long startTime = millis();
  for (unsigned long elapsed=0; elapsed < 1000; elapsed = millis() - startTime)
  {
    // 多工掃瞄，輪流點亮個、十、百、以及千位數的七段顯示器
     
    // 顯示年分
    // 顯示個位數
    lightDigit1(d1);
    delay(delay_time);
    // 顯示十位數
    lightDigit2(d2);
    delay(delay_time);
    // 顯示百位數
    lightDigit3(m1);            
    delay(delay_time);
    // 顯示千位數
    lightDigit4(m2);
    delay(delay_time);
   }    
}


//顯示完整年月日時間的副程式  
void printTime(const RtcDateTime& dt)
{
  char datestring[20];
  int yyyy = dt.Year();


  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);


  int t1 = dt.Minute()%10;
  int t2 = (dt.Minute()/10)%10;
  int t3 = dt.Second()%10;
  int t4 = (dt.Second()/10)%10;
 
  unsigned long startTime = millis();
  for (unsigned long elapsed=0; elapsed < 1000; elapsed = millis() - startTime)
  {
    // 多工掃瞄，輪流點亮個、十、百、以及千位數的七段顯示器
     
    // 顯示年分
    // 顯示個位數
    lightDigit1(t3);
    delay(delay_time);
    // 顯示十位數
    lightDigit2(t4);
    delay(delay_time);
    // 顯示百位數
    lightDigit3(t1);            
    delay(delay_time);
    // 顯示千位數
    lightDigit4(t2);
    delay(delay_time);
   }
}


