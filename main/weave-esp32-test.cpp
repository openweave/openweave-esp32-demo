#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <Weave/Support/ErrorStr.h>
#include <WeavePlatform.h>

#include <Weave/Core/WeaveKeyIds.h>
#include <Weave/Profiles/security/WeaveApplicationKeys.h>

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::WeavePlatform;

static const char *TAG = "weave-esp32-test";

enum
{
    kAliveInterval = 5000
};

void DumpGroupKeys()
{
    WEAVE_ERROR err;
    Profiles::Security::AppKeys::WeaveGroupKey key;
    enum { kKeyIdListSize = 32 };
    uint32_t keyIds[kKeyIdListSize];
    uint8_t keyCount;

    err = ::WeavePlatform::FabricState.GroupKeyStore->EnumerateGroupKeys(WeaveKeyId::kType_None, keyIds, kKeyIdListSize, keyCount);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "EnumerateGroupKeys() failed: %s", ErrorStr(err));
        return;
    }

    printf("DumpGroupKeys: %" PRId8 " keys\n", keyCount);

    for (uint8_t i = 0; i < keyCount; i++)
    {
        err = ::WeavePlatform::FabricState.GroupKeyStore->RetrieveGroupKey(keyIds[i], key);
        if (err != WEAVE_NO_ERROR)
        {
            ESP_LOGE(TAG, "RetrieveGroupKey() failed: %s", ErrorStr(err));
            return;
        }

        if (WeaveKeyId::IsAppEpochKey(key.KeyId))
        {
            printf("  Key %" PRId8 ": id %" PRIx32 ", len %" PRId8 ", data %" PRIx8 ", start time %" PRId32 "\n", i, key.KeyId, key.KeyLen, key.Key[0], key.StartTime);
        }
        else if (WeaveKeyId::IsAppGroupMasterKey(key.KeyId))
        {
            printf("  Key %" PRId8 ": id %" PRIx32 ", len %" PRId8 ", data %" PRIx8 ", global id %" PRIx32 "\n", i, key.KeyId, key.KeyLen, key.Key[0], key.GlobalId);
        }
        else
        {
            printf("  Key %" PRId8 ": id %" PRIx32 ", len %" PRId8 ", data %" PRIx8 "\n", i, key.KeyId, key.KeyLen, key.Key[0]);
        }
    }
}

void TestGroupKeyStore()
{
    WEAVE_ERROR err;

    Profiles::Security::AppKeys::WeaveGroupKey key;

    DumpGroupKeys();

    key.KeyId = WeaveKeyId::kServiceRootKey;
    key.KeyLen = Profiles::Security::AppKeys::kWeaveAppRootKeySize;
    memset(key.Key, 0x34, key.KeyLen);

    err = ::WeavePlatform::FabricState.GroupKeyStore->StoreGroupKey(key);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "StoreGroupKey() failed: %s", ErrorStr(err));
        return;
    }

    key.KeyId = WeaveKeyId::MakeEpochKeyId(2);
    key.KeyLen = Profiles::Security::AppKeys::kWeaveAppEpochKeySize;
    memset(key.Key, 0x73, key.KeyLen);
    key.StartTime = 0x74;

    err = ::WeavePlatform::FabricState.GroupKeyStore->StoreGroupKey(key);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "StoreGroupKey() failed: %s", ErrorStr(err));
        return;
    }

    key.KeyId = WeaveKeyId::MakeEpochKeyId(3);
    key.KeyLen = Profiles::Security::AppKeys::kWeaveAppEpochKeySize;
    memset(key.Key, 0x74, key.KeyLen);
    key.StartTime = 0x74;

    err = ::WeavePlatform::FabricState.GroupKeyStore->StoreGroupKey(key);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "StoreGroupKey() failed: %s", ErrorStr(err));
        return;
    }

    key.KeyId = WeaveKeyId::MakeAppGroupMasterKeyId(0x42);
    key.KeyLen = Profiles::Security::AppKeys::kWeaveAppGroupMasterKeySize;
    memset(key.Key, 0x42, key.KeyLen);
    key.GlobalId = 0x42424242;

    err = ::WeavePlatform::FabricState.GroupKeyStore->StoreGroupKey(key);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "StoreGroupKey() failed: %s", ErrorStr(err));
        return;
    }

    key.KeyId = WeaveKeyId::MakeAppGroupMasterKeyId(0x24);
    key.KeyLen = Profiles::Security::AppKeys::kWeaveAppGroupMasterKeySize;
    memset(key.Key, 0x24, key.KeyLen);
    key.GlobalId = 0x24242424;

    err = ::WeavePlatform::FabricState.GroupKeyStore->StoreGroupKey(key);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "StoreGroupKey() failed: %s", ErrorStr(err));
        return;
    }

    DumpGroupKeys();

    err = ::WeavePlatform::FabricState.GroupKeyStore->DeleteGroupKey(WeaveKeyId::MakeAppGroupMasterKeyId(0x24));
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "StoreGroupKey() failed: %s", ErrorStr(err));
        return;
    }

    DumpGroupKeys();

    err = ::WeavePlatform::FabricState.GroupKeyStore->DeleteGroupKeysOfAType(WeaveKeyId::kType_AppEpochKey);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "StoreGroupKey() failed: %s", ErrorStr(err));
        return;
    }

    DumpGroupKeys();

    err = ::WeavePlatform::FabricState.GroupKeyStore->Clear();
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "StoreGroupKey() failed: %s", ErrorStr(err));
        return;
    }

    DumpGroupKeys();
}

void HandleAliveTimer(System::Layer * aLayer, void * aAppState, System::Error aError)
{
    WEAVE_ERROR err;

    ESP_LOGI(TAG, "Alive");

    err = SystemLayer.StartTimer(kAliveInterval, HandleAliveTimer, NULL);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "SystemLayer.StartTimer() failed: %s", ErrorStr(err));
        return;
    }
}

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

    err = ::WeavePlatform::SystemLayer.StartTimer(kAliveInterval, HandleAliveTimer, NULL);
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "SystemLayer.StartTimer() failed: %s", ErrorStr(err));
        return;
    }

    ESP_LOGI(TAG, "Ready");

    ::WeavePlatform::PlatformMgr.RunEventLoop();
}
