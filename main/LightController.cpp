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
#include "driver/ledc.h"

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <LightController.h>
#include <nest/trait/lighting/LogicalCircuitControlTrait.h>

using namespace ::nl::Weave::DeviceLayer;
using namespace ::nl::Weave::Profiles::DataManagement_Current;
using namespace ::nl::Weave::TLV;
using namespace ::Schema::Nest::Trait::Lighting;

extern const char * TAG;

#define DIMMER_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define DIMMER_TIMER_NUM LEDC_TIMER_0
#define DIMMER_CHANNEL_NUM LEDC_CHANNEL_0
#define DIMMER_FREQ 128
#define DIMMER_RESOLUTION LEDC_TIMER_10_BIT
#define DIMMER_DUTY_CYCLE_MAX_VALUE ((1u << LEDC_TIMER_10_BIT) - 1)

LightController::LightController(void)
    : mStateDS(*this), mControlDS(*this)
{
    mState = OFF;
    mLevel = 100;
    mGPIONum = GPIO_NUM_MAX;
}

WEAVE_ERROR LightController::Init(gpio_num_t gpioNum)
{
    WEAVE_ERROR err;
    ledc_timer_config_t dimmerTimerConfig;
    ledc_channel_config_t dimmerChanConfig;

    VerifyOrExit(gpioNum < GPIO_NUM_MAX, err = WEAVE_ERROR_INVALID_ARGUMENT);

    // Publish the LogicalCircuitStateTrait for the light.
    err = TraitMgr().PublishTrait(0, &mStateDS);
    SuccessOrExit(err);

    // Publish the LogicalCircuitControlTrait for the light.
    err = TraitMgr().PublishTrait(0, &mControlDS);
    SuccessOrExit(err);

    mState = OFF;
    mLevel = 100;
    mGPIONum = gpioNum;

    if (gpioNum < GPIO_NUM_MAX)
    {
        memset(&dimmerTimerConfig, 0, sizeof(dimmerTimerConfig));
        dimmerTimerConfig.duty_resolution = LEDC_TIMER_10_BIT;
        dimmerTimerConfig.freq_hz = DIMMER_FREQ;
        dimmerTimerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
        dimmerTimerConfig.timer_num = LEDC_TIMER_0;
        err = ledc_timer_config(&dimmerTimerConfig);
        SuccessOrExit(err);

        memset(&dimmerChanConfig, 0, sizeof(dimmerChanConfig));
        dimmerChanConfig.channel = LEDC_CHANNEL_0;
        dimmerChanConfig.duty = 0;
        dimmerChanConfig.gpio_num = gpioNum;
        dimmerChanConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
        dimmerChanConfig.timer_sel = LEDC_TIMER_0;
        err = ledc_channel_config(&dimmerChanConfig);
        SuccessOrExit(err);
    }

exit:
    return err;
}

void LightController::Set(int8_t state, uint8_t level)
{
    uint32_t dimmerDutyCycle = 0;

    mState = state;
    mLevel = level;
    if (mGPIONum < GPIO_NUM_MAX)
    {
        dimmerDutyCycle = (state == ON) ? (DIMMER_DUTY_CYCLE_MAX_VALUE * level * 2 + 1) / 200 : 0;
        ledc_set_duty(DIMMER_SPEED_MODE, DIMMER_CHANNEL_NUM, dimmerDutyCycle);
        ledc_update_duty(DIMMER_SPEED_MODE, DIMMER_CHANNEL_NUM);
    }
    ESP_LOGI(TAG, "Light state changed to %s, level %" PRIu8 " (pwm %" PRIu32 "/%" PRIu32 ")",
             (mState == ON) ? "ON" : "OFF", mLevel, dimmerDutyCycle, DIMMER_DUTY_CYCLE_MAX_VALUE);
    mStateDS.Lock();
    mStateDS.SetDirty(LogicalCircuitStateTrait::kPropertyHandle_Root);
    mStateDS.Unlock();
    nl::Weave::Profiles::DataManagement::SubscriptionEngine::GetInstance()->GetNotificationEngine()->Run();
}

void LightController::Toggle(void)
{
    Set((mState == ON) ? OFF : ON, mLevel);
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
        err = aWriter.Put(aTagToWrite, mLightController.mLevel);
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
    uint8_t newState, newLevel;
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

        err = aArgumentReader.Next();
        SuccessOrExit(err);
        VerifyOrExit(aArgumentReader.GetTag() == ContextTag(LogicalCircuitControlTrait::kSetLogicalCircuitStateRequestParameter_State),
                     (err = WEAVE_ERROR_UNEXPECTED_TLV_ELEMENT, statusCode = ::nl::Weave::Profiles::Common::kStatus_BadRequest));
        if (aArgumentReader.GetType() == kTLVType_SignedInteger)
        {
            err = aArgumentReader.Get(newState);
            SuccessOrExit(err);
            VerifyOrExit(newState == PhysicalCircuitStateTrait::CIRCUIT_STATE_ON || newState == PhysicalCircuitStateTrait::CIRCUIT_STATE_OFF,
                         (err = WEAVE_ERROR_INVALID_ARGUMENT, statusCode = ::nl::Weave::Profiles::Common::kStatus_BadRequest));
        }
        else
        {
            VerifyOrExit(aArgumentReader.GetType() == kTLVType_Null,
                         (err = WEAVE_ERROR_WRONG_TLV_TYPE, statusCode = ::nl::Weave::Profiles::Common::kStatus_BadRequest));
            newState = mLightController.mState;
        }

        err = aArgumentReader.Next();
        SuccessOrExit(err);
        VerifyOrExit(aArgumentReader.GetTag() == ContextTag(LogicalCircuitControlTrait::kSetLogicalCircuitStateRequestParameter_Level),
                     (err = WEAVE_ERROR_UNEXPECTED_TLV_ELEMENT, statusCode = ::nl::Weave::Profiles::Common::kStatus_BadRequest));
        if (aArgumentReader.GetType() == kTLVType_UnsignedInteger)
        {
            err = aArgumentReader.Get(newLevel);
            SuccessOrExit(err);
            VerifyOrExit(newLevel <= 100,
                         (err = WEAVE_ERROR_INVALID_ARGUMENT, statusCode = ::nl::Weave::Profiles::Common::kStatus_BadRequest));
        }
        else
        {
            VerifyOrExit(aArgumentReader.GetType() == kTLVType_Null,
                         (err = WEAVE_ERROR_WRONG_TLV_TYPE, statusCode = ::nl::Weave::Profiles::Common::kStatus_BadRequest));
            newLevel = mLightController.mLevel;
        }

        err = aArgumentReader.ExitContainer(container);
        SuccessOrExit(err);
    }

    // Update the state of the light.
    mLightController.Set(newState, newLevel);

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
