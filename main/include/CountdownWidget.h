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

#ifndef COUNTDOWN_WIDGET_H
#define COUNTDOWN_WIDGET_H

#include "Display.h"
#include "StatusIndicatorWidget.h"

#if CONFIG_HAVE_DISPLAY

class CountdownWidget : private StatusIndicatorWidget
{
public:
    enum
    {
        kMaxIndicators = StatusIndicatorWidget::kMaxIndicators
    };

    using StatusIndicatorWidget::Color;
    using StatusIndicatorWidget::Size;
    using StatusIndicatorWidget::VPos;
    using StatusIndicatorWidget::HMargin;

    void Init(uint8_t numIndicators, uint32_t intervalMS = 1000);
    void Start(uint32_t elapsedTime = 0);
    void Update();
    bool IsDone();
    uint32_t TotalDurationMS();

private:
    int64_t mStartTimeUS;
    uint32_t mIntervalMS;

    uint8_t GetElapsedCount();
    void DrawIndicator(bool state, uint8_t indicatorPos) const;
};

#endif // CONFIG_HAVE_DISPLAY

#endif // COUNTDOWN_WIDGET_H
