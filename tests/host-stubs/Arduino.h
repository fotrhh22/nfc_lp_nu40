#pragma once
#include <stdint.h>
#include <string.h>
#include <vector>
#include <deque>
constexpr bool HIGH=true,LOW=false;
constexpr int INPUT_PULLUP=2,OUTPUT=1;
extern uint64_t mockUs;
extern bool mockPins[64];
inline uint32_t micros(){return uint32_t(mockUs);}
inline uint32_t millis(){return uint32_t(mockUs/1000);}
inline void pinMode(uint8_t,int){}
inline void digitalWrite(uint8_t p,bool v){mockPins[p]=v;}
inline bool digitalRead(uint8_t p){return mockPins[p];}
inline void yield(){}
inline void delay(uint32_t ms){mockUs+=uint64_t(ms)*1000;}
struct MockAudioSerial {
 std::vector<std::vector<uint8_t>> packets;
 std::deque<uint8_t> incoming;
 void begin(int baud){if(baud!=9600)__builtin_trap();}
 size_t write(const uint8_t* data,size_t size){packets.emplace_back(data,data+size);return size;}
 int available(){return incoming.size();}
 int read(){const auto b=incoming.front();incoming.pop_front();return b;}
};
extern MockAudioSerial Serial1;
struct MockSerial {
 void begin(int){} int available(){return 0;} char read(){return 0;}
 void println(){}
 explicit operator bool()const{return false;}
 template<class T>void print(T){} template<class T>void println(T){}
};
extern MockSerial Serial;
