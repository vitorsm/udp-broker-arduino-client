/*
 * processEspData.c
 *
 *  Created on: 25 de jul de 2018
 *      Author: vitor
 */

#include "processEspData.h"

void proccessReceivedData(sendDataFunc *sendData, char *data, serialPrintFunc *serialPrint, printConstantsMessages *printConstants) {

  int startMessage = 0;
  int startType = 0;
  int endsType = 0;
  
  getMessageBounds(data, &startType, &endsType, &startMessage, serialPrint, printConstants);
  
  //int startMessage = getStartsMessage(data);

  if (DEBUG == 1) {
    char strStartMessage[4];
    char strStartType[4];
    char strEndsType[4];

    convertIntToBytes(startMessage, strStartMessage, 4);
    convertIntToBytes(startType, strStartType, 4);
    convertIntToBytes(endsType, strEndsType, 4);

    printConstants(MESSAGE_INDEX_PROCESS_RECEIVED_DATA_1, 0);
    serialPrint(strStartMessage, 0);
    
    printConstants(MESSAGE_INDEX_PROCESS_RECEIVED_DATA_2, 0);
    serialPrint(strStartType, 0);

    printConstants(MESSAGE_INDEX_PROCESS_RECEIVED_DATA_3, 0);
    serialPrint(strEndsType, 1);

    char messageTest[] = {data[startMessage], data[startMessage + 1], 0};
    printConstants(MESSAGE_INDEX_PROCESS_RECEIVED_DATA_4, 0);
    serialPrint(messageTest, 1);

    char typeTest[] = {data[startType], data[endsType], 0};
    printConstants(MESSAGE_INDEX_PROCESS_RECEIVED_DATA_5, 0);
    serialPrint(typeTest, 1);
    
  }
  
  if (startMessage >= 0 && startType >= 0 && endsType >= 0) {
    char type[MAX_SIZE_AT_TYPE_MESSAGE];
    clearString(brokerIpAddress, MAX_SIZE_AT_TYPE_MESSAGE);
    
    subvectorBytes(data, startType, endsType, type);
   
    if (DEBUG == 1) {
      printConstants(MESSAGE_INDEX_MESSAGE_TYPE_AT, 0);
      serialPrint(type, 1);
      
      int resp = compareBytes("+IPD", type, 4);
      char strResp[4];
      convertIntToBytes(resp, strResp, 4);
      
      printConstants(MESSAGE_INDEX_RESPONSE, 0);
      serialPrint(strResp, 1);
    }
    
    if (compareBytes("+CWLAP1", type, 5) == 1) {
      printConstants(MESSAGE_INDEX_WIFI_LIST, 1);
      processResponseListAPs(sendData, data);
    } else if (compareBytes("+IPD", type, 4) == 1) {
      
      int connectionId = 0;
      int messageLength = 0;
      char ipAddress[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      int port = 0;
      
      getDataFromReceivedData(data, &connectionId, &messageLength, ipAddress, &port, serialPrint, printConstants);

      if (DEBUG == 1) {
          //printar os dados recebidos
          printConstants(MESSAGE_INDEX_IP_ADDRESS, 0);
          serialPrint(ipAddress, 1);
          char strPort[6];
          convertIntToBytes(port, strPort, 6);
          
          printConstants(MESSAGE_INDEX_PORT, 0);
          serialPrint(strPort, 1);
      }

      if (startMessage >= 0) {
        char message[messageLength + 1];
        subvectorBytes(data, startMessage, startMessage + messageLength, message);
        message[messageLength] = 0;

        if (DEBUG == 1) {
          printConstants(MESSAGE_INDEX_GET_THIS, 1);
          serialPrint(message, 1);
        }

        proccessReceivedMessage(sendData, message, ipAddress, port, serialPrint, printConstants);
      }

    } else if (DEBUG == 1) {
      printConstants(MESSAGE_INDEX_AT_TYPE_UNDEFINED, 1);
    }
  }
    
  
}

void getDataFromReceivedData(char *data, int *connectionId, int *messageLength, char *ipAddress, int *port, serialPrintFunc *serialPrint, printConstantsMessages *printConstants) {

  int dataLength = strlen(data);

  int commaCount = 0;
  
  int countStr = 0;
  char messageLengthStr[] = {0, 0, 0, 0, 0, 0, 0};
  char connectionIdStr[] = {0, 0};
  char portStr[] = {0, 0, 0, 0, 0, 0, 0, 0};

  if (DEBUG == 1) {
    printConstants(MESSAGE_INDEX_MESSAGE_TO_IP, 0);
    serialPrint(data, 1);  
  }
  
  for (int i = 0; i < dataLength; i++) {
    char c = data[i];
    
    if (c == ':') {
      break;
    } else if (c == ',') {
      commaCount++;
      countStr = 0;
    } else {

      if (commaCount == 1) {
        connectionIdStr[0] = c;
      } else if (commaCount == 2) {
        messageLengthStr[countStr] = c;
      } else if (commaCount == 3) {
        ipAddress[countStr] = c;
      } else if (commaCount == 4) {
        portStr[countStr] = c;
      }

      countStr++;
    }
  }
  
  *connectionId = convertBytesToInt(connectionIdStr);
  *messageLength = convertBytesToInt(messageLengthStr);
  *port = convertBytesToInt(portStr);
}

void proccessReceivedMessage(sendDataFunc *sendData, char *message, char *originIp, int originPort, serialPrintFunc *serialPrint, printConstantsMessages *printConstants) {
 
  char strMessageType[2];
  strMessageType[1] = 0;
  subvectorBytes(message, 0, 1, strMessageType);

  printConstants(MESSAGE_INDEX_PROCESS_RECEIVED_MESSAGE_1, 0);
  serialPrint(strMessageType, 1);
  
  int messageType = convertBytesToInt(strMessageType);

  char topic[MESSAGE_BODY_LENGTH - MESSAGE_TOKEN_LENGTH + 1]; // case MESSAGE_TYPE_DATA:
  
  if (DEBUG == 1) {
    printConstants(MESSAGE_INDEX_PROCESS_RECEIVED_MESSAGE_2, 1);
    serialPrint(message, 1);
    
    printConstants(MESSAGE_INDEX_MESSAGE_TYPE, 0);
    serialPrint(strMessageType, 1);
    printConstants(MESSAGE_INDEX_TOPIC, 0);
    serialPrint(topic, 1);
  }
    
  float value = 12; //case MESSAGE_TYPE_DATA:
  char strValue[15]; //case MESSAGE_TYPE_DATA:
  switch (messageType) {
    case MESSAGE_TYPE_UPDATE_PARAM:
      if (DEBUG == 1) {
        //concatString(strLog, "\r\recebeu uma mensagem de atualizacao de parametros", strLog);
        //serialPrint("Recebeu uma mensagem de atualizacao de parametros", 1);
      }
      clearString(brokerIpAddress, 16);
      concatString(brokerIpAddress, originIp, brokerIpAddress);
      brokerIpAddressFound = 1;

      setParams(message);
      break;
    case MESSAGE_TYPE_DATA:
      proccessDataMessage(message, topic, &value, strValue);
      topic[MESSAGE_TOKEN_LENGTH] = 0;

      if (DEBUG == 1) {
        printConstants(MESSAGE_INDEX_MESSAGE_TYPE, 1);
        printConstants(MESSAGE_INDEX_VALUE, 0);
        serialPrint(strValue, 1);

        printConstants(MESSAGE_INDEX_RECEIVED_TOPIC, 0);
        serialPrint(topic, 1);
      }
      
      break;
    case MESSAGE_TYPE_PUBLISH:
      if (DEBUG == 1) {
        //concatString(strLog, "\r\recebeu uma mensagem de publish", strLog);
        printConstants(MESSAGE_INDEX_PUBLISH_MESSAGE_RECEIVED, 1);
      }
      break;
    case MESSAGE_TYPE_SUBSCRIBE:
      if (DEBUG == 1) {
        printConstants(MESSAGE_INDEX_SUBSCRIBE_MESSAGE_RECEIVED, 1);
      }
      break;
    case MESSAGE_TYPE_KEEP_ALIVE:
      if (DEBUG == 1) {
        printConstants(MESSAGE_INDEX_KEEP_ALIVE_MESSAGE_RECEIVED, 1);
      }
      break;
    case MESSAGE_TYPE_HELLO:
      if (DEBUG == 1) {
        printConstants(MESSAGE_INDEX_HELLO_MESSAGE_RECEIVED, 1);
      }
      break;
    case MESSAGE_TYPE_NETWORKS:
      if (DEBUG == 1) {
        printConstants(MESSAGE_INDEX_NETWORK_MESSAGE_RECEIVED, 1);
        processWifiConfig(sendData, message, serialPrint, printConstants);
      }
      break;
    case MESSAGE_TYPE_CREDENTIALS_REQUEST:
      if (DEBUG == 1) {
        serialPrint("recebeu uma mensagem de requisicao de credenciais", 1);
      }
      break;
    case MESSAGE_TYPE_ERROR:
      if (DEBUG == 1) {
        printConstants(MESSAGE_INDEX_ERROR_MESSAGE_RECEIVED, 1);
      }
      break;
    default:
      if (DEBUG == 1) {
        printConstants(MESSAGE_INDEX_MESSAGE_TYPE_UNDEFINED, 1);
      }
      break;
  }

}

void getBrokerIpAddress(char *ipAddress, serialPrintFunc *serialPrint, printConstantsMessages *printConstants) {
  if (brokerIpAddressFound == 1) {
    clearString(ipAddress, 16);
    concatString("", brokerIpAddress, ipAddress);
  } else if (DEBUG == 1) {
    printConstants(MESSAGE_INDEX_IP_NOT_FOUND, 1);
  }
}


void getMessageBounds(char *message, int *startType, int *endsType, int *startMessage, serialPrintFunc *serialPrint, printConstantsMessages *printConstants) {

  int strLength = strlen(message);

  int _startMessage = -1;
  int i = 0;
  for (i = 0; i < strLength; i++) {
    char c = message[i];
    if (c == ':') {
      _startMessage =  i + 1;
    }
  }

  i = _startMessage - 2;
  int commaIndex = -1;
  int plusIndex = -1;
  
  if (_startMessage >= 0) {
    
    while (i >= 0) {
      char c = message[i];
      if (c == ',') {
        commaIndex = i;
      } else if (c == '+') {
        plusIndex = i;
        break;
      }
      i--;
    }
    
  }

  *(startType) = plusIndex;
  *(endsType) = commaIndex;
  *(startMessage) = _startMessage;
    
}

