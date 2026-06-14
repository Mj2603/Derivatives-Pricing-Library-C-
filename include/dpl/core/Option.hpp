#pragma once

#include "dpl/core/Types.hpp"

namespace dpl {

class Option {
public:
    virtual ~Option() = default;

    virtual OptionType type() const = 0;
    virtual ExerciseStyle exercise() const = 0;
    virtual double strike() const = 0;
    virtual double maturity() const = 0;
};

}  // namespace dpl
