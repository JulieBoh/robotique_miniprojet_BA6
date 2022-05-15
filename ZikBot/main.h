#ifndef MAIN_H
#define MAIN_H

#include "msgbus/messagebus.h"
#include "parameter/parameter.h"
#include <camera/dcmi_camera.h>

//constants for the differents parts of the project
#define IMAGE_BUFFER_SIZE		640

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

#endif
