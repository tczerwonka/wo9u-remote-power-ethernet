
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


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01
};
IPAddress ip(192, 168, 1, 80);
EthernetServer server(80);

//control variables
String readString;
int pwrstrip = 5;
int flexpwr = 4;

//////////////////////////////////////////////////////////////////////////
//setup
//////////////////////////////////////////////////////////////////////////
void setup() {
  ////////////////////////////////////////
  //BMP085 related
  ////////////////////////////////////////
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }

  ////////////////////////////////////////
  //ethernet shield-related
  ////////////////////////////////////////
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardw        are
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  ////////////////////////////////////////
  //control-relay related
  ////////////////////////////////////////
  pinMode(pwrstrip, OUTPUT);
  pinMode(flexpwr, OUTPUT);


  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


//////////////////////////////////////////////////////////////////////////
//loop
//////////////////////////////////////////////////////////////////////////
void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
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

          Serial.println(readString);

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
          client.print("<tr><td>temperature ");
          float tempF = bmp.readTemperature();
          tempF = ((tempF * 1.8) + 32);
          client.print(tempF); 
          client.print(" F");
          client.println("</td></tr>");
          client.print("<tr><td>pressure ");
          client.print((bmp.readPressure() / 100));
          client.print(" Pa");
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
    Serial.println("client disconnected");

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
