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

#include "qrcode.h"

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <Weave/Profiles/device-description/DeviceDescription.h>

#include "Display.h"
#include "PairingWidget.h"

#if CONFIG_HAVE_DISPLAY

extern const char * TAG;

using namespace ::nl;
using namespace ::nl::Weave::DeviceLayer;
using namespace ::nl::Weave::Profiles::DeviceDescription;

enum
{
    kQRCodeVersion = 4,
    kQRCodeECC = ECC_LOW,
    kQRCodeModuleSizePix = 4,
    kQRCodeQuietZone = 2
};

void PairingWidget::Init()
{
    PairingCodeColor = { 4, 173, 201 }; // PANTONE 3125 C
    QRCodeColor = { 141, 151, 155 }; // PANTONE 443 C
    VMargin = 7;
}

void PairingWidget::Display()
{
    WEAVE_ERROR err;
    QRCode qrCode;
    enum { kMaxQRCodeStrLength = 120 };
    char * qrCodeStr = NULL;
    uint8_t * qrCodeDataBuf = NULL;
    uint16_t qrCodeSize, qrCodeX, qrCodeY, qrCodeXOffset, qrCodeYOffset;
    uint16_t pairingCodeHeight, pairingCodeWidth, pairingCodeX, pairingCodeY;

    // Construct the string to be encoded in the QR code.
    err = GetQRCodeString(qrCodeStr);
    SuccessOrExit(err);

    // Generate the QR code.
    qrCodeDataBuf = (uint8_t *)malloc(qrcode_getBufferSize(kQRCodeVersion));
    VerifyOrExit(qrCodeDataBuf != NULL, err = WEAVE_ERROR_NO_MEMORY);
    if (qrcode_initText(&qrCode, qrCodeDataBuf, kQRCodeVersion, kQRCodeECC, qrCodeStr) != 0)
    {
        ESP_LOGE(TAG, "qrcode_initText() failed");
        ExitNow(err = WEAVE_ERROR_INCORRECT_STATE);
    }

    // Draw the QR code image on the screen.
    qrCodeSize = (qrCode.size + kQRCodeQuietZone * 2) * kQRCodeModuleSizePix;
    qrCodeX = (DisplayWidth - qrCodeSize) / 2;
    qrCodeY = (DisplayHeight * VMargin) / 100;
    qrCodeXOffset = qrCodeX + (kQRCodeQuietZone * kQRCodeModuleSizePix);
    qrCodeYOffset = qrCodeY + (kQRCodeQuietZone * kQRCodeModuleSizePix);
    TFT_fillRect(qrCodeX, qrCodeY, (int)qrCodeSize, (int)qrCodeSize, QRCodeColor);
    for (uint8_t y = 0; y < qrCode.size; y++)
    {
        for (uint8_t x = 0; x < qrCode.size; x++)
        {
            if (qrcode_getModule(&qrCode, x, y))
            {
                TFT_fillRect(x * kQRCodeModuleSizePix + qrCodeXOffset, y * kQRCodeModuleSizePix + qrCodeYOffset,
                             (int)kQRCodeModuleSizePix, (int)kQRCodeModuleSizePix, TFT_BLACK);
            }
        }
    }

    // Display the pairing code
    TFT_setFont(DEJAVU24_FONT, NULL);
    pairingCodeHeight = TFT_getfontheight();
    pairingCodeWidth = TFT_getStringWidth((char *)FabricState.PairingCode);
    pairingCodeX = (DisplayWidth - pairingCodeWidth) / 2;
    pairingCodeY = qrCodeY + qrCodeSize + (DisplayHeight - (qrCodeY + qrCodeSize) - pairingCodeHeight) / 2;
    _fg = PairingCodeColor;
    _bg = TFT_BLACK;
    TFT_print((char *)FabricState.PairingCode, (int)pairingCodeX, (int)pairingCodeY);

exit:
    if (qrCodeStr != NULL)
    {
        free(qrCodeStr);
    }
    if (qrCodeDataBuf != NULL)
    {
        free(qrCodeDataBuf);
    }
    if (err != WEAVE_NO_ERROR)
    {
        ESP_LOGE(TAG, "PairingWidget::Display() failed: %s", ErrorStr(err));
    }
}

WEAVE_ERROR PairingWidget::GetQRCodeString(char *& qrCodeStr)
{
    WEAVE_ERROR err;
    WeaveDeviceDescriptor deviceDesc;
    uint32_t encodedLen;

    enum {
        kMaxQRCodeStrLength = 120
    };

    qrCodeStr = NULL;

    // Get a Weave device descriptor describing the local device.
    PlatformMgr().LockWeaveStack();
    err = ConfigurationMgr().GetDeviceDescriptor(deviceDesc);
    PlatformMgr().UnlockWeaveStack();
    SuccessOrExit(err);

    // The QR code form of a device descriptor contains the pairing code, which
    // mobile applications use to authenticate the device. For security reasons
    // the pairing code is not automatically included in the device descriptor
    // structure.  So insert it now.
    strncpy(deviceDesc.PairingCode, FabricState.PairingCode, WeaveDeviceDescriptor::kMaxPairingCodeLength);
    deviceDesc.PairingCode[WeaveDeviceDescriptor::kMaxPairingCodeLength] = 0;

    // Encode the device descriptor into the string form used within a QR code.
    qrCodeStr = (char *)malloc(kMaxQRCodeStrLength);
    VerifyOrExit(qrCodeStr != NULL, err = WEAVE_ERROR_NO_MEMORY);
    err = WeaveDeviceDescriptor::EncodeText(deviceDesc, qrCodeStr, (uint32_t)kMaxQRCodeStrLength, encodedLen);
    SuccessOrExit(err);

exit:
    if (err != WEAVE_NO_ERROR && qrCodeStr != NULL)
    {
        free(qrCodeStr);
        qrCodeStr = NULL;
    }
    return err;
}

#endif // CONFIG_HAVE_DISPLAY
