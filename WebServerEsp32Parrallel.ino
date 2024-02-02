#include <WiFi.h>
#include <WebServer.h> // Include the Web Server library

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


// Replace with your network credentials
const char* ssid = "NOC_TL";
const char* password = "TRAFF1CNOC23";

// Create a web server object that listens for HTTP request on port 80
WebServer server(80);

TaskHandle_t Task1;
TaskHandle_t Task2;

// Username and password for basic authentication
const char* www_username = "admin";
const char* www_password = "admin";

void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Check your display's I2C address
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0,0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font


  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 1, &Task2, 1);
}

void loop() {
  
}

void Task1code(void * pvParameters) {
  Serial.print("Task1 (Server) running on core ");
  Serial.println(xPortGetCoreID());

  server.on("/", HTTP_GET, []() {
    if (!server.authenticate(www_username, www_password)) {
      return server.requestAuthentication();
    }
    server.send(200, "text/html", "<form action='/data' method='POST'>"
                                   "<input type='number' name='num1' placeholder='Number 1'><br>"
                                   "<input type='number' name='num2' placeholder='Number 2'><br>"
                                   "<input type='number' name='num3' placeholder='Number 3'><br>"
                                   "<input type='number' name='num4' placeholder='Number 4'><br>"
                                   "<input type='submit' value='Submit'>"
                                   "</form>");
  });

  server.on("/data", HTTP_POST, []() {
    String message = "Received numbers: ";
    message += "Num1: " + server.arg("num1") + ", ";
    message += "Num2: " + server.arg("num2") + ", ";
    message += "Num3: " + server.arg("num3") + ", ";
    message += "Num4: " + server.arg("num4");
    Serial.println(message);
    server.send(200, "text/plain", "Data received. Check the serial monitor.");
  });

  server.begin();
  Serial.println("HTTP server started");

  for (;;) {
    server.handleClient();
    // Allow other tasks to run
    vTaskDelay(1); // Delay for 1 tick period
  }
}

void Task2code(void * pvParameters){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  unsigned long lastMillis = 0;
  unsigned long counter = 0; // Counter for the display

  for(;;){
    if(millis() - lastMillis >= 1000){
      lastMillis = millis();
      counter++; // Increment the counter every second
      
      // Prepare the display
      display.clearDisplay();
      display.setCursor(0,0); // Start at top-left corner
      
      // Display the counter value
      display.print("Counter: ");
      display.println(counter);
      
      // Show the display buffer on the screen
      display.display();
      
      // Optional: Serial output for debugging
      //Serial.print("Counter: ");
      //Serial.println(counter);
    }
  }
}
