#include "dpl/instruments/AmericanOption.hpp"

namespace dpl {

AmericanOption::AmericanOption(OptionType type, double strike, double maturity)
    : type_(type), strike_(strike), maturity_(maturity) {}

OptionType AmericanOption::type() const { return type_; }
ExerciseStyle AmericanOption::exercise() const { return ExerciseStyle::American; }
double AmericanOption::strike() const { return strike_; }
double AmericanOption::maturity() const { return maturity_; }

}  // namespace dpl
