// https://mytectutor.com/esp8266-nodemcu-relay-control-from-web-page-over-wifi/

#include <ESP8266WiFi.h>
// Replace with your network credentials
const char* ssid = "SSID NAME"; // Input your wifi network name
const char* password = "PASSWORD"; // Input your wifi password
const char* customhostname = "ESP8266Node.local"; // Your custom local name
// Set web server port number to 80
WiFiServer server(80);
// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
int relayAState = LOW;
int relayBState = LOW;
int buttonAState = HIGH; 
int lastButtonAState = LOW;
int buttonBState = HIGH; 
int lastButtonBState = LOW;

unsigned long lastDebounceTimeA = 0;  // the last time the output pin was toggled
unsigned long debounceDelayA = 50;    // the debounce time; increase if the output flickers
unsigned long lastDebounceTimeB = 0;  // the last time the output pin was toggled
unsigned long debounceDelayB = 50;    // the debounce time; increase if the output flickers

// Assign output variables to GPIO pins
const int relayAPin = 13; // Relay at GPIO13 D7
const int relayBPin = 12; // Relay at GPIO12 D6
const int buttonAPin = 5;    // Button to GPIO5 D1
const int buttonBPin = 4;    // Button to GPIO4 D2, solder to select

int connection_timeout_threshold = 15;  /* Number of attempts to connect to a Wi-Fi network before
                                         * the device will suggest changing connection credentials.
                                         */

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(relayAPin, OUTPUT);
  pinMode(relayBPin, OUTPUT);
  pinMode(buttonAPin, INPUT);
  pinMode(buttonBPin, INPUT);

  // Set outputs to HIGH. relay active LOW
  digitalWrite(relayAPin, HIGH);
  digitalWrite(relayBPin, HIGH);
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int timeout_counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    ++timeout_counter;

    if (timeout_counter > connection_timeout_threshold) {
        Serial.println("Connection failed, please consider changing your login credentials.");
    }
  }
  
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.printf("Default hostname: %s\n", WiFi.hostname().c_str());
  WiFi.hostname(customhostname);
  Serial.printf("New hostname: %s\n", WiFi.hostname().c_str());
  
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // turns the GPIOs on and off
            if (header.indexOf("GET /swA/on") >= 0)
            {
              Serial.println("GPIO 5 on");
              relayAState = HIGH;
              digitalWrite(relayAPin, LOW);
            }
            else if (header.indexOf("GET /swA/off") >= 0)
            {
              Serial.println("GPIO 5 off");
              relayAState = LOW;
              digitalWrite(relayAPin, HIGH);
            }
            else if (header.indexOf("GET /swB/on") >= 0) {
              Serial.println("GPIO 4 on");
              relayBState = HIGH;
              digitalWrite(relayBPin, LOW);
            }
            else if (header.indexOf("GET /swB/off") >= 0) {
              Serial.println("GPIO 4 off");
              relayBState = LOW;
              digitalWrite(relayBPin, HIGH);
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name='viewport' content='width=device-width, initial-scale=1'>");
            // client.println("<link rel='icon' href='data:,'>");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html, body { font-family: Helvetica; display: block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #c20a0a; border: none; color: white; padding: 12px 24px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #209e48;}");
            client.println(".textbox {width: 80px; border: 1px solid #333; padding: 16px 20px 0px 24px; background-image: linear-gradient(180deg, #fff, #ddd 40%, #ccc);}");
            client.println(".mytext {font-size: 16px; font-weight:bold; font-family:Arial ; text-align: justify;}");
            client.println("#container {width: 100%; height: 100%; margin-left: 5px; margin-top: 20px; padding: 10px; display: -webkit-flex; -webkit-justify-content: center; display: flex; justify-content: center;} ");
            client.println("</style></head>");
            // Web Page Heading
            client.println("<body><h1>NodeMCU Web Server Relay Control</h1>");
            // Display current state, and ON/OFF buttons for GPIO 5
            client.println("<div id='container'>");
            client.println("<p><div class='textbox mytext'>RELAY 1 </div> ");
            // If the relayAState is off, it displays the ON button
            if (relayAState == LOW) {
              client.println("<a href='/swA/on'><button class='button'>OFF</button></a></p>");
            } else {
              client.println("<a href='/swA/off'><button class='button button2'>ON</button></a></p>");
            }
            client.println("</div>");
            // Display current state, and ON/OFF buttons for GPIO 4
            client.println("<div id='container'>");
            client.println("<p><div class='textbox mytext'>RELAY 2 </div> ");
            // If the relayBState is off, it displays the ON button
            if (relayBState == LOW) {
              client.println("<a href='/swB/on'><button class='button'>OFF</button></a></p>");
            } else {
              client.println("<a href='/swB/off'><button class='button button2'>ON</button></a></p>");
            }
            client.println("</div>");

            client.println("</body></html>");
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  abuttonpressed();
  bbuttonpressed();
}

void abuttonpressed() {
  int readingA = digitalRead(buttonAPin);
  if (readingA != lastButtonAState) {
    // reset the debouncing timer
    lastDebounceTimeA = millis();
  }

    if ((millis() - lastDebounceTimeA) > debounceDelayA) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
      if (readingA != buttonAState) {
      buttonAState = readingA;

      // only toggle the LED if the new button state is HIGH
        if (buttonAState == HIGH) {
        relayAState = !relayAState;
        }
      }
    }
 
  // set the LED:
  digitalWrite(relayAPin, !relayAState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonAState = readingA;
  
}

void bbuttonpressed() {
  int readingB = digitalRead(buttonBPin);
  if (readingB != lastButtonBState) {
    // reset the debouncing timer
    lastDebounceTimeB = millis();
  }

    if ((millis() - lastDebounceTimeB) > debounceDelayB) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
      if (readingB != buttonBState) {
      buttonBState = readingB;

      // only toggle the LED if the new button state is HIGH
        if (buttonBState == HIGH) {
        relayBState = !relayBState;
        }
      }
    }
 
  // set the LED:
  digitalWrite(relayBPin, !relayBState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonBState = readingB;
  
}
