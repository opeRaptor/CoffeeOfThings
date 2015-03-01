#include "Energia.h"

void setup();
void loop();
void dump_radio_status_to_serialport(uint8_t status);
void WiFisend(int field,int data);
boolean connectWiFi();

#define SSID "WongKei_FreeDialUp" //SSID of the Network
#define PASS "elpsycongroo" //Password
#define IP "184.106.153.149" // thingspeak.com
String GET = "GET /update?key=F9TT87197XWW1SDO&field1="; //URL to update Field 1 with Private Key.


#include <Enrf24.h>
#include <nRF24L01.h>
#include <string.h>
#include <SPI.h>

Enrf24 radio(P2_0, P2_1, P2_2);
const uint8_t rxaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };


void setup() {
  Serial.begin(9600);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  
  radio.begin();  // Defaults 1Mbps, channel 0, max TX power

  radio.setRXaddress((void*)rxaddr);
  
  pinMode(P1_0, OUTPUT);
  digitalWrite(P1_0, LOW);
  
  radio.enableRX();  // Start listening

  Serial.println("AT");
  delay(5000);
  if(Serial.find("OK")){
    connectWiFi();
  }
}

void loop() {
  int inbuf[33];
  int nodeNub;
  int state=0;
  int temp=0;
  
 while (!radio.available(true));
  if (radio.read(inbuf))
  {
    nodeNub=inbuf[0];
    state =inbuf[1];
    temp = inbuf[2];
  }

if(nodeNub==1){
  WiFisend(1,state);
  delay(6000);
  WiFisend(2,temp);
}
else{
	  Serial.println("No valid node sending");
}
  delay(6000);
}

void WiFisend(int field,int data){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);
  delay(2000);
  if(Serial.find("Error")){
    return;
  }
  cmd = GET; //Most of the GET request with API key
  //cmd += field; //Field select
  //cmd +="=";
  cmd += data; //Data input
  cmd += "\r\n"; //End character

  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  if(Serial.find(">")){
    Serial.print(cmd);
  //}else{
    //delay(2000);
    Serial.println("AT+CIPCLOSE");
  }
  delay(8000);
}

boolean connectWiFi(){
  Serial.println("AT+CWMODE=0");
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);
  delay(5000);
  if(Serial.find("OK")){
    return true;
  }else{
    return false;
  }
}
