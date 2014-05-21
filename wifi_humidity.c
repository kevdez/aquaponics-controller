/*
 Humidity sensor to Arduino connections:
 1:     5V
 2:     D2 (Digital 2)
 3:     Not connected
 4:     GND
 
 Water temp. sensor to Arduino connections:
 Black:    A0 (Analog 0)
 Red:      GND
 White:    D4 (Digital 4)
 
 
 WiFi Web Server
 
 A simple web server that shows the value of the analog input pins.
 using a WiFi shield.
 
 This example is written for a network using WPA encryption. For 
 WEP or WPA, change the Wifi.begin() call accordingly.
 
 Circuit:
 * WiFi shield attached
 * Analog inputs attached to pins A0 through A5 (optional)
 */

#include <SPI.h>
#include <WiFi.h>
#include <dht.h>

#define DHT21_PIN 2

//void XML_response(WiFiClient cl);

// Humidity Sensor
dht DHT;

char ssid[] = "NETWORK-SSID";      // your network SSID (name) 
char pass[] = "";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(80);

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  }  

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    //status = WiFi.begin(ssid, pass);
    status = WiFi.begin(ssid);

    // wait 10 seconds for connection:
    delay(10000);
  } 

  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();

}


void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        // buffer first part of HTTP request in HTTP_req array (string)
        // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
        
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/xml");
            client.println("Connection: close");
            client.println();
            // send web page
                    client.println("<!DOCTYPE html>");
                    client.println("<html>");
                    client.println("<head>");
                    client.println("<title>Arduino Read Sensors</title>");
                    client.println("<meta http-equiv=\"refresh\" content=\"1\">");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<h1>Humidity</h1>");
                    client.println("<p>State of humidity</p>");
                    GetHumidityState(client);
                    client.println("</body>");
                    client.println("</html>");
                    break;
  
          }  

        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(100);

    // close the connection:
    client.stop();
  }
}


void GetHumidityState(WiFiClient cl)
{
  cl.print("<p>The humidity is:</p>");
  cl.print(DHT.humidity,1);

  cl.print("<p>The air temeperature is:</p>");
  cl.print(DHT.temperature,1);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}



