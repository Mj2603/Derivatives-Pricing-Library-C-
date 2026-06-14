#include "dpl/math/NormalDistribution.hpp"

#include <cmath>

namespace dpl {

double norm_cdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double norm_pdf(double x) {
    static constexpr double inv_sqrt_2pi = 0.3989422804014327;
    return inv_sqrt_2pi * std::exp(-0.5 * x * x);
}

}  // namespace dpl
