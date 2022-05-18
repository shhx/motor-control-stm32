/*
 * config.c
 *
 */
#include "config.h"

void init_config(motor_config_t* config, float freq, float duty, float delay){
	motor_set_freq(config, freq);
	motor_set_duty(config, duty);
//	motor_set_delay_us(config, delay);
}

void motor_set_freq(motor_config_t* config, float freq){
	config->freq = freq;
	TIM_HandleTypeDef* tim = config->tim;
	uint32_t preescaler = 0;
	uint32_t period = SYSCLK / config->freq / (preescaler + 1);
	while(period > (1<<16) - 1){
		preescaler++;
		period = SYSCLK / config->freq / (preescaler + 1);
	}
	tim->Instance->PSC = preescaler;
	config->period = period;
	config->freq = freq;
	__HAL_TIM_SET_AUTORELOAD(config->tim, period - 1);
}

void motor_set_duty(motor_config_t* config, float duty){
	uint32_t pwm_value = duty * config->period;
	if(pwm_value > 0){
		pwm_value -= 1;
	}
	config->duty = duty;
	__HAL_TIM_SET_COMPARE(config->tim, TIM_CHANNEL_1, pwm_value);
}

void motor_set_delay(motor_config_t* config, float delay){
	TIM_HandleTypeDef* tim = config->tim;
	config->delay = delay;
	uint16_t phase_shift = config->delay * config->period;
  __HAL_TIM_SET_COUNTER(tim, phase_shift);
}

void motor_set_delay_us(motor_config_t* config, float delay_us){
	TIM_HandleTypeDef* tim = config->tim;
	uint32_t ARR = (config->period - 1);//tim->Instance->ARR;
	float period_us = 1e6 / config->freq;
	if(delay_us > period_us){
		delay_us = 0;
	}
	uint16_t phase_shift = delay_us/period_us * ARR;
  __HAL_TIM_SET_COUNTER(tim, ARR - phase_shift);
}

void start_motors(motor_config_t* config, uint16_t length){
	for (int i = length-1; i >= 0; --i) {
		HAL_TIM_PWM_Start(config[i].tim, TIM_CHANNEL_1);
	}
}

void stop_motors(motor_config_t* config, uint16_t length){
	for (int i = 0; i < length; ++i) {
		HAL_TIM_PWM_Stop(config[i].tim, TIM_CHANNEL_1);
	}
	reset_motor_timers(config, length);
}

void reset_motor_timers(motor_config_t* config, uint16_t length){
	for (int i = 0; i < length; ++i) {
		// Triggering update event resets CNT and PSC cnt
		config[i].tim->Instance->EGR |= TIM_EGR_UG;
	}
}
