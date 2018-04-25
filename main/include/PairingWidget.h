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

#ifndef PAIRING_WIDGET_H
#define PAIRING_WIDGET_H

#if CONFIG_EXAMPLE_DISPLAY_TYPE != 0

class PairingWidget
{
public:
    color_t QRCodeColor;
    color_t PairingCodeColor;
    uint16_t VMargin;

    void Init();
    void Display();

private:
    WEAVE_ERROR GetQRCodeString(char *& qrCodeStr);
};

#endif // CONFIG_EXAMPLE_DISPLAY_TYPE != 0

#endif // PAIRING_WIDGET_H
