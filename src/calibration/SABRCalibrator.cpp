#include "dpl/calibration/SABRCalibrator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace dpl {

namespace {

// z / x(z) with x(z) = ln[(sqrt(1 - 2 rho z + z^2) + z - rho) / (1 - rho)].
double z_over_x(double z, double rho) {
    if (std::abs(z) < 1e-4) {
        // x(z) = int_0^z (1 - 2 rho t + t^2)^(-1/2) dt = sum_n P_n(rho) z^(n+1) / (n+1)
        // (Legendre generating function), inverted to third order.
        return 1.0 - 0.5 * rho * z + (2.0 - 3.0 * rho * rho) / 12.0 * z * z
               + rho * (5.0 - 6.0 * rho * rho) / 24.0 * z * z * z;
    }
    const double s = std::sqrt(1.0 - 2.0 * rho * z + z * z);
    const double s_minus_one = (z * z - 2.0 * rho * z) / (s + 1.0);
    double x;
    if (z - rho >= 0.0) {
        // (s + z - rho) / (1 - rho) - 1 = (s - 1 + z) / (1 - rho)
        x = std::log1p((s_minus_one + z) / (1.0 - rho));
    } else {
        // s + z - rho = (1 - rho^2) / (s - z + rho), avoiding cancellation for z << 0
        x = std::log1p((z - s_minus_one) / (s - z + rho));
    }
    return z / x;
}

void validate_sabr_inputs(const SABRParams& p, double forward, double strike, double maturity) {
    if (!std::isfinite(forward) || forward <= 0.0) {
        throw std::invalid_argument("SABR forward must be finite and positive");
    }
    if (!std::isfinite(strike) || strike <= 0.0) {
        throw std::invalid_argument("SABR strike must be finite and positive");
    }
    if (!std::isfinite(maturity) || maturity < 0.0) {
        throw std::invalid_argument("SABR maturity must be finite and non-negative");
    }
    if (!std::isfinite(p.alpha) || p.alpha <= 0.0) {
        throw std::invalid_argument("SABR alpha must be finite and positive");
    }
    if (!(p.beta >= 0.0 && p.beta <= 1.0)) {
        throw std::invalid_argument("SABR beta must be in [0, 1]");
    }
    if (!(p.rho > -1.0 && p.rho < 1.0)) {
        throw std::invalid_argument("SABR rho must be in (-1, 1)");
    }
    if (!std::isfinite(p.nu) || p.nu < 0.0) {
        throw std::invalid_argument("SABR nu must be finite and non-negative");
    }
}

double logistic(double w) { return 1.0 / (1.0 + std::exp(-w)); }

using Vec3 = std::array<double, 3>;

struct Transform {
    double beta;
    double rho_bound;
    double nu_max;

    SABRParams to_params(const Vec3& u) const {
        return {std::exp(u[0]), beta, rho_bound * std::tanh(u[1]), nu_max * logistic(u[2])};
    }

    Vec3 from_params(double alpha, double rho, double nu) const {
        const double r = std::clamp(rho / rho_bound, -0.999999, 0.999999);
        const double n = std::clamp(nu / nu_max, 1e-9, 1.0 - 1e-9);
        return {std::log(alpha), std::atanh(r), std::log(n / (1.0 - n))};
    }
};

double mean_squared_error(const std::vector<SABRMarketQuote>& quotes, const SABRParams& p) {
    double sum = 0.0;
    for (const auto& q : quotes) {
        const double v = sabr_implied_vol(p, q.forward, q.strike, q.maturity);
        if (!std::isfinite(v)) {
            return std::numeric_limits<double>::max();
        }
        const double e = v - q.market_vol;
        sum += e * e;
    }
    return sum / static_cast<double>(quotes.size());
}

// Solve sabr_implied_vol(alpha; rho, nu) = target at one quote by safeguarded secant.
double solve_alpha(const SABRMarketQuote& q, double beta, double rho, double nu) {
    const double scale = std::pow(q.forward * q.strike, 0.5 * (1.0 - beta));
    double a0 = q.market_vol * scale;
    double a1 = 1.05 * a0;
    auto f = [&](double a) {
        return sabr_implied_vol({a, beta, rho, nu}, q.forward, q.strike, q.maturity)
               - q.market_vol;
    };
    double f0 = f(a0);
    double f1 = f(a1);
    for (int i = 0; i < 50 && std::abs(f1) > 1e-14; ++i) {
        if (f1 == f0) {
            break;
        }
        double a2 = a1 - f1 * (a1 - a0) / (f1 - f0);
        if (!std::isfinite(a2) || a2 <= 0.0) {
            a2 = 0.5 * a1;
        }
        a0 = a1;
        f0 = f1;
        a1 = a2;
        f1 = f(a1);
    }
    return std::isfinite(a1) && a1 > 0.0 ? a1 : q.market_vol * scale;
}

struct NelderMeadResult {
    Vec3 x;
    double f;
    int iterations;
    bool converged;
};

template <typename F>
NelderMeadResult nelder_mead(F&& f, const Vec3& start, const Vec3& step, int max_iter,
                             double tol) {
    constexpr int n = 3;
    std::array<Vec3, n + 1> pts;
    std::array<double, n + 1> vals;
    pts[0] = start;
    for (int i = 0; i < n; ++i) {
        pts[i + 1] = start;
        pts[i + 1][i] += step[i];
    }
    for (int i = 0; i <= n; ++i) {
        vals[i] = f(pts[i]);
    }

    int iter = 0;
    bool converged = false;
    for (; iter < max_iter; ++iter) {
        std::array<int, n + 1> idx{0, 1, 2, 3};
        std::sort(idx.begin(), idx.end(), [&](int a, int b) { return vals[a] < vals[b]; });
        std::array<Vec3, n + 1> sp;
        std::array<double, n + 1> sv;
        for (int i = 0; i <= n; ++i) {
            sp[i] = pts[idx[i]];
            sv[i] = vals[idx[i]];
        }
        pts = sp;
        vals = sv;

        double size = 0.0;
        for (int i = 1; i <= n; ++i) {
            for (int j = 0; j < n; ++j) {
                size = std::max(size, std::abs(pts[i][j] - pts[0][j]));
            }
        }
        if (vals[n] - vals[0] <= tol * std::max(1.0, std::abs(vals[0])) || size < 1e-12) {
            converged = true;
            break;
        }

        Vec3 centroid{0.0, 0.0, 0.0};
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                centroid[j] += pts[i][j] / n;
            }
        }
        auto along = [&](double t) {
            Vec3 p;
            for (int j = 0; j < n; ++j) {
                p[j] = centroid[j] + t * (pts[n][j] - centroid[j]);
            }
            return p;
        };

        const Vec3 xr = along(-1.0);
        const double fr = f(xr);
        if (fr < vals[0]) {
            const Vec3 xe = along(-2.0);
            const double fe = f(xe);
            if (fe < fr) {
                pts[n] = xe;
                vals[n] = fe;
            } else {
                pts[n] = xr;
                vals[n] = fr;
            }
        } else if (fr < vals[n - 1]) {
            pts[n] = xr;
            vals[n] = fr;
        } else {
            const bool outside = fr < vals[n];
            const Vec3 xc = along(outside ? -0.5 : 0.5);
            const double fc = f(xc);
            if (fc < (outside ? fr : vals[n])) {
                pts[n] = xc;
                vals[n] = fc;
            } else {
                for (int i = 1; i <= n; ++i) {
                    for (int j = 0; j < n; ++j) {
                        pts[i][j] = pts[0][j] + 0.5 * (pts[i][j] - pts[0][j]);
                    }
                    vals[i] = f(pts[i]);
                }
            }
        }
    }

    const auto best = std::min_element(vals.begin(), vals.end()) - vals.begin();
    return {pts[best], vals[best], iter, converged};
}

}  // namespace

double sabr_implied_vol(const SABRParams& params, double forward, double strike,
                        double maturity) {
    validate_sabr_inputs(params, forward, strike, maturity);

    const double alpha = params.alpha;
    const double beta = params.beta;
    const double rho = params.rho;
    const double nu = params.nu;
    const double omb = 1.0 - beta;

    const double log_fk = std::log(forward / strike);
    const double fk_pow = std::pow(forward * strike, 0.5 * omb);  // (FK)^((1-beta)/2)
    const double log2 = log_fk * log_fk;
    const double omb2 = omb * omb;
    const double denom = fk_pow * (1.0 + omb2 / 24.0 * log2 + omb2 * omb2 / 1920.0 * log2 * log2);

    const double z = nu / alpha * fk_pow * log_fk;
    const double smile = z_over_x(z, rho);

    const double time_correction =
        1.0
        + (omb2 / 24.0 * alpha * alpha / (fk_pow * fk_pow) + 0.25 * rho * beta * nu * alpha / fk_pow
           + (2.0 - 3.0 * rho * rho) / 24.0 * nu * nu)
              * maturity;

    return alpha / denom * smile * time_correction;
}

SABRCalibrationResult calibrate_sabr(const std::vector<SABRMarketQuote>& quotes, double beta,
                                     const SABRCalibrationOptions& options) {
    if (quotes.size() < 3) {
        throw std::invalid_argument("SABR calibration requires at least 3 quotes");
    }
    if (!(beta >= 0.0 && beta <= 1.0)) {
        throw std::invalid_argument("SABR beta must be in [0, 1]");
    }
    if (!(options.rho_bound > 0.0 && options.rho_bound < 1.0) || !(options.nu_max > 0.0)
        || options.max_iterations <= 0 || options.restarts < 0) {
        throw std::invalid_argument("invalid SABR calibration options");
    }
    for (const auto& q : quotes) {
        if (!std::isfinite(q.market_vol) || q.market_vol <= 0.0 || !std::isfinite(q.forward)
            || q.forward <= 0.0 || !std::isfinite(q.strike) || q.strike <= 0.0
            || !std::isfinite(q.maturity) || q.maturity < 0.0) {
            throw std::invalid_argument("SABR quotes need positive forward, strike and vol");
        }
    }

    const Transform tr{beta, options.rho_bound, options.nu_max};
    auto objective = [&](const Vec3& u) { return mean_squared_error(quotes, tr.to_params(u)); };

    // Quote closest to ATM anchors alpha for each (rho, nu) seed.
    const auto atm = std::min_element(quotes.begin(), quotes.end(), [](const auto& a, const auto& b) {
        return std::abs(std::log(a.strike / a.forward)) < std::abs(std::log(b.strike / b.forward));
    });

    Vec3 best_u{};
    double best_f = std::numeric_limits<double>::infinity();
    for (double rho : {-0.6, -0.3, 0.0, 0.3, 0.6}) {
        for (double nu : {0.1, 0.3, 0.6, 1.0}) {
            const double r = std::clamp(rho, -0.95 * options.rho_bound, 0.95 * options.rho_bound);
            const double n = std::min(nu, 0.9 * options.nu_max);
            const double alpha = solve_alpha(*atm, beta, r, n);
            const Vec3 u = tr.from_params(alpha, r, n);
            const double f = objective(u);
            if (f < best_f) {
                best_f = f;
                best_u = u;
            }
        }
    }

    int total_iter = 0;
    bool converged = false;
    for (int run = 0; run <= options.restarts; ++run) {
        const NelderMeadResult nm =
            nelder_mead(objective, best_u, {0.1, 0.2, 0.3}, options.max_iterations, options.tolerance);
        total_iter += nm.iterations;
        converged = nm.converged;
        if (nm.f <= best_f) {
            best_f = nm.f;
            best_u = nm.x;
        }
    }

    return {tr.to_params(best_u), std::sqrt(best_f), total_iter, converged};
}

}  // namespace dpl
