#include "dpl/market/VolSurface.hpp"

namespace dpl {

VolSurface::VolSurface(const std::vector<double>& strikes,
                       const std::vector<double>& volatilities)
    : strikes_(strikes), volatilities_(volatilities), spline_(strikes, volatilities) {}

double VolSurface::volatility(double strike) const {
    return spline_.interpolate(strike);
}

const std::vector<double>& VolSurface::strikes() const { return strikes_; }
const std::vector<double>& VolSurface::volatilities() const { return volatilities_; }

}  // namespace dpl
