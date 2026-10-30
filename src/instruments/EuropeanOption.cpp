#include "dpl/instruments/EuropeanOption.hpp"
#include "dpl/core/Validation.hpp"

namespace dpl {

EuropeanOption::EuropeanOption(OptionType type, double strike, double maturity)
    : type_(type), strike_(strike), maturity_(maturity) {
    validate_option_parameters(type, strike, maturity);
}

OptionType EuropeanOption::type() const { return type_; }
ExerciseStyle EuropeanOption::exercise() const { return ExerciseStyle::European; }
double EuropeanOption::strike() const { return strike_; }
double EuropeanOption::maturity() const { return maturity_; }

}  // namespace dpl
