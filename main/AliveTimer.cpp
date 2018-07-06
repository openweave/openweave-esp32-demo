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
#include "esp_heap_caps_init.h"
#include <new>

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <Weave/Support/ErrorStr.h>

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::nl::Weave::DeviceLayer;

extern const char * TAG;

uint32_t AliveIntervalMS;

void HandleAliveTimer(System::Layer * /* unused */, void * /* unused */, System::Error /* unused */)
{
    WEAVE_ERROR err;

    ESP_LOGI(TAG, "Alive");

    // heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

    err = SystemLayer.StartTimer(AliveIntervalMS, HandleAliveTimer, NULL);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "SystemLayer.StartTimer() failed: %s", ErrorStr(err));
        return;
    }
}

WEAVE_ERROR StartAliveTimer(uint32_t intervalMS)
{
    AliveIntervalMS = intervalMS;
    HandleAliveTimer(&SystemLayer, NULL, WEAVE_SYSTEM_CONFIG_NO_ERROR);
    return WEAVE_NO_ERROR;
}
