#include "esp_system.h"
#include "esp_log.h"

#include <WeavePlatform.h>
#include "ServiceEcho.h"

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::WeavePlatform;

using ::nl::Weave::Profiles::Echo_Next::WeaveEchoClient;

extern const char * TAG;

ServiceEchoClient ServiceEcho;

static void EchoBindingEventHandler(void * apAppState, Binding::EventType aEvent, const Binding::InEventParam & aInParam, Binding::OutEventParam & aOutParam)
{
    Binding *binding = aInParam.Source;

#define kServiceEndpoint_CoreRouter             (0x18B4300200000012ull)     ///< Core router endpoint
#define kServiceEndpoint_ServiceProvisioning    (0x18B4300200000010ull)     ///< Service provisioning profile endpoint

    switch (aEvent)
    {
    case Binding::kEvent_PrepareRequested:
        aOutParam.PrepareRequested.PrepareError = binding->BeginConfiguration()
                .Target_ServiceEndpoint(kServiceEndpoint_ServiceProvisioning)
                .Transport_UDP_WRM()
                .Security_SharedCASESession()
//                .Security_None()
                .Exchange_ResponseTimeoutMsec(5000)
                .PrepareBinding();
        break;
    default:
        binding->DefaultEventHandler(apAppState, aEvent, aInParam, aOutParam);
    }
}

static void EchoClientEventHandler(void * appState, WeaveEchoClient::EventType eventType, const WeaveEchoClient::InEventParam & inParam, WeaveEchoClient::OutEventParam & outParam)
{
    switch (eventType)
    {
    case WeaveEchoClient::kEvent_PreparePayload:
        outParam.PreparePayload.Payload = PacketBuffer::New();
        outParam.PreparePayload.PrepareError = (outParam.PreparePayload.Payload != NULL) ? WEAVE_NO_ERROR : WEAVE_ERROR_NO_MEMORY;
        break;
    case WeaveEchoClient::kEvent_ResponseReceived:
        ESP_LOGI(TAG, "Echo response received from service");
        PacketBuffer::Free(inParam.ResponseReceived.Payload);
        break;
    case WeaveEchoClient::kEvent_ResponseTimeout:
        ESP_LOGI(TAG, "Timeout waiting for echo response from service");
        break;
    case WeaveEchoClient::kEvent_CommuncationError:
        ESP_LOGE(TAG, "Communication error sending echo request to service: %s", ErrorStr(inParam.CommuncationError.Reason));
        break;
    default:
        ServiceEcho.DefaultEventHandler(appState, eventType, inParam, outParam);
    }
}

void ServiceEchoClient::PlatformEventHandler(const WeavePlatformEvent * event, intptr_t arg)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    if (event->Type == WeavePlatformEvent::kEventType_ServiceConnectivityChange)
    {
        if (event->ServiceConnectivityChange.Result == kConnectivity_Established)
        {
            ESP_LOGI(TAG, "Starting periodic echos to service");
            err = ServiceEcho.SendRepeating(ServiceEcho.mIntervalMS);
            SuccessOrExit(err);
        }
        else if (event->ServiceConnectivityChange.Result == kConnectivity_Lost)
        {
            ESP_LOGI(TAG, "Stopping periodic echos to service");
            ServiceEcho.Stop();
        }
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "ServiceEchoClient::PlatformEventHandler() failed: %s", ErrorStr(err));
    }
}

WEAVE_ERROR ServiceEchoClient::Init(uint32_t intervalMS)
{
    WEAVE_ERROR err;
    Binding * binding;

    binding = ::WeavePlatform::ExchangeMgr.NewBinding(EchoBindingEventHandler, NULL);
    VerifyOrExit(binding != NULL, err = WEAVE_ERROR_NO_MEMORY);

    err = WeaveEchoClient::Init(binding, EchoClientEventHandler, NULL);
    SuccessOrExit(err);

    err = PlatformMgr.AddEventHandler(PlatformEventHandler);
    SuccessOrExit(err);

    mIntervalMS = intervalMS;

exit:
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "ServiceEchoClient::Init() failed: %s", ErrorStr(err));
    }
    return err;
}
