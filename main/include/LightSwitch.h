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

#ifndef LIGHT_SWITCH_H
#define LIGHT_SWITCH_H

#include <Weave/Profiles/data-management/TraitData.h>
#include <nest/trait/lighting/LogicalCircuitStateTrait.h>

class LightSwitch
{
public:
    enum
    {
        ON = ::Schema::Nest::Trait::Lighting::LogicalCircuitStateTrait::CIRCUIT_STATE_ON,
        OFF = ::Schema::Nest::Trait::Lighting::LogicalCircuitStateTrait::CIRCUIT_STATE_OFF,
    };

    WEAVE_ERROR Init(uint64_t controllerNodeId);

    int8_t GetState(void);
    uint8_t GetLevel(void);

    void Set(uint8_t state, uint8_t level);
    void Toggle(void);

private:
    uint64_t mControllerNodeId;
    ::nl::Weave::Binding * mControllerBinding;
    ::nl::Weave::ExchangeContext * mCommandEC;
    int8_t mState;
    uint8_t mLevel;
    bool mChangePending;

    void SendCommand(void);
    WEAVE_ERROR EncodeCommandRequest(::nl::Weave::PacketBuffer * buf);

    static void HandleBindingEvent(void *apAppState, ::nl::Weave::Binding::EventType aEvent,
            const ::nl::Weave::Binding::InEventParam & aInParam, ::nl::Weave::Binding::OutEventParam & aOutParam);
    static void HandleSendError(::nl::Weave::ExchangeContext *ec, WEAVE_ERROR err, void *msgCtxt);
    static void HandleWRMPAckRcvd(::nl::Weave::ExchangeContext *ec, void *msgCtxt);
};

inline int8_t LightSwitch::GetState(void)
{
    return mState;
}

inline uint8_t LightSwitch::GetLevel(void)
{
    return mLevel;
}

#endif // LIGHT_SWITCH_H
