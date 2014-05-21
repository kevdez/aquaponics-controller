/* (Editing comments and code WIP)

Refer to wifi_ajax0 to see a more complete set of comments.

 Humidity sensor to Arduino connections:
 1:     5V
 2:     D2 (Digital 2)
 3:     Not connected
 4:     GND
 
 Water temp. sensor to Arduino connections:
 Black:    A0 (Analog 0)
 Red:      GND
 White:    D0 (Digital 0) 
 */

#include <SPI.h>
#include <WiFi.h>
#include <SD.h>
#include <dht.h>
#include <OneWire.h>

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   50
#define DHT21_PIN 2
#define H20TEMP_PIN 0
#define EVERYHOUR 1

dht DHT; // Humidity sensor
float temp; // Water temperature sensor
char ssid[] = "UCInet Mobile Access";      // your network SSID (name) 
char pass[] = "";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)


int status = WL_IDLE_STATUS;

WiFiServer server(80);
File webFile;               // the web page file on the SD card
File dataFile;
char HTTP_req[REQ_BUF_SZ] = {
  0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer

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
    status = WiFi.begin(ssid);
    //status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  } 


  // initialize SD card
  Serial.println("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }
  Serial.println("SUCCESS - SD card initialized.");
  // check for index.htm file
  if (!SD.exists("index.htm")) {
    Serial.println("ERROR - Can't find index.htm file!");
    return;  // can't find index file
  }
  Serial.println("SUCCESS - Found index.htm file.");

  pinMode(H20TEMP_PIN,OUTPUT);

  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();

}


void loop() {
  int timeInterval = 3600;
  int counter = 0;  
  
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
        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c;          // save HTTP request character
          req_index++;
        }
        // last line of client request is blank and ends with \n
        // respond to client only after last line received
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          // remainder of header follows below, depending on if
          // web page or XML page is requested
          // Ajax request - send XML file
          if (StrContains(HTTP_req, "ajax_inputs")) {
            // send rest of HTTP header
            client.println("Content-Type: text/xml");
            client.println("Connection: keep-alive");
            client.println();
            // send XML file containing input states
            XML_response(client);
          }
          else {  // web page request
            // send rest of HTTP header
            client.println("Content-Type: text/html");
            client.println("Connection: keep-alive");
            client.println();
            // send web page
            webFile = SD.open("index.htm");        // open web page file
            if (webFile) {
              while(webFile.available()) {
                client.write(webFile.read()); // send web page to client
              }
              webFile.close();
            }
          }
          // display received HTTP request on serial port
          Serial.print(HTTP_req);
          // reset buffer index and all buffer elements to 0
          req_index = 0;
          StrClear(HTTP_req, REQ_BUF_SZ);
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
    
    File dataFile = SD.open("datalog.txt", FILE_WRITE);    
    String dataString = "";
    temp = readTemp();
    DHT.read21(DHT21_PIN);
    for (counter%(timeInterval*EVERYHOUR)==0)
    {
      dataString = String(DHT.humidity + ", " + DHT.temperature + ", " + temp);
         // if the file is available, write to it:
        if (dataFile) {
          dataFile.println(dataString);
          dataFile.close();
          // print to the serial port too:
          Serial.println(dataString);
        }  
        // if the file isn't open, pop up an error:
        else {
          Serial.println("error opening datalog.txt");
        }       
    }
    
    // give the web browser time to receive the data
    delay(1000);
    counter += 1;
    // close the connection:
    client.stop();
  }
}

// send the XML file with switch statuses and analog value
void XML_response(WiFiClient cl)
{
  cl.print("<?xml version = \"1.0\" ?>");
  cl.print("<inputs>");

  cl.print("<button1>");
  DHT.read21(DHT21_PIN);
  cl.print(DHT.humidity,1);
  cl.print("</button1>");

  cl.print("<button2>");
  cl.print(DHT.temperature,1);
  cl.print("</button2>");

  // read analog pin A2
  cl.print("<analog1>");
  temp = readTemp();
  cl.print(temp);
  cl.print("</analog1>");

  cl.print("</inputs>");
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
  char found = 0;
  char index = 0;
  char len;

  len = strlen(str);

  if (strlen(sfind) > len) {
    return 0;
  }
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    }
    else {
      found = 0;
    }
    index++;
  }

  return 0;
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


float readTemp(void){   //the read temperature function
  float v_out;             //voltage output from temp sensor 
  float temp;              //the final temperature is stored here
  digitalWrite(A0, LOW);   //set pull-up on analog pin
  digitalWrite(H20TEMP_PIN, HIGH);   //set pin 2 high, this will turn on temp sensor
  delay(2);                //wait 2 ms for temp to stabilize
  v_out = analogRead(0);   //read the input pin
  digitalWrite(H20TEMP_PIN, LOW);    //set pin 2 low, this will turn off temp sensor
  v_out*=.0048;            //convert ADC points to volts (we are using .0048 because this device is running at 5 volts)
  v_out*=1000;             //convert volts to millivolts
  temp= 0.0512 * v_out -20.5128; //the equation from millivolts to temperature
  return temp;             //send back the temp
}





