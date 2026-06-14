#pragma once

#include "dpl/core/Option.hpp"

namespace dpl {

class AsianOption : public Option {
public:
    AsianOption(OptionType type, double strike, double maturity, int num_observations);

    OptionType type() const override;
    ExerciseStyle exercise() const override;
    double strike() const override;
    double maturity() const override;
    int num_observations() const;

private:
    OptionType type_;
    double strike_;
    double maturity_;
    int num_observations_;
};

}  // namespace dpl
