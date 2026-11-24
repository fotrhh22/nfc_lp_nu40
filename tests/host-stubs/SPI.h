#pragma once
#include <cassert>
#include <stdint.h>
struct MockSPI{void setPins(uint8_t miso,uint8_t sck,uint8_t mosi){assert(miso==46&&sck==47&&mosi==45);}};
extern MockSPI SPI;
