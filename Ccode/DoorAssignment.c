#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "SafeInput.h"
#include <stdbool.h>
#include <time.h>
#include <Windows.h>
#include "serial.h"

typedef struct {
	char cardNumber[10];
	bool access;
	struct tm* date;
} CARD;

typedef struct {
	CARD* allCards;
	int amountCards;
}SYSTEM_STATE;

void SendCardToArduino(int index, SERIALPORT port, SYSTEM_STATE* state) {
	int count = 0;
	if (state->allCards[index].access == true) {
		char cardNumber[50];
		strcpy(cardNumber, "<ADDCARD");
		strcat(cardNumber, state->allCards[index].cardNumber);
		strcat(cardNumber, ">");
		SerialWritePort(port, ("%s", cardNumber), strlen(cardNumber));
	}
}

void ScanCardArduino(SERIALPORT port, SYSTEM_STATE* state) {
	char cardNumber[50];
	char numberToSend[50];
	GetInput("Enter cardnumber: ", cardNumber, strlen(cardNumber));
	strcpy(numberToSend, "<SCANCARD");
	strcat(numberToSend, cardNumber);
	strcat(numberToSend, ">");
	SerialWritePort(port, ("%s", numberToSend), strlen(numberToSend));
}

void ClearCardsInArduino(SERIALPORT port, SYSTEM_STATE* state) {
	SerialWritePort(port, "<CLEARALLCARDS>", strlen("<CLEARALLCARDS>"));
}

void SendValidCardsToArduino(SERIALPORT port, SYSTEM_STATE* state){
	for (int i = 0; i < state->amountCards; i++) {
		if (state->allCards[i].access == true) {
			char cardNumber[50];
			strcpy(cardNumber, "<ADDCARD");
			strcat(cardNumber, state->allCards[i].cardNumber);
			strcat(cardNumber, ">");
			SerialWritePort(port, ("%s", cardNumber), strlen(cardNumber));
		}
	}
}

char* IsCardInList(char* inputCardNumber, SYSTEM_STATE* state){ 
	for (int i = 0; i < state->amountCards; i++){
		if (strcmp(inputCardNumber, state->allCards[i].cardNumber) == 0){
			return state->allCards[i].cardNumber;
		}
	}
	return;
}

int GetCardIndex(char* inputCardNumber, SYSTEM_STATE* state){
	for (int i = 0; i < state->amountCards; i++){
		if (strcmp(inputCardNumber, state->allCards[i].cardNumber) == 0){
			return i;
		}
	}
	return 0;
}

void AddDate(int index, SYSTEM_STATE* state) {
	time_t dateNow = time(NULL);
	state->allCards[index].date = localtime(&dateNow);
}

void SetAccessInput(SERIALPORT port, int index, SYSTEM_STATE* state) {
	int inputAccess;
	GetInputInt("Enter 1 for access, 2 for no access>", &inputAccess);
	if (inputAccess == 1) {
		state->allCards[index].access = true;
	}
	else if (inputAccess == 2) {
		state->allCards[index].access = false;
		ClearCardsInArduino(port, state);
		SendValidCardsToArduino(port, state);
	}
}

void AddRemoveAccess(SERIALPORT port, SYSTEM_STATE* state) {
	char inputCardNumber[20];
	int index;
	
	GetInput("Enter cardnumber>", inputCardNumber, sizeof(inputCardNumber));

	if (state->amountCards == 0) {
		state->allCards = malloc(sizeof(CARD));
		state->amountCards = 1;
		index = 0;

		strcpy(state->allCards[index].cardNumber, inputCardNumber);
		AddDate(index, state);
		SetAccessInput(port, index, state);
		SendCardToArduino(index, port, state);

	}
	else if (strcmp(IsCardInList(inputCardNumber, state), inputCardNumber) == 0) {
		index = GetCardIndex(inputCardNumber, state);
		printf("This card has %s\n", state->allCards[index].access ? "access" : "no access");
		AddDate(index, state);
		SetAccessInput(port, index, state);
		SendCardToArduino(index, port, state);
	}
	else if (strcmp(IsCardInList(inputCardNumber, state), inputCardNumber) != 0) {
		state->allCards = realloc(state->allCards, sizeof(CARD) * (state->amountCards + 1));
		index = state->amountCards;
		state->amountCards++;
		strcpy(state->allCards[index].cardNumber, inputCardNumber);
		AddDate(index, state);
		SetAccessInput(port, index, state);
		SendCardToArduino(index, port, state);
	}
}

void RemoteOpenDoor(SERIALPORT port) {
	SerialWritePort(port, "<REMOTEOPENDOOR>", strlen("<REMOTEOPENDOOR>"));
}

void ListAllCards(SYSTEM_STATE* state) {
	printf("All cards in system\n");
	for (int i = 0; i < state->amountCards; i++) {
		CARD c = state->allCards[i];
		printf("%s, %s Added to system: %d-%02d-%02d\n", c.cardNumber, c.access ? "Access" : "No Access", c.date->tm_year + 1900, c.date->tm_mon + 1, c.date->tm_mday);
	}
	printf("press key to continue\n");
	getch();
}

int MainMenu(SYSTEM_STATE* state, SERIALPORT port) {
	while (true) {
		printf(
			"----------MENY----------\n"
			"1.Remote open door\n"
			"2.List all cards in system\n"
			"3.Add/remove access\n"
			"4.Exit\n"
			"9.Scan card\n"
		);
		int selection = 0;

		if (!GetInputInt("Enter choice:>", &selection)) {
			continue;
		}
		switch (selection) {

		case 1:
			RemoteOpenDoor(port);
			break;
		case 2:
			ListAllCards(state);
			break;
		case 3:
			AddRemoveAccess(port, state);
			break;
		case 4:
			return;
		case 9:
			ScanCardArduino(port, state);
			break;
		}
	}
	return 0;
}

int main() {

	SYSTEM_STATE state;
	SERIALPORT port = SerialInit("\\\\.\\COM7");
	if (!SerialIsConnected(port))
	{
		return NULL;
	}
	state.allCards = NULL;
	state.amountCards = 0;
	MainMenu(&state, port);
	free(state.allCards);
	SerialClose(port);
	return 0;
}