#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
#include <msgbus/messagebus.h>

extern messagebus_t bus;

//starts the sound thread
void sound_start(void);

void sound_test(int16_t* default_speed);



#endif /* TEMPO_H */
