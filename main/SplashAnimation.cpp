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

#include "Display.h"
#include "SplashAnimation.h"

extern "C" {
#include "tftspi.h"
#include "tft.h"
} // extern "C"

#include <string.h>

extern const char *TAG;

namespace {

const uint8_t OpenWeaveLogo[] =
{
#include "OpenWeaveLogo.dat"
};

const uint16_t OpenWeaveLogo_Width = 280;
const uint16_t OpenWeaveLogo_Height = 28;

} // unnamed namespace

void SplashAnimation::Init(const char * title)
{
    Title = title;
    LogoVPos = 20;
    TitleVPos = 45;
    AnimationTimeMS = 1000;
    TitleDelayMS = 300;
    TitleColor = { 141, 151 , 155 }; // PANTONE 443 C
    mStartTimeUS = 0;
    mLogoX = (DisplayWidth - OpenWeaveLogo_Width) / 2;
    mLogoY = UINT16_MAX;
    Done = true;
}

void SplashAnimation::Start()
{
    mStartTimeUS = ::esp_timer_get_time();
    mLogoY = UINT16_MAX;
    Done = false;
}

void SplashAnimation::Animate()
{
    if (Done)
    {
        return;
    }

    uint32_t relativeTimeMS = (uint32_t)((::esp_timer_get_time() - mStartTimeUS) / 1000);

    uint16_t logoEndY = (DisplayHeight * LogoVPos) / 100;

    if (mLogoY == UINT16_MAX || mLogoY < logoEndY)
    {
        uint16_t newLogoY;

        if (AnimationTimeMS != 0 && relativeTimeMS < AnimationTimeMS)
        {
            newLogoY = (uint16_t)((logoEndY * relativeTimeMS) / AnimationTimeMS);
        }
        else
        {
            newLogoY = logoEndY;
        }

        if (mLogoY != UINT16_MAX)
        {
            enum { kMinLogoStep = 1 };

            uint16_t stepY = newLogoY - mLogoY;

            if (newLogoY < logoEndY && stepY < kMinLogoStep)
            {
                return;
            }

            TFT_fillRect((int)mLogoX, (int)mLogoY, (int)OpenWeaveLogo_Width, (stepY < OpenWeaveLogo_Height) ? (int)stepY : (int)OpenWeaveLogo_Height, TFT_BLACK);
        }

        mLogoY = newLogoY;
        // TFT_drawRect((int)mLogoX, (int)mLogoY, (int)OpenWeaveLogo_Width, (int)OpenWeaveLogo_Height, TFT_CYAN);
        TFT_bmp_image((int)mLogoX, (int)mLogoY, 0, NULL, (uint8_t *)OpenWeaveLogo, sizeof(OpenWeaveLogo));
    }

    if (relativeTimeMS >= (AnimationTimeMS + TitleDelayMS))
    {
        uint16_t titleWidth = (uint16_t)TFT_getStringWidth((char *)Title);
        uint16_t titleX = (DisplayWidth - titleWidth) / 2;
        uint16_t titleY = (DisplayHeight * TitleVPos) / 100;

        _fg = TitleColor;
        TFT_print((char *)Title, titleX, titleY);

        ESP_LOGI(TAG, "Splash animation done");

        Done = true;
    }
}


