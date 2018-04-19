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
#include "driver/gpio.h"

#include <WeaveDevice.h>
#include "Button.h"

extern const char * TAG;

esp_err_t Button::Init(gpio_num_t gpioNum, uint16_t debouncePeriod)
{
    esp_err_t err;

    mGPIONum = gpioNum;
    mDebouncePeriod = debouncePeriod;
    mEffectiveState = false;
    mLastState = false;

    err = gpio_set_direction(gpioNum, GPIO_MODE_INPUT);
    SuccessOrExit(err);

    Poll();

exit:
    return err;
}

bool Button::Poll()
{
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    bool newState = gpio_get_level(mGPIONum) == 0;

    if (newState != mLastState)
    {
        ESP_LOGE(TAG, "Button: last = %s new = %s", mLastState ? "true" : "false", newState ? "true" : "false");
        mLastState = newState;
        mLastReadTime = now;
    }

    else if (newState != mEffectiveState && (now - mLastReadTime) >= mDebouncePeriod)
    {
        mEffectiveState = newState;
        return true;
    }

    return false;
}

uint32_t Button::GetStateDuration()
{
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    return now - mLastReadTime;
}
