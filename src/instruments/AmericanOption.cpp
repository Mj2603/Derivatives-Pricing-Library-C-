#include "dpl/instruments/AmericanOption.hpp"
#include "dpl/core/Validation.hpp"

namespace dpl {

AmericanOption::AmericanOption(OptionType type, double strike, double maturity)
    : type_(type), strike_(strike), maturity_(maturity) {
    validate_option_parameters(type, strike, maturity);
}

OptionType AmericanOption::type() const { return type_; }
ExerciseStyle AmericanOption::exercise() const { return ExerciseStyle::American; }
double AmericanOption::strike() const { return strike_; }
double AmericanOption::maturity() const { return maturity_; }

}  // namespace dpl
