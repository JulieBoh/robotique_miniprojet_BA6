#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <tempo.h>
#include <stdint.h>
#include <chprintf.h>
#include <msgbus/messagebus.h>
#include <sensors/proximity.h>
#include <leds.h>

#define PROX_RIGHT	 3 //unused
#define PROX_LEFT	 6 //unused
//#define PROX_NB_USED 4
#define IR_THRESHOLD 250

void get_tempo(int16_t default_speed, messagebus_topic_t *proximity_topic)
{
//check IR
	proximity_msg_t prox_buf;
	enum prox_sensors_used{r = 2, r_bck, l_bck, l};
	//check msgbus
 	if(messagebus_topic_read(proximity_topic, &prox_buf, sizeof(proximity_msg_t)))
	{
		// check each proximity sensor
		if(prox_buf.delta[r] > IR_THRESHOLD)
		{
		}
		else
		{
		}
		if(prox_buf.delta[r_bck] > IR_THRESHOLD)
		{
		}
		else
		{
		}
		if(prox_buf.delta[l_bck] > IR_THRESHOLD)
		{
		}
		else
		{
		}
		if(prox_buf.delta[l] > IR_THRESHOLD)
		{
		}
		else
		{
		}
	}



//compute tempo -> 4 different values
//define acquisition rythm
//define motor speed

}
