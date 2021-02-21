/*
 * FreeRTOS V202012.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdio.h>
#include <string.h>

#include <psa/client.h>
#include <tfm_ns_interface.h>

#include <firmware_update.h>

static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

#define mainQUEUE_RECEIVE_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define mainQUEUE_LENGTH                    ( 1 )
#define mainQUEUE_SEND_FREQUENCY_MS         ( 200 / portTICK_PERIOD_MS )
/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

void main_blinky( void )
{
    /* Create the queue. */
    xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

    if( xQueue != NULL )
    {
        /* Start the two tasks as described in the comments at the top of this
        file. */
        xTaskCreate( prvQueueReceiveTask,            /* The function that implements the task. */
                    "Rx",                            /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                    configMINIMAL_STACK_SIZE,        /* The size of the stack to allocate to the task. */
                    NULL,                            /* The parameter passed to the task - not used in this case. */
                    mainQUEUE_RECEIVE_TASK_PRIORITY, /* The priority assigned to the task. */
                    NULL );                          /* The task handle is not required, so NULL is passed. */

        xTaskCreate( prvQueueSendTask,
                    "TX",
                    configMINIMAL_STACK_SIZE,
                    NULL,
                    mainQUEUE_SEND_TASK_PRIORITY,
                    NULL );

        /* Start the tasks and timer running. */
        vTaskStartScheduler();
    }

    /* If all is well, the scheduler will now be running, and the following
    line will never be reached.  If the following line does execute, then
    there was insufficient FreeRTOS heap memory available for the Idle and/or
    timer tasks to be created.  See the memory management section on the
    FreeRTOS web site for more details on the FreeRTOS heap
    http://www.freertos.org/a00111.html. */
    for( ;; );
}

static void prvQueueSendTask( void *pvParameters )
{
TickType_t xNextWakeTime;
const uint32_t ulValueToSend = 100UL;

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for( ;; )
    {
        /* Place this task in the blocked state until it is time to run again. */
        vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

        /* Send to the queue - causing the queue receive task to unblock and
        toggle the LED.  0 is used as the block time so the sending operation
        will not block - it shouldn't need to block as the queue should always
        be empty at this point in the code. */
        xQueueSend( xQueue, &ulValueToSend, 0U );
    }
}

int _read(int file, char *buf, int len);
int _write(int file, char *buf, int len);

#define LOG_OUT(string) _write(0, string, strlen(string))

//#define TFM_NS_INSTALL 1

#ifdef TFM_S_INSTALL
void ota_update_tfm_s(void)
{
    tfm_image_info_t info = { 0 };
    tfm_image_id_t image_id = 0;

    image_id = FWU_IMAGE_ID_SLOT_1 | (FWU_IMAGE_TYPE_SECURE << FWU_IMAGE_ID_TYPE_POSITION);
    int status, bytes_read = 0;
    char *ota_image = pvPortMalloc(1024);
    if (ota_image == NULL) {
	LOG_OUT("Failed to allocate buffer for FOTA image\n");
	tfm_fwu_request_reboot();
    }

    psa_status_t uxStatus;
    while (bytes_read < 0x80000) {
	    LOG_OUT("Start reading image from UART TFM-S...\n");
	    status = _read(1, ota_image, 1024);
	    if (status == -1) {
		LOG_OUT("Failed to read OTA image from UART!\n");
		tfm_fwu_request_reboot();
	    }

	    LOG_OUT("Triggering buffer write of the image from the flash to SLOT 1...\n");
	    uxStatus = tfm_fwu_write(image_id, bytes_read, (const void *)ota_image, 1024); // starting from 4Mb of the file injected, size of image 1 MB SE + NS TFM
	    if( uxStatus != PSA_SUCCESS )
	    {
		LOG_OUT("tfm_fwu_write failed with error\n");
		tfm_fwu_request_reboot();
	    } else {
		LOG_OUT("Success on writing buffer to SLOT 1\n");
	    }
	    bytes_read+=1024;
    }
    LOG_OUT("Success on writing TFM-S image to SLOT 1\n");

    LOG_OUT("Triggering install of TFM-S image from the flash to SLOT 1...\n");
    tfm_image_id_t dependency_uuid;
    tfm_image_version_t dependency_version;

    uxStatus = tfm_fwu_install(image_id, &dependency_uuid, &dependency_version);
    if( uxStatus != PSA_SUCCESS )
    {
        LOG_OUT("tfm_fwu_install failed with error\n");
	if (uxStatus > 0) {
		 if (uxStatus == TFM_SUCCESS_DEPENDENCY_NEEDED) 
			 LOG_OUT("tfm_fwu_install status > 0 Dependency\n");
		else LOG_OUT("tfm_fwu_install status > 0 Reboot\n");
	} else {
		 LOG_OUT("tfm_fwu_install status < 0\n");
		tfm_fwu_request_reboot();
	}
    } else {
        LOG_OUT("Success on installing image to SLOT 1");
    }
}
#endif

#ifdef TFM_NS_INSTALL
void ota_update_tfm_ns(void)
{
    tfm_image_info_t info = { 0 };
    tfm_image_id_t image_id = 0;

    image_id = FWU_IMAGE_ID_SLOT_1 | (FWU_IMAGE_TYPE_NONSECURE << FWU_IMAGE_ID_TYPE_POSITION);
    LOG_OUT("Write of the image from the flash to SLOT 1 was triggered...\n");
    int status, bytes_read = 0;
    char *ota_image = pvPortMalloc(1024);
    if (ota_image == NULL) {
	LOG_OUT("Failed to allocate buffer for FOTA image\n");
	tfm_fwu_request_reboot();
    }

    psa_status_t uxStatus;
    while (bytes_read < 0x80000) {
	    //LOG_OUT("Start reading image from UART TFM-S...\n");
	    status = _read(1, ota_image, 1024);
	    if (status == -1) {
		LOG_OUT("Failed to read OTA image from UART!\n");
		tfm_fwu_request_reboot();
	    }

	    //LOG_OUT("Triggering buffer write of the image from the flash to SLOT 1...\n");
	    uxStatus = tfm_fwu_write(image_id, bytes_read, (const void *)ota_image, 1024); // starting from 4Mb of the file injected, size of image 1 MB SE + NS TFM
	    if( uxStatus != PSA_SUCCESS )
	    {
		LOG_OUT("tfm_fwu_write failed with error\n");
		tfm_fwu_request_reboot();
	    } else {
		//LOG_OUT("Success on writing buffer to SLOT 1\m");
	    }
	    bytes_read+=1024;
    }
    LOG_OUT("Success on writing TFM-NS image to SLOT 1\n");

    //LOG_OUT("Triggering install of TFM-S image from the flash to SLOT 1...\n");
    tfm_image_id_t dependency_uuid;
    tfm_image_version_t dependency_version;

    uxStatus = tfm_fwu_install(image_id, &dependency_uuid, &dependency_version);
    if( uxStatus != PSA_SUCCESS )
    {
        LOG_OUT("tfm_fwu_install failed with error\n");
	if (uxStatus > 0) {
		 if (uxStatus == TFM_SUCCESS_DEPENDENCY_NEEDED) 
			 LOG_OUT("tfm_fwu_install status > 0 Dependency\n");
		else LOG_OUT("tfm_fwu_install status > 0 Reboot\n");
	} else {
		 LOG_OUT("tfm_fwu_install status < 0\n");
		tfm_fwu_request_reboot();
	}
    } else {
        LOG_OUT("Success on installing image to SLOT 1");
    }
}
#endif


volatile uint32_t ulRxEvents = 0;
static void prvQueueReceiveTask( void *pvParameters )
{
uint32_t ulReceivedValue;
const uint32_t ulExpectedValue = 100UL;

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    tfm_ns_interface_init(); 

    LOG_OUT("Starting FreeRTOS in NS Mode...\n");
    uint32_t version = psa_framework_version();
    if (version == PSA_FRAMEWORK_VERSION) {
    } else {
        LOG_OUT("The version of the PSA Framework API is not valid!\n");
    }

#ifdef TFM_S_INSTALL
	LOG_OUT("Updating TFM-S FW...\n");
	ota_update_tfm_s();
#elif defined(TFM_NS_INSTALL)
	LOG_OUT("Updating TFM-NS FW...\n");
	ota_update_tfm_ns();
#else
	LOG_OUT("OTA Update was successfull. Loaded new NS image!\n");
#endif

    LOG_OUT("Rebooting after install ...\n");
    tfm_fwu_request_reboot();

    for( ;; )
    {
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h. */
        xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

        /*  To get here something must have been received from the queue, but
        is it the expected value?  If it is, toggle the LED. */
        if( ulReceivedValue == ulExpectedValue )
        {
            LOG_OUT("blinking");
            vTaskDelay(1000);
            ulReceivedValue = 0U;
            ulRxEvents++;
        }
    }
}
/*-----------------------------------------------------------*/
