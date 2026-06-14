#pragma once

namespace dpl {

enum class OptionType { Call, Put };

enum class ExerciseStyle { European, American };

enum class BarrierType { UpAndOut, DownAndOut };

struct Greeks {
    double delta = 0.0;
    double gamma = 0.0;
    double vega = 0.0;
    double theta = 0.0;
    double rho = 0.0;
};

struct PricingResult {
    double price = 0.0;
    Greeks greeks;
};

}  // namespace dpl
