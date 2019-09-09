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

#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <Weave/Profiles/data-management/TraitData.h>
#include <nest/trait/lighting/LogicalCircuitStateTrait.h>

/**
 *  @class LightController
 *
 *  @brief
 *    Drives the GPIO that controls the light.  Maintains the master copy of the light state and
 *    publishes this for others to consume (via WDM subscriptions).  Accepts WDM commands to change
 *    the state remotely.
 */
class LightController
{
public:
    enum
    {
        ON = ::Schema::Nest::Trait::Lighting::LogicalCircuitStateTrait::CIRCUIT_STATE_ON,
        OFF = ::Schema::Nest::Trait::Lighting::LogicalCircuitStateTrait::CIRCUIT_STATE_OFF,
    };

    LightController(void);

    WEAVE_ERROR Init(gpio_num_t gpioNum);

    int8_t GetState(void);
    uint8_t GetLevel(void);

    void Set(int8_t state, uint8_t level);
    void Toggle(void);

private:

    class LogicalCircuitStateTraitDataSource : public ::nl::Weave::Profiles::DataManagement_Current::TraitDataSource
    {
    public:
        LogicalCircuitStateTraitDataSource(LightController & lightController);

    private:
        LightController & mLightController;

        WEAVE_ERROR GetLeafData(::nl::Weave::Profiles::DataManagement_Current::PropertyPathHandle aLeafHandle, uint64_t aTagToWrite,
                        ::nl::Weave::TLV::TLVWriter & aWriter) __OVERRIDE;
    };

    class LogicalCircuitControlTraitDataSource : public ::nl::Weave::Profiles::DataManagement_Current::TraitDataSource
    {
    public:
        LogicalCircuitControlTraitDataSource(LightController & lightController);

    private:
        LightController & mLightController;

        WEAVE_ERROR GetLeafData(::nl::Weave::Profiles::DataManagement_Current::PropertyPathHandle aLeafHandle, uint64_t aTagToWrite,
                        ::nl::Weave::TLV::TLVWriter & aWriter) __OVERRIDE;

        virtual void OnCustomCommand(::nl::Weave::Profiles::DataManagement_Current::Command * aCommand,
                const nl::Weave::WeaveMessageInfo * aMsgInfo,
                ::nl::Weave::PacketBuffer * aPayload, const uint64_t & aCommandType, const bool aIsExpiryTimeValid,
                const int64_t & aExpiryTimeMicroSecond, const bool aIsMustBeVersionValid,
                const uint64_t & aMustBeVersion, ::nl::Weave::TLV::TLVReader & aArgumentReader);
    };

    LogicalCircuitStateTraitDataSource mStateDS;
    LogicalCircuitControlTraitDataSource mControlDS;
    gpio_num_t mGPIONum;
    int8_t mState;
    uint8_t mLevel;
};

inline int8_t LightController::GetState(void)
{
    return mState;
}

inline uint8_t LightController::GetLevel(void)
{
    return mLevel;
}

#endif // LIGHT_CONTROLLER_H
