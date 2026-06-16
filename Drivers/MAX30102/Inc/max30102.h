#ifndef MAX30102_H
#define MAX30102_H

#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

// perform initialization sequence
int max30102_init(I2C_HandleTypeDef *hi2c);

int max30102_write(I2C_HandleTypeDef *hi2c, uint8_t memAddr, const uint8_t *pData, uint16_t size);
int max30102_read(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *pData, uint16_t size);
int max30102_write_byte(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t byte);
int max30102_read_byte(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *byte);

#endif
