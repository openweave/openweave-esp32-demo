#ifndef SERVICE_ECHO_H
#define SERVICE_ECHO_H

#include <WeavePlatform.h>
#include <Weave/Profiles/echo/Next/WeaveEchoClient.h>

class ServiceEchoClient : public ::nl::Weave::Profiles::Echo_Next::WeaveEchoClient
{
public:
    WEAVE_ERROR Init(uint32_t intervalMS);

private:
    uint32_t mIntervalMS;

    static void PlatformEventHandler(const ::WeavePlatform::WeavePlatformEvent * event, intptr_t arg);
};

extern ServiceEchoClient ServiceEcho;

#endif
