/********** ITDB02 Networked Temperature **********
This program requires the ITDB02_Graph16 library.
It uses the OneWire Library and an Arduino-compatable
ethernet shield The display can be used with small or
large fonts, and in portrait or landscape mode.

The OneWire runs on digital pin 9 to avoid a conflict
with the LCD software.
**************************************************/

/********** Various Defines **********/

// #define SerialDebug    // Used for debugging with the Serial Terminal, disable for normal use
// #define GetAddresses   // Used to get Sensor Addresses 
// #define EthernetDebug  // Used to debug Ethernet problems

// #define P15X16         // define 15 column by 16 row display in portrait mode
// #define P30X21         // define 30 column by 21 row display in portrait mode
// #define L40X16         // define 40 column by 20 row display in landscape mode
// #define L20X12         // define 20 column by 15 row display in landscapr mode

#include <Wire.h>
#include <ITDB02_Graph16.h>
#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>

// Declare which fonts we will be using
#if defined P30X21
extern uint8_t SmallFont[];
char *blankLine = "                              ";
#endif

#if defined L40X16
extern uint8_t SmallFont[];
char *blankLine = "                                        ";
#endif

#if defined P15X16
extern uint8_t BigFont[];
char *blankLine = "               ";
#endif

#if defined L20X12
extern uint8_t BigFont[];
char *blankLine = "                    ";
#endif

//myGLCD(RS,WR,CS,RST,ALE,mode);
ITDB02 myGLCD(A1,A2,A0,A4,A5,2);   //

const byte nbsp            = 0x20; // space character
const byte zeroChar        = 0x30; // zero character
const char degChar         = 0xdf; // degree character
const byte orientLandscape = 0x01;
const byte orientPortrait  = 0x00;

// if set to TRUE, blank lines are printed for lines with no probe
// if set to FALSE, lines with no probe are marked in yellow
bool showBlankLine = TRUE;

/********** Ethernet / Web Stuff **********
 Enter a MAC address and IP address for your controller below.
 The IP address will be dependent on your local network:
 The Ethernet hardware utilizes digital pins 10, 11, 12, and 13.
 ********** Ethernet / Web Stuff **********/
byte mac[] = { 0xDE, 0xAD, 0xBA, 0xBE, 0x00, 0x04 };
IPAddress ip(192,168,1,175);
const int BUFSIZE = 100; // buffer for internet request
const int rowSize = 4;

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

/********** OneWire Stuff ************/

const int  dsDataPin  = 9; // One Wire data Bus
OneWire  ds(dsDataPin); // OneWire bus on pin 2
bool showCelsius = FALSE;

typedef struct
{
  bool  sensorActive; // set to "TRUE" if you put a sensor address in the "addr" array.
  char  *sensorName;
  char  *xmlName;
  byte  addr[8];
  float deg;
  int   tooCold;  // if temperature is below this value, display is blue
  int   tooHot;   // if temperature is above this value, display is red 
} Sensor;

#if defined P15X16
const int  maxSensors = 16; // total number of DS18B20 sensors
Sensor ds18[maxSensors] =
{

  {FALSE, "Sensor  1", "Sensor1",  {0,0,0,0,0,0,0,0}, 0.0, 90, 100},
  {FALSE, "Sensor  2", "Sensor2",  {0,0,0,0,0,0,0,0}, 0.0, 70, 80},
  {FALSE, "Sensor  3", "Sensor3",  {0,0,0,0,0,0,0,0}, 0.0, 70, 90},
  {FALSE, "Sensor  4", "Sensor4",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  5", "Sensor5",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  6", "Sensor6",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  7", "Sensor7",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  8", "Sensor8",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  9", "Sensor9",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 10", "Sensor10", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 11", "Sensor11", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 12", "Sensor12", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 13", "Sensor13", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 14", "Sensor14", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 15", "Sensor15", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 16", "Sensor16", {0,0,0,0,0,0,0,0}, 0.0, 45, 55}
};
#endif

#if defined L20X12
const int  maxSensors = 12; // total number of DS18B20 sensors
Sensor ds18[maxSensors] =
{
  {FALSE, "Sensor  1", "Sensor1",  {0,0,0,0,0,0,0,0}, 0.0, 90, 100},
  {FALSE, "Sensor  2", "Sensor2",  {0,0,0,0,0,0,0,0}, 0.0, 70, 80},
  {FALSE, "Sensor  3", "Sensor3",  {0,0,0,0,0,0,0,0}, 0.0, 70, 90},
  {FALSE, "Sensor  4", "Sensor4",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  5", "Sensor5",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  6", "Sensor6",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  7", "Sensor7",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  8", "Sensor8",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor  9", "Sensor9",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 10", "Sensor10", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 11", "Sensor11", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor 12", "Sensor12", {0,0,0,0,0,0,0,0}, 0.0, 45, 55}
};
#endif

#if defined P30X21
const int  maxSensors = 21; // total number of DS18B20 sensors
Sensor ds18[maxSensors] =
{
  {FALSE, "Sensor      1", "Sensor1",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor      2", "Sensor2",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor      3", "Sensor3",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor      4", "Sensor4",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor      5", "Sensor5",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor      6", "Sensor6",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor      7", "Sensor7",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor      8", "Sensor8",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor      9", "Sensor9",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     10", "Sensor10", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     11", "Sensor11", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     12", "Sensor12", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     13", "Sensor13", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     14", "Sensor14", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     15", "Sensor15", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     16", "Sensor16", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     17", "Sensor17", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     18", "Sensor18", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     19", "Sensor19", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     20", "Sensor20", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor     21", "Sensor21", {0,0,0,0,0,0,0,0}, 0.0, 45, 55}
};
#endif

#if defined L40X16
const int  maxSensors = 16; // total number of DS18B20 sensors
Sensor ds18[maxSensors] =
{
  {FALSE, "Sensor                  1", "Sensor1",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                  2", "Sensor2",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                  3", "Sensor3",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                  4", "Sensor4",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                  5", "Sensor5",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                  6", "Sensor6",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                  7", "Sensor7",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                  8", "Sensor8",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                  9", "Sensor9",  {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                 10", "Sensor10", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                 11", "Sensor11", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                 12", "Sensor12", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                 13", "Sensor13", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                 14", "Sensor14", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                 15", "Sensor15", {0,0,0,0,0,0,0,0}, 0.0, 45, 55},
  {FALSE, "Sensor                 16", "Sensor16", {0,0,0,0,0,0,0,0}, 0.0, 45, 55}
};
#endif

/********** IMPORTANT!! **********
BE SURE TO SET THIS VALUE TO THE
   NUMBER OF ACTIVE SENSORS!!
*********** IMPORTANT!! **********/
int activeSensors = 0;
int cnt = 0;

void setup()
{
  
#if defined SerialDebug || defined GetAddresses || defined EthernetDebug
  Serial.begin(9600);
  delay(1000);
  Serial.println(F("Serial Debug running"));
#endif

#if defined P15X16 || defined P30X21
  myGLCD.InitLCD(PORTRAIT);
#endif

#if defined L40X16 || defined L20X12
  myGLCD.InitLCD(LANDSCAPE);
#endif

#if defined P15X16 || defined L20X12
  myGLCD.setFont(BigFont);
#endif

#if defined L40X16 || defined P30X21
  myGLCD.setFont(SmallFont);
#endif

#if defined SerialDebug
  Serial.println(F("InitLCD"));
#endif

  myGLCD.clrScr();
  myGLCD.setBackColor(0,0,0);
  myGLCD.fillScr(0,0,0);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
#if defined SerialDebug || defined EthernetDebug
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
#endif
}

void checkEthernet(void)
{
  char clientline[BUFSIZE];
  int index = 0;

 // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    
    Serial.println("new client");

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        // If it isn't a new line, add the character to the buffer
        if (c != '\n' && c != '\r')
        {
          clientline[index] = c;
          index++;
          // are we too big for the buffer? start tossing out data
          if (index >= BUFSIZE) 
            index = BUFSIZE -1;
          
          // continue to read more data!
          continue;
        }
        
        // got a \n or \r new line, which means the string is done
        clientline[index] = 0;
        
        // Print it out for debugging
        
        Serial.println(clientline);


        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          if((strstr(clientline, "GET / ") != 0) ||
             (strstr(clientline, "index.htm") != 0) ||
             (strstr(clientline, "index.html") != 0))
          {
            // send a standard http response header
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: text/html"));
            client.println(F("Connnection: close"));
            client.println();
            client.println(F("<!DOCTYPE HTML>"));
            client.println("<html><head>");
                    // add a meta refresh tag, so the browser pulls again every 5 seconds:
            client.println(F("<meta http-equiv=\"refresh\" content=\"5\">"));
            client.println(F("</head>"));
            client.println(F("<body><table border=\"5\" align=\"center\"><tr>"));
            if(activeSensors == 0)
            {
              client.println(F("<td align=\"center\" valign=\"center\"><font size=\"10\" color=\"red\">&nbsp;&nbsp;No Sensors Selected&nbsp;&nbsp;</font></td></tr></table></body></html>"));
              break;
            }else{
              for (int n=0; n<activeSensors; n++)
              {
                if(n<=maxSensors)
                {
                  client.print(F("<td align=\"center\" valign=\"center\"><font size=\"10\">&nbsp;&nbsp;"));
                  if((int)ds18[n].deg > ds18[n].tooHot)
                  {
                    client.print(F("<font color=\"red\">"));
                  }else if((int)ds18[n].deg < ds18[n].tooCold){
                    client.print(F("<font color=\"blue\">"));
                  }else{
                    client.print(F("<font color=\"green\">"));
                  }
                  client.print(ds18[n].sensorName);
                  client.print(F("&nbsp;&nbsp;<br />"));
                  client.print(ds18[n].deg, 0);
                  if (showCelsius == TRUE)
                  {
                    client.print(F("&deg;C"));
                  }else{
                    client.print(F("&deg;F"));
                  }
                  client.println(F("</font></font></td>"));
                }
                if((n+1) % rowSize == 0)
                {
                  client.println(F("</tr><tr>"));
                }
              }
              client.println(F("</tr></table></body>"));
              client.println(F("</html>"));
              break;
            }
          }else if (strstr(clientline, "buttcrack.xml") !=0){
#if defined SerialDebug
            Serial.print(F("Sending buttcrack.xml..."));
#endif
            client.println(F("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"));
            client.println(F("<!--Arduino Buttcrack-->"));
            client.println(F("<buttcrack>"));
            for (int n = 0; n < activeSensors; n++)
            {
              client.print(F("<"));
              client.print(ds18[n].xmlName);
              client.print(F(">"));
              client.print(ds18[n].deg,0);
              client.print(F("</"));
              client.print(ds18[n].xmlName);
              client.println(F(">"));
            }
            client.println(F("</buttcrack>"));
            delay(1);
            client.stop();
          }else{
            // everything else is a 404
            client.println(F("HTTP/1.1 404 Not Found"));
            client.println(F("Content-Type: text/html"));
            client.println();
            client.println(F("<h2>File Not Found</h2>"));
            delay(1);
            client.stop();
          }
          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          } else if (c != '\r') {
          // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
#if defined SerialDebug || defined EthernetDebug
    Serial.println(F("client disonnected"));
#endif
  }
}

void getOneWire(void)
{
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];

#if defined SerialDebug
  Serial.print(F("ROM "));
  Serial.print(cnt);
  Serial.print(F(" = "));
  for( i = 0; i < 8; i++)
  {
    Serial.write(nbsp);
    if(ds18[cnt].addr[i] < 16){Serial.write(zeroChar);} // prepend hex 0-15 with a 0
    Serial.print(ds18[cnt].addr[i], HEX);
  }

  if (OneWire::crc8(ds18[cnt].addr, 7) != ds18[cnt].addr[7]) {
      Serial.println(F("CRC is not valid!"));
      return;
  }
  Serial.println();
#endif
  if (ds18[cnt].sensorActive == TRUE)
  {
  type_s = 0;

  ds.reset();
  ds.select(ds18[cnt].addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
  
  delay(750);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(ds18[cnt].addr);    
  ds.write(0xBE);         // Read Scratchpad

#if defined SerialDebug
  Serial.print(F("  Data = "));
  if(present < 16){Serial.write(zeroChar);} // prepend hex 0-15 with a 0
  Serial.print(present, HEX);
  Serial.write(nbsp);
#endif
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
#if defined SerialDebug
    if(data[i] < 16){Serial.write(zeroChar);} // prepend hex 0-15 with a 0
    Serial.print(data[i], HEX);
    Serial.write(nbsp);
#endif
  }
#if defined SerialDebug
  Serial.print(F(" CRC="));
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();
#endif
  // convert the data to actual temperature
  unsigned int raw = (data[1] << 8) | data[0];
  unsigned char t_mask[4] = {0x7, 0x3, 0x1, 0x0};
  byte cfg = (data[4] & 0x60) >> 5;
  raw &= ~t_mask[cfg];
  if( showCelsius == TRUE)
  {
    ds18[cnt].deg = ((float)raw / 16.0);
  }else{
    ds18[cnt].deg = ((((float)raw / 16.0) * 1.8) + 31.0); //the raw value is Celsius, convert to Fahrenheit
  }

#if defined SerialDebug
  Serial.print("  Temperature = ");
  Serial.print(ds18[cnt].deg);
  if( showCelsius == TRUE)
  {
    Serial.println(" Celsius");
  }else{
    Serial.println(" Fahrenheit");
  }
#endif
  }
  if( ++cnt >= maxSensors){cnt = 0;}

}

void displayLCD(void)
{
#if defined SerialDebug
  Serial.println(F("Entering displayLCD()"));
#endif
// Clear the screen and draw the frame
//  myGLCD.clrScr();
  
  myGLCD.setColor(255,255,255);
  myGLCD.setBackColor(0, 0, 0);
#if defined P30X21
  for (int x=0, y=0;x<310;x+=15,y++)
#endif
#if defined P15X16
  for (int x=0, y=0;x<319;x+=20,y++)
#endif
#if defined L40X16
  for (int x=0, y=0;x<230;x+=15,y++)
#endif
#if defined L20X12
  for (int x=0, y=0;x<239;x+=20,y++)
#endif
  {
    if(ds18[y].sensorActive == TRUE)
    {
      if(ds18[y].deg > ds18[y].tooHot)
      {
        myGLCD.setBackColor(255, 0, 0);
      }else if(ds18[y].deg < ds18[y].tooCold){
        myGLCD.setBackColor(0, 0, 255);
      }else{
        myGLCD.setBackColor(0, 255, 0);
      }
#if defined P30X21
//      myGLCD.print(blankLine,0,x);
      myGLCD.print(ds18[y].sensorName,0,x);
      int temp = (int) ds18[y].deg;
      if(temp >= 100)
      {
        myGLCD.print(" = ", (13*8), x);
        myGLCD.printNumI(temp, (17*8), x);
      }else{
        myGLCD.print(" =  ", (13*8), x);
        myGLCD.printNumI(temp, (17*8), x);
      }
#endif
#if defined P15X16
//      myGLCD.print(blankLine,0,x);
      myGLCD.print(ds18[y].sensorName,0,x);
      int temp = (int) ds18[y].deg;
      if(temp >= 100)
      {
        myGLCD.print(" = ", (8*16), x);
        myGLCD.printNumI(temp, (11*16), x);
      }else{
        myGLCD.print(" =  ", (9*16), x);
        myGLCD.printNumI(temp, (12*16), x);
      }
#endif
#if defined L40X16
//      myGLCD.print(blankLine,0,x);
      myGLCD.print(ds18[y].sensorName,0,x);
      int temp = (int) ds18[y].deg;
      if(temp >= 100)
      {
        myGLCD.print(" = ", (24*8), x);
        myGLCD.printNumI(temp, (28*8), x);
      }else{
        myGLCD.print(" =  ", (25*8), x);
        myGLCD.printNumI(temp, (29*8), x);
      }
#endif
#if defined L20X12
//      myGLCD.print(blankLine,0,x);
      myGLCD.print(ds18[y].sensorName,0,x);
      int temp = (int) ds18[y].deg;
      if(temp >= 100)
      {
        myGLCD.print(" = ", (9*16), x);
        myGLCD.printNumI(temp, (12*16), x);
      }else{
        myGLCD.print(" =  ", (9*16), x);
        myGLCD.printNumI(temp, (13*16), x);
      }
#endif
    }else{
      if(showBlankLine == FALSE)
      {
        myGLCD.setColor(255,0,0);
        myGLCD.setBackColor(255, 255, 0);
        myGLCD.print(ds18[y].sensorName,0,x);
#if defined P30X21
        myGLCD.print(" NOT ACTIVE ", (13*8), x);
#endif
#if defined L40X16
        myGLCD.print(" NOT ACTIVE ", (25*8), x);
#endif
#if defined P15X16
        myGLCD.print(" OFF ", (9*16), x);
#endif
#if defined L20X12
        myGLCD.print(" NOT ACTIVE", (9*16), x);
#endif
        myGLCD.setColor(255,255,255);
      }else{
        myGLCD.setBackColor(0,0,0);
        myGLCD.setColor(255,255,255);
        myGLCD.print(blankLine, 0, x);
      }
    }
  }
}

#if defined SerialDebug || defined GetAddresses
void displayAddresses(void)
{

/********** Get Sensor Addresses **********
   Used to get initial values for Sensor structure. Enable SerialDebug, and obtain DS18B20 addresses
   from the serial terminal. Plug those values into the Sensor's "addr" array, re-compile and upload
   with SeralDebug disabled. Note that when SerialDebug is enabled, response times on buttons and
   display are greatly reduced, so be sure to disable SerialDebug when not needed.
******************************************/

 byte addr[8];
 int cntx = 0;
 
  while ( ds.search(addr))
  {
    Serial.print(F("Sensor "));
    Serial.print(cntx);
    Serial.print(F("= {"));
    for( int i = 0; i < 8; i++)
    {
      Serial.print(F("0x"));
      if(addr[i] < 16){Serial.write(0x30);} // prepend hex 0-15 with a 0
      Serial.print(addr[i], HEX);
      if(i < 7){Serial.print(F(","));}
    }
    Serial.println(F("}"));
    cntx++;
    delay(500);
  }
  Serial.print(cntx);
  Serial.print(F(" Sensor"));
  if(cntx == 1)
  {
    Serial.println(F(" detected"));
  }else{
     Serial.println(F("s detected"));
 }
  
  ds.reset_search();
}
#endif

void loop()
{
#if defined SerialDebug || defined GetAddresses 
  displayAddresses();
#endif
  getOneWire();
  checkEthernet();
  displayLCD();
}

