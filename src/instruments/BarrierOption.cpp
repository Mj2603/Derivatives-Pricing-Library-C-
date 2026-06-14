#include "dpl/instruments/BarrierOption.hpp"

namespace dpl {

BarrierOption::BarrierOption(OptionType type, double strike, double maturity,
                             double barrier, BarrierType barrier_type)
    : type_(type),
      strike_(strike),
      maturity_(maturity),
      barrier_(barrier),
      barrier_type_(barrier_type) {}

OptionType BarrierOption::type() const { return type_; }
ExerciseStyle BarrierOption::exercise() const { return ExerciseStyle::European; }
double BarrierOption::strike() const { return strike_; }
double BarrierOption::maturity() const { return maturity_; }
double BarrierOption::barrier() const { return barrier_; }
BarrierType BarrierOption::barrier_type() const { return barrier_type_; }

}  // namespace dpl
