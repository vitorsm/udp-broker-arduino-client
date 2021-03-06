/*
 * accessPoint.c
 *
 *  Created on: 28 de jul de 2018
 *      Author: vitor
 */

#include "accessPoint.h"

void startAccessPointConfig() {

  startAccessPoint(WIFI_SSID);
  startServerConfig();
  
}

void stopAccessPointConfig() {
  
  stopAccessPoint();
  
}

void startServerConfig() {

//  startTCPServer(sendData, CLIENT_PORT);
  startServer();
  
}

void stopServerConfig() {

  stopTCPServer(CLIENT_PORT);
  
}

void processRequestNetworks() {
  listAPs(); 
}

void processResponseListAPs(char *command) {
  int commandSize = strlen(command);

  // colocar aqui um tratamento de string para mandar a lista de redes wifi disponiveis
  // é necessário avaliar quais dados serão necessários enviar
  // o aplicativo irá ler os dados

  // implementar o build message networks information
}

void processWifiConfig(char *command) {

  int commandSize = strlen(command);

  char ssid[commandSize];
  clearString(ssid, commandSize);
  
  char netMacAddress[commandSize];
  clearString(netMacAddress, commandSize);
  
  char passwordWifi[commandSize];
  clearString(passwordWifi, commandSize);
  
  char id[CLIENT_ID_LENGTH];
  clearString(id, CLIENT_ID_LENGTH);
  
  char password[CLIENT_PASSWORD_LENGTH];
  clearString(password, CLIENT_PASSWORD_LENGTH);
  
  if (DEBUG == 1) {
    printConstants(MESSAGE_INDEX_PROCESS_WIFI_CONFIG_1, 1);
    serialPrint(command, 1);
  }
  
  getDataWifiConfig(command, ssid, netMacAddress, passwordWifi, id, password);

  if (DEBUG == 1) {
    printConstants(MESSAGE_INDEX_PROCESS_WIFI_CONFIG_SSID, 0);
    serialPrint(ssid, 1);

    printConstants(MESSAGE_INDEX_PROCESS_WIFI_CONFIG_MAC, 0);
    serialPrint(netMacAddress, 1);

    printConstants(MESSAGE_INDEX_PROCESS_WIFI_CONFIG_PASS_W, 0);
    serialPrint(passwordWifi, 1);

    printConstants(MESSAGE_INDEX_PROCESS_WIFI_CONFIG_LOGIN, 0);
    serialPrint(id, 1);
    
    printConstants(MESSAGE_INDEX_PROCESS_WIFI_CONFIG_PASS_L, 0);
    serialPrint(password, 1);
  }
  
  int response = stopTCPServer(CLIENT_PORT);
  serialPrint("1", 1);
  // analisar se é possível testar as credenciais de uma rede sem deixar de ser AP

  if (response == 1) {
    response = stopAccessPoint();
  }

  serialPrint("2", 1);
  
  if (response == 1) {
    response = connectToWifi(ssid, passwordWifi);
  }

  serialPrint("3", 1);
  
  if (response == 1) {
    response = startServer();
  }

  serialPrint("4", 1);
  
  setCredentials(id, password);
  
  if (response == 0) {
    serialPrint("ruim", 1);
    // se tiver um lcd mostrar aqui que a autenticação falhou
    if (DEBUG == 1) {
      printConstants(MESSAGE_INDEX_ERROR, 1);
      printLCD(MESSAGE_INDEX_ERROR, 0);
      printLCD(MESSAGE_INDEX_NEW_ATTEMPT, 1);

      delay(3000);
      processWifiConfig(command);
    }
  } else {
    serialPrint("bom", 1);
    printLCD(MESSAGE_INDEX_CONNECTED, 1);
    wifiConnected = 1;
  }
}

void getDataWifiConfig(char *command, char *ssid, char *netMacAddress, char *passwordWifi, char *id, char *password) {
  int emptyCharCount = 0;
  
  printLCD(MESSAGE_INDEX_GET_CREDENTIALS_1, 0);
  printLCD(MESSAGE_INDEX_GET_CREDENTIALS_2, 1);
  
  if (DEBUG == 1) {
    printConstants(MESSAGE_INDEX_DATA_WIFI_CONFIG, 1);
    serialPrint(command, 1);
  }
  int commandSize = strlen(command);

  int wordCount = 0;
  
  for (int i = 0; i < commandSize; i++) {
    char c = command[i];

    if (c == EMPTY_CHAR) {
      emptyCharCount++;
      wordCount = 0;
    } else {
      if (emptyCharCount == 1) {
        ssid[wordCount] = c;
        wordCount++;
        ssid[wordCount] = 0;
        //Coloquei esse 10 aqui so pra ele nunca entrar nesse if
        //Quando eu coloco o MAC Address na mensagem ela nem chega no main
        //Por motivos de tempo nao me preocupei em descobrir o q aconteceu
      } else if (emptyCharCount == 10) {
        netMacAddress[wordCount] = c;
        wordCount++;
        netMacAddress[wordCount] = 0;
      } else if (emptyCharCount == 2) {
        passwordWifi[wordCount] = c;
        wordCount++;
        passwordWifi[wordCount] = 0;
      } else if (emptyCharCount == 3) {
        id[wordCount] = c;
        wordCount++;
        id[wordCount] = 0;
      } else if (emptyCharCount == 4) {
        password[wordCount] = c;
        wordCount++;
        password[wordCount] = 0;
      }
    }
  }  
}

