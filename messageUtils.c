/*
 * messageUtils.c
 *
 *  Created on: 28 de jul de 2018
 *      Author: vitor
 */

#include "messageUtils.h"

void buildHelloMassage(char *id, char *password, char *message) {

	for (int i = 0; i < MESSAGE_LENGTH; i++) {
		message[i] = EMPTY_CHAR;
	}

	//build header
	message[0] = '0' + MESSAGE_TYPE_HELLO;

	//build body
	for (int i = 0; i < CLIENT_ID_LENGTH; i++) {
		if (id[i] == 0) break;

		message[i + MESSAGE_HEADER_LENGTH] = id[i];
	}

	for (int i = 0; i < CLIENT_PASSWORD_LENGTH; i++) {
		if (password[i] == 0) break;

		message[i + MESSAGE_HEADER_LENGTH + CLIENT_ID_LENGTH] = password[i];
	}

}

void buildKeepAliveMessage(char *token, char *message) {

	for (int i = 0; i < MESSAGE_LENGTH; i++) {
		message[i] = EMPTY_CHAR;
	}

	//build header
	message[0] = '0' + MESSAGE_TYPE_PUBLISH;

	for (int i = 0; i < MESSAGE_TOKEN_LENGTH; i++) {
		if (token[i] == 0) break;

		message[i + 1] = token[i];
	}

}

void buildMessagePublish(char *topic, char *token, float value, int isIntValue, char *message) {

	for (int i = 0; i < MESSAGE_LENGTH; i++) {
		message[i] = EMPTY_CHAR;
	}

	//build header
	message[0] = '0' + MESSAGE_TYPE_PUBLISH;

	for (int i = 0; i < MESSAGE_TOKEN_LENGTH; i++) {
		if (token[i] == 0) break;

		message[i + 1] = token[i];
	}

	//build body
	for (int i = 0; i < MESSAGE_TOPIC_LENGTH; i++) {
		if (topic[i] == 0) break;

		message[i + MESSAGE_HEADER_LENGTH] = topic[i];
	}

	char contentValue[MESSAGE_BODY_CONTENT_LENGTH];
	if (isIntValue == 0) {
		convertFloatToBytes(value, contentValue, MESSAGE_BODY_CONTENT_LENGTH);
	} else {
		convertIntToBytes(value, contentValue, MESSAGE_BODY_CONTENT_LENGTH);
	}


	for (int i = 0; i < MESSAGE_BODY_CONTENT_LENGTH; i++) {
		if (contentValue[i] == 0) break;

		message[i + MESSAGE_HEADER_LENGTH + MESSAGE_TOPIC_LENGTH] = contentValue[i];
	}

}


int getMessageType(char *message) {

	return message[0] - '0';

}

void proccessDataMessage(char *message, char *topic, float *value, char *strValue) {

	//char receivedToken[MESSAGE_TOKEN_LENGTH + 1];
	//char receivedTopic[MESSAGE_TOPIC_LENGTH + 1];
  //char receivedTopic[MESSAGE_INPUT_ID_LENGTH + 1];
	char receivedValue[MESSAGE_VALUE_LENGTH + 1];

  int index = 1;
  
	//subvectorBytes(message, index, index + MESSAGE_TOKEN_LENGTH, receivedToken);
  //index += MESSAGE_TOKEN_LENGTH;
  //receivedToken[MESSAGE_TOKEN_LENGTH] = 0;
  
	subvectorBytes(message, index, index + MESSAGE_INPUT_ID_LENGTH, topic);
  index += MESSAGE_INPUT_ID_LENGTH;
  topic[MESSAGE_INPUT_ID_LENGTH] = 0;
  
	subvectorBytes(message, index, index + MESSAGE_VALUE_LENGTH, receivedValue);
  index += MESSAGE_VALUE_LENGTH;
  receivedValue[MESSAGE_VALUE_LENGTH] = 0;
  
  
//	for (int i = 0; i < MESSAGE_INPUT_ID_LENGTH; i++) {
//		if (receivedTopic[i] == EMPTY_CHAR) {
//			topic[i] = 0;
//		} else {
//			topic[i] = receivedTopic[i];
//		}
//	}
  
  
	//removeEmptyChar(receivedToken, 1);
	removeEmptyChar(receivedValue, 9);

  concatString("", receivedValue, strValue);
	float floatValue = convertBytesToFloat(receivedValue);
	*value = floatValue;
}

void proccessRuleMessage(char *token, struct Condition **conditions, int *ids, char *typeIO, int *sampleTime, float *kp, float *ki, float *kd, float *setPoints, int *inputsId, char *message) {

  int indexMessage = 1; // a partir do 1 porque o 1 é o tipo de mensagem

  // Obtendo token

  //char receivedToken[MESSAGE_TOKEN_LENGTH + 1];
  clearString(token, MESSAGE_TOKEN_LENGTH + 1);

  subvectorBytes(message, indexMessage, indexMessage + MESSAGE_TOKEN_LENGTH, token);

  indexMessage += MESSAGE_TOKEN_LENGTH;

  // Obtendo ids
  for (int i = 0; i < PORTS_AMOUNT; i++) {
    int endValue = indexMessage + MESSAGE_INPUT_ID_LENGTH;
    ids[i] = getSampleTime(indexMessage, endValue, message);
    indexMessage += MESSAGE_INPUT_ID_LENGTH;
  }

  // Obtendo tipo do pino
  for (int i = 0; i < PORTS_AMOUNT; i++) {
    char c = message[indexMessage + i];
    typeIO[i] = c;
  }

  indexMessage += PORTS_AMOUNT;

  // Obtendo kps
  for (int i = 0; i < PORTS_AMOUNT; i++) {
    int endValue = indexMessage + MESSAGE_K_LENGTH;
    kp[i] = getKParam(indexMessage, endValue, message);
    indexMessage += MESSAGE_K_LENGTH;
  }
  // Obtendo kis
  for (int i = 0; i < PORTS_AMOUNT; i++) {
    int endValue = indexMessage + MESSAGE_K_LENGTH;
    ki[i] = getKParam(indexMessage, endValue, message);
    indexMessage += MESSAGE_K_LENGTH;
  }
  // Obtendo kds
  for (int i = 0; i < PORTS_AMOUNT; i++) {
    int endValue = indexMessage + MESSAGE_K_LENGTH;
    kd[i] = getKParam(indexMessage, endValue, message);
    indexMessage += MESSAGE_K_LENGTH;
  }

  //Obtendo sample times
  for (int i = 0; i < PORTS_AMOUNT; i++) {
    int endValue = indexMessage + MESSAGE_SAMPLE_LENGTH;
    sampleTime[i] = getSampleTime(indexMessage, endValue, message);
    indexMessage += MESSAGE_SAMPLE_LENGTH;
  }

  // Obtendo setpoints
  for (int i = 0; i < PORTS_AMOUNT; i++) {
    int endValue = indexMessage + MESSAGE_VALUE_LENGTH;
    setPoints[i] = getKParam(indexMessage, endValue, message);
    indexMessage += MESSAGE_VALUE_LENGTH;
  }

  // Obtendo inputs id
  for (int i = 0; i < PORTS_AMOUNT; i++) {
    int endValue = indexMessage + MESSAGE_INPUT_ID_LENGTH;
    inputsId[i] = getSampleTime(indexMessage, endValue, message);
    indexMessage += MESSAGE_INPUT_ID_LENGTH;
  }

  struct Condition *condition;
  //Obtendo conditions
  for (int i = 0; i < PORTS_AMOUNT; i++) {

    condition = conditions[i];

    if (message[indexMessage] != EMPTY_CHAR) {
      int nextEmptyChar = getNextEmptyChar(message, indexMessage);

      while (nextEmptyChar > indexMessage) {
        char strInputId[MESSAGE_INPUT_ID_LENGTH + 1];
        subvectorBytes(message, indexMessage, indexMessage + MESSAGE_INPUT_ID_LENGTH, strInputId);
        strInputId[MESSAGE_INPUT_ID_LENGTH] = 0;

        indexMessage += MESSAGE_INPUT_ID_LENGTH;

        char operationType = message[indexMessage];
        indexMessage += 1;

        char strValue[MESSAGE_VALUE_LENGTH + 1];
        subvectorBytes(message, indexMessage, indexMessage + MESSAGE_VALUE_LENGTH, strValue);
        strValue[MESSAGE_VALUE_LENGTH] = 0;

        indexMessage += MESSAGE_VALUE_LENGTH;

        condition->inputId = convertBytesToInt(strInputId);
        condition->operation = operationType;
        condition->value = convertBytesToFloat(strValue);
        condition->next = malloc(sizeof(struct  Condition));

        condition = condition->next;
        condition->inputId = -1;
      }
    } else {
      indexMessage++;
    }
  }
}

int getNextEmptyChar(char *txt, int startIndex) {
  int len = strlen(txt);

  for (int i = startIndex; i < len; i++) {
    char c = txt[i];
    if (c == EMPTY_CHAR)
      return i;
  }

  return -1;
}

//int getNextConditionSeparator(char *txt, int startIndex) {
//    int len = strlen(txt);
//  
//  for (int i = startIndex; i < len; i++) {
//    char c = txt[i];
//    if (c == ';')
//      return i;
//  }
//
//  return -1;
//}

int getSampleTime(int startIndex, int endIndex, char *message) {

  char strValue[endIndex - startIndex + 1];
  subvectorBytes(message, startIndex, endIndex, strValue);
  strValue[endIndex - startIndex] = 0;
  
  return convertBytesToInt(strValue);
}

float getKParam(int startIndex, int endIndex, char *message) {

  char strValue[endIndex - startIndex + 1];
  subvectorBytes(message, startIndex, endIndex, strValue);
  strValue[endIndex - startIndex] = 0;

  return convertBytesToFloat(strValue);
}

//void addEmptyChar(char *str, int size) {
//	for (int i = 0; i < size; i++) {
//		if (str[i] == 0) {
//			str[i] = EMPTY_CHAR;
//		}
//	}
//}

void removeEmptyChar(char *str, int size) {
	for (int i = 0; i < size; i++) {
		if (str[i] == EMPTY_CHAR) {
			str[i] = 0;
		}
	}
}
