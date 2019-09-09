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
#include <nest/trait/lighting/LogicalCircuitControlTrait.h>
#include <LightSwitch.h>

using namespace ::nl::Weave;
using namespace ::nl::Weave::DeviceLayer;
using namespace ::nl::Weave::Profiles::DataManagement_Current;
using namespace ::nl::Weave::TLV;
using namespace ::Schema::Nest::Trait::Lighting;

extern const char * TAG;

WEAVE_ERROR LightSwitch::Init(uint64_t controllerNodeId)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    mControllerBinding = ::nl::Weave::DeviceLayer::ExchangeMgr.NewBinding(HandleBindingEvent, this);
    VerifyOrExit(mControllerBinding != NULL, err = WEAVE_ERROR_NO_MEMORY);

    mControllerNodeId = controllerNodeId;
    mCommandEC = NULL;
    mState = OFF;
    mLevel = 100;
    mChangePending = false;

exit:
    return err;
}

void LightSwitch::Set(uint8_t state, uint8_t level)
{
    if (state != mState || level != mLevel)
    {
        mState = state;
        mLevel = level;
        SendCommand();
    }
}

void LightSwitch::Toggle(void)
{
    mState = (mState == OFF) ? ON : OFF;
    SendCommand();
}

void LightSwitch::SendCommand(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    PacketBuffer * buf = NULL;

    if (mCommandEC != NULL)
    {
        mCommandEC->Abort();
        mCommandEC = NULL;
    }

    if (mControllerBinding->GetState() != Binding::kState_Ready)
    {
        mChangePending = true;

        if (!mControllerBinding->IsPreparing())
        {
            err = mControllerBinding->RequestPrepare();
        }

        ExitNow();
    }

    mChangePending = false;

    err = mControllerBinding->NewExchangeContext(mCommandEC);
    SuccessOrExit(err);
    mCommandEC->AppState = this;
    mCommandEC->OnAckRcvd = HandleWRMPAckRcvd;
    mCommandEC->OnSendError = HandleSendError;

    buf = PacketBuffer::New();
    VerifyOrExit(buf != NULL, err = WEAVE_ERROR_NO_MEMORY);

    err = EncodeCommandRequest(buf);
    SuccessOrExit(err);

    ESP_LOGI(TAG, "Sending LogicalCircuitControlTrait::SetLogicalCircuitState command to %016" PRIx64 " (state %s, level %" PRIu8 ")",
             mControllerNodeId, (mState == ON) ? "ON" : "OFF", mLevel);

    err = mCommandEC->SendMessage(::nl::Weave::Profiles::kWeaveProfile_WDM, kMsgType_OneWayCommand, buf);
    buf = NULL;
    SuccessOrExit(err);

exit:
    if (err != WEAVE_NO_ERROR)
    {
        if (mCommandEC != NULL)
        {
            mCommandEC->Abort();
            mCommandEC = NULL;
        }
    }
    PacketBuffer::Free(buf);
}

WEAVE_ERROR LightSwitch::EncodeCommandRequest(PacketBuffer * buf)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    TLVWriter tlvWriter;
    TLVType container;

    tlvWriter.Init(buf);

    {
        err = tlvWriter.StartContainer(AnonymousTag, kTLVType_Structure, container);
        SuccessOrExit(err);

        {
            err = tlvWriter.StartContainer(ContextTag(CustomCommand::kCsTag_Path), kTLVType_Path, container);
            SuccessOrExit(err);

            {
                err = tlvWriter.StartContainer(ContextTag(Path::kCsTag_InstanceLocator), kTLVType_Structure, container);
                SuccessOrExit(err);

                err = tlvWriter.Put(ContextTag(Path::kCsTag_TraitProfileID), (uint32_t)LogicalCircuitControlTrait::kWeaveProfileId);
                SuccessOrExit(err);

                err = tlvWriter.Put(ContextTag(Path::kCsTag_TraitInstanceID), (uint32_t)0);
                SuccessOrExit(err);

                err = tlvWriter.EndContainer(kTLVType_Path);
                SuccessOrExit(err);
            }

            err = tlvWriter.EndContainer(kTLVType_Structure);
            SuccessOrExit(err);
        }

        err = tlvWriter.Put(ContextTag(CustomCommand::kCsTag_CommandType), (uint32_t)LogicalCircuitControlTrait::kSetLogicalCircuitStateRequestId);
        SuccessOrExit(err);

        {
            err = tlvWriter.StartContainer(ContextTag(CustomCommand::kCsTag_Argument), kTLVType_Structure, container);
            SuccessOrExit(err);

            err = tlvWriter.Put(ContextTag(LogicalCircuitControlTrait::kSetLogicalCircuitStateRequestParameter_State), mState);
            SuccessOrExit(err);

            err = tlvWriter.Put(ContextTag(LogicalCircuitControlTrait::kSetLogicalCircuitStateRequestParameter_Level), mLevel);
            SuccessOrExit(err);

            err = tlvWriter.EndContainer(kTLVType_Structure);
            SuccessOrExit(err);
        }

        err = tlvWriter.EndContainer(kTLVType_NotSpecified);
        SuccessOrExit(err);
    }

    err = tlvWriter.Finalize();
    SuccessOrExit(err);

exit:
    return err;
}

void LightSwitch::HandleBindingEvent(void *apAppState, Binding::EventType aEvent, const Binding::InEventParam & aInParam, Binding::OutEventParam & aOutParam)
{
    LightSwitch * self = (LightSwitch *)apAppState;
    static const WRMPConfig wrmpConfig =
    {
        100,        // Initial Retransmit Timeout
        100,        // Active Retransmit Timeout
        100,        // ACK Piggyback Timeout
        4           // Max Restransmissions
    };

    switch (aEvent)
    {
    case Binding::kEvent_PrepareRequested:
        aOutParam.PrepareRequested.PrepareError = self->mControllerBinding->BeginConfiguration()
            .Target_NodeId(self->mControllerNodeId)
            .TargetAddress_WeaveFabric(kWeaveSubnetId_PrimaryWiFi)
            .Transport_UDP_WRM()
            .Transport_DefaultWRMPConfig(wrmpConfig)
            .Security_None()
            .PrepareBinding();
        break;

    case Binding::kEvent_BindingReady:
        if (self->mChangePending)
        {
            self->SendCommand();
        }
        break;

    default:
        Binding::DefaultEventHandler(apAppState, aEvent, aInParam, aOutParam);
        break;
    }
}

void LightSwitch::HandleSendError(::nl::Weave::ExchangeContext *ec, WEAVE_ERROR err, void *msgCtxt)
{
    LightSwitch * self = (LightSwitch *)ec->AppState;
    if (self->mCommandEC == ec)
    {
        self->mCommandEC = NULL;
    }
    ec->Abort();
}

void LightSwitch::HandleWRMPAckRcvd(::nl::Weave::ExchangeContext *ec, void *msgCtxt)
{
    HandleSendError(ec, WEAVE_NO_ERROR, msgCtxt);
}
