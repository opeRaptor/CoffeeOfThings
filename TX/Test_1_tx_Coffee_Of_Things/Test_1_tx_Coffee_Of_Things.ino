#include "Energia.h"

#define EMPTY              0
#define LOWER              1
#define MID                2
#define FULL               3
#define ERROR              -1
#define SENSORTHRESHOLD    30
#define NODE_ID            1   

void setup();
void loop();
void dump_radio_status_to_serialport(uint8_t status);
int DeterminState (int Lowersensor, int Middlesensor, int Uppersensor, int Temperature) ;


#include <Enrf24.h>
#include <nRF24L01.h>
#include <string.h>
#include <SPI.h>

Enrf24 radio(P2_0, P2_1, P2_2);  // P2.0=CE, P2.1=CSN, P2.2=IRQ
const uint8_t txaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };

void dump_radio_status_to_serialport(uint8_t);

void setup() {
  Serial.begin(9600);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  radio.begin();  // Defaults 1Mbps, channel 0, max TX power
  dump_radio_status_to_serialport(radio.radioState());

  radio.setTXaddress((void*)txaddr);
}

void loop() {
  
  int message[3];
  int VoltageTempSensor = analogRead(A4) * 3;
  int Lowersensor = analogRead(A1);
  int Middlesensor = analogRead(A3);
  int Uppersensor = analogRead(A0);
  int Temperature = (VoltageTempSensor - 500)/10;
  
  // Tempsensor output is VoltageTMP36/3mV
  // at 750 mV, 25 C, then 1C per 10 mV --> so everytime Tempsensor increase by 3.3
  // Offset is 500 mV
  
  Serial.print(Lowersensor);
  Serial.print(";");
  Serial.print(Middlesensor);
  Serial.print(";");
  Serial.print(Uppersensor);
  Serial.print("---");
  Serial.print(VoltageTempSensor);
  Serial.print("---");
  Serial.print(NODE_ID);
  Serial.print(";");
  Serial.print(DeterminState (Lowersensor, Middlesensor, Uppersensor, Temperature));
  Serial.print(";");
  Serial.println(Temperature);
	message[0]=NODE_ID;
	message[1]=DeterminState (Lowersensor, Middlesensor, Uppersensor, Temperature);
	message[2]=Temperature;

  Serial.print("Sending packet: ");
  //Serial.write(message);
  radio.write(message,sizeof(message));//message,sizeof(message)
  radio.flush();  // Force transmit (don't wait for any more data)
  dump_radio_status_to_serialport(radio.radioState());  // Should report IDLE
  delay(1000);
}

void dump_radio_status_to_serialport(uint8_t status)
{
  Serial.print("Enrf24 radio transceiver status: ");
  switch (status) {
    case ENRF24_STATE_NOTPRESENT:
      Serial.println("NO TRANSCEIVER PRESENT");
      break;

    case ENRF24_STATE_DEEPSLEEP:
      Serial.println("DEEP SLEEP <1uA power consumption");
      break;

    case ENRF24_STATE_IDLE:
      Serial.println("IDLE module powered up w/ oscillators running");
      break;

    case ENRF24_STATE_PTX:
      Serial.println("Actively Transmitting");
      break;

    case ENRF24_STATE_PRX:
      Serial.println("Receive Mode");
      break;

    default:
      Serial.println("UNKNOWN STATUS CODE");
  }
}

/*   Determin coffee state
     Input 3 int: sensor analog read values
     Output : State to transmit, Error is 4, Full is 3, half-full 2, Low is 1, Empty is 0 */

 int DeterminState (int Lowersensor, int Middlesensor, int Uppersensor, int Temperature) 
 {
   boolean Low = 0, Mid = 0, High = 0;
   if (Uppersensor > Middlesensor*2+20) Mid = 1;
   if (Uppersensor > Lowersensor*2+20) Low = 1;
   if ((Low == 1)&&(Mid == 0)) return LOWER;
   else if ((Low == 1)&&(Mid == 1)) return MID;
   else
   {
     if (Temperature > 50)             return FULL;
     else                              return EMPTY;                                     
   }
 }
