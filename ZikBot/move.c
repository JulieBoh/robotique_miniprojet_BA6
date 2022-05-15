#include "ch.h"
#include "hal.h"
#include <msgbus/messagebus.h>
#include <sensors/proximity.h>
#include <motors.h>

#include <stdint.h>

#include "main.h"
#include "move.h"

#define DEFAULT_BASE_SPEED 400
#define BASE_SPEED_LIMIT 900 //for the purpose of this project
#define BASE_SPEED_INCREMENT 50
#define IR_THRESHOLD 250
#define IR3 2
#define IR6 5


static int16_t base_speed = DEFAULT_BASE_SPEED;
static messagebus_topic_t *proximity_topic = NULL;

void move_init(void){
	proximity_topic = messagebus_find_topic_blocking(&bus, "/proximity");
}

void get_tempo(messagebus_topic_t *proximity_topic)
{
	proximity_msg_t prox_buf;
	//check msgbus
 	if(messagebus_topic_read(proximity_topic, &prox_buf, sizeof(proximity_msg_t)))
	{
		// check each proximity sensor
		if(prox_buf.delta[IR3] > IR_THRESHOLD)
			base_speed += BASE_SPEED_INCREMENT;
		else if(prox_buf.delta[IR6] > IR_THRESHOLD)
			base_speed -= BASE_SPEED_INCREMENT;
	}
}

void move(int16_t speed_corr){
	if(proximity_topic == NULL)
		return;
	get_tempo(proximity_topic);
	if(base_speed > BASE_SPEED_LIMIT)
		base_speed = BASE_SPEED_LIMIT;
	if(base_speed < -BASE_SPEED_LIMIT)
		base_speed = -BASE_SPEED_LIMIT;

	right_motor_set_speed(base_speed-speed_corr);
	left_motor_set_speed( base_speed+speed_corr);
}
