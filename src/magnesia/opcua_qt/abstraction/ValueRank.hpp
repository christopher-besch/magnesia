#pragma once

#include <cstdint>

#include <QString>

namespace magnesia::opcua_qt::abstraction {
    /**
     * @class ValueRank
     * @brief Indicates whether the value attribute of the variable is an array and how many dimensions the array has.
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

    inline QString value_rank_to_string(ValueRank value_rank) {
        switch (value_rank) {
            case ValueRank::SCALAR_OR_ONE_DIMENSION:
                return "Scalar or one dimension";
            case ValueRank::ANY:
                return "Any";
            case ValueRank::SCALAR:
                return "Scalar";
            case ValueRank::ONE_OR_MORE_DIMENSION:
                return "One or more dimension";
            case ValueRank::ONE_DIMENSION:
                return "One dimension";
            case ValueRank::TWO_DIMENSIONS:
                return "Two dimensions";
            case ValueRank::THREE_DIMENSIONS:
                return "Three dimensions";
            default:
                return "";
        }
    }
} // namespace magnesia::opcua_qt::abstraction
