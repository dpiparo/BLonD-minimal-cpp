/*
* @Author: Konstantinos Iliakis
* @Date:   2016-05-04 11:51:39
* @Last Modified by:   Konstantinos Iliakis
* @Last Modified time: 2016-05-04 15:25:33
*/

#include <blond/Intensity.h>
#include <blond/constants.h>
#include <blond/math_functions.h>
#include <blond/utilities.h>

Resonators::Resonators(std::vector <ftype> &RS, std::vector <ftype> &FrequencyR,
                       std::vector <ftype> &Q) {

    /*
    *Impedance contribution from resonators, analytic formulas for both wake and
    impedance. The resonant modes (and the corresponding R and Q)
    can be inputed as a list in case of several modes.*

    *The model is the following:*

    .. math::

      Z(f) = \\frac{R}{1 + j Q \\left(\\frac{f}{f_r}-\\frac{f_r}{f}\\right)}

    .. math::

      W(t>0) = 2\\alpha R e^{-\\alpha t}\\left(\\cos{\\bar{\\omega}t} -
    \\frac{\\alpha}{\\bar{\\omega}}\\sin{\\bar{\\omega}t}\\right)

      W(0) = \\alpha R

    .. math::

      \\omega_r = 2 \\pi f_r

      \\alpha = \\frac{\\omega_r}{2Q}

      \\bar{\\omega} = \\sqrt{\\omega_r^2 - \\alpha^2}
    */

    fRS = RS;
    fFrequencyR = FrequencyR;
    fQ = Q;
    // printf("Resonator Sizes %lu %lu\n",fRS.size(), fQ.size() );
    fNResonators = RS.size();

    for (unsigned int i = 0; i < fNResonators; ++i) {
        fOmegaR.push_back(2 * constant::pi * fFrequencyR[i]);
    }
}

void Resonators::wake_calc(std::vector <ftype> &NewTimeArray) {
    /*
    * Wake calculation method as a function of time.*
    */
    fTimeArray = NewTimeArray;
    fWake.resize(fTimeArray.size());
    std::fill_n(fWake.begin(), fWake.size(), 0);
    // util::dump(&fWake[0], 10, "start wake ");

    for (uint i = 0; i < fNResonators; ++i) {
        ftype alpha = fOmegaR[i] / (2 * fQ[i]);
        ftype omega_bar = std::sqrt(fOmegaR[i] * fOmegaR[i] - alpha * alpha);
        // util::dump(&alpha, 1, "alpha ");
        // util::dump(&omega_bar, 1, "omega_bar ");

        for (uint j = 0; j < fWake.size(); ++j) {
            ftype temp = fTimeArray[j];
            // util::dump(&temp, 1, "temp ");
            int sign = (temp > 0) - (temp < 0);
            fWake[j] += (sign + 1) * fRS[i] * alpha * std::exp(-alpha * temp) *
                        (std::cos(omega_bar * temp) -
                         alpha / omega_bar * std::sin(omega_bar * temp));
        }
    }
}

void Resonators::imped_calc(std::vector <ftype> &NewFrequencyArray) {
    /*
    * Impedance calculation method as a function of frequency.*
    */

    fFreqArray = NewFrequencyArray;
    fImpedance.resize(fFreqArray.size());
    // fImpedance[0] = complex_t(0, 0);
    std::fill_n(fImpedance.begin(), fImpedance.size(), complex_t(0, 0));
    // std::cout << complex_t(1,0) / complex_t(1,1) << '\n';
    for (uint i = 0; i < fNResonators; ++i) {
        for (uint j = 1; j < fImpedance.size(); ++j) {
            fImpedance[j] +=
                    complex_t(fRS[i], 0) /
                    complex_t(1, fQ[i] * (fFreqArray[j] / fFrequencyR[i] -
                                          fFrequencyR[i] / fFreqArray[j]));
        }
    }
}

InputTable::InputTable(std::vector <ftype> &input1, std::vector <ftype> &input2,
                       std::vector <ftype> input3) {
    if (input3.empty()) {

        fTimeArray = input1;
        fWakeArray = input2;

    } else {
        fFrequencyArrayLoaded = input1;
        fReZArrayLoaded = input2;
        fImZArrayLoaded = input3;
        assert(fReZArrayLoaded.size() == fImZArrayLoaded.size());

        for (uint i = 0; i < fReZArrayLoaded.size(); ++i) {
            complex_t z(fReZArrayLoaded[i], fImZArrayLoaded[i]);
            fImpedanceLoaded.push_back(z);
        }

        if (fFrequencyArrayLoaded[0] != 0) {

            fFrequencyArrayLoaded.insert(fFrequencyArrayLoaded.begin(), 0);
            fReZArrayLoaded.insert(fReZArrayLoaded.begin(), 0);
            fImZArrayLoaded.insert(fImZArrayLoaded.begin(), 0);
        }
    }
}

void InputTable::wake_calc(std::vector <ftype> &NewTimeArray) {
    mymath::lin_interp(NewTimeArray, fTimeArray, fWakeArray, fWake, 0.0f, 0.0f);
}

void InputTable::imped_calc(std::vector <ftype> &NewFrequencyArray) {
    // Impedance calculation method as a function of frequency.*
    std::vector <ftype> ReZ;
    std::vector <ftype> ImZ;

    mymath::lin_interp(NewFrequencyArray, fFrequencyArrayLoaded,
                       fReZArrayLoaded, ReZ, 0.0f, 0.0f);

    mymath::lin_interp(NewFrequencyArray, fFrequencyArrayLoaded,
                       fImZArrayLoaded, ImZ, 0.0f, 0.0f);

    fFreqArray = NewFrequencyArray;

    // Initializing real and imaginary part separately has been
    // omitted
    fImpedance.resize(ReZ.size());
    for (uint i = 0; i < ReZ.size(); ++i) {
        fImpedance[i] = complex_t(ReZ[i], ImZ[i]);
    }
}
