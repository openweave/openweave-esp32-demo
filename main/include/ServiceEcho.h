#ifndef SERVICE_ECHO_H
#define SERVICE_ECHO_H

#include <WeavePlatform.h>
#include <Weave/Profiles/echo/Next/WeaveEchoClient.h>

class ServiceEchoClient : public ::nl::Weave::Profiles::Echo_Next::WeaveEchoClient
{
public:
    WEAVE_ERROR Init();
    WEAVE_ERROR Start(uint32_t delayMS, uint32_t intervalMS);
};

extern ServiceEchoClient ServiceEcho;

#endif
