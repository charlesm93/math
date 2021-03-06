#ifndef STAN_MATH_TORSTEN_PKMODEL_MODELPARAMETERS_HPP
#define STAN_MATH_TORSTEN_PKMODEL_MODELPARAMETERS_HPP

#include <Eigen/Dense>
#include <stan/math/torsten/PKModel/ExtractVector.hpp>
#include <stan/math/torsten/PKModel/SearchReal.hpp>
#include <algorithm>
#include <vector>

template<typename T_time, typename T_parameters, typename T_biovar,
         typename T_tlag, typename T_system>
  class ModelParameterHistory;
/**
 * The ModelParameters class defines objects that contain the parameters of
 * a model at a given time.
 */
template<typename T_time, typename T_parameters, typename T_biovar,
         typename T_tlag, typename T_system>
class ModelParameters {
private:
  T_time time;
  std::vector<T_parameters> RealParameters;
  std::vector<T_biovar> biovar;
  std::vector<T_tlag> tlag;
  Eigen::Matrix<T_system, Eigen::Dynamic, Eigen::Dynamic> K;

public:
  ModelParameters() {
    time = 0;

    std::vector<T_parameters> p_RealParameters(1, 0);
    RealParameters = p_RealParameters;

    std::vector<T_biovar> p_biovar(1, 0);
    biovar = p_biovar;

    std::vector<T_tlag> p_tlag(1, 0);
    tlag = p_tlag;

    Eigen::Matrix<T_system, Eigen::Dynamic, Eigen::Dynamic> K_(0, 0);
    K = K_;
  }

  ModelParameters(const T_time& p_time,
                  const std::vector<T_parameters>& p_RealParameters,
                  const std::vector<T_biovar>& p_biovar,
                  const std::vector<T_tlag>& p_tlag,
                  const Eigen::Matrix<T_system, Eigen::Dynamic,
                    Eigen::Dynamic>& p_K) {
    time = p_time;
    RealParameters = p_RealParameters;
    biovar = p_biovar;
    tlag = p_tlag;
    K = p_K;
  }

  int CountParameters() const {
    return RealParameters.size();  // FIX ME - account for parameters in K,
                                   // biovar, and tlag?
  }

  void Print() {
    std::cout << time << " ";
    for (int i = 0; i < RealParameters.size(); i++)
      std::cout << RealParameters[i] << " ";
    for (int i = 0; i < biovar.size(); i++)
      std::cout << biovar[i] << " ";
    for (int i = 0; i < tlag.size(); i++)
      std::cout << tlag[i] << " ";
    if (K.rows() != 0) std::cout << K;
    std::cout << std::endl;
  }

  // access functions
  T_time get_time() const { return time; }
  std::vector<T_parameters> get_RealParameters() const {
    return RealParameters;
  }
  std::vector<T_biovar> get_biovar() const {
    return biovar;
  }
  std::vector<T_tlag> get_tlag() const {
    return tlag;
  }
  Eigen::Matrix<T_system, Eigen::Dynamic, Eigen::Dynamic> get_K() const {
    return K;
  }

  friend class ModelParameterHistory<T_time, T_parameters, T_biovar,
                                     T_tlag, T_system>;
};

/**
 * The ModelParameterHistory class defines objects that contain a vector
 * of ModelParameters, along with a series of functions that operate on
 * them.
 */
template<typename T_time, typename T_parameters, typename T_biovar,
         typename T_tlag, typename T_system>
class ModelParameterHistory{
private:
  std::vector< ModelParameters<T_time, T_parameters,
                               T_biovar, T_tlag, T_system> > MPV;

public:
  template<typename T0, typename T1, typename T2, typename T3, typename T4>
  ModelParameterHistory(std::vector<T0> p_time,
                        std::vector<std::vector<T1> > p_RealParameters,
                        std::vector<std::vector<T2> > p_biovar,
                        std::vector<std::vector<T3> > p_tlag,
                        std::vector< Eigen::Matrix<T4, Eigen::Dynamic,
                          Eigen::Dynamic> > p_K) {
    using std::max;
    int nParameters = max(p_RealParameters.size(),
                          max(p_K.size(), p_biovar.size()));
    MPV.resize(nParameters);
    int j, k, l, m;
    // FIX ME - is this an efficient way to store data?
    for (int i = 0; i < nParameters; i++) {
      (p_RealParameters.size() == 1) ? j = 0 : j = i;
      (p_biovar.size() == 1) ? k = 0 : k = i;
      (p_tlag.size() == 1) ? l = 0 : l = i;
      (p_K.size() == 1) ? m = 0 : m = i;
       MPV[i] = ModelParameters<T_time, T_parameters, T_biovar,
                                T_tlag, T_system>
         (p_time[i], p_RealParameters[j], p_biovar[k], p_tlag[l], p_K[m]);
    }
  }

  ModelParameters<T_time, T_parameters, T_biovar, T_tlag, T_system>
    GetModelParameters(int i) {
    ModelParameters<T_time, T_parameters, T_biovar, T_tlag, T_system>
      newPara(MPV[i].time, MPV[i].RealParameters, MPV[i].biovar,
              MPV[i].tlag, MPV[i].K);
    return newPara;
  }

  /**
   * MPV.size gives us the number of events.
   * MPV[i].RealParameters.size gives us the number of parameters
   * (not including additional parameters) for the first event. 
   */
  T_parameters GetValue(int iEvent, int iParameter) {
    assert((iEvent >= 0) && ((size_t) iEvent < MPV.size()));
    assert((iParameter >= 0) && ((size_t) iParameter
      < MPV[iEvent].RealParameters.size()));
    return MPV[iEvent].RealParameters[iParameter];
  }

  T_biovar GetValueBio(int iEvent, int iParameter) {
    assert(iEvent >= 0 && (size_t) iEvent < MPV.size());
    assert(iParameter >= 0 && (size_t) iParameter
             < MPV[iEvent].biovar.size());
    return MPV[iEvent].biovar[iParameter];
  }

  T_tlag GetValueTlag(int iEvent, int iParameter) {
    assert(iEvent >= 0 && (size_t) iEvent < MPV.size());
    assert(iParameter >= 0 && (size_t) iParameter
             < MPV[iEvent].tlag.size());
    return MPV[iEvent].tlag[iParameter];
  }

  void InsertModelParameters(ModelParameters<T_time, T_parameters,
    T_biovar, T_tlag, T_system> p_M) {
    MPV.push_back(p_M);
  }

  int get_size() {
    return MPV.size();
  }

  struct by_time {
    bool operator()(ModelParameters<T_time, T_parameters, T_biovar,
                                    T_tlag, T_system> const &a,
                    ModelParameters<T_time, T_parameters, T_biovar,
                                    T_tlag, T_system> const &b) {
      return a.time < b.time;
    }
  };

  void Sort() {
    std::sort(MPV.begin(), MPV.end(), by_time());
  }

  bool Check() {
  // check that elements are in chronological order.
    int i = MPV.size() - 1;
    bool ordered = true;

    while (i > 0 && ordered) {
      ordered = (MPV[i].time >= MPV[i-1].time);
      i--;
    }
    return ordered;
  }

  void Print(int j) {
    std::cout << MPV[j].time << " ";
      for (size_t i = 0; i < MPV[j].RealParameters.size(); i++)
        std::cout << MPV[j].RealParameters[i] << " ";
      for (size_t i = 0; i < MPV[j].biovar.size(); i++)
        std::cout << MPV[j].biovar[i] << " ";
      for (size_t i = 0; i < MPV[j].tlag.size(); i++)
        std::cout << MPV[j].tlag[i] << " ";
      std::cout << std::endl;
  }

  /**
   * COMPLETE MODEL PARAMETERS
   *
   * Completes parameters so that it contains model parameters for each event in events.
   * If parameters contains only one set of parameters (case where the parameters are
   * constant), this set is replicated for each event in events.
   * Otherwise a new parameter vector is added for each new event (isnew = true). This
   * parameter vector is identical to the parameter vector at the subsequent event. If the
   * new event occurs at a time posterior to the time of the last event, than the
   * new vector parameter equals the parameter vector of the last event. This amounts to
   * doing an LOCF (Last Observation Carried Forward).
   *
   * Events and Parameters are sorted at the end of the procedure.
   *
   * @param[in] parameters at each event
   * @param[in] events elements (following NONMEM convention) at each event
   * @return - modified parameters and events.
   *
   */
  template<typename T_amt, typename T_rate, typename T_ii>
  void CompleteParameterHistory(EventHistory<T_time, T_amt, T_rate,
                                T_ii>& events) {
    int nEvent = events.get_size();
    assert(nEvent > 0);
    int len_Parameters = MPV.size();  // numbers of events for which parameters
                                      // are determined
    assert(len_Parameters > 0);

    if (!Check()) Sort();
    if (!events.Check()) events.Sort();
    MPV.resize(nEvent);

    int iEvent = 0;
    for (int i = 0; i < len_Parameters - 1; i++) {
      while (events.get_isnew(iEvent)) iEvent++;  // skip new events
      assert(MPV[i].time == events.get_time(iEvent));  // compare time of "old"
                                                       // events to time of
                                                       // parameters.
      iEvent++;
    }

    if (len_Parameters == 1)  {
      for (int i = 0; i < nEvent; i++) {
        // FIX ME - inefficient data storage
        MPV[i].RealParameters = MPV[0].RealParameters;
        MPV[i].biovar = MPV[0].biovar;
        MPV[i].tlag = MPV[0].tlag;
        MPV[i].K = MPV[0].K;
        MPV[i].time = events.get_time(i);
        events.Events[i].isnew = false;
      }
    } else {  // parameters are event dependent.
      std::vector<T_time> times(nEvent, 0);
      for (int i = 0; i < nEvent; i++) times[i] = MPV[i].time;
      iEvent = 0;

      int k, j = 0;
      ModelParameters<T_time, T_parameters, T_biovar,
                      T_tlag, T_system> newParameter;

      for (int i = 0; i < nEvent; i++) {
        while (events.get_isnew(iEvent)) {
          /* Three cases:
           * (a) The time of the new event is higher than the time of the last
           *     parameter vector in parameters (k = len_parameters).
           *     Create a parameter vector at the the time of the new event,
           *     with the parameters of the last parameter vector.
           *     (Last Observation Carried Forward)
           * (b) The time of the new event matches the time of a parameter vector
           *     in parameters. This parameter vector gets replicated.
           * (c) (a) is not verified and no parameter vector occurs at the time
           *     of the new event. A new parameter vector is created at the time
           *     of the new event, and its parameters are equal to the parameters
           *     of the subsequent parameter vector in parameters.
           */
          // Find the index corresponding to the time of the new event in the
          // times vector.
          k = SearchReal(times, len_Parameters - 1, events.get_time(iEvent));

          if ((k == len_Parameters) ||
            (events.get_time(iEvent) == MPV[k - 1].time))
            newParameter = GetModelParameters(k - 1);
          else
            newParameter = GetModelParameters(k);

          newParameter.time = events.get_time(iEvent);
          MPV[len_Parameters + j] = newParameter;  // Include new element in MPV
          events.Events[iEvent].isnew = false;
          if (iEvent < nEvent - 1) iEvent++;
          j++;
        }

        if (iEvent < nEvent - 1) iEvent++;
      }
    }
    if (!Check()) Sort();
  }

  // declare friends
  friend class ModelParameters<T_time, T_parameters, T_biovar,
                               T_tlag, T_system>;
  template<typename T1, typename T2, typename T3, typename T4>
    friend class Events;
};

#endif
