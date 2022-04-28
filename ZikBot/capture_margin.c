#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <capture_margin.h>


//semaphore
static BSEMAPHORE_DECL(margin_ready_sem, TRUE);

static THD_WORKING_AREA(waCaptureMargin, 256);
static THD_FUNCTION(CaptureMargin, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&margin_ready_sem);
    }
}

void capture_margin_start(void){
	chThdCreateStatic(waCaptureMargin, sizeof(waCaptureMargin), NORMALPRIO, CaptureMargin, NULL);
}
