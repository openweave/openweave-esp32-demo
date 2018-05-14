
/**
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    THIS FILE IS GENERATED. DO NOT MODIFY.
 *
 *    SOURCE TEMPLATE: trait.cpp.h
 *    SOURCE PROTO: nest/trait/lighting/logical_circuit_control_trait.proto
 *
 */
#ifndef _NEST_TRAIT_LIGHTING__LOGICAL_CIRCUIT_CONTROL_TRAIT_H_
#define _NEST_TRAIT_LIGHTING__LOGICAL_CIRCUIT_CONTROL_TRAIT_H_

#include <Weave/Profiles/data-management/DataManagement.h>
#include <Weave/Support/SerializationUtils.h>

#include <nest/trait/lighting/PhysicalCircuitStateTrait.h>


namespace Schema {
namespace Nest {
namespace Trait {
namespace Lighting {
namespace LogicalCircuitControlTrait {

extern const nl::Weave::Profiles::DataManagement::TraitSchemaEngine TraitSchema;

enum {
      kWeaveProfileId = (0x235aU << 16) | 0x20dU
};

//
// Commands
//

enum {
    kSetLogicalCircuitStateRequestId = 0x1,
};

enum SetLogicalCircuitStateRequestParameters {
    kSetLogicalCircuitStateRequestParameter_State = 1,
    kSetLogicalCircuitStateRequestParameter_Level = 2,
};

} // namespace LogicalCircuitControlTrait
} // namespace Lighting
} // namespace Trait
} // namespace Nest
} // namespace Schema
#endif // _NEST_TRAIT_LIGHTING__LOGICAL_CIRCUIT_CONTROL_TRAIT_H_
