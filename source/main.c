#include "logging_levels.h"

#define LIBRARY_LOG_LEVEL    LOG_DEBUG

#include "logging_stack.h"

#include "core_mqtt.h"
#include "transport.h"
#include "clock.h"

#include <strings.h>

static uint8_t ucMQTTFixedBuffer[ 1024 ];

void userCallback( struct MQTTContext * pContext,
                   struct MQTTPacketInfo * pPacketInfo,
                   struct MQTTDeserializedInfo * pDeserializedInfo )
{
}

int main( void )
{
    MQTTContext_t xMQTTContext = { 0 };
    MQTTFixedBuffer_t xMQTTFixedBuffer = { 0 };
    MQTTConnectInfo_t xMQTTConnectInfo = { 0 };
    MQTTStatus_t xMQTTStatus;
    bool xSessionPresent = false;
    NetworkContext_t xNetworkContext = { 0 };
    TransportInterface_t xTransportInterface = { 0 };
    TransportCredentials_t xTransportCredentials = { 0 };
    TransportStatus_t xTransportStatus;
    ServerInfo_t xServerInfo = { 0 };

    /* Setup the MQTT connections buffer. */
    xMQTTFixedBuffer.pBuffer = ucMQTTFixedBuffer;
    xMQTTFixedBuffer.size = sizeof( ucMQTTFixedBuffer );

    /* Setup the MQTT connect info. */
    xMQTTConnectInfo.cleanSession = true;
    xMQTTConnectInfo.pClientIdentifier = THING_NAME;
    xMQTTConnectInfo.clientIdentifierLength = strlen( THING_NAME );

    LogInfo( ( THING_NAME ) );
    xMQTTConnectInfo.keepAliveSeconds = 30;

    /* Setup transport interface. */
    xTransportInterface.pNetworkContext = &xNetworkContext;
    xTransportInterface.recv = Transport_Recv;
    xTransportInterface.send = Transport_Send;
    xTransportInterface.writev = NULL;

    /* Setup connection credentials. */
    xTransportCredentials.pClientCert = CLIENT_CERT;
    xTransportCredentials.pPrivateKey = PRIVATE_KEY;
    xTransportCredentials.pRootCa = ROOT_CA;
    xTransportCredentials.sniHostName = MQTT_BROKER_ENDPOINT;

    /* Setup server info. */
    xServerInfo.pHostName = MQTT_BROKER_ENDPOINT;
    xServerInfo.hostNameLength = strlen( MQTT_BROKER_ENDPOINT );
    xServerInfo.port = 8883;

    vLoggingInit();

    LogInfo( ( "Starting coreMQTT demo..." ) );

    /* Initialize MQTT context */
    xMQTTStatus = MQTT_Init( &xMQTTContext, &xTransportInterface, Clock_GetTimeMs, userCallback, &xMQTTFixedBuffer );

    if( xMQTTStatus == MQTTSuccess )
    {
        LogInfo( ( "MQTT context initialized successfully" ) );
    }
    else
    {
        LogInfo( ( "Failed to initialize MQTT context: %d", xMQTTStatus ) );
    }

    xTransportStatus = Transport_Connect( &xNetworkContext, &xServerInfo, &xTransportCredentials, 1000, 1000 );

    if( xTransportStatus == TRANSPORT_SUCCESS )
    {
        LogInfo( ( "Connection established." ) );
    }
    else
    {
        LogInfo( ( "Failed to establish connection." ) );
    }

    do
    {
        xMQTTStatus = MQTT_Connect( &xMQTTContext, &xMQTTConnectInfo, NULL, 10000, &xSessionPresent );

        if( xMQTTStatus == MQTTSuccess )
        {
            LogInfo( ( "MQTT connection established." ) );
        }
        else
        {
            LogInfo( ( "Failed to connect: %d", xMQTTStatus ) );
        }
    } while( xMQTTStatus != MQTTSuccess );

    char pcSendBuffer[ 1000 ];
    snprintf( pcSendBuffer, 1000,
              "["
              "{"
              "\"label\" : \"Random\","
              "\"display_type\" : \"line_graph\","
              "\"values\" :"
              "["
              "{"
              "\"unit\" : \"Number\","
              "\"value\" : %d,"
              "\"label\" : \"\""
              "}"
              "]"
              "}"
              "]", 2 );
    MQTTPublishInfo_t xPubInfo = { 0 };

    xPubInfo.qos = MQTTQoS0;
    xPubInfo.pTopicName = THING_NAME;
    xPubInfo.topicNameLength = strlen( THING_NAME );
    xPubInfo.pPayload = pcSendBuffer;
    xPubInfo.payloadLength = strlen( pcSendBuffer );

    while( 1 )
    {
        xMQTTStatus = MQTT_Publish( &xMQTTContext, &xPubInfo, MQTT_GetPacketId( &xMQTTContext ) );

        if( xMQTTStatus == MQTTSuccess )
        {
            LogInfo( ( "MQTT publish successful." ) );
        }
        else
        {
            LogInfo( ( "Failed to connect: %d", xMQTTStatus ) );
        }

        Clock_SleepMs( 1000 );
    }

    MQTT_Disconnect( &xMQTTContext );
    Transport_Disconnect( &xNetworkContext );
}


/* TO REMOVE */
#include "FreeRTOS.h"
#include "task.h"

void vAssertCalled( const char * const pcFileName,
                    unsigned long ulLine )
{
    volatile unsigned long ulSetToNonZeroInDebuggerToContinue = 0;

    LogError( ( "Assert at line %lu of file %s\r\n", ulLine, pcFileName ) );

    taskENTER_CRITICAL();
    {
        /* You can step out of this function to debug the assertion by using
         * the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
         * value. */
        while( ulSetToNonZeroInDebuggerToContinue == 0 )
        {
        }
    }
    taskEXIT_CRITICAL();
}

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * pulIdleTaskStackSize )
{
    /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationTickHook( void )
{
    /* This function will be called by each tick interrupt if
    * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    * added here, but the tick hook is called from an interrupt context, so
    * code must not attempt to block, and only the interrupt safe FreeRTOS API
    * functions can be used (those that end in FromISR()). */

    /*LogInfo( ( "In vApplicationTickHook." ) ); */
}

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
     * task.  It is essential that code added to this hook function never attempts
     * to block in any way (for example, call xQueueReceive() with a block time
     * specified, or call vTaskDelay()).  If application tasks make use of the
     * vTaskDelete() API function to delete themselves then it is also important
     * that vApplicationIdleHook() is permitted to return to its calling function,
     * because it is the responsibility of the idle task to clean up memory
     * allocated by the kernel to any task that has since deleted itself. */


    /*LogInfo( ( "In vApplicationIdleHook" ) ); */
}

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     configSTACK_DEPTH_TYPE * pulTimerTaskStackSize )
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationDaemonTaskStartupHook( void )
{
    /* This function will be called once only, when the daemon task starts to
     * execute (sometimes called the timer task).  This is useful if the
     * application includes initialisation code that would benefit from executing
     * after the scheduler has been started. */

    /*LogInfo( ( "In vApplicationDaemonStartupHook" ) ); */
}

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
     * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
     * function that will get called if a call to pvPortMalloc() fails.
     * pvPortMalloc() is called internally by the kernel whenever a task, queue,
     * timer or semaphore is created.  It is also called by various parts of the
     * demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
     * size of the    heap available to pvPortMalloc() is defined by
     * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
     * API function can be used to query the size of free heap space that remains
     * (although it does not provide information on how the remaining heap might be
     * fragmented).  See http://www.freertos.org/a00111.html for more
     * information. */
    vAssertCalled( __FILE__, __LINE__ );
}
