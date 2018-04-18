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
#include "StatusIndicatorSet.h"

extern "C" {
#include "tftspi.h"
#include "tft.h"
} // extern "C"

#include <string.h>

extern const char *TAG;

void StatusIndicatorSet::Init(uint8_t numIndicators)
{
    Color = { 4, 173, 201 }; // PANTONE 3125 C
    Size = 15;
    VPos = 75;
    HMargin = 10;
    memset(Char, 0, sizeof(Char));
    memset(State, 0, sizeof(State));
    mNumIndicators = (numIndicators <= kMaxIndicators) ? numIndicators : (uint16_t)kMaxIndicators;
    memset(mLastChar, 0, sizeof(mLastChar));
    memset(mLastState, 0, sizeof(mLastState));
}

void StatusIndicatorSet::Update()
{
    for (uint8_t i = 0; i < mNumIndicators; i++)
    {
        if (Char[i] != mLastChar[i] || State[i] != mLastState[i])
        {
            DrawIndicator(Char[i], State[i], i);
            mLastChar[i] = Char[i];
            mLastState[i] = State[i];
        }
    }
}

void StatusIndicatorSet::DrawIndicator(char indicatorChar, bool state, uint8_t indicatorPos) const
{
    uint16_t sizePix = (DisplayHeight * Size) / 100;
    uint16_t marginPix = (DisplayWidth * HMargin) / 100;

    uint16_t indicatorY = (DisplayHeight * VPos) / 100;
    uint16_t indicatorX = marginPix + ((DisplayWidth - (2 * marginPix) - sizePix) * indicatorPos) / (mNumIndicators - 1);

    if (indicatorChar != 0 && !state)
    {
        TFT_fillRect((int)indicatorX, (int)indicatorY, (int)sizePix, (int)sizePix, TFT_BLACK);
        TFT_drawRect((int)indicatorX, (int)indicatorY, (int)sizePix, (int)sizePix, Color);
        _fg = Color;
        _bg = TFT_BLACK;
    }
    else
    {
        TFT_fillRect((int)indicatorX, (int)indicatorY, (int)sizePix, (int)sizePix, (indicatorChar != 0) ? Color : TFT_BLACK);
        _fg = TFT_BLACK;
        _bg = Color;
    }

    if (indicatorChar != 0)
    {
        char indicatorStr[2] = { indicatorChar, 0 };
        int fontWidth, fontHeight;

        TFT_setFont(DEJAVU24_FONT, NULL);

        TFT_getfontsize(&fontWidth, &fontHeight);
        fontHeight = TFT_getfontheight();
        fontWidth = TFT_getStringWidth(indicatorStr);

        uint16_t charX = indicatorX + (sizePix / 2) - (fontWidth / 2);
        uint16_t charY = indicatorY + (sizePix / 2) - (fontHeight / 2) + 2;

        TFT_print(indicatorStr, (int)charX, (int)charY);
    }
}

