
/**
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    THIS FILE IS GENERATED. DO NOT MODIFY.
 *
 *    SOURCE TEMPLATE: trait.cpp
 *    SOURCE PROTO: nest/trait/lighting/logical_circuit_state_trait.proto
 *
 */

#include <nest/trait/lighting/LogicalCircuitStateTrait.h>

namespace Schema {
namespace Nest {
namespace Trait {
namespace Lighting {
namespace LogicalCircuitStateTrait {

using namespace ::nl::Weave::Profiles::DataManagement;

//
// Property Table
//

const TraitSchemaEngine::PropertyInfo PropertyMap[] = {
    { kPropertyHandle_Root, 1 }, // state
    { kPropertyHandle_Root, 2 }, // brightness
};

//
// IsNullable Table
//

uint8_t IsNullableHandleBitfield[] = {
        0x2
};

//
// Schema
//

const TraitSchemaEngine TraitSchema = {
    {
        kWeaveProfileId,
        PropertyMap,
        sizeof(PropertyMap) / sizeof(PropertyMap[0]),
        1,
#if (TDM_EXTENSION_SUPPORT) || (TDM_VERSIONING_SUPPORT)
        2,
#endif
        NULL,
        NULL,
        NULL,
        &IsNullableHandleBitfield[0],
        NULL,
#if (TDM_EXTENSION_SUPPORT)
        NULL,
#endif
#if (TDM_VERSIONING_SUPPORT)
        NULL,
#endif
    }
};

} // namespace LogicalCircuitStateTrait
} // namespace Lighting
} // namespace Trait
} // namespace Nest
} // namespace Schema
