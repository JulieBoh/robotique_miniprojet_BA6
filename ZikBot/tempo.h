#ifndef TEMPO_H
#define TEMPO_H
#include <stdint.h>
#include <msgbus/messagebus.h>
//starts the tempo thread
void tempo_start(void);

//reads IR sensors and
void get_tempo(int16_t* default_speed, messagebus_topic_t *proximity_topic);
#endif /* TEMPO_H */
