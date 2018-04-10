#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps_init.h"
#include <new>

#include <WeavePlatform.h>
#include <Weave/Support/ErrorStr.h>

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::WeavePlatform;

extern const char * TAG;

uint32_t AliveIntervalMS;

void HandleAliveTimer(System::Layer * /* unused */, void * /* unused */, System::Error /* unused */)
{
    WEAVE_ERROR err;

    ESP_LOGI(TAG, "Alive");

    heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

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
