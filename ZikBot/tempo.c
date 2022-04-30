#include <tempo.h>
#include <stdint.h>
#include <msgbus/messagebus.h>
#include <sensors/proximity.h>
#include <leds.h>

#define PROX_RIGHT	 3 //unused
#define PROX_LEFT	 6 //unused
//#define PROX_NB_USED 4

void get_tempo(int16_t default_speed, messagebus_topic_t *proximity_topic)
{
//check IR
	proximity_msg_t prox_buf;
	enum prox_sensors_used{r = 3, r_bck = 4,l_bck = 5, l = 6,PROX_NB_USED};
	//check msgbus
	if(messagebus_topic_read(proximity_topic, &prox_buf, sizeof(proximity_msg_t)))
	{
		// check each proximity sensor
		if(prox_buf->delta[3] > 10)
			set_led(1,1);
		if(prox_buf->delta[4] > 10)
			set_led(3,1);
		if(prox_buf->delta[5] > 10)
			set_led(5,1);
		if(prox_buf->delta[6] > 10)
			set_led(7,1);
	}


//compute tempo -> 4 different values
//define acquisition rythm
//define motor speed

}
