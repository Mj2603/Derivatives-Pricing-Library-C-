#pragma once

#include "dpl/core/Option.hpp"

namespace dpl {

class EuropeanOption : public Option {
public:
    EuropeanOption(OptionType type, double strike, double maturity);

    OptionType type() const override;
    ExerciseStyle exercise() const override;
    double strike() const override;
    double maturity() const override;

private:
    OptionType type_;
    double strike_;
    double maturity_;
};

}  // namespace dpl
