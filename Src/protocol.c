#include "protocol.h"

void decoder_initialise(decoder_t *decoder, UART_HandleTypeDef *huart){
	decoder->buffer_length = 0;
	decoder->huart = huart;
	decoder->head = decoder->buffer;
	decoder->tail = decoder->buffer;
	decoder->input_msg_byte_count = 0;
	decoder->input_msg_str_current_count = 0;
	memset(decoder->buffer, 0, DECODE_BUFFER_SIZE);
	memset(decoder->input_msg_buf, 0, 3 * MAX_PACKET_LENGTH);
}

//uint8_t packet_decode(decoder_t *decoder, packet_t *packet) {
//	uint16_t decode_iterator = 0;
//	uint16_t length, header;
//	uint8_t bool_status = 0;
//
//	while (decode_iterator + HEADER_SIZE <= decoder->buffer_length) {
//		header = decoder->buffer[decode_iterator++] << 8;
//		header |= decoder->buffer[decode_iterator++];
//		if (header == 0xFFFA) {
//			length = decoder->buffer[decode_iterator++];
//			length |= decoder->buffer[decode_iterator++] << 8;
//
//			if (decode_iterator + length > decoder->buffer_length) {
//				decode_iterator -= HEADER_SIZE;
//				break;
//			}
//			packet->length = length;
//			packet->command = decoder->buffer[decode_iterator++];
//			memcpy(packet->data, &decoder->buffer[decode_iterator], length * sizeof(uint8_t));
//			decode_iterator += length;
//			bool_status = 1;
//			break;
//		} else {
//			decode_iterator--;
//		}
//	}
//	if (decode_iterator < decoder->buffer_length) {
//		if (decode_iterator > 0) {
//			memmove(&decoder->buffer[0], &decoder->buffer[decode_iterator], (decoder->buffer_length - decode_iterator) * sizeof(uint8_t));
//			decoder->buffer_length -= decode_iterator;
//		}
//	} else {
//		decoder->buffer_length = 0;
//	}
//	return bool_status;
//}

retval_t get_new_data(decoder_t *decoder) {
	decoder->head = decoder->buffer + DECODE_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(decoder->huart->hdmarx);
	if (decoder->head > decoder->tail) {
		uint32_t new_data = decoder->head - decoder->tail;
		if ((decoder->input_msg_byte_count + new_data) > DECODE_BUFFER_SIZE) {
			decoder->input_msg_byte_count = 0;
			return RET_ERROR;	//COMMAND BUFFER OVERFLOW
		}
		for (int i = 0; i < new_data; i++) {
			decoder->input_msg_buf[i + decoder->input_msg_byte_count] = decoder->tail[i];
		}
		decoder->input_msg_byte_count += new_data;
		decoder->tail = decoder->head;
	} else if (decoder->head < decoder->tail) {
		uint32_t diff_end = decoder->buffer + DECODE_BUFFER_SIZE - decoder->tail;
		uint32_t diff_start = decoder->head - decoder->buffer;
		uint32_t new_data = diff_end + diff_start;

		if ((decoder->input_msg_byte_count + new_data) > DECODE_BUFFER_SIZE) {
			decoder->input_msg_byte_count = 0;
			decoder->input_msg_str_current_count = 0;
			return RET_ERROR;	//COMMAND BUFFER OVERFLOW
		}
		for (int i = 0; i < new_data; i++) {
			if (i < diff_end) {
				decoder->input_msg_buf[i + decoder->input_msg_byte_count] = decoder->tail[i];
			} else {
				decoder->input_msg_buf[i + decoder->input_msg_byte_count] = decoder->buffer[i - diff_end];
			}
		}
		decoder->input_msg_byte_count += new_data;
		decoder->tail = decoder->head;
	}
	return RET_OK;
}

retval_t check_new_msg(decoder_t *decoder, packet_t *packet) {
	for (int i = decoder->input_msg_str_current_count; i < decoder->input_msg_byte_count - HEADER_SIZE + 1; i++) {
		if ((decoder->input_msg_buf[i] == 0xFF) && (decoder->input_msg_buf[i + 1] == 0xFA)) {
			uint8_t length = decoder->input_msg_buf[i+2];
			uint16_t msg_length = HEADER_SIZE + length;
			if (i + msg_length > decoder->input_msg_byte_count) {
				break;
			}
			packet->length = length;
			packet->command = decoder->input_msg_buf[i+3];
			memcpy(packet->data, &decoder->input_msg_buf[i+4], length * sizeof(uint8_t));

			uint16_t extra_bytes = decoder->input_msg_byte_count - i - msg_length; //number of bytes after message
			for (int j = 0; j < extra_bytes; j++) {	//Moves everything (if exists) after message detected to the beginning
				decoder->input_msg_buf[j] = decoder->input_msg_buf[i + j + msg_length];
			}
			decoder->input_msg_str_current_count = 0;
			decoder->input_msg_byte_count = extra_bytes;
			return RET_OK;
		} else {
			decoder->input_msg_str_current_count += HEADER_SIZE;
		}
	}
	return ERROR;
}

void process_packet(packet_t *packet, motor_config_t *motors){
	enum commands cmd = packet->command;
	switch(cmd){
	case SET_MOTOR:{
		uint8_t n_motor = packet->data[0];
		if(n_motor > num_motors){
			return;
		}
		float freq = buf_to_float(&packet->data[1]);
		float duty = buf_to_float(&packet->data[1+2*4]);
		float delay = buf_to_float(&packet->data[1+3*4]);
		motor_set_freq(&motors[n_motor], freq);
		motor_set_duty(&motors[n_motor], duty);
		motor_set_delay_us(&motors[n_motor], delay);
		break;
	}
	case SET_FREQ:{
		uint8_t n_motor = packet->data[0];
		if(n_motor > num_motors){
			return;
		}
		float freq = buf_to_float(&packet->data[1]);
		motor_set_freq(&motors[n_motor], freq);
		break;
	}
	case SET_DUTY:{
		uint8_t n_motor = packet->data[0];
		if(n_motor > num_motors){
			return;
		}
		float duty = buf_to_float(&packet->data[1]);
		motor_set_duty(&motors[n_motor], duty);
		break;
	}
	case SET_DELAY:{
		uint8_t n_motor = packet->data[0];
		if(n_motor > num_motors){
			return;
		}
		float delay = buf_to_float(&packet->data[1]);
		motor_set_delay_us(&motors[n_motor], delay);
		break;
	}
	case ACTIVATE_MOTORS:
		start_motors(motors, 6);
		break;
	case STOP_MOTORS:
		stop_motors(motors, 6);
		break;
	}
}

float buf_to_float(uint8_t *buf){
	float t;
	memcpy(&t, buf, sizeof(float));
	return t;
}
