/*
 * config.h
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "tim.h"

#define DEFAULT_FREQ 		100
extern uint32_t SYSCLK;

typedef struct {
	float freq;
	float duty;
	float delay;
	uint32_t period;
	TIM_HandleTypeDef* tim;
} motor_config_t;

void init_config(motor_config_t* config, float freq, float duty, float delay);
void motor_set_freq(motor_config_t* config, float freq);
void motor_set_duty(motor_config_t* config, float duty);
void motor_set_delay(motor_config_t* config, float delay);
void motor_set_delay_us(motor_config_t* config, float delay_us);
void start_motors(motor_config_t* config, uint16_t length);
void stop_motors(motor_config_t* config, uint16_t length);
void reset_motor_timers(motor_config_t* config, uint16_t length);

#endif /* CONFIG_H_ */
