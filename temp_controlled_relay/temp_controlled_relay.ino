
#include <splash.h>
#include <Adafruit_SSD1306.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example


#include <dht11.h>
#define DHT11PIN 4

dht11 DHT11;
int lifecycle_index = 0;
bool heater_state = true;
const int RELAY_PIN = 5;  // the Arduino pin, which connects to the IN pin of relay


/*Average the temp so we don't get random data as the true value
 * 
 */
#define WINDOW_SIZE 5

int INDEX = 0;
int VALUE = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE];
int AVERAGED = 0;

//minutes used per day
int daily_usage = 0;
//minute clock count, resets once per day
int daily_clock = 0;

void setup()
{
  Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  pinMode(RELAY_PIN, OUTPUT);
 
}


void write_text_to_screen(float t, float h) {
  char msgBuffer[20];     
  char msgBuffer1[20];     
  char * temp;
  char * humid;
  //char * usage;
  int len = String(daily_usage).length();
  char usage[len + 1];  // String.length() does not include the null terminator
  itoa(daily_usage, usage, 10);   // or you could use String(b).toCharArray(cStr, len);
  //itoa(daily_usage, usage, 10);  // int value, pointer to string, base number
  humid = dtostrf(h, 6, 2, msgBuffer);
  temp = dtostrf(t, 6, 2, msgBuffer1);
  
  display.clearDisplay();

  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.write("T C:");
  display.write(temp);
  display.write("\n");
  display.write("H %:");
  display.write(humid);
  display.write("\n");
  display.write("Heat:");
  if(heater_state){
    display.write("On");
    digitalWrite(RELAY_PIN, LOW);
  } else {
    display.write("Off");
    digitalWrite(RELAY_PIN, HIGH);
    
  }
  
  display.write("\n");
  display.write("usg:");
  display.write(usage);
  display.display();
  
  delay(200);
}

void loop()
{
  Serial.println();
  lifecycle_index +=1;

  int chk = DHT11.read(DHT11PIN);
  
  float humidity = 0.0;
  float temp = 0.0;
  
  humidity = (float)DHT11.humidity;
  temp = (float)DHT11.temperature;
  
  Serial.print("Humidity (%): ");
  //Serial.println((float)DHT11.humidity, 2);
  Serial.println(humidity);
  
  Serial.print("Temperature (C): ");
  //Serial.println((float)DHT11.temperature, 2);
  Serial.println(temp);
  
  Serial.println("life cycle Index");
  Serial.println(lifecycle_index);
  
  SUM = SUM - READINGS[INDEX];       // Remove the oldest entry from the sum
  VALUE = (float)DHT11.temperature;       // Read the next sensor value
  READINGS[INDEX] = VALUE;           // Add the newest reading to the window
  SUM = SUM + VALUE;                 // Add the newest reading to the sum
  INDEX = (INDEX+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

  AVERAGED = SUM / WINDOW_SIZE;      // Divide the sum of the window by the window size for the result
  Serial.println(AVERAGED);
  //At most, change heater state once even  ry minute
  if(lifecycle_index >= 30){
    //increase clock
    
    //
    int daily_clock = 0;
    lifecycle_index = 0;
    Serial.println("CHECK TEMPERATURE AND ACTIVATE ");
    if(AVERAGED <= 5){
      heater_state = true;
      daily_usage +=1;
      daily_clock +=1;
      if(daily_clock >= 1440){
        daily_clock = 0;
        daily_usage = 0;
      }
    } else {
      heater_state = false;
    }
  }
  write_text_to_screen(AVERAGED,humidity);
  
  delay(2000);

}
