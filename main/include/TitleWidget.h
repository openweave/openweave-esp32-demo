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

#ifndef TITLE_WIDGET_H
#define TITLE_WIDGET_H

#if CONFIG_EXAMPLE_DISPLAY_TYPE != 0

class TitleWidget
{
public:
    void Init(const char * title);
    void Start();
    void Animate();

    const char * Title;
    uint16_t LogoVPos;
    uint16_t TitleVPos;
    color_t TitleColor;
    uint32_t AnimationTimeMS;
    uint32_t TitleDelayMS;
    uint32_t LingerDelayMS;
    bool Done;

private:
    int64_t mStartTimeUS;
    uint16_t mLogoX;
    uint16_t mLogoY;
    bool mTitleDisplayed;
};

#endif // CONFIG_EXAMPLE_DISPLAY_TYPE != 0

#endif // TITLE_WIDGET_H


