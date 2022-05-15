#ifndef TEMPO_H
#define TEMPO_H
#include <stdint.h>
#include <msgbus/messagebus.h>

void move_init(void);
void move(int16_t speed_corr);

//reads IR sensors and change default speed accordingly
#endif /* TEMPO_H */
