#include "dpl/instruments/AsianOption.hpp"
#include "dpl/core/Validation.hpp"

#include <stdexcept>

namespace dpl {

AsianOption::AsianOption(OptionType type, double strike, double maturity,
                         int num_observations)
    : type_(type),
      strike_(strike),
      maturity_(maturity),
      num_observations_(num_observations) {
    validate_option_parameters(type, strike, maturity);
    if (num_observations_ < 1) {
        throw std::invalid_argument("Asian option requires at least one observation");
    }
}

OptionType AsianOption::type() const { return type_; }
ExerciseStyle AsianOption::exercise() const { return ExerciseStyle::European; }
double AsianOption::strike() const { return strike_; }
double AsianOption::maturity() const { return maturity_; }
int AsianOption::num_observations() const { return num_observations_; }

}  // namespace dpl
