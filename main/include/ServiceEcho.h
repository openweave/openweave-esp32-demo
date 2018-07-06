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

#ifndef SERVICE_ECHO_H
#define SERVICE_ECHO_H

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>
#include <Weave/Profiles/echo/Next/WeaveEchoClient.h>

class ServiceEchoClient : public ::nl::Weave::Profiles::Echo_Next::WeaveEchoClient
{
public:
    WEAVE_ERROR Init(uint32_t intervalMS);

    bool ServiceAlive;

private:
    uint32_t mIntervalMS;

    static void PlatformEventHandler(const ::nl::Weave::DeviceLayer::WeaveDeviceEvent * event, intptr_t arg);
};

extern ServiceEchoClient ServiceEcho;

#endif // SERVICE_ECHO_H
