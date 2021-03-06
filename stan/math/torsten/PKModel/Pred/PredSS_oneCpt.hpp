#ifndef STAN_MATH_TORSTEN_PKMODEL_PRED_PREDSS_ONECPT_HPP
#define STAN_MATH_TORSTEN_PKMODEL_PRED_PREDSS_ONECPT_HPP

#include <stan/math/torsten/PKModel/Pred/PolyExp.hpp>
#include <iostream>
#include <vector>

/**
 * One compartment model with first-order absorption.
 * Calculate amount in each compartment at the end of a steady-state dosing interval
 * or during a steady-state constant input (if ii=0)
 *
 * If the initial time equals the time of the event, than the code does
 * not run the ode integrator, and sets the predicted amount equal to the
 * initial condition. This can happen when we are dealing with events that
 * occur simultaneously. The change to the predicted amount caused by bolus
 * dosing events is handled later in the main Pred function.
 *
 * @tparam T_time type of scalar for time
 * @tparam T_amt type of scalar for amount
 * @tparam T_rate type of scalar for rate
 * @tparam T_ii type of scalar for interdose interval
 * @tparam T_parameters type of scalar for model parameters
 * @tparam T_addParm type of scalar for additional model parameters
 * @param[in] parameter model parameters at current event
 * @param[in] rate
 * @param[in] ii interdose interval
 * @param[in] cmt compartment in which the event occurs
 * @return an eigen vector that contains predicted amount in each compartment
 *   at the current event.
 */
template<typename T_time, typename T_amt, typename T_rate, typename T_ii,
         typename T_parameters, typename T_biovar, typename T_tlag,
         typename T_system>
Eigen::Matrix<typename boost::math::tools::promote_args< T_time, T_amt, T_rate,
  typename boost::math::tools::promote_args< T_ii, T_parameters>
  ::type>::type, 1, Eigen::Dynamic>
PredSS_one(const ModelParameters<T_time, T_parameters, T_biovar,
                                 T_tlag, T_system>& parameter,
           const T_amt& amt,
           const T_rate& rate,
           const T_ii& ii,
           const int& cmt) {
  typedef typename boost::math::tools::promote_args< T_time, T_amt, T_rate,
    typename boost::math::tools::promote_args< T_ii, T_parameters>
      ::type>::type scalar;

  T_parameters CL = parameter.get_RealParameters()[0],
    V2 = parameter.get_RealParameters()[1],
    ka = parameter.get_RealParameters()[2],
    k10 = CL/V2;

  std::vector<scalar> alpha(2, 0);
  alpha[0] = k10;
  alpha[1] = ka;

  Eigen::Matrix<scalar, 1, Eigen::Dynamic> pred
    = Eigen::Matrix<scalar, 1, Eigen::Dynamic>::Zero(2);
  std::vector<scalar> a(2, 0);
  if (rate == 0) {  // bolus dose
    if (cmt == 1) {
      a[0] = 0;
      a[1] = 1;
      pred(0, 0) = PolyExp(ii, amt, 0, 0, ii, true, a, alpha, 2);
      a[0] = ka / (ka - alpha[0]);
      a[1] = -a[0];
      pred(0, 1) = PolyExp(ii, amt, 0, 0, ii, true, a, alpha, 2);
    } else {  // cmt=2
      a[0] = 1;
      pred(0, 1) = PolyExp(ii, amt, 0, 0, ii, true, a, alpha, 1);
    }
  } else if (ii > 0) {  // multiple truncated infusions
      if (cmt == 1) {
        a[0] = 0;
        a[1] = 1;
        pred(0, 0) = PolyExp(ii, 0, rate, amt / rate, ii, true, a, alpha, 2);
        a[0] = ka / (ka - alpha[0]);
        a[1] = -a[0];
        pred(0, 1) = PolyExp(ii, 0, rate, amt / rate, ii, true, a, alpha, 2);
    } else {  // cmt = 2
      a[0] = 1;
      pred(0, 1) = PolyExp(ii, 0, rate, amt / rate, ii, true, a, alpha, 1);
    }
  } else {  // constant infusion
    if (cmt == 1) {
      a[0] = 0;
      a[1] = 1;
      pred(0, 0) = PolyExp(0, 0, rate, 0, 0, true, a, alpha, 2);
      a[0] = ka / (ka - alpha[0]);
      a[1] = -a[0];
      pred(0, 1) = PolyExp(0, 0, rate, 0, 0, true, a, alpha, 2);
    } else {  // cmt = 2
      a[0] = 1;
      pred(0, 1) = PolyExp(0, 0, rate, 0, 0, true, a, alpha, 1);
    }
  }
  return pred;
}

#endif
