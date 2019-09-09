
/**
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    THIS FILE IS GENERATED. DO NOT MODIFY.
 *
 *    SOURCE TEMPLATE: trait.cpp.h
 *    SOURCE PROTO: nest/trait/lighting/logical_circuit_state_trait.proto
 *
 */
#ifndef _NEST_TRAIT_LIGHTING__LOGICAL_CIRCUIT_STATE_TRAIT_H_
#define _NEST_TRAIT_LIGHTING__LOGICAL_CIRCUIT_STATE_TRAIT_H_

#include <Weave/Profiles/data-management/DataManagement.h>
#include <Weave/Support/SerializationUtils.h>



namespace Schema {
namespace Nest {
namespace Trait {
namespace Lighting {
namespace LogicalCircuitStateTrait {

extern const nl::Weave::Profiles::DataManagement::TraitSchemaEngine TraitSchema;

enum {
      kWeaveProfileId = (0x235aU << 16) | 0x237U
};

//
// Properties
//

enum {
    kPropertyHandle_Root = 1,

    //---------------------------------------------------------------------------------------------------------------------------//
    //  Name                                IDL Type                            TLV Type           Optional?       Nullable?     //
    //---------------------------------------------------------------------------------------------------------------------------//

    //
    //  state                               CircuitState                         int               NO              NO
    //
    kPropertyHandle_State = 2,

    //
    //  brightness                          uint32                               uint8             NO              YES
    //
    kPropertyHandle_Brightness = 3,

    //
    // Enum for last handle
    //
    kLastSchemaHandle = 3,
};

//
// Enums
//

enum CircuitState {
    CIRCUIT_STATE_ON = 1,
    CIRCUIT_STATE_OFF = 2,
    CIRCUIT_STATE_INCONSISTENT = 3,
};

} // namespace LogicalCircuitStateTrait
} // namespace Lighting
} // namespace Trait
} // namespace Nest
} // namespace Schema
#endif // _NEST_TRAIT_LIGHTING__LOGICAL_CIRCUIT_STATE_TRAIT_H_
