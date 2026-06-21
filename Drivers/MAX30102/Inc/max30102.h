#ifndef MAX30102_H
#define MAX30102_H

#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define MAX30102_SAMPLE_LEN_MAX 32

typedef struct max30102_t
{
    I2C_HandleTypeDef *_ui2c;
    uint32_t _ir_samples[MAX30102_SAMPLE_LEN_MAX];
    uint8_t _interrupt_flag;
} max30102_t;

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

int max30102_write(max30102_t *dev, uint8_t memAddr, const uint8_t *pData, uint16_t size);
int max30102_read(max30102_t *dev, uint8_t memAddr, uint8_t *pData, uint16_t size);
int max30102_write_byte(max30102_t *dev, uint8_t memAddr, uint8_t byte);
int max30102_read_byte(max30102_t *dev, uint8_t memAddr, uint8_t *byte);

// perform initialization sequence
int max30102_init(max30102_t *dev, I2C_HandleTypeDef *hi2c);
int max30102_reset(max30102_t *dev);
int max30102_clear_fifo(max30102_t *dev);
int max30102_init_fifo(max30102_t *dev, max30102_smp_ave_t smp_ave, uint8_t rollover_en, uint8_t fifo_a_full);

int max30102_set_led_pulse_width(max30102_t *dev, max30102_led_pw_t pulse_width);
int max30102_set_adc_resolution(max30102_t *dev, max30102_adc_t adc_rge);
int max30102_set_sampling_rate(max30102_t *dev, max30102_sr_t sample_rate);
int max30102_set_led_current_1(max30102_t *dev, float ma); // IR

int max30102_set_mode(max30102_t *dev, max30102_mode_t mode);
int max30102_set_a_full(max30102_t *dev, uint8_t enable);
int max30102_set_ppg_ready(max30102_t *dev, uint8_t enable);

int max30102_on_interrupt(max30102_t *dev);
int max30102_has_interrupt(max30102_t *dev);
int max30102_interrupt_handler(max30102_t *dev);
int max30102_read_fifo(max30102_t *dev);





#endif
