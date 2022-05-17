#ifndef protocol_h
#define protocol_h

#include <inttypes.h>
#include <string.h>
#include "config.h"

#define HEADER_SIZE 		(2+2) //0xFFFA+length+command
#define DECODE_BUFFER_SIZE 	256
#define MAX_PACKET_LENGTH 	(1+4*8) //in bytes

#define packet_size(packet) (packet->length + HEADER_SIZE)*sizeof(uint8_t)
#define decoder_increment(decoder, bytes_received) (decoder)->buffer_length += bytes_received

typedef enum{
	RET_OK,
	RET_ERROR,
}retval_t;

enum commands {SET_MOTOR, SET_FREQ, SET_DUTY, SET_DELAY, ACTIVATE_MOTORS, STOP_MOTORS};

typedef struct {
	uint8_t length;
	uint8_t command;
	uint8_t data[MAX_PACKET_LENGTH];
} packet_t;

typedef struct {
	uint8_t* head;
	uint8_t* tail;
	uint8_t buffer[DECODE_BUFFER_SIZE];
	uint8_t input_msg_buf[3 * MAX_PACKET_LENGTH];
	uint16_t input_msg_byte_count;
	uint16_t input_msg_str_current_count;
	uint16_t buffer_length;
	UART_HandleTypeDef* huart;
} decoder_t;

extern uint32_t num_motors;

void decoder_initialise(decoder_t *decoder, UART_HandleTypeDef *huart);
retval_t get_new_data(decoder_t *decoder);
retval_t check_new_msg(decoder_t *decoder, packet_t *packet);
void process_packet(packet_t *packet, motor_config_t *motors);
float buf_to_float(uint8_t *buf);

#endif
