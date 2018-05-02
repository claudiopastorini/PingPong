/**
* Ping Pong example with LoRa for the Pervasive System course A.A. 2017/2018 - University of Roma La Sapienza
* 
* The example is a simple Ping Pong application with two different LoRa boards
* communicate each other.
* 
* One board is the Adafruit Feather M0 with LoRa Radio module the other one is
* the Seeduino LoRaWAN.
* Both boards use the same Arduino sketch, the first using the RadioHead Driver,
* the other one instead use AT commands over serial communication.
*
* Claudio Pastorini - 1792086
**/

#if defined(ARDUINO_ARCH_SEEEDUINO_SAMD)
  #define SERIAL SerialUSB
#else
  #define SERIAL Serial
#endif
  
#if defined(ARDUINO_ARCH_SEEEDUINO_SAMD)
  #include <LoRaWan.h>

  // Max number of octets the LORA Rx/Tx FIFO can hold
  #define RH_RF95_FIFO_SIZE 255
  
  // This is the maximum number of bytes that can be carried by the LORA.
  // We use some for headers, keeping fewer for RadioHead messages
  #define RH_RF95_MAX_PAYLOAD_LEN RH_RF95_FIFO_SIZE
  
  // The length of the headers we add.
  // The headers are inside the LORA's payload
  #define RH_RF95_HEADER_LEN 4
  
  // This is the maximum message length that can be supported by this driver. 
  // Can be pre-defined to a smaller size (to save SRAM) prior to including this header
  // Here we allow for 1 byte message length, 4 bytes headers, user data and 2 bytes of FCS
  #ifndef RH_RF95_MAX_MESSAGE_LEN
    #define RH_RF95_MAX_MESSAGE_LEN (RH_RF95_MAX_PAYLOAD_LEN - RH_RF95_HEADER_LEN)
  #endif
#elif defined(ARDUINO_SAMD_FEATHER_M0)
  #include <SPI.h>
  #include <RH_RF95.h>

  // Chip select pin
  #define RFM95_CS 8
  // Reset pin
  #define RFM95_RST 4
  // Interrupt pin
  #define RFM95_INT 3
  
  // Must match the frequency of others nodes
  #define RF95_FREQ 433

  // One second in milliseconds
  #define MILLIS 1000
  
  // Singleton instance of the radio driver
  RH_RF95 rf95(RFM95_CS, RFM95_INT);
#endif

void setup(void) {
#if defined(ARDUINO_ARCH_SEEEDUINO_SAMD)
  SERIAL.begin(115200);
  lora.init();
  // Inits the Seeduino LoRaWAN board passing frequency, spreadingFactor (125 chips/symbol), bandwidth, txPreamble, rxPreamble, power
  lora.initP2PMode(433, SF7, BW125, 8, 8, 20);

  delay(1000);
  
  SERIAL.println("Seeduino LoRa radio init OK!");
#elif defined(ARDUINO_SAMD_FEATHER_M0)
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  SERIAL.begin(115200);
  while (!SERIAL) {
    delay(1);
  }

  delay(100);

  // Manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    SERIAL.println("Feather LoRa radio init failed");
    while (1);
  }
  SERIAL.println("Feather LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    SERIAL.println("setFrequency failed");
    while (1);
  }
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
#else
 SERIAL.println("NOT A VALID BOARD");
#endif
}

#if defined(ARDUINO_ARCH_SEEEDUINO_SAMD)
  unsigned char RXBuffer[RH_RF95_MAX_MESSAGE_LEN];

  // All messages sent and received by this RH_RF95 Driver conform to this packet format:
  //
  // - LoRa mode:
  // - 8 symbol PREAMBLE
  // - Explicit header with header CRC (handled internally by the radio)
  // - 4 octets HEADER: (TO, FROM, ID, FLAGS)
  // - 0 to 251 octets DATA 
  // - CRC (handled internally by the radio)
  // So we need to send 4 octect of header in order to allow the RF95 to understand the message
  unsigned char TXBuffer[RH_RF95_MAX_MESSAGE_LEN] = {0xFF, 0xFF, 0x0, 0x0};
#else
  char RXBuffer[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(RXBuffer);
  unsigned char TXBuffer[RH_RF95_MAX_MESSAGE_LEN];
#endif

char *ping = "PING";
char *pong = "PONG";

char *lastMessage;  

uint8_t sentCounter = 0;
uint8_t receivedCounter = 0;

void loop(void) {
  
  char *message = NULL;
  short rssi = 0;

  // Receives a message
  if (!receiveMessage(&message, &rssi, 2)) {
    // Sends ping
    sendMessage(ping);
    printTransmittedMessage(ping);
    return;
  }

  // Prints out the received message
  printReceivedMessage(message, rssi);  
  
  if (strcmp(message, ping) == 0) {
    // Sends pong
    sendMessage(pong);
    printTransmittedMessage(pong); 
  } else if (strcmp(message, pong) == 0) {
    // Sends ping
    sendMessage(ping);
    printTransmittedMessage(ping); 
  } else {
    SERIAL.println("Message not recognized");
  }

}

bool receiveMessage(char **message, short *rssi, uint8_t seconds) {
#if defined(ARDUINO_ARCH_SEEEDUINO_SAMD)
  // Clears receive buffer for incoming message
  memset(RXBuffer, 0, RH_RF95_FIFO_SIZE);
  
  // Receives
  if (lora.receivePacketP2PMode(RXBuffer, RH_RF95_FIFO_SIZE,  rssi, seconds) <= 0) {
    SERIAL.println("No message, is there a transmitter around?");
    return false;
  }

  // Gets only the message payload to print without the header
  *message = (char *) (RXBuffer + 4);
#elif defined(ARDUINO_SAMD_FEATHER_M0)
  // Waits for a message
  if (!rf95.waitAvailableTimeout(1 * MILLIS)) { 
    SERIAL.println("No message, is there a transmitter around?");
    return false;
  }
  
  // Should be a reply message for us now   
  if (!rf95.recv((uint8_t *) RXBuffer, &len)) {  
    SERIAL.println("Receive failed");
    return false;
  }

  *message = (char *) RXBuffer;
  *rssi = rf95.lastRssi();
#else
  SERIAL.println("NOT A VALID BOARD");
#endif
  
  // Increase counter of received messages
  receivedCounter++;
  
  return true;
}

void sendMessage(char *message) {
#if defined(ARDUINO_ARCH_SEEEDUINO_SAMD)
  // Clears sender buffer for outcoming message
  memset(TXBuffer + RH_RF95_HEADER_LEN, 0, RH_RF95_MAX_MESSAGE_LEN);
  
  // Adds message into the buffer
  memcpy(TXBuffer + RH_RF95_HEADER_LEN, message, RH_RF95_MAX_MESSAGE_LEN);
  
  lora.transferPacketP2PMode(TXBuffer, RH_RF95_HEADER_LEN + strlen(message) + 1);
 #elif defined(ARDUINO_SAMD_FEATHER_M0)
  // Clears sender buffer for outcoming message
  memset(TXBuffer, 0, RH_RF95_MAX_MESSAGE_LEN);

  // Adds message into the buffer
  memcpy(TXBuffer, message, RH_RF95_MAX_MESSAGE_LEN);
  
  // Sends
  rf95.send((uint8_t *)TXBuffer, strlen(message));

  // Waits for complete transmission
  delay(10);
  rf95.waitPacketSent(); 
 #else
  SERIAL.println("NOT A VALID BOARD");
#endif

  // Increase counter of sent messages
  sentCounter++;
}

void printReceivedMessage(char *message, int8_t rssi) {
  SERIAL.println();
  SERIAL.print("<<<<<<<<<<<<< ");
  SERIAL.print(receivedCounter);
  SERIAL.println(" <<<<<<<<<<<<<");
  SERIAL.print("Got: ");
  SERIAL.println(message);
  SERIAL.print("Lenght: ");
  SERIAL.println(strlen(message));
  SERIAL.print("RSSI: ");
  SERIAL.println(rssi, DEC);
  SERIAL.print("<<<<<<<<<<<<< ");
  SERIAL.print(receivedCounter);
  SERIAL.println(" <<<<<<<<<<<<<");
  SERIAL.println();
}

void printTransmittedMessage(char *message) {
  SERIAL.print("\t\t\t\t\t\t\t\t\t>>>>>>>>>>>> ");
  SERIAL.print(sentCounter);
  SERIAL.println(" >>>>>>>>>>>>");
  SERIAL.print("\t\t\t\t\t\t\t\t\tSending: "); 
  SERIAL.println(message);
  SERIAL.println("\t\t\t\t\t\t\t\t\tSending...");
  SERIAL.print("\t\t\t\t\t\t\t\t\t>>>>>>>>>>>> ");
  SERIAL.print(sentCounter);
  SERIAL.println(" >>>>>>>>>>>>");
  SERIAL.println();
}
