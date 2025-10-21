#include "core_mqtt.h"
#include "logging_levels.h"

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_DEBUG
#endif

#include "logging_stack.h"

static uint8_t buffer[1024];

static MQTTStatus_t transportSend(NetworkContext_t * pNetworkContext,
                                  const void * pBuffer,
                                  size_t bytesToSend)
{
    (void)pNetworkContext;
    (void)pBuffer;
    (void)bytesToSend;
    
    LogInfo(("Transport send called with %zu bytes", bytesToSend));
    return MQTTSuccess;
}

static MQTTStatus_t transportRecv(NetworkContext_t * pNetworkContext,
                                  void * pBuffer,
                                  size_t bytesToRecv)
{
    (void)pNetworkContext;
    (void)pBuffer;
    (void)bytesToRecv;
    
    LogInfo(("Transport recv called for %zu bytes", bytesToRecv));
    return MQTTNoDataAvailable;
}

int main(void)
{
    MQTTContext_t mqttContext;
    TransportInterface_t transport;
    MQTTFixedBuffer_t networkBuffer;
    MQTTStatus_t mqttStatus;

    vLoggingInit();

    LogInfo(("Starting coreMQTT demo..."));

    // Setup transport interface
    transport.pNetworkContext = NULL;
    transport.send = transportSend;
    transport.recv = transportRecv;

    // Setup network buffer
    networkBuffer.pBuffer = buffer;
    networkBuffer.size = sizeof(buffer);

    // Initialize MQTT context
    mqttStatus = MQTT_Init(&mqttContext, &transport, NULL, NULL, &networkBuffer);

    if (mqttStatus == MQTTSuccess)
    {
        LogInfo(("MQTT context initialized successfully"));
    }
    else
    {
        LogInfo(("Failed to initialize MQTT context: %d", mqttStatus));
    }

    return 0;
}
