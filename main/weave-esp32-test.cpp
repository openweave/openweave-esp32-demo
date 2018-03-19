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

#define UDP_PORT CONFIG_EXAMPLE_PORT

#define LISTEN_DEFAULT_IF CONFIG_EXAMPLE_LISTEN_DEFAULT_IF

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   we use two - one for IPv4 "got ip", and
   one for IPv6 "got ip". */
const int IPV4_GOTIP_BIT = BIT0;
const int IPV6_GOTIP_BIT = BIT1;

static const char *TAG = "weave-esp32-test";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        /* enable ipv6 */
        tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
        xEventGroupClearBits(wifi_event_group, IPV6_GOTIP_BIT);
        break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
        xEventGroupSetBits(wifi_event_group, IPV6_GOTIP_BIT);
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    wifi_config_t wifi_config;
    memcpy(wifi_config.sta.ssid, EXAMPLE_WIFI_SSID, strlen(EXAMPLE_WIFI_SSID) + 1);
    memcpy(wifi_config.sta.password, EXAMPLE_WIFI_PASS, strlen(EXAMPLE_WIFI_PASS) + 1);

    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void HandleAliveTimer(System::Layer * aLayer, void * aAppState, System::Error aError)
{
    WEAVE_ERROR err;

    ESP_LOGI(TAG, "Alive");

    err = WeavePlatform::SystemLayer.StartTimer(5000, HandleAliveTimer, NULL);
    if (err != WEAVE_NO_ERROR) {
        ESP_LOGE(TAG, "Failed to start timer: %s", ErrorStr(err));
        return;
    }
}

extern "C" void app_main()
{
    WEAVE_ERROR err;

    ESP_ERROR_CHECK( nvs_flash_init() );

    if (!WeavePlatform::InitLwIPCoreLock()) {
        return;
    }

    initialise_wifi();

    if (!WeavePlatform::InitWeaveStack()) {
        return;
    }

    /* Wait for all the IPs we care about to be set
    */
    uint32_t bits = 0;
    bits |= IPV4_GOTIP_BIT;
    bits |= IPV6_GOTIP_BIT;

    ESP_LOGI(TAG, "Waiting for AP connection...");
    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");

    err = WeavePlatform::SystemLayer.StartTimer(5000, HandleAliveTimer, NULL);
    if (err != WEAVE_NO_ERROR) {
        ESP_LOGE(TAG, "Failed to start timer: %s", ErrorStr(err));
        return;
    }

    WeaveLogError(Support, "Ready");

    WeavePlatform::SystemLayer.DispatchEvents();
}
