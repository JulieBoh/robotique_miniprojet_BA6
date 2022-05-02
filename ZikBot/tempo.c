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

void get_tempo(int16_t default_speed, messagebus_topic_t *proximity_topic)
{
//check IR
	proximity_msg_t prox_buf;
	enum prox_sensors_used{r = 2, r_bck, l_bck, l};
	//check msgbusj
 	if(messagebus_topic_read(proximity_topic, &prox_buf, sizeof(proximity_msg_t)))
	{
		// check each proximity sensor
		if(prox_buf.delta[r] > 50)
		{
			set_led(1,1);
			chprintf((BaseSequentialStream *)&SD3, "r > 50\r\n");

		}
		else
		{
			set_led(1,0);
		}
		if(prox_buf.delta[r_bck] > 50)
		{
			set_led(3,1);
			chprintf((BaseSequentialStream *)&SD3, "r_bck > 50\r\n");
		}
		else
		{
			set_led(3,0);
		}
		if((int)prox_buf.delta[l_bck] > 50)
		{
			set_led(5,1);
			chprintf((BaseSequentialStream *)&SD3, "l_bck > 50\r\n");
		}
		else
		{
			set_led(5,0);
		}
		if((int)prox_buf.delta[l] > 50)
		{
			set_led(7,1);
			chprintf((BaseSequentialStream *)&SD3, "l > 50\r\n");
		}
		else
		{
			set_led(7,0);
		}

/*		chprintf((BaseSequentialStream *)&SD3, "ambient \n\r "\
												"r = %u, r_bck = %u, l_bck = %u, l = %u \n\r", \
												prox_buf.ambient[r], prox_buf.ambient[r_bck], \
												prox_buf.ambient[l_bck], prox_buf.ambient[l]);
		chprintf((BaseSequentialStream *)&SD3, "reflected\n\r "\
												"r = %u, r_bck = %u, l_bck = %u, l = %u \n\r", \
												prox_buf.reflected[r], prox_buf.reflected[r_bck], \
												prox_buf.reflected[l_bck], prox_buf.reflected[l]);
		chprintf((BaseSequentialStream *)&SD3, "delta \n\r "\
												"r = %d, r_bck = %d, l_bck = %d, l = %d \n\r", \
												prox_buf.delta[r], prox_buf.delta[r_bck], \
												prox_buf.delta[l_bck], prox_buf.delta[l]);
*/
	}



//compute tempo -> 4 different values
//define acquisition rythm
//define motor speed

}
