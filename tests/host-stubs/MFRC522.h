#pragma once
#include <stdint.h>
struct MFRC522 {
 enum StatusCode {STATUS_OK,STATUS_COLLISION,STATUS_TIMEOUT,STATUS_ERROR};
 enum PCD_Register {VersionReg};
 struct {uint8_t size=4;uint8_t uidByte[10]={1,2,3,4};} uid;
 bool hostCard=false,hostReadOK=true;
 uint8_t hostVersion=0x92;
 unsigned hostHalts=0;
 MFRC522(uint8_t,uint8_t){}
 void PCD_Init(){}
 uint8_t PCD_ReadRegister(PCD_Register){return hostVersion;}
 StatusCode PICC_WakeupA(uint8_t*,uint8_t*){return hostCard?STATUS_OK:STATUS_TIMEOUT;}
 bool PICC_ReadCardSerial(){return hostReadOK;}
 void PICC_HaltA(){++hostHalts;}
 void PCD_StopCrypto1(){}
};
