#ifndef MAX30102_H
#define MAX30102_H

#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef enum max30102_mode_t
{
    max30102_heart_rate = 0x02,
    max30102_spo2 = 0x03,
    max30102_multi_led = 0x07
} max30102_mode_t;

typedef enum max30102_smp_ave_t
{
    max30102_smp_ave_1,
    max30102_smp_ave_2,
    max30102_smp_ave_4,
    max30102_smp_ave_8,
    max30102_smp_ave_16,
    max30102_smp_ave_32,
} max30102_smp_ave_t;

typedef enum max30102_sr_t
{
    max30102_sr_50,
    max30102_sr_100,
    max30102_sr_200,
    max30102_sr_400,
    max30102_sr_800,
    max30102_sr_1000,
    max30102_sr_1600,
    max30102_sr_3200
} max30102_sr_t;

typedef enum max30102_led_pw_t
{
    max30102_pw_15_bit,
    max30102_pw_16_bit,
    max30102_pw_17_bit,
    max30102_pw_18_bit
} max30102_led_pw_t;

typedef enum max30102_adc_t
{
    max30102_adc_2048,
    max30102_adc_4096,
    max30102_adc_8192,
    max30102_adc_16384
} max30102_adc_t;

typedef enum max30102_multi_led_ctrl_t
{
    max30102_led_off,
    max30102_led_red,
    max30102_led_ir
} max30102_multi_led_ctrl_t;

int max30102_write(I2C_HandleTypeDef *hi2c, uint8_t memAddr, const uint8_t *pData, uint16_t size);
int max30102_read(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *pData, uint16_t size);
int max30102_write_byte(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t byte);
int max30102_read_byte(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *byte);

// perform initialization sequence
int max30102_init(I2C_HandleTypeDef *hi2c);
int max30102_reset(I2C_HandleTypeDef *hi2c);
int max30102_clear_fifo(I2C_HandleTypeDef *hi2c);
int max30102_init_fifo(I2C_HandleTypeDef *hi2c, max30102_smp_ave_t smp_ave, uint8_t rollover_en, uint8_t fifo_a_full);

int max30102_set_led_pulse_width(I2C_HandleTypeDef *hi2c, max30102_led_pw_t pulse_width);
int max30102_set_adc_resolution(I2C_HandleTypeDef *hi2c, max30102_adc_t adc_rge);
int max30102_set_sampling_rate(I2C_HandleTypeDef *hi2c, max30102_sr_t sample_rate);
int max30102_set_led_current_1(I2C_HandleTypeDef *hi2c, float ma); // IR

int max30102_set_mode(I2C_HandleTypeDef *hi2c, max30102_mode_t mode);
int max30102_set_a_full(I2C_HandleTypeDef *hi2c, uint8_t enable);



#endif
