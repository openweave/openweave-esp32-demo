#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_heap_caps_init.h"
#include <new>

#include <WeavePlatform.h>
#include <Weave/Support/ErrorStr.h>
#include "AliveTimer.h"
#include "ServiceEcho.h"

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::WeavePlatform;

const char *TAG = "weave-esp32-test";

extern "C" void app_main()
{
    WEAVE_ERROR err;

    ESP_ERROR_CHECK( nvs_flash_init() );

    err = ::WeavePlatform::PlatformMgr.InitLwIPCoreLock();
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr.InitLwIPCoreLock() failed: %s", ErrorStr(err));
        return;
    }

    tcpip_adapter_init();

    ESP_ERROR_CHECK( esp_event_loop_init(::WeavePlatform::PlatformManager::HandleESPSystemEvent, NULL) );

    {
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

        ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_FLASH) );

        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );

        ESP_ERROR_CHECK( esp_wifi_start() );
    }

    err = ::WeavePlatform::PlatformMgr.InitWeaveStack();
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr.InitWeaveStack() failed: %s", ErrorStr(err));
        return;
    }

    ::WeavePlatform::ConnectivityMgr.SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_OnDemand_NoStationProvision);

    err = StartAliveTimer(5000);
    if (err != WEAVE_NO_ERROR)
    {
        return;
    }

    err = ServiceEcho.Init();
    if (err != WEAVE_NO_ERROR)
    {
        return;
    }

    err = ServiceEcho.Start(20000, 10000);
    if (err != WEAVE_NO_ERROR)
    {
        return;
    }

    ESP_LOGI(TAG, "Ready");

    ::WeavePlatform::PlatformMgr.RunEventLoop();
}
