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

#include <string.h>

#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Display.h"

#if CONFIG_HAVE_DISPLAY

#include "CountdownWidget.h"

extern const char *TAG;

void CountdownWidget::Init(uint8_t numIndicators, uint32_t intervalMS)
{
    StatusIndicatorWidget::Init(numIndicators);

    Color = { 4, 173, 201 }; // PANTONE 3125 C
    Size = 15;
    VPos = 55;
    HMargin = 20;
    mStartTimeUS = 0;
    mIntervalMS = intervalMS;

    for (uint8_t i = 0; i < mNumIndicators; i++)
    {
        Char[i] = '0' + (mNumIndicators - i);
        State[i] = false;
    }
}

void CountdownWidget::Start(uint32_t elapsedTime)
{
    mStartTimeUS = ::esp_timer_get_time() - elapsedTime;
    Display();
}

void CountdownWidget::Update()
{
    if (mStartTimeUS != 0)
    {
        uint8_t elapsedCount = GetElapsedCount();
        if (elapsedCount < mNumIndicators + 1)
        {
            for (uint8_t i = 0; i < mNumIndicators; i++)
            {
                State[i] = (i < elapsedCount);
            }
            StatusIndicatorWidget::Update();
        }
    }
}

uint8_t CountdownWidget::GetElapsedCount()
{
    if (mStartTimeUS != 0)
    {
        uint32_t elapsedTimeMS = (uint32_t)((::esp_timer_get_time() - mStartTimeUS) / 1000);
        return (uint8_t)(elapsedTimeMS / mIntervalMS);
    }
    else
    {
        return 0;
    }
}

bool CountdownWidget::IsDone()
{
    return GetElapsedCount() <  mNumIndicators + 1;
}

uint32_t CountdownWidget::TotalDurationMS()
{
    return (mNumIndicators + 1) * mIntervalMS;
}

#endif // CONFIG_HAVE_DISPLAY
