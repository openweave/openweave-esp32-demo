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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_timer.h>
#include <string.h>

#include "UI.h"
#include "Display.h"
#include "SplashAnimation.h"
#include "StatusIndicatorSet.h"

#include <WeaveDevice.h>
#include <ServiceEcho.h>

using namespace ::nl::Weave::Device;

void RunUI(void)
{
    SplashAnimation splashAnimation;
    StatusIndicatorSet statusIndicators;
    bool initialSplashDone = false;

    InitDisplay();

    if (HaveDisplay)
    {
        splashAnimation.Init("Blue Sky Demo");
        splashAnimation.Start();

        statusIndicators.Init(5);
        statusIndicators.Char[0] = 'W';
        statusIndicators.Char[1] = 'I';
        statusIndicators.Char[2] = 'T';
        statusIndicators.Char[3] = 'S';
        statusIndicators.Char[4] = 'A';
    }

    while (true)
    {
        if (HaveDisplay)
        {
            if (PlatformMgr.TryLockWeaveStack())
            {
                statusIndicators.State[0] = ConnectivityMgr.IsWiFiStationConnected();
                statusIndicators.State[1] = ConnectivityMgr.HaveIPv4InternetConnectivity();
                statusIndicators.State[2] = ConnectivityMgr.HaveServiceConnectivity();
                statusIndicators.State[3] = ServiceEcho.ServiceAlive;
                statusIndicators.State[4] = ConnectivityMgr.IsWiFiAPActive();

                PlatformMgr.UnlockWeaveStack();
            }

            splashAnimation.Animate();
            if (splashAnimation.Done && !initialSplashDone)
            {
                initialSplashDone = true;
            }

            if (initialSplashDone)
            {
                statusIndicators.Update();
            }
        }

        vTaskDelay(50 / portTICK_RATE_MS);
    }
}
