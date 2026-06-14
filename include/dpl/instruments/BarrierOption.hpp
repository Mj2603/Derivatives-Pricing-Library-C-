#pragma once

#include "dpl/core/Option.hpp"

namespace dpl {

class BarrierOption : public Option {
public:
    BarrierOption(OptionType type, double strike, double maturity,
                  double barrier, BarrierType barrier_type);

    OptionType type() const override;
    ExerciseStyle exercise() const override;
    double strike() const override;
    double maturity() const override;
    double barrier() const;
    BarrierType barrier_type() const;

private:
    OptionType type_;
    double strike_;
    double maturity_;
    double barrier_;
    BarrierType barrier_type_;
};

}  // namespace dpl
