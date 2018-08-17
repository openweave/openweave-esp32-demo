/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "esp_system.h"
#include "esp_log.h"

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include "ServiceEcho.h"

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::nl::Weave::DeviceLayer;

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
        ServiceEcho.ServiceAlive = true;
        break;
    case WeaveEchoClient::kEvent_ResponseTimeout:
        ESP_LOGI(TAG, "Timeout waiting for echo response from service");
        ServiceEcho.ServiceAlive = false;
        break;
    case WeaveEchoClient::kEvent_CommuncationError:
        ESP_LOGE(TAG, "Communication error sending echo request to service: %s", ErrorStr(inParam.CommuncationError.Reason));
        ServiceEcho.ServiceAlive = false;
        break;
    default:
        ServiceEcho.DefaultEventHandler(appState, eventType, inParam, outParam);
    }
}

void ServiceEchoClient::PlatformEventHandler(const WeaveDeviceEvent * event, intptr_t arg)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    if (event->Type == DeviceEventType::kServiceTunnelStateChange)
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
            ServiceEcho.ServiceAlive = false;
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

    binding = ::nl::Weave::DeviceLayer::ExchangeMgr.NewBinding(EchoBindingEventHandler, NULL);
    VerifyOrExit(binding != NULL, err = WEAVE_ERROR_NO_MEMORY);

    err = WeaveEchoClient::Init(binding, EchoClientEventHandler, NULL);
    SuccessOrExit(err);

    err = PlatformMgr().AddEventHandler(PlatformEventHandler);
    SuccessOrExit(err);

    ServiceAlive = false;
    mIntervalMS = intervalMS;

exit:
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "ServiceEchoClient::Init() failed: %s", ErrorStr(err));
    }
    return err;
}
