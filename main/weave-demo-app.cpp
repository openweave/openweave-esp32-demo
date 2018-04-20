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
#include "Display.h"
#include "TitleWidget.h"
#include "StatusIndicatorWidget.h"
#include "PairingWidget.h"
#include "Button.h"

using namespace ::nl;
using namespace ::nl::Inet;
using namespace ::nl::Weave;
using namespace ::nl::Weave::Device;

const char * TAG = "demo-app";

#define M5STACK_BUTTON_A_PIN GPIO_NUM_39
#define M5STACK_BUTTON_B_PIN GPIO_NUM_38
#define M5STACK_BUTTON_C_PIN GPIO_NUM_37

enum DisplayMode
{
    kDisplayMode_Uninitialized,
    kDisplayMode_StatusScreen,
    kDisplayMode_PairingScreen
};

static DisplayMode displayMode = kDisplayMode_Uninitialized;
static TitleWidget titleWidget;
static StatusIndicatorWidget statusIndicator;
static PairingWidget pairingWidget;
static Button attentionButton;
static bool isPairedToAccount = true;
static volatile bool commissionerDetected = false;

static void DeviceEventHandler(const WeaveDeviceEvent * event, intptr_t arg);

extern "C" void app_main()
{
    esp_err_t err;      // A quick note about errors: Weave adopts the error type and numbering
                        // convention of the environment into which it is ported.  Thus esp_err_t
                        // and WEAVE_ERROR are in fact the same type, and both ESP-IDF errors
                        // and Weave-specific errors can be stored in the same value without
                        // ambiguity.  For convenience, ESP_OK and WEAVE_NO_ERROR are mapped
                        // to the same value.

    // Initialize the ESP NVS layer.
    err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", esp_err_to_name(err));
        return;
    }

    // Initialize locks in the Weave Device code.  This must be done before the ESP
    // tcpip_adapter layer is initialized.
    err = PlatformMgr.InitLocks();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "PlatformMgr.InitLocks() failed: %s", ErrorStr(err));
        return;
    }

    // Initialize the ESL tcpip adapter.
    tcpip_adapter_init();

    // Arrange for the ESP event loop to deliver events into the Weave Device layer.
    err = esp_event_loop_init(PlatformManager::HandleESPSystemEvent, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_event_loop_init() failed: %s", esp_err_to_name(err));
        return;
    }

    // Initialize the ESP WiFi layer.
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_event_loop_init() failed: %s", esp_err_to_name(err));
        return;
    }

    // Initialize the Weave stack.
    err = PlatformMgr.InitWeaveStack();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "PlatformMgr.InitWeaveStack() failed: %s", ErrorStr(err));
        return;
    }

    // Configure the Weave Connectivity Manager to automatically enable the WiFi AP interface
    // whenever the WiFi station interface has not be configured.
    ConnectivityMgr.SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_OnDemand_NoStationProvision);

    // Register a function to receive events from the Weave device layer.  Note that calls to
    // this function will happen on the Weave event loop thread, not the app_main thread.
    PlatformMgr.AddEventHandler(DeviceEventHandler, 0);

    // Start a Weave-based timer that will print an 'Alive' message on a periodic basis.  This
    // could be done with an ESP or FreeRTOS timer, but running it on a Weave timer confirms that
    // the Weave thread is still alive.
    err = StartAliveTimer(5000);
    if (err != ESP_OK)
    {
        return;
    }

    // Start a Weave echo client that will periodically send Weave Echo requests to the Nest service
    // whenever service connectivity is established.
    err = ServiceEcho.Init(10000);
    if (err != ESP_OK)
    {
        return;
    }

    // Look for an attempt to initialize a display device.  But don't fail if one isn't found.
    err = InitDisplay();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "InitDisplay() failed: %s", esp_err_to_name(err));
    }

    // Initialize the attention button.
    err = attentionButton.Init(M5STACK_BUTTON_C_PIN, 50);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Button.Init() failed: %s", esp_err_to_name(err));
        return;
    }

    // If the device has a display, initialize the UI widgets.
    if (HaveDisplay)
    {
        titleWidget.Init("Blue Sky Demo");

        pairingWidget.Init();

        statusIndicator.Init(5);
        statusIndicator.Char[0] = 'W';
        statusIndicator.Char[1] = 'I';
        statusIndicator.Char[2] = 'T';
        statusIndicator.Char[3] = 'S';
        statusIndicator.Char[4] = 'A';
    }

    ESP_LOGI(TAG, "Ready");

    // If the device has a display, run the title animation before starting the Weave stack.
    if (HaveDisplay)
    {
        titleWidget.Start();
        while (true)
        {
            titleWidget.Animate();
            if (titleWidget.Done)
            {
                break;
            }
            vTaskDelay(50 / portTICK_RATE_MS);
        }

        statusIndicator.Display();

        displayMode = kDisplayMode_StatusScreen;
    }

    // Start a task to run the Weave Device event loop.
    err = PlatformMgr.StartEventLoopTask();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "PlatformMgr.StartEventLoopTask() failed: %s", ErrorStr(err));
        return;
    }

    // Repeatedly loop to drive the UI...
    while (true)
    {
        bool attentionButtonPressDetected = false;

        // Poll the attention button.  Whenever we detect a *release* of the button
        // demand start the WiFi AP interface.
        if (attentionButton.Poll() && !attentionButton.IsPressed())
        {
            PlatformMgr.LockWeaveStack();
            ConnectivityMgr.DemandStartWiFiAP();
            PlatformMgr.UnlockWeaveStack();
            attentionButtonPressDetected = true;
        }

        // If the attention button has been pressed for more that the factory reset
        // press duration, initiate a factory reset of the device.
        if (attentionButton.IsPressed() &&
            attentionButton.GetStateDuration() > CONFIG_FACTORY_RESET_BUTTON_DURATION)
        {
            PlatformMgr.LockWeaveStack();
            ConfigurationMgr.InitiateFactoryReset();
            PlatformMgr.UnlockWeaveStack();
            return;
        }

        // If the device has a display...
        if (HaveDisplay)
        {
            // Collect connectivity and configuration state from the Weave stack.  Because the
            // Weave event loop is being run in a separate task, the stack must be locked
            // while these values are queried.  However we use a non-blocking lock request
            // (TryLockWeaveStack()) to avoid blocking other UI activities when the Weave
            // tasks is busy, e.g. with a long crypto operation.
            if (PlatformMgr.TryLockWeaveStack())
            {
                statusIndicator.State[0] = ConnectivityMgr.IsWiFiStationConnected();
                statusIndicator.State[1] = ConnectivityMgr.HaveIPv4InternetConnectivity();
                statusIndicator.State[2] = ConnectivityMgr.HaveServiceConnectivity();
                statusIndicator.State[3] = ServiceEcho.ServiceAlive;
                statusIndicator.State[4] = ConnectivityMgr.IsWiFiAPActive();

                isPairedToAccount = ConfigurationMgr.IsPairedToAccount();

                PlatformMgr.UnlockWeaveStack();
            }

            bool wifiAPActive = statusIndicator.State[4];

            // If currently displaying the status screen and the attention button
            // is pressed while the device is not paired to an account, switch to
            // the pairing screen.
            if (displayMode == kDisplayMode_StatusScreen)
            {
                if (!isPairedToAccount && attentionButtonPressDetected)
                {
                    ClearDisplay();
                    pairingWidget.Display();
                    commissionerDetected = false;
                    displayMode = kDisplayMode_PairingScreen;
                }
            }

            // If currently displaying the pairing screen and...
            //     the WiFI AP is no longer active,
            //     OR the device is now paired to an account,
            //     OR a session from a commissioner application has been established,
            //     OR the attention button is pressed
            // then switch back to the status screen.
            else if (displayMode == kDisplayMode_PairingScreen)
            {
                if (!wifiAPActive || isPairedToAccount || commissionerDetected || attentionButtonPressDetected)
                {
                    ClearDisplay();
                    titleWidget.Start();
                    statusIndicator.Display();
                    displayMode = kDisplayMode_StatusScreen;
                }
            }

            // If displaying the status screen, run the title animation and update the status indicators.
            if (displayMode == kDisplayMode_StatusScreen)
            {
                titleWidget.Animate();
                statusIndicator.Update();
            }
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }
}

/* Handle events from the Weave Device layer.
 *
 * NOTE: This function runs on the Weave event loop task.
 */
void DeviceEventHandler(const WeaveDeviceEvent * event, intptr_t arg)
{
    if (event->Type == WeaveDeviceEvent::kEventType_SessionEstablished &&
        event->SessionEstablished.IsCommissioner)
    {
        commissionerDetected = true;
    }
}
