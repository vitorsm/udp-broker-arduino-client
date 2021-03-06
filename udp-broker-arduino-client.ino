#include "constants.h"
#include "messageUtils.c"
#include "utils.c"
#include <SoftwareSerial.h>
#include "espUtils.c"
#include "accessPoint.c"
#include "processEspData.c"
#include "digitalControl.c"
//Carrega a biblioteca LiquidCrystal
#include <LiquidCrystal.h>

//Define os pinos que serão utilizados para ligação ao display
// era 12, 11, 5, 4, 3, 2 mudei para 8, 9, 5, 4, 3, 2
LiquidCrystal lcd(8, 9, 5, 4, 3, 2);


char brokerIp[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int showConected = 0;

int connectionId = 0;
long lastRequestBroker = 0;
int lengthTextLcd = 0;
int sendingHello = 0;

int showWait = 0;

// era 8 e 9, mudei para 7 e 6
SoftwareSerial esp8266(7, 6);

void setup() {
  
  
  serialPrint = serialPrintText;
  printConstants = printConstantsMessage;
  sendData = sendDataEsp;
  printLCD = printLCDConstant;
  updateTypeIO = updateTypeIOArduino;
  
  wifiConnected = 0;
  
  Serial.begin(9600);
  esp8266.begin(19200);
  
  moduleReset();
  //connectToWifi(sendData, "2.4Ghz Virtua 302", "3207473600");

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Iniciando setup"));
  
  for (int i = 0; i < 8; i++) {
    delay(100);  
    Serial.print(".");
    lcd.setCursor(i, 1);
    lcd.print(".");
  }
  Serial.println(".");
    
  for (int i = 0; i < 8; i++) {
    delay(100);  
    Serial.print(".");
    lcd.setCursor(8 + i, 1);
    lcd.print(".");
  }
  Serial.println(".");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Iniciando setup"));

  lcd.setCursor(0, 1);
  lcd.print(F("Configurando ESP"));
  
  setMultipleConnections();
  enableShowRemoteIp();
  //setStationMode(sendData);
  
  //startServer(sendData);
  
  startAccessPointConfig();
  showLocalIpAddress();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Iniciando setup"));
  
  for (int i = 0; i < 16; i++) {
    delay(300);  
    Serial.print(".");
    lcd.setCursor(i, 1);
    lcd.print(".");
  }

  lcd.clear();
  //startServer(sendData);

  //Serial.println("Digital Control Init");
  initDigitalControl();
  lcd.setCursor(0,0);
  lcd.print(F("Terminou setup"));
  Serial.println(F("Terminou setup"));

  delay(1000);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Nome rede wifi:"));
  
  lcd.setCursor(0,1);
  lcd.print(WIFI_SSID);

}

void loop() {
  
  String message = "";

//  for (int i = 0; i < 10; i++) {
    while (esp8266.available()) {
      message += (char) esp8266.read();
    }
//    delay(100);
//  }
  
  if (message != "") {
    if (DEBUG == 1){
      Serial.print(F("tamanho comando: "));
      Serial.println(message.length());
      Serial.println(message);
    }
    
    char strMessage[message.length() + 1];

    convertStringToChar(message, strMessage);

    if (DEBUG == 1) {
      Serial.print(F("Vai passar uma mensagem para ser processada: "));
      Serial.println(strMessage);
      Serial.println(F("---------------------------------------------------"));
    }
    
    proccessReceivedData(strMessage);
  }

  // o módulo já está conectado a uma rede wifi
  if (wifiConnected == 1) {
    if (sendingHello == 0) {
      sendingHello = 1;
      printLCDText("Conectado", 0);
      printLCDText("modo controle", 1);
    }
    // o módulo ainda não sabe quem é o broker
    if (brokerIpAddressFound == 0) {
      long curTime = millis();
      if (curTime - lastRequestBroker > TIME_REQUEST_HELLO) {
        lastRequestBroker = curTime;
        printLCDText("Enviando hello", 0);
        sendHelloMessage();
        showWait = 1;
      } else if (showWait == 1) {
        showWait = 0;
        printLCDText("aguardando...", 1);
      }
    } else {
      if (showConected == 0) {
        showConected = 1;
        printLCDConstant(MESSAGE_BROKER_FOUND_1, 0);
        printLCDConstant(MESSAGE_BROKER_FOUND_2, 1);
      }
      // enviar dados e tals
      readInputs();
      doControl();
    }
  }
  
}

int sendDataEsp(char *command, const int timeout, int debug, int maxAttempts) {
  
  int okCommand = 0;
  int attempts = 0;

  String response;
  while (okCommand != 1 && attempts <= maxAttempts) {
    response = "";

    if (okCommand != 2) {
      attempts++;
      esp8266.print(command);
      esp8266.print("\r\n");
    }

    int ok = 0;
    long int time = millis();
    char lastChar = 0;
    int okFound = 0;
    while ( (time + timeout) > millis() && okFound != 1) {
      while (esp8266.available()) {
        char c = esp8266.read(); // read the next character.
        response += c;
        if ((lastChar == 'O' || lastChar == 'o') && (c == 'K' || c == 'k')) {
          Serial.println(F("encontrou ok"));
          okFound = 1;
          break;
        }
        lastChar = c;
        //ok = 1;
      }
//      if (okFound == 1)
//        break;

//      if ((response.indexOf("+") > 0 || response.indexOf("OK") > 0) && response.length() > 1) {
//        ok = 1;
//        break;
//      }
//        if (response.indexOf("\r\n") > 0) {
//          ok = 1;
//          break;          
//        }
      //if (ok == 1) break;
    }

    if (DEBUG == 1) {
      Serial.println(F("resposta do esp: "));
      Serial.print(response);
      Serial.println(F("\r\nfim da resposta do esp: "));
    }
  
    if (response.indexOf("OK") > 0 || response.indexOf("busy") || okFound == 1) {
      okCommand = 1;
    } else {
      okCommand = 0;
    }
  }

//  delay(1000);
  
  return okCommand;
}


void prepareMessage(char *originalMessage, char *preparedMessage) {

  for (int i = 0; i < MESSAGE_LENGTH; i++) {
    preparedMessage[i] = originalMessage[i];
  }

  preparedMessage[MESSAGE_LENGTH] = '>';
}

void convertStringToChar(String str, char *charStr) {
  Serial.print(F("vai converter: "));
  Serial.println(str);
  Serial.println(F("---------------------"));
  
  int strLength = str.length();
  
  for (int i = 0; i < strLength; i++) {
    charStr[i] = str.charAt(i);  
  }

  charStr[strLength] = 0;
}

void serialPrintText(char *msg, int isPrintln) {
  if (isPrintln == 1) {
    Serial.println(msg);
  } else {
    Serial.print(msg);
  }
}

const int countMessages = 51;
const int countColumnMessages = 75;

const char MESSAGES[countMessages] [countColumnMessages] PROGMEM = {
  { "msg n encontrada" },
  { "Vai iniciar o processWifiConfig com: " }, //MESSAGE_INDEX_PROCESS_WIFI_CONFIG_1 1
  { "SSID: " }, //MESSAGE_INDEX_PROCESS_WIFI_CONFIG_SSID 2
  { "MACAddress: " }, //MESSAGE_INDEX_PROCESS_WIFI_CONFIG_MAC 3
  { "PassWifi: " }, //MESSAGE_INDEX_PROCESS_WIFI_CONFIG_PASS_W 4
  { "LoginToken: " }, //MESSAGE_INDEX_PROCESS_WIFI_CONFIG_LOGIN 5
  { "PassToken: " }, //MESSAGE_INDEX_PROCESS_WIFI_CONFIG_PASS_L 6
  { "Deu ruim, Clovis" }, //MESSAGE_INDEX_ERROR 7
  { "getDataWifiConfig: " }, //MESSAGE_INDEX_DATA_WIFI_CONFIG 8
  { "StartMessage: " }, //MESSAGE_INDEX_PROCESS_RECEIVED_DATA_1 9
  { " | StartType: " }, //MESSAGE_INDEX_PROCESS_RECEIVED_DATA_2 10
  { " | EndType: " }, //MESSAGE_INDEX_PROCESS_RECEIVED_DATA_3 11
  { "Primeiro caracter mensagem: " }, //MESSAGE_INDEX_PROCESS_RECEIVED_DATA_4 12
  { "Primeiro e ultimo caracter tipo: " }, //MESSAGE_INDEX_PROCESS_RECEIVED_DATA_5 13
  { "Type AT message: " }, //MESSAGE_INDEX_MESSAGE_TYPE_AT 14
  { "Resposta: " }, //MESSAGE_INDEX_RESPONSE 15
  { "Mensagem de lista wifi: " }, //MESSAGE_INDEX_WIFI_LIST 16
  { "IP Address: " }, //MESSAGE_INDEX_IP_ADDRESS 17
  { "Port: " }, //MESSAGE_INDEX_PORT 18
  { "Pegou essa mensagem: " }, //MESSAGE_INDEX_GET_THIS 19
  { "Tipo de mensagem AT nao configurado" }, //MESSAGE_INDEX_AT_TYPE_UNDEFINED 20
  { "chegou no receivedMessage: " }, //MESSAGE_INDEX_PROCESS_RECEIVED_MESSAGE_1 21
  { "Vai processar a seguinte mensagem: " }, //MESSAGE_INDEX_PROCESS_RECEIVED_MESSAGE_2 22
  { "Message Type: " }, //MESSAGE_INDEX_MESSAGE_TYPE 23
  { "Topic: "}, //MESSAGE_INDEX_TOPIC 24
  { "Mensagem de dados recebida: " }, //MESSAGE_INDEX_RECEIVED_MESSAGE 25
  { "Value: "}, //MESSAGE_INDEX_VALUE 26
  { "Topic: " }, //MESSAGE_INDEX_RECEIVED_TOPIC 27
  { "Mensagem de publish recebida: " }, //MESSAGE_INDEX_PUBLISH_MESSAGE_RECEIVED 28
  { "Recebeu uma mensagem de subscribe" }, //MESSAGE_INDEX_SUBSCRIBE_MESSAGE_RECEIVED 29
  { "Recebeu uma mensagem de keep alieve" }, //MESSAGE_INDEX_KEEP_ALIVE_MESSAGE_RECEIVED 30
  { "Recebeu uma mensagem de hello" }, //MESSAGE_INDEX_HELLO_MESSAGE_RECEIVED 31
  { "Recebeu uma mensagem de network" }, //MESSAGE_INDEX_NETWORK_MESSAGE_RECEIVED 32
  { "Recebeu uma mensagem de erro" }, //MESSAGE_INDEX_ERROR_MESSAGE_RECEIVED 33
  { "Recebeu uma mensagem sem um tipo válido" }, //MESSAGE_INDEX_MESSAGE_TYPE_UNDEFINED 34
  { "Ainda não tem IP" }, //MESSAGE_INDEX_IP_NOT_FOUND 35
  { "Vai pegar o IP dessa msg: " }, //MESSAGE_INDEX_MESSAGE_TO_IP 36
  { "Obtendo os dados" }, //MESSAGE_INDEX_GET_CREDENTIALS_1 37
  { "Mensagem recebid" }, //MESSAGE_INDEX_RECEIVED_MESSAGE 38
  { "de credenciais" }, //MESSAGE_INDEX_GET_CREDENTIALS_2 39
  { "nova tent em 3s" }, //MESSAGE_INDEX_NEW_ATTEMPT 40
  { "Conectado" }, //MESSAGE_INDEX_CONNECTED 41
  { "Parametros foram" }, //MESSAGE_BROKER_FOUND_1 42
  { "obtidos" }, //MESSAGE_BROKER_FOUND_2 43
  { "PinsId: " }, //MESSAGE_INDEX_PINS_ID 44
  { "TypesIO: " }, //MESSAGE_INDEX_TYPES_IO 45
  { "SampleTimes: " }, //MESSAGE_INDEX_SAMPLES_TIME 46
  { "kp, ki, kd: " }, //MESSAGE_INDEX_K_PARAM 47
  { "Setpoints: " }, //MESSAGE_INDEX_SETPOINT 48
  { "Condition (id, value, operation): " }, //MESSAGE_INDEX_CONDITION 49
  { "Entradas (id, value, local): " }, //MESSAGE_INDEX_INPUTS 50
};

void printLCDText(char *text, int keepLastText) {
   if (keepLastText != 1) {
    lengthTextLcd = 0;
    lcd.clear();
   }

  int xPos = 0;
  int yPos = 0;
  int strLen = strlen(text);

  if (strLen + lengthTextLcd >= 32 && lengthTextLcd > 0) {
    lengthTextLcd = 0;
    lcd.clear();
    lcd.setCursor(0,0);
    lengthTextLcd = strLen;
  } else {
    if (lengthTextLcd > 16) {
      yPos = 1;
      xPos = lengthTextLcd - 16;
    } else if (strLen + lengthTextLcd > 16) {
      yPos = 1;
      xPos = 0;
    }else {
      yPos = 0;
      xPos = lengthTextLcd;
    }
    lengthTextLcd += strLen;
  }
  
  lcd.setCursor(xPos, yPos);
  lcd.print(text);
}

void printLCDConstant(int messageIndex, int keepLastText) {

 char text[countColumnMessages];
  if (messageIndex >= countColumnMessages) {
    memcpy_P(&text,&MESSAGES[0],sizeof text);  
  } else {
    memcpy_P(&text,&MESSAGES[messageIndex],sizeof text);
  }
  
  printLCDText(text, keepLastText);
}

void printConstantsMessage(int messageIndex, int isPrintln) {

  char text[countColumnMessages];
  if (messageIndex >= countColumnMessages) {
    memcpy_P(&text,&MESSAGES[0],sizeof text);  
  } else {
    memcpy_P(&text,&MESSAGES[messageIndex],sizeof text);
  }
    
  if (isPrintln == 1) {
    Serial.println(text);
  } else {
    Serial.print(text);
  }
}

void updateTypeIOArduino(int *types) {
  for (int i = 0; i < PORTS_AMOUNT; i++) {
    if (types[i] == 1) {
      pinMode(pinIndexControl[i], types[i]);
      Serial.print("Pino ");
      Serial.print(pinIndexControl[i]);
      Serial.println(" setado como saida");
    }
//    pinMode(pinIndexControl[i], types[i]);
  }
}

void doControl() {


  int pinsValue[PORTS_AMOUNT];
  
  digitalControl(pinsValue);

  for (int i = 0; i < PORTS_AMOUNT; i++) {
    int inputIndex = getIndexInputById(inputsId[i]);
    float valueInput = -1;
    if (inputIndex >= 0 && inputIndex < MAX_AMOUNT_INPUT) {
      valueInput = inputs[inputIndex]->value;
    }
    
    Serial.print(F("valor-saida ("));
    Serial.print(i);
    Serial.print(F("): "));
    Serial.print(pinsValue[i]);
    Serial.print(F(" | pino: "));
    Serial.print(pinIndexControl[i]);
    Serial.print(F(" | setpoint: "));
    Serial.print(setPoint[i]);
    Serial.print(F(" | typeIO: "));
    Serial.print(typeIO[i]);
    Serial.print(F(" | inputIndex: "));
    Serial.print(inputIndex);
    Serial.print(F(" | inputValue: "));
    Serial.print(valueInput);
    Serial.print(F(" | time: "));
    Serial.println(millis());

    if (typeIO[i] == '2') {
      analogWrite(pinIndexControl[i], pinsValue[i] / 4);
    } else if (typeIO[i] == '3') {
      if (pinsValue[i] > 0) {
        digitalWrite(pinIndexControl[i], HIGH);
      } else if (pinsValue[i] == 0) {
        digitalWrite(pinIndexControl[i], LOW);
      }
    }
    
  }

  
}


void readInputs() {
  int values[PORTS_AMOUNT];

  for (int i = 0; i < PORTS_AMOUNT; i++) {
    if (typeIO[i] == '1') {
      values[i] = analogRead(pinIndexControl[i]);

//      if (i == 0) {
//        analogWrite(13, values[i] / 4);
//      }
//      values[i] = analogRead(A0);
      Serial.print(i);
      Serial.print(" (");
      Serial.print(pinIndexControl[i]);
      Serial.print("): ");
      Serial.println(values[i]);
    }
  }

  updateInputValues(values);

//  for (int i = 0; i < MAX_AMOUNT_INPUT; i++) {
//    if (inputs[i]->id > 0) {
//      Serial.print("id: ");
//      Serial.print(inputs[i]->id);
//      Serial.print(" | value: ");
//      Serial.print(inputs[i]->value);
//      Serial.print(" | localPin: ");
//      Serial.println(inputs[i]->localPin);
//    }
//  }
}

