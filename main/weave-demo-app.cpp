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
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_heap_caps_init.h"
#include <new>

#include <WeaveDevice.h>
#include <Weave/Support/ErrorStr.h>
#include "AliveTimer.h"
#include "ServiceEcho.h"

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::nl::Weave::Device;

const char *TAG = "demo-app";

extern "C" void app_main()
{
    WEAVE_ERROR err;

    ESP_ERROR_CHECK( nvs_flash_init() );

    err = ::nl::Weave::Device::PlatformMgr.InitLocks();
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr.InitLocks() failed: %s", ErrorStr(err));
        return;
    }

    tcpip_adapter_init();

    ESP_ERROR_CHECK( esp_event_loop_init(::nl::Weave::Device::PlatformManager::HandleESPSystemEvent, NULL) );

    {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

        ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_FLASH) );

        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );

        ESP_ERROR_CHECK( esp_wifi_start() );
    }

    err = ::nl::Weave::Device::PlatformMgr.InitWeaveStack();
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr.InitWeaveStack() failed: %s", ErrorStr(err));
        return;
    }

    ::nl::Weave::Device::ConnectivityMgr.SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_OnDemand_NoStationProvision);

    err = StartAliveTimer(5000);
    if (err != WEAVE_NO_ERROR)
    {
        return;
    }

    err = ServiceEcho.Init(10000);
    if (err != WEAVE_NO_ERROR)
    {
        return;
    }

    ESP_LOGI(TAG, "Ready");

    err = ::nl::Weave::Device::PlatformMgr.StartEventLoopTask();
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr.StartEventLoopTask() failed: %s", ErrorStr(err));
        return;
    }

    ESP_LOGI(TAG, "Main task exiting");
}
