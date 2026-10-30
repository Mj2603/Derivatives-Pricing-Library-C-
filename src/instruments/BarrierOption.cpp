#include "dpl/instruments/BarrierOption.hpp"
#include "dpl/core/Validation.hpp"

#include <cmath>
#include <stdexcept>

namespace dpl {

BarrierOption::BarrierOption(OptionType type, double strike, double maturity,
                             double barrier, BarrierType barrier_type)
    : type_(type),
      strike_(strike),
      maturity_(maturity),
      barrier_(barrier),
      barrier_type_(barrier_type) {
    validate_option_parameters(type, strike, maturity);
    if (!std::isfinite(barrier) || barrier <= 0.0) {
        throw std::invalid_argument("barrier must be finite and positive");
    }
    if (barrier_type != BarrierType::UpAndOut &&
        barrier_type != BarrierType::DownAndOut) {
        throw std::invalid_argument("unsupported barrier type");
    }
}

OptionType BarrierOption::type() const { return type_; }
ExerciseStyle BarrierOption::exercise() const { return ExerciseStyle::European; }
double BarrierOption::strike() const { return strike_; }
double BarrierOption::maturity() const { return maturity_; }
double BarrierOption::barrier() const { return barrier_; }
BarrierType BarrierOption::barrier_type() const { return barrier_type_; }

}  // namespace dpl
