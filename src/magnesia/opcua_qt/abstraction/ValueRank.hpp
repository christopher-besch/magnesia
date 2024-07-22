#pragma once

#include <cstdint>

namespace magnesia::opcua_qt::abstraction {
    /**
     * Indicates whether the value attribute of the variable is an array and how many dimensions the array has.
     *
     * @see VariableNode
     *
     * See https://reference.opcfoundation.org/Core/Part3/v105/docs/5.6.2
     */
    enum class ValueRank : int32_t {
        SCALAR_OR_ONE_DIMENSION = -3,
        ANY,
        SCALAR,
        ONE_OR_MORE_DIMENSION,
        ONE_DIMENSION,
        TWO_DIMENSIONS,
        THREE_DIMENSIONS,
    };
} // namespace magnesia::opcua_qt::abstraction
