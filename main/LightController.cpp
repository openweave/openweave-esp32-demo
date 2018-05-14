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

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <LightController.h>
#include <nest/trait/lighting/LogicalCircuitControlTrait.h>

using namespace ::nl::Weave::DeviceLayer;
using namespace ::nl::Weave::Profiles::DataManagement_Current;
using namespace ::nl::Weave::TLV;
using namespace ::Schema::Nest::Trait::Lighting;

extern const char * TAG;

LightController::LightController(void)
    : mStateDS(*this), mControlDS(*this)
{
    mState = OFF;
    mBrightness = 100;
    mGPIONum = GPIO_NUM_MAX;
}

WEAVE_ERROR LightController::Init(gpio_num_t gpioNum)
{
    WEAVE_ERROR err;

    // Publish the LogicalCircuitStateTrait for the light.
    err = TraitMgr.PublishTrait(0, &mStateDS);
    SuccessOrExit(err);

    // Publish the LogicalCircuitControlTrait for the light.
    err = TraitMgr.PublishTrait(0, &mControlDS);
    SuccessOrExit(err);

    mState = OFF;
    mBrightness = 100;
    mGPIONum = gpioNum;

    if (gpioNum < GPIO_NUM_MAX)
    {
        gpio_set_direction(gpioNum, GPIO_MODE_OUTPUT);
    }

exit:
    return err;
}


void LightController::ChangeState(uint8_t state, uint8_t brightness)
{
    mState = state;
    mBrightness = brightness;
    if (mGPIONum < GPIO_NUM_MAX)
    {
        gpio_set_level(mGPIONum, (state == ON) ? 1 : 0);
    }
    ESP_LOGI(TAG, "Light state changed to %s, level %" PRIu8, (mState == ON) ? "ON" : "OFF", mBrightness);
    mStateDS.SetDirty(LogicalCircuitStateTrait::kPropertyHandle_Root);
}

LightController::LogicalCircuitStateTraitDataSource::LogicalCircuitStateTraitDataSource(LightController & lightController)
    : TraitDataSource(&LogicalCircuitStateTrait::TraitSchema),
      mLightController(lightController)
{
}

WEAVE_ERROR LightController::LogicalCircuitStateTraitDataSource::GetLeafData(PropertyPathHandle aLeafHandle, uint64_t aTagToWrite, TLVWriter & aWriter)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    switch (aLeafHandle)
    {
    case LogicalCircuitStateTrait::kPropertyHandle_State:
        err = aWriter.Put(aTagToWrite, mLightController.mState);
        SuccessOrExit(err);
        break;

    case LogicalCircuitStateTrait::kPropertyHandle_Brightness:
        err = aWriter.Put(aTagToWrite, mLightController.mBrightness);
        SuccessOrExit(err);
        break;

    default:
        break;
    }

exit:
    return err;
}

LightController::LogicalCircuitControlTraitDataSource::LogicalCircuitControlTraitDataSource(LightController & lightController)
    : TraitDataSource(&LogicalCircuitControlTrait::TraitSchema),
      mLightController(lightController)
{
}

WEAVE_ERROR LightController::LogicalCircuitControlTraitDataSource::GetLeafData(PropertyPathHandle aLeafHandle, uint64_t aTagToWrite, TLVWriter & aWriter)
{
    // Nothing to do
    return WEAVE_NO_ERROR;
}

void LightController::LogicalCircuitControlTraitDataSource::OnCustomCommand(::nl::Weave::Profiles::DataManagement_Current::Command * aCommand,
        const nl::Weave::WeaveMessageInfo * aMsgInfo,
        ::nl::Weave::PacketBuffer * aPayload, const uint64_t & aCommandType, const bool aIsExpiryTimeValid,
        const int64_t & aExpiryTimeMicroSecond, const bool aIsMustBeVersionValid,
        const uint64_t & aMustBeVersion, ::nl::Weave::TLV::TLVReader & aArgumentReader)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    uint8_t newState, newBrightness;
    uint32_t statusProfileId = ::nl::Weave::Profiles::kWeaveProfile_Common;
    uint32_t statusCode = ::nl::Weave::Profiles::Common::kStatus_InternalError;
    bool respSent = false;

    // Verify that the requested command is supported (only SetLogicalCircuitState in this case).
    if (aCommandType != LogicalCircuitControlTrait::kSetLogicalCircuitStateRequestId)
    {
        statusProfileId = ::nl::Weave::Profiles::kWeaveProfile_Common;
        statusCode = ::nl::Weave::Profiles::Common::kStatus_UnsupportedMessage;
        ExitNow(err = WEAVE_ERROR_INVALID_ARGUMENT);
    }

    // If a command expiry time has been given, fail the command if that time has passed, or if the system
    // doesn't know the current time.
    if (aIsExpiryTimeValid)
    {
        uint64_t nowMS;

        err = ::nl::Weave::System::Layer::GetClock_RealTimeMS(nowMS);
        VerifyOrExit(err == WEAVE_NO_ERROR || err == WEAVE_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED, /* */);

        if (err == WEAVE_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED || nowMS >= aExpiryTimeMicroSecond)
        {
            statusProfileId = ::nl::Weave::Profiles::kWeaveProfile_WDM;
            statusCode = ::nl::Weave::Profiles::DataManagement_Current::kStatus_RequestExpiredInTime;
            ExitNow();
        }
    }

    // If a required version has been specified, verify that the current trait version matches; otherwise
    // fail the command.
    if (aIsMustBeVersionValid && aMustBeVersion != GetVersion())
    {
        statusProfileId = ::nl::Weave::Profiles::kWeaveProfile_WDM;
        statusCode = ::nl::Weave::Profiles::DataManagement_Current::kStatus_VersionMismatch;
        ExitNow();
    }

    // TODO: support action time

    // Parse and verify the command arguments.
    {
        TLVType container;
        err = aArgumentReader.EnterContainer(container);
        SuccessOrExit(err);
        err = aArgumentReader.Next(kTLVType_UnsignedInteger, ContextTag(LogicalCircuitControlTrait::kSetLogicalCircuitStateRequestParameter_State));
        SuccessOrExit(err);
        err = aArgumentReader.Get(newState);
        SuccessOrExit(err);
        err = aArgumentReader.Next(kTLVType_UnsignedInteger, ContextTag(LogicalCircuitControlTrait::kSetLogicalCircuitStateRequestParameter_Level));
        SuccessOrExit(err);
        err = aArgumentReader.Get(newBrightness);
        SuccessOrExit(err);
        err = aArgumentReader.ExitContainer(container);
        SuccessOrExit(err);
    }

    VerifyOrExit(newState == PhysicalCircuitStateTrait::CIRCUIT_STATE_ON || newState == PhysicalCircuitStateTrait::CIRCUIT_STATE_OFF,
                 (err = WEAVE_ERROR_INVALID_ARGUMENT, statusCode = ::nl::Weave::Profiles::Common::kStatus_BadRequest));
    VerifyOrExit(newBrightness <= 100,
                 (err = WEAVE_ERROR_INVALID_ARGUMENT, statusCode = ::nl::Weave::Profiles::Common::kStatus_BadRequest));

    // Update the state of the light.
    mLightController.ChangeState(newState, newBrightness);

    // Send the response.
    err = aCommand->SendResponse(GetVersion(), NULL);
    respSent = true;
    SuccessOrExit(err);

exit:

    if (!respSent)
    {
        aCommand->SendError(statusProfileId, statusCode, err);
    }

    aCommand->Close();

    PacketBuffer::Free(aPayload);
}
