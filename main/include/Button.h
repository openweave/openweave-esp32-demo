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

#ifndef BUTTON_H
#define BUTTON_H

#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"

class Button
{
public:
    esp_err_t Init(gpio_num_t gpioNum, uint16_t debouncePeriod);
    bool Poll();
    bool IsPressed();
    uint32_t GetStateDuration();

private:
    uint32_t mLastReadTime;
    gpio_num_t mGPIONum;
    uint16_t mDebouncePeriod;
    bool mEffectiveState;
    bool mLastState;
};

inline bool Button::IsPressed()
{
    return mEffectiveState;
}

#endif // BUTTON_H
