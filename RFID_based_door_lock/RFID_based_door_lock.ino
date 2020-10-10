


// Necessary Libraries
#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
#include <SPI.h>
#include<WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <HTTPClient.h>


// Keypad Configurations
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {13, 12, 14, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33, 32}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// OLED Configurations
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Pinouts
#define RST_PIN         36        // Configurable, see typical pin layout above
#define SS_PIN          5         // Configurable, see typical pin layout above
#define BUZZER          15
#define proximity_sens  39
#define LOCK_PIN        2


int Secret_Code = 1234; // Code used to unlock the door via keypad

// Necessary Variables
String ID_e;
int value = 0;
bool done_flag = 0;
char* response = " ";
String res = "";
char* succ_code = "200 OK";
bool rfid_flag = 1;

MFRC522 mfrc522(SS_PIN, RST_PIN); // Setting up RFID Pins

const char* NAME; // Variable to save the name of the person

//Necessary parameters for IFTTT request
String Event_Name = "your_webhooks_event_name";
String Key = "your_webhooks_key";
String resource = "/trigger/" + Event_Name + "/with/key/" + Key;
const char* server = "maker.ifttt.com";

// Provide your WiFi Credentials
const char* ssid     = "SSID name ";
const char* password = "PASS";



void setup()
{

  // put your setup code here, to run once:

  Serial.begin(9600);   // Initialize serial communications with the PC

  pinMode(BUZZER, OUTPUT);
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(proximity_sens, INPUT_PULLUP);
  digitalWrite(BUZZER, LOW);

  // Checking OLED Display connections
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);
  display.print("RFID Door Lock");
  display.display();

  SPI.begin();
  mfrc522.PCD_Init();

  initWiFi(); // Initialising WiFi connectivity if router is available

  display.clearDisplay();
  Serial.println("Scan tag");
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 30);
  display.print("Scan tag");
  display.display();
}



void loop()
{

  char key = keypad.getKey(); // Store the key pressed on the Keypad

  if (key) {
    Serial.println(key);
    beep(200);
  }




  if (key == 'A') // If 'A' is pressed, goto pincode mode
  {
    Serial.println("A");
    Serial.println("Pin Code");
    display.clearDisplay();
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(10, 0);            // Start at top-left corner
    display.println(("Pin Code"));
    display.setCursor(25, 20);
    display.println(("Mode"));
    display.display();
    beep(200);
    beep(200);
    delay(1000);

    Serial.println("Enter the \nFour Digit \nCode");

    rfid_flag = 0;
  }

  if (key == 'C') // If 'C' is pressed, manually reset the board 
  {
    
   ESP.restart();
  }
  

    // RFID Mode
    if (rfid_flag == 1)
    {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(10, 30);
      display.print("Scan Tag");
      display.display();

      if (digitalRead(proximity_sens) == LOW)
        Exit_Sensor();

      Serial.println("Waiting for the tag1");

      if ( ! mfrc522.PICC_IsNewCardPresent())
      {
        return;
      }
      // Select one of the cards
      if ( ! mfrc522.PICC_ReadCardSerial())
      {
        return;
      }

      String content = "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        //Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }


      content.toUpperCase();
      Serial.println("Waiting for the tag2");

      if (content.substring(1) == "52 17 13 4C") //change here the UID of the card that you want to give access
      {
        Serial.print("!!--");
        Serial.println(content.substring(1));
        NAME = "Sachin";
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(1, 1);
        display.println("Welcome ");
        display.println(NAME);
        display.setCursor(1, 40);
        display.print("Door Open");
        display.display();
        Door_Open();
        digitalWrite(BUZZER, LOW);
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(1, 1);
        display.print("Door");
        display.setCursor(1, 20);
        display.print("Closed");
        display.display();
        delay(1000);

        makeIFTTTRequest(); // Making Registrations on Google Sheet via IFTTT
      }


      else if (content.substring(1) == "12 39 0F 4C") //change here the UID of the card that you want to give access
      {
        Serial.print("!!--");
        Serial.println(content.substring(1));
        NAME = "Harsh";
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(1, 1);
        display.println("Welcome ");
        display.println(NAME);
        display.setCursor(1, 40);
        display.print("Door Open");
        display.display();
        Door_Open();
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(1, 1);
        display.print("Door");
        display.setCursor(1, 20);
        display.print("Closed");
        display.display();
        delay(1000);

        makeIFTTTRequest(); // Making Registrations on Google Sheet via IFTTT

      }
      else
      {
        Serial.println("Not Registered");
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(38, 1);
        display.print("Not");
        display.setCursor(5, 30);
        display.print("Registered");
        display.display();
        beep(200);
        beep(200);
        beep(200);
        Serial.println("You can't enter Studio.");
      }
      content.substring(1) = "";


    }


    // Pincode Mode
    if (rfid_flag == 0)
    {

      if (Keypad_Input() == Secret_Code) // Checking the code entered by user
      {
        Serial.println("Correct Code");
        display.clearDisplay();
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);        // Draw white text
        display.setCursor(20, 20);            // Start at top-left corner
        display.println("Correct");
        display.setCursor(30, 40);            // Start at top-left corner
        display.println("Code");
        display.display();
        Door_Open();

        makeIFTTTRequest();
        rfid_flag = 1;
      }
      else
      {
        Serial.println("Wrong Code");
        display.clearDisplay();
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);        // Draw white text
        display.setCursor(0, 20);            // Start at top-left corner
        display.println(("Wrong Code"));
        display.display();
        beep(200);
        beep(200);
        delay(1000);
        display.clearDisplay();
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);        // Draw white text
        display.setCursor(35, 20);            // Start at top-left corner
        display.println(("Code"));
        display.display();
      }
      ID_e = "";
    }



  }




  void makeIFTTTRequest()
  {
    HTTPClient http;
    Serial.print("Connecting to ");
    Serial.print(server);

    WiFiClient client;
    int retries = 5;
    while (!!!client.connect(server, 80) && (retries-- > 0)) {
      Serial.print(".");
    }
    Serial.println();
    if (!!!client.connected()) {
      Serial.println("Failed to connect...");
    }

    Serial.print("Request resource: ");
    Serial.println(resource);

    String jsonObject = String("{\"value1\":\"") + NAME + "\"}";

    client.println(String("POST ") + resource + " HTTP/1.1");
    client.println(String("Host: ") + server);
    client.println("Connection: close\r\nContent-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonObject.length());
    client.println();
    client.println(jsonObject);

    int timeout = 5 * 10; // 5 seconds
    while (!!!client.available() && (timeout-- > 0))
    {
      delay(100);
    }
    if (!!!client.available())
    {
      Serial.println("No response...");
    }

    while (client.available())
    {
      // Serial.write(client.read());
      char add = client.read();
      res = res + add;
    }
    response = &res[0];
    Serial.println("=======");
    Serial.println(response);

    if (strstr(response, succ_code)) // If connected to internet, make registration
    {
      Serial.println("Registered");
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(0, 30);            // Start at top-left corner
      display.println(("Registered"));
      display.display();
      delay(1000);
    }

    else // If not connected to internet, don't make registration
    {
      Serial.println("Not Registered");
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(30, 10);            // Start at top-left corner
      display.println(("Not"));
      display.setCursor(0, 30);            // Start at top-left corner
      display.println(("Registered"));
      display.display();
      delay(1000);
    }
    response = "";
    res = "";
    Serial.println("\nclosing connection");
    client.stop();


  }


  // Initialising WiFi connectivity if router is available
  void initWiFi()

  {
    Serial.print("Connecting to: ");
    Serial.print(ssid);
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0, 0);            // Start at top-left corner
    display.println(("Connecting"));
    display.setCursor(0, 50);
    display.println((ssid));
    display.display();
    delay(2000);
    WiFi.begin(ssid, password);

    int timeout = 10 * 4; // 10 seconds
    while (WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
      delay(250);
      Serial.print(".");
    }
    Serial.println("");

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to connect");
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(0, 30);            // Start at top-left corner
      display.println(("Not \nConnected"));
      display.display();
    }
    else
    {
      Serial.print("WiFi connected in: ");
      Serial.print(millis());
      Serial.print(", IP address: ");
      Serial.println(WiFi.localIP());
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(0, 30);            // Start at top-left corner
      display.println(("Connected"));
      display.display();
    }
    delay(2000);
  }


  // Just a normal beep sound
  void beep(int duration)
  {
    digitalWrite(BUZZER, HIGH);
    delay(duration);
    digitalWrite(BUZZER, LOW);
    delay(30);
  }


  // Reading and Storing the code entered by user
  int Keypad_Input(void)
  {
    int i = 0;
    display.clearDisplay();
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0, 0);            // Start at top-left corner
    display.println(("Enter Code"));
    display.display();
    i = 0;
    ID_e = "";
    value = 0;
    while (1)
    {
      char key = keypad.getKey();

      if (key && i < 4)
      {
        ID_e = ID_e + key;
        value = ID_e.toInt();
        Serial.println(value);
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);        // Draw white text
        display.setCursor(i * 35, 50);            // Start at top-left corner
        display.println(("*"));
        display.display();
        beep(100);
        i++;
      }
      if (key == '#')
      {
        done_flag = 1;
        Serial.println("DONE");
        i = 0;
        ID_e = "";
        return (value);
      }
      if (key == 'B')
      {
        rfid_flag = 1;
        ID_e = "";
        return (0);
      }

      if (digitalRead(proximity_sens) == LOW)
      {
        Exit_Sensor();
        rfid_flag = 1;
        ID_e = "";
        return (0);
      }
    }
  }


  // Opens the Door, and close it after 5 sec
  void Door_Open()
  {
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LOCK_PIN, HIGH);
    Serial.println("DOOR OPENED");
    delay(5000);
    digitalWrite(LOCK_PIN, LOW);
    digitalWrite(BUZZER, LOW);
    Serial.println("DOOR CLOSED");
  }

  void Exit_Sensor()
  {
    // Opens the Door, when the Button inside the room is pressed
    if (digitalRead(proximity_sens) == LOW)
    {
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(20, 20);            // Start at top-left corner
      display.println(("Visit"));
      display.setCursor(10, 40);            // Start at top-left corner
      display.println(("Again"));
      display.display();
      Door_Open(); // Opens the Door, and close it after 5 sec
      display.clearDisplay();
      display.display();
    }
  }
