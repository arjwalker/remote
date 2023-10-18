#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>

const char* ssid = "RemoteApp";     // Set your AP SSID (Wi-Fi name)
const char* password = "password1";   // Set your AP password
WebServer server(80);

// Define RS232 parameters
const int rs232BaudRate = 9600; // Adjust as needed
const int rs232TxPin = 17;       // GPIO1 (U0TX)
const int rs232RxPin = 16;       // GPIO3 (U0RX)

// Initialize HardwareSerial for RS232 communication
HardwareSerial rs232Serial(1); // UART 1

void setup() {
  Serial.begin(115200);

  Serial.println("Before setup"); 

  // Initialize RS232 serial communication
 rs232Serial.begin(rs232BaudRate, SERIAL_8N1, rs232TxPin, rs232RxPin);

  // Start the Wi-Fi AP
  WiFi.softAP(ssid, password);

  // Set up the DHCP server
  IPAddress IP(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(IP, IP, subnet);

  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Initialize the SPIFFS file system
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
  }

  // Configure request handling
  server.on("/", HTTP_GET, handleRoot);
  server.on("/logo.png", HTTP_GET, handleLogo);
  for (int i = 1; i <= 6; i++) {
    server.on("/button" + String(i), HTTP_GET, [i]() {
      // Handle button press and send RS232 command
      handleButton(i);
    });
  }

  // Intercept all requests and redirect to the root page
  server.onNotFound(captivePortal);

  server.begin();
}

void handleButton(int i) {
  Serial.println("Button " + String(i) + " clicked.");
  switch (i) {
    case 1:
      // Send "Projector UP" command
      Serial.println("Sending 'Projector UP' command");
      if (rs232Serial.println("#DUP")) {
        Serial.println("Command sent successfully");
      } else {
        Serial.println("Error sending command");
      }
      break;
    case 2:
      // Send "Projector Down" command
      Serial.println("Sending 'Projector Down' command");
      if (rs232Serial.println("#DDOWN")) {
        Serial.println("Command sent successfully");
      } else {
        Serial.println("Error sending command");
      }
      break;
    // Add more cases for other buttons if needed
  }

  server.send(200, "text/plain", "Command sent");
}


void handleRoot() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>";
  html += "body { background: #222; color: #fff; text-align: center; font-family: Arial, sans-serif; }";
  html += ".iphone-button { background: #3498db; color: #fff; padding: 15px 30px; font-size: 18px; margin: 10px; border: none; cursor: pointer; border-radius: 8px; }";
  html += ".logo { display: block; margin: 20px auto; }";
  html += "</style></head><body>";
  html += "<img class='logo' src='/logo.png' alt='Your Logo' width='200'>";
  
  for (int i = 1; i <= 6; i++) {
    html += "<button class='iphone-button' onclick='sendCommand(" + String(i) + ")'>Button " + String(i) + "</button><br>";
  }

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleLogo() {
  File logoFile = SPIFFS.open("/logo.png", "r");
  if (logoFile) {
    server.streamFile(logoFile, "image/png");
    logoFile.close();
  } else {
    server.send(404, "text/plain", "Logo not found");
  }
}

void captivePortal() {
  // Send a redirect response to the root page
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void loop() {
  //Serial.println("loop setup"); 
  server.handleClient();
  // Your loop code here  
}
