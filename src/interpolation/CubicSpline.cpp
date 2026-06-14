#include "dpl/interpolation/CubicSpline.hpp"

#include <cmath>
#include <stdexcept>

namespace dpl {

namespace {

std::vector<double> solve_tridiagonal_system(const std::vector<double>& lower,
                                             const std::vector<double>& diag,
                                             const std::vector<double>& upper,
                                             const std::vector<double>& rhs) {
    const int n = static_cast<int>(rhs.size());
    std::vector<double> c_prime(n), d_prime(n), x(n);

    c_prime[0] = upper[0] / diag[0];
    d_prime[0] = rhs[0] / diag[0];

    for (int i = 1; i < n; ++i) {
        const double denom = diag[i] - lower[i - 1] * c_prime[i - 1];
        if (i < n - 1) {
            c_prime[i] = upper[i] / denom;
        }
        d_prime[i] = (rhs[i] - lower[i - 1] * d_prime[i - 1]) / denom;
    }

    x[n - 1] = d_prime[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = d_prime[i] - c_prime[i] * x[i + 1];
    }
    return x;
}

}  // namespace

CubicSpline::CubicSpline(const std::vector<double>& x, const std::vector<double>& y) : x_(x) {
    const int n = static_cast<int>(x.size()) - 1;
    if (n < 1 || y.size() != x.size()) {
        throw std::invalid_argument("CubicSpline requires at least two points");
    }

    a_ = y;
    b_.resize(n);
    c_.resize(n + 1);
    d_.resize(n);

    std::vector<double> h(n);
    for (int i = 0; i < n; ++i) {
        h[i] = x[i + 1] - x[i];
    }

    std::vector<double> alpha(n - 1);
    for (int i = 1; i < n; ++i) {
        alpha[i - 1] = 3.0 / h[i] * (a_[i + 1] - a_[i]) - 3.0 / h[i - 1] * (a_[i] - a_[i - 1]);
    }

    std::vector<double> lower(n - 1, 1.0);
    std::vector<double> diag(n - 1, 2.0);
    std::vector<double> upper(n - 1, 1.0);

    for (int i = 0; i < n - 1; ++i) {
        diag[i] *= (h[i] + h[i + 1]);
    }

    c_[0] = 0.0;
    c_[n] = 0.0;

    if (n > 1) {
        std::vector<double> rhs(n - 1);
        for (int i = 0; i < n - 1; ++i) {
            rhs[i] = alpha[i];
        }
        const auto solved = solve_tridiagonal_system(lower, diag, upper, rhs);
        for (int i = 1; i < n; ++i) {
            c_[i] = solved[i - 1];
        }
    }

    for (int i = 0; i < n; ++i) {
        b_[i] = (a_[i + 1] - a_[i]) / h[i] - h[i] * (c_[i + 1] + 2.0 * c_[i]) / 3.0;
        d_[i] = (c_[i + 1] - c_[i]) / (3.0 * h[i]);
    }
}

double CubicSpline::interpolate(double x) const {
    const int n = static_cast<int>(x_.size()) - 1;

    if (x <= x_[0]) {
        return a_[0];
    }
    if (x >= x_[n]) {
        return a_[n];
    }

    int lo = 0;
    int hi = n;
    while (hi - lo > 1) {
        const int mid = (lo + hi) / 2;
        if (x_[mid] <= x) {
            lo = mid;
        } else {
            hi = mid;
        }
    }

    const double dx = x - x_[lo];
    return a_[lo] + b_[lo] * dx + c_[lo] * dx * dx + d_[lo] * dx * dx * dx;
}

}  // namespace dpl
