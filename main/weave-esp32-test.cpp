/* UDP MultiCast Send/Receive Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "arch/sys_arch.h"
#include <lwip/netdb.h>

#include <WeavePlatform-ESP32.h>
#include <InetLayer/InetLayer.h>
#include <InetLayer/IPAddress.h>
#include <Weave/Support/logging/WeaveLogging.h>
#include <Weave/Support/ErrorStr.h>
#include <InetLayer/UDPEndPoint.h>

using namespace nl;
using namespace nl::Inet;
using namespace nl::Weave;

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

static const char *TAG = "weave-esp32-test";

void HandleAliveTimer(System::Layer * aLayer, void * aAppState, System::Error aError)
{
    WEAVE_ERROR err;

    ESP_LOGI(TAG, "Alive");

    err = WeavePlatform::SystemLayer.StartTimer(15000, HandleAliveTimer, NULL);
    if (err != WEAVE_NO_ERROR) {
        ESP_LOGE(TAG, "Failed to start timer: %s", ErrorStr(err));
        return;
    }

    bool stationEnabled = ::WeavePlatform::ConnectivityMgr.GetWiFiStationEnabled();
    ::WeavePlatform::ConnectivityMgr.SetWiFiStationEnabled(!stationEnabled);
}

extern "C" void app_main()
{
    WEAVE_ERROR err;

    ESP_ERROR_CHECK( nvs_flash_init() );

    if (!WeavePlatform::InitLwIPCoreLock()) {
        return;
    }

    tcpip_adapter_init();

    ESP_ERROR_CHECK( esp_event_loop_init(WeavePlatform::HandleESPSystemEvent, NULL) );

    {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    }

    {
        wifi_config_t wifi_config;
        memset(&wifi_config, 0, sizeof(wifi_config));
        err = esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "esp_wifi_get_config return error: %d", err);
        }
        else
        {
            ESP_LOGI(TAG, "wifi_config.sta.ssid is '%s'", wifi_config.sta.ssid);
            ESP_LOGI(TAG, "wifi_config.sta.password is '%s'", wifi_config.sta.password);
        }
    }

    if (!WeavePlatform::InitWeaveStack())
    {
        return;
    }

    {
        wifi_config_t wifi_config;

        ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
        ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );

        memset(&wifi_config, 0, sizeof(wifi_config));
        memcpy(wifi_config.sta.ssid, EXAMPLE_WIFI_SSID, strlen(EXAMPLE_WIFI_SSID) + 1);
        wifi_config.sta.ssid[1] = 'F';
        memcpy(wifi_config.sta.password, EXAMPLE_WIFI_PASS, strlen(EXAMPLE_WIFI_PASS) + 1);
        wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );

        ESP_ERROR_CHECK( esp_wifi_set_auto_connect(true) );

        ESP_ERROR_CHECK( esp_wifi_start() );
    }

    err = WeavePlatform::SystemLayer.StartTimer(15000, HandleAliveTimer, NULL);
    if (err != WEAVE_NO_ERROR) {
        ESP_LOGE(TAG, "Failed to start timer: %s", ErrorStr(err));
        return;
    }

    ESP_LOGI(TAG, "Ready");

    WeavePlatform::SystemLayer.DispatchEvents();
}
