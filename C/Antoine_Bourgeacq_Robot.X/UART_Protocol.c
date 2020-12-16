#include <xc.h>
#include "UART_Protocol.h"

int msgDecodedFunction =0;
int msgDecodedPayloadLength =0;
unsigned char msgDecodedPayload[128];
int msgDecodedPayloadIndex =0;

unsigned char UartCalculateChecksum(int msgFunction, int msgPayloadLength, unsigned char* msgPayload){
    //Fonction prenant en entree la trame et sa longueur pour calculer le checksum
    
}

void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, unsigned char* msgPayload){
    //Fonction d'encodage et d'envoi d'un message
}

void UartDecodeMessage(unsigned char c){
    //Fonction prenant en entree un octet et servant a reconstituer les trames
}

void UartProcessDecodeMessage(int function, int payloadLength, unsigned char* payload){
    //fonction appelee apres le decodage pour executer l'action
    //correspondant au message recu
}