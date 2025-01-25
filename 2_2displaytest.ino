//2X2 DISPLAY WITH ESP32 - DEMO DISPLAY EXAMPLE 
#include <WiFi.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <cmath>
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

using namespace std;

bool core1_separate_stack = true; // 8k stack for both prosessors

bool core0initcomplete = false; // both core ->  true when core0 init is finish
                                // core1 is waiting for core 0 to complete
void setClock() {
  
  datetime_t t;
  
  NTP.begin("dk.pool.ntp.org", "time.nist.gov");

  Serial.println ("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print (".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.println(asctime(&timeinfo));
  t.year = timeinfo.tm_year+1900;
  t.month = timeinfo.tm_mon+1;
  t.day = timeinfo.tm_mday;
  t.dotw = timeinfo.tm_wday; // 0 is Sunday, so 5 is Friday
  t.hour = timeinfo.tm_hour;
  t.min = timeinfo.tm_min;
  t.sec = timeinfo.tm_sec;

  rtc_init();//initialiser hardware rtc
  rtc_set_datetime(&t);
  delay(100);// vent for rtc hardware til at initialiser 
}

void setup(void) {
    Serial.begin(115200);
 /* while(!Serial){
    ;
  }*/
  
  int status = -1;
  Serial.println("Hello! 2x2 Display DEMO");
  status = WiFi.begin("ssid", "password");
  if(status != WL_CONNECTED){
    Serial.println("Kunne IKKE connecte netværket, rebooter om 5 sekund...");
    delay(5000);
    rp2040.reboot();
  }//end if
  setClock();
  rp2040.idleOtherCore();// stop core1
    core0initcomplete = true;
  rp2040.resumeOtherCore();//start core1 
}

void loop() {
    delay(250);
  }//end loop
//-------------------------------------core 1 --------------------------
  
//Define and configure Display SPI interface
//LCD1
#define TFT1_CS         5
#define TFT1_MOSI       3
#define TFT1_SCLK       2
#define TFT1_RST        -1
#define TFT1_DC         4

//LCD2
#define TFT2_CS         9
#define TFT2_MOSI       7
#define TFT2_SCLK       6
#define TFT2_RST        -1
#define TFT2_DC         8


//LCD3
#define TFT3_CS         13
#define TFT3_MOSI       11
#define TFT3_SCLK       10
#define TFT3_RST        -1
#define TFT3_DC         12


//LCD4
#define TFT4_CS         28
#define TFT4_MOSI       19
#define TFT4_SCLK       18
#define TFT4_RST        -1
#define TFT4_DC         16

#define BACKLIGHT       14

// OR for the ST7789-based displays, we will use this call
Adafruit_ST7789 *tft1 = new Adafruit_ST7789(TFT1_CS, TFT1_DC, TFT1_MOSI, TFT1_SCLK, TFT1_RST);
Adafruit_ST7789 *tft2 = new Adafruit_ST7789(TFT2_CS, TFT2_DC, TFT2_MOSI, TFT2_SCLK, TFT2_RST);
Adafruit_ST7789 *tft3 = new Adafruit_ST7789(TFT3_CS, TFT3_DC, TFT3_MOSI, TFT3_SCLK, TFT3_RST);
Adafruit_ST7789 *tft4 = new Adafruit_ST7789(TFT4_CS, TFT4_DC, TFT4_MOSI, TFT4_SCLK, TFT4_RST);

#define pi  3.1415926
#define Display_Color_Black     0x0000
#define Display_Color_Blue      0x001F
#define Display_Color_Red       0xF800
#define Display_Color_Green     0x07E0
#define Display_Color_Cyan      0x07FF
#define Display_Color_Magenta   0xF81F
#define Display_Color_Yellow    0xFFE0
#define Display_Color_White     0xFFFF

class clockanalog{
  #define defaultrotation 2
  #define rotationtxt 3
  
   private:
    Adafruit_ST7789 *display;
    uint16_t color_marks = Display_Color_Yellow  ;
    uint16_t color_center = Display_Color_Yellow  ;
    uint16_t color_background = Display_Color_Blue ;
    uint16_t color_sec = Display_Color_Yellow ;
    uint16_t color_hour = Display_Color_Yellow ;
    uint16_t color_min = Display_Color_Yellow;
    uint16_t color_text = Display_Color_Yellow;
    
    uint16_t old_sec = 0 , old_min = 0 , old_hour = 0 ;
    int lasthourmin, hourlastangel;
    int utcOffset = 0, utcoffsetmin = 0; 
    
    bool summertime = false , ampmLastRead = false;

    void printCenter(void){display->fillCircle(120, 120, 20, color_center);}

    void drawCentreString(String buf, int x, int y)
    {
        int16_t x1, y1;
        uint16_t w, h;
        this->display->getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
        this->display->setCursor(x - (w / 2), y);
        this->display->print(buf);       
    }
    
    void setAmPmTxt(bool ampm = false){
        this->printCenter();
        this->display->setRotation(rotationtxt);
        this->display->setTextSize(3);
        this->display->setCursor(103,108);
        this->display->setTextColor(color_background);
        if(ampm)
            this->display->print("PM");
        else
            this->display->print("AM");
            
        this->display->setRotation(defaultrotation);
        
      }//end
      
    void setSeconds(int seconds, int16_t color){
      uint16_t x , y , ix , iy;
        seconds *= 6;
        x = (95 * sin(seconds * (pi/180))) + 120;
        y = (95 * cos(seconds * (pi/180))) + 120;
        ix =(25 * sin(seconds * (pi/180))) + 120;
        iy = (25 * cos(seconds * (pi/180))) + 120;
        display->drawLine((int16_t) iy ,(int16_t)ix ,(int16_t) y ,(int16_t) x ,color );       
      }//end 
      
     void setHour(int hour , int16_t color){
        uint16_t x , y , ix , iy;
        int temp;
 
        if (color != color_background){
            lasthourmin = old_min;
          }//end if
        hour *=30;
        if(lasthourmin > 10)
            hour = hour +((lasthourmin -10)/2); 
        hourlastangel = hour;
        temp = hour -4;
        if (temp < 0)
          temp = 360 - temp;
        x = (70 * sin(hour * (pi/180))) + 120;
        y = (70 * cos(hour * (pi/180))) + 120;
        ix =(25 * sin(temp * (pi/180))) + 120;
        iy = (25 * cos(temp * (pi/180))) + 120;
        this->display->drawLine((int16_t) iy ,(int16_t)ix ,(int16_t) y ,(int16_t) x ,color ); 
        temp = hour + 4;
        if (temp >= 360)
          temp = temp - 360;
        ix =(25 * sin(temp * (pi/180))) + 120;
        iy = (25 * cos(temp * (pi/180))) + 120; 
        this->display->drawLine((int16_t) iy ,(int16_t)ix ,(int16_t) y ,(int16_t) x ,color );
      }// end sethour

    void setMin(int minn , int16_t color){
        uint16_t x , y , ix , iy;
        int temp;

        minn *=6;
        temp = minn -3;
        if (temp < 0)
          temp = 360 - temp;
        x = (95 * sin(minn * (pi/180))) + 120;
        y = (95 * cos(minn * (pi/180))) + 120;
        ix =(25 * sin(temp * (pi/180))) + 120;
        iy = (25 * cos(temp * (pi/180))) + 120;
        this->display->drawLine( iy ,ix , y , x ,color ); 
        temp = minn + 3;
        if (temp >= 360)
          temp = temp - 360;
        ix =(25 * sin(temp * (pi/180))) + 120;
        iy = (25 * cos(temp * (pi/180))) + 120; 
        this->display->drawLine( iy ,ix , y , x ,color );
      }// end setMin

    void drawbackground(void){
        this->display->setRotation(defaultrotation);
        this->display->fillScreen(color_background);
        printCenter();    
        int xpos = 0 , ypos = 118 , wpos = 15 , hpos = 5;
        this->display->fillRect(xpos,ypos,wpos,hpos,color_marks);
        xpos =225 , ypos = 118 , wpos = 15 , hpos = 5;
        this->display->fillRect(xpos,ypos,wpos,hpos,color_marks);
        xpos = 118 , ypos = 0 , wpos = 5 , hpos = 15;
        this->display->fillRect(xpos,ypos,wpos,hpos,color_marks);
        xpos = 118 , ypos = 225 , wpos = 5 , hpos = 15;
        this->display->fillRect(xpos,ypos,wpos,hpos,color_marks);
        for (int i = 30; i < 360 ; i += 30 ){
          float lx , ly , ox , oy; 
            lx = (105 * sin(i * (pi/180)))+ 120;
            ly = (105 * cos(i * (pi/180)))+ 120;
            ox = (120 * sin(i * (pi/180)))+ 120;
            oy = (120 * cos(i * (pi/180)))+ 120;
            this->display->drawLine((int16_t) ly ,(int16_t)lx ,(int16_t) oy ,(int16_t) ox ,color_marks ); 
          }
      }// end drawbackground 
      
      void calculatelokaltime(int *hour ,int *minuttes , int ofsettmin , int offsethour){
       
       if(ofsettmin != 0){
          *minuttes += ofsettmin;
          if(*minuttes > 59){
            ++*hour;
            if(*hour > 23)
              *hour -=24;
            }//end if
            *minuttes -=60;
          }//end if
       *hour += offsethour;
       if(*hour > 23)
          *hour -= 24;
         if(*hour < 0)
            *hour = 24 - *hour;
      }//end
         
    public:
    
    clockanalog(Adafruit_ST7789 *display ,int utctimeoffset = 0, bool summertime = false, int utcoffsetmin= 0, int16_t with = 240 , int16_t height = 240 ){
      this->display = display;
      display->init(with, height);           // Init ST7789 240x240
      drawbackground();
      this->utcOffset = utctimeoffset;
      this->utcoffsetmin = utcoffsetmin;
      this->summertime = summertime;
      this->setAmPmTxt();
      }
      
    void setTime(int hour , int minuttes , int seconds)
    {
      bool redraw = false;
      int red = hourlastangel - (old_sec *6);
      if((red > -10) and (red < 10))
          redraw = true;
      if(summertime == true)
        hour +=1;
      if(old_sec == old_min)
          redraw = true; 
          
      calculatelokaltime(&hour , &minuttes ,utcoffsetmin ,utcOffset );
      
      int utchour = hour;
      if (hour > 12)// reduce from 24h to 12h
        hour -= 12;
      setSeconds(this->old_sec , color_background );
      this->old_sec = seconds;
      setSeconds(seconds , color_sec );
      if(old_min != minuttes)
          redraw = true;
      if( redraw == true ){
              setMin(this->old_min , color_background );
              setMin(minuttes , color_min );
              this->old_min = minuttes;
              setHour(this->old_hour , color_background);
              setHour(hour , color_hour);
              redraw = false;
              this->old_hour = hour;
      }//end if
      if(( utchour > 11) and (this->ampmLastRead == false)){
        this->ampmLastRead = true;
        this->setAmPmTxt(true);
        }//end if
      if((utchour < 12) and (this->ampmLastRead == true)){
        this->ampmLastRead = false;
        this->setAmPmTxt(false);
        }//end if
    }// end setTime

    void settext(String txt){
      //max 8 char
      if (txt.length() > 14)
          txt = txt.substring(0,14);
      this->display->setRotation(rotationtxt);
      this->display->setTextSize(3);
      this->display->setTextColor(color_text ,color_background);
      this->drawCentreString(txt,120,217);
      this->display->setRotation(defaultrotation);
     }//end settext
     
    bool changeUtcOffset(int utcoffset, int utcoffsetmin = 0){
      if((utcoffset < -23) or (utcoffset > 23) )
          return false;
      this->utcOffset = utcoffset;
      this->utcoffsetmin = utcoffsetmin; 
      return true;
      }// end
  };

#define utcCopenhagen 1
#define utcCopenhagentxt "   Esbjerg "
#define utcNewYork -5
#define utcNewYorktxt "   New York "
#define utcTokyo 9
#define utcTokyotxt "Tokyo"
#define utcKyiv 2
#define utcBangkok 7
#define utcBangkoktxt "   Bangkok"
#define utcNewDelhi 5
#define utcNewDelhiMin 30
#define utcNewDelhitxt "  New Delhi"

clockanalog *clk1 = new clockanalog(tft1, utcNewYork , false);
clockanalog *clk2 = new clockanalog(tft2, utcCopenhagen , false );
clockanalog *clk3 = new clockanalog(tft3, utcTokyo , false );
clockanalog *clk4 = new clockanalog(tft4, utcBangkok , false );

void setup1(void) {
  
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);

  Serial.println ("Display Initialized");
  
  while(!core0initcomplete){// wait for core0 to complete
    delay(250);
  }
  clk2->settext(utcCopenhagentxt);
  clk1->settext(utcNewYorktxt);
  clk3->settext(utcTokyotxt);
  clk4->settext(utcBangkoktxt);
  clk4->changeUtcOffset( utcNewDelhi , utcNewDelhiMin );
  clk4->settext(utcNewDelhitxt);
  
}

int last_read_sec = 0;
datetime_t t;

void loop1() {
  rtc_get_datetime(&t);
  if (last_read_sec != t.sec){
    last_read_sec = t.sec;
    clk1->setTime(t.hour,t.min,t.sec);
    clk2->setTime(t.hour,t.min,t.sec);
    clk3->setTime(t.hour,t.min,t.sec);
    clk4->setTime(t.hour,t.min,t.sec);
  }
  
  delay(10);
}
