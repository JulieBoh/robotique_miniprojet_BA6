#include <tempo.h>
#include <stdint.h>
#include <msgbus/messagebus.h>
#include <sensors/proximity.h>


void get_tempo(int16_t default_speed, messagebus_topic_t *proximity_topic)
{
//check IR
	proximity_msg_t prox_buf;
	//check msgbus
	if(messagebus_topic_read(proximity_topic, prox_buf, sizeof(proximity_msg_t)));
	{
		set_body_led(1);
	}


//compute tempo -> 4 different values
//define acquisition rythm
//define motor speed

}
