#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "WiFiEsp.h"
#include <SPI.h>
#include <Adafruit_PN532.h>

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(2, 3); // RX, TX
#endif

#define PN532_IRQ   (4)
#define PN532_RESET (5)
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

int red = 9;
int green = 10;
int blue = 11;

int i =0;
char ssid[] = "";            // 네트워크 이름 작성
char pass[] = "";        // 네트워크 비밀번호
int status = WL_IDLE_STATUS;
int reqCount = 0;
boolean getIsConnected = false;
unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 5000L;

//서버의 정보//
IPAddress hostIp(192, 168, 0, 4);
WiFiEspClient client;


int RFID_Count = 0;
boolean success;
String ID = "";
String data = "";
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  
  // LCD 초기화
  lcd.init();
  
  digitalWrite(blue, HIGH);
  lcd.backlight();
  lcd.print("GSM LMSS Porject");
  LCD("Starting...",1, 2);
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial1.begin(9600);
  WiFi.init(&Serial1);
  
  // 현재 아두이노에 연결된 실드를 확인
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // 와이파이 실드가 아닐 경우
    while(true);
  } 
 // 와이파이에 연결 시도
  while ( status != WL_CONNECTED) { // 연결이 될 때 까지 반복
    Serial.print("Attempting to CONNECT to WPA SSID: ");
    Serial.println(ssid);
    // WPA/WAP2 네트워크에 연결 (시간이 필요)
    status = WiFi.begin(ssid, pass);

    delay(5000);
  }


  // NFC 사용을 위한 셋팅
  nfc.begin();
  
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  nfc.setPassiveActivationRetries(0xFF);

  nfc.SAMConfig();
  lcd.clear();
}

void loop()
{
  digitalWrite(blue, LOW);
  digitalWrite(green, HIGH);
  if(i == 0){
    data = RFID();
    
    i++;
    ID = "";
  }else{
    if (i < 2){
      httpRequest_RFID(data);
      i++;
    } else i = 0;
    digitalWrite(red, LOW); 
    digitalWrite(green, HIGH);
  }

}
  
void httpRequest_RFID(String rfid) {
  LCD("Sending...",1 ,1);
  Serial.println();
  client.flush();
  client.stop();
  
  delay(1000);
  
  // if there's a successful connection
  if (client.connect(hostIp, 3000)) {
    Serial.println("Connecting...");
    
    //post data set//
    String postdata = "";
    
    //데이터 유무에 따라 값을 넣어주거나 넣지 않는다.//
    postdata.concat(rfid);

    Serial.print("post data [");
    Serial.print(postdata);
    Serial.println("]");

    // send the HTTP POST request
    client.print(F("POST /data"));
    client.print(F(" HTTP/1.1\r\n"));
    client.print(F("Cache-Control: no-cache\r\n"));
    client.print(F("Host: 192.168.0.4\r\n"));
    client.print(F("User-Agent: Arduino\r\n"));
    client.print(F("Content-Type: application/x-www-form-urlencoded\r\n"));
    client.print(F("Content-Length: "));
    client.println(postdata.length());
    client.println();
    client.println(postdata);
    client.print(F("\r\n\r\n"));
    
    // note the time that the connection was made
    lastConnectionTime = millis();
    getIsConnected = true;
    delay(1000);
  }
  
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
    getIsConnected = false;
  }
}


String RFID(){
  digitalWrite(green, LOW);
  digitalWrite(red, HIGH);
  LCD("Swipe your ID", 1, 1);
  Serial.println("Waiting for an ISO14443A card");
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  
  if (success) {
    Serial.println("Found a card!");
    Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++) 
    {
      //Serial.print(" 0x"); Serial.print(uid[i], HEX);
      ID = ID + String(uid[i], HEX);
    }
    Serial.println("");
    Serial.println(ID);
    return ID;
  // Wait 1 second before continuing
  delay(1000);
  }
  else
  {
    Serial.println("Timed out waiting for a card");
  }
  lcd.clear();
}

void LCD(String data, int x, int y){
  
  lcd.setCursor(x-1, y-1);
  lcd.print(data);
}
