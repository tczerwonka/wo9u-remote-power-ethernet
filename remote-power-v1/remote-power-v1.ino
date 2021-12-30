
//////////////////////////////////////////////////////////////////////////
//arduino remote power v1
//starting with sample code from here
//https://www.arduino.cc/en/Tutorial/LibraryExamples/WebServer
//////////////////////////////////////////////////////////////////////////
// code will eventually need...
//   -- some sort of status page
//   -- ability to turn an output on/off for power control
//   -- momentary / addressable output control
//   -- environmental monitoring?
//   -- unit status report?
//////////////////////////////////////////////////////////////////////////


//ethernet shield
#include <SPI.h>
#include <Ethernet.h>

//BMP085 because it's on the shield in my junkbox
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

//1wire stuff
#include <OneWire.h>
#include <DallasTemperature.h>

//ethernet shield stuff
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01 };
IPAddress ip(192, 168, 1, 80);
EthernetServer server(80);

//1wire stuff
#define ONE_WIRE_BUS 3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress cabinet_thermo1 = { 0x28, 0x47, 0x17, 0x15, 0x06, 0x00, 0x00, 0xAB };


//control variables
String readString;
int pwrstrip = 5;
int flexpwr = 4;

//////////////////////////////////////////////////////////////////////////
//setup
//////////////////////////////////////////////////////////////////////////
void setup() {
  //serial
  Serial.begin(9600);

  ////////////////////////////////////////
  //BMP085 related
  ////////////////////////////////////////
  if (!bmp.begin()) {
    Serial.println("BMP085 notfound");
    while (1) {}
  }

  ////////////////////////////////////////
  //ethernet shield-related
  ////////////////////////////////////////
  Ethernet.init(10); 
  Serial.print("WebServer at ");
  Ethernet.begin(mac, ip);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("No eth if found");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardw        are
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("no phy link");
  }

  ////////////////////////////////////////
  //control-relay related
  ////////////////////////////////////////
  pinMode(pwrstrip, OUTPUT);
  pinMode(flexpwr, OUTPUT);


  // start the server
  server.begin();
  Serial.println(Ethernet.localIP());

  //1wire
  sensors.begin();
  sensors.setResolution(cabinet_thermo1, 10);
  
}


//////////////////////////////////////////////////////////////////////////
//loop
//////////////////////////////////////////////////////////////////////////
void loop() {

  sensors.requestTemperatures();
  


  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("client connect");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (readString.length() < 100) {
          readString += c;
        }


        // Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n') {

          Serial.print(readString);

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();

          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          client.println("<H1><center>Radio Control</center></h1>");

          client.print("<p>AC power state: ");
          client.print(digitalRead(pwrstrip));
          client.println("<br />");

          client.println("<input type=button value=\'AC ON\' onmousedown=location.href='/?on5;'>");
          client.println("<input type=button value=\"AC OFF\" onmousedown=location.href='/?off5;'>");
          client.println("<p>");

          client.println("<input type=button value=\'Flex-6300 momentary\' onmousedown=location.href='/?flexpwr;'>");
          client.println("<p>");



          client.println("<table border=1>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.println("<tr>");
            client.print("<td>analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("</td></tr>");
            //client.println("<br />");
          }



          //BMP085 info
          client.print("<tr><td>board temp ");
          float tempF = bmp.readTemperature();
          tempF = ((tempF * 1.8) + 32);
          client.print(tempF); 
          client.print(" F");
          client.println("</td></tr>");
          
          client.print("<tr><td>pressure ");
          client.print((bmp.readPressure() / 100));
          client.print(" Pa");
          client.println("</td></tr>");

          client.print("<tr><td>lead temp ");
          client.println(printTemperature(cabinet_thermo1));
          client.print(" F");
          client.println("</td></tr>");



          client.println("</table>");



          client.println("</html>");
          break;
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
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disco");

    //AC power strip
    if (readString.indexOf("on5") > 0) {
      digitalWrite(pwrstrip, HIGH);
    }
    if (readString.indexOf("off5") > 0) {
      digitalWrite(pwrstrip, LOW);
    }

    //momentary for flx
    if (readString.indexOf("flexpwr") > 0)
    {
      digitalWrite(flexpwr, HIGH);
      delay(500);
      digitalWrite(flexpwr, LOW);
    }

    //clearing string for next read
    readString = "";


  }

}



int printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    return (999);
    //Serial.print("Error getting temperature");
  } else {
    //Serial.print("C: ");
    //Serial.print(tempC);
    //Serial.print(" F: ");
    //Serial.print(DallasTemperature::toFahrenheit(tempC));
    return(DallasTemperature::toFahrenheit(tempC));
  }
}
