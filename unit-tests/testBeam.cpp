#include <iostream>
#include <string>
#include <list>

#include <unistd.h>

#include <gtest/gtest.h>
#include "math_functions.h"
#include "utilities.h"
#include "../beams/Distributions.h"
#include "../input_parameters/GeneralParameters.h"
#include "constants.h"

const ftype epsilon = 1e-8;
const std::string statistics_params = "../unit-tests/references/Beam/Beam_statistics_params/";
const std::string long_cut_params = "../unit-tests/references/Beam/Beam_long_cut_params/";
const std::string energy_cut_params = "../unit-tests/references/Beam/Beam_energy_cut_params/";

GeneralParameters *GP;
Beams *Beam;
RfParameters *RfP;

class testBeam : public ::testing::Test {

protected:
    const long N_b = 1e9;           // Intensity
    const ftype tau_0 = 0.4e-9;          // Initial bunch length, 4 sigma [s]

    virtual void SetUp() {
        ftype *momentum = new ftype[N_t + 1];
        mymath::linspace(momentum, p_i, p_f, N_t + 1);

        ftype *alpha_array = new ftype[(alpha_order + 1) * n_sections];

        std::fill_n(alpha_array, (alpha_order + 1) * n_sections, alpha);

        ftype *C_array = new ftype[n_sections];
        std::fill_n(C_array, n_sections, C);

        ftype *h_array = new ftype[n_sections * (N_t + 1)];
        std::fill_n(h_array, (N_t + 1) * n_sections, h);

        ftype *V_array = new ftype[n_sections * (N_t + 1)];
        std::fill_n(V_array, (N_t + 1) * n_sections, V);

        ftype *dphi_array = new ftype[n_sections * (N_t + 1)];
        std::fill_n(dphi_array, (N_t + 1) * n_sections, dphi);

        GP = new GeneralParameters(N_t, C_array, alpha_array, alpha_order, momentum,
                                   proton);

        Beam = new Beams(N_p, N_b);

        RfP = new RfParameters(n_sections, h_array, V_array, dphi_array);

        longitudinal_bigaussian(tau_0 / 4, 0, 1, false);
        Beam->statistics();

    }


    virtual void TearDown() {
        // Code here will be called immediately after each test
        // (right before the destructor).
        delete GP;
        delete Beam;
        delete RfP;
    }


private:

    // Machine and RF parameters
    const ftype C = 26658.883;          // Machine circumference [m]
    const long p_i = 450e9;          // Synchronous momentum [eV/c]
    const ftype p_f = 460.005e9;          // Synchronous momentum, final
    const long h = 35640;          // Harmonic number
    const ftype V = 6e6;          // RF voltage [V]
    const ftype dphi = 0;          // Phase modulation/offset
    const ftype gamma_t = 55.759505;          // Transition gamma
    const ftype alpha = 1.0 / gamma_t / gamma_t; // First order mom. comp. factor
    const int alpha_order = 1;
    const int n_sections = 1;
    // Tracking details

    const int N_t = 2000;    // Number of turns to track
    const int N_p = 100;         // Macro-particles
    const int N_slices = 10;


};



TEST_F(testBeam, test_sigma_dE) {
    //putenv("FIXED_PARTICLES=1");

    std::vector<ftype> v;

    read_vector_from_file(v, statistics_params + "sigma_dE");
    ftype ref = v[0];
    ftype real = Beam->sigma_dE;
    ASSERT_NEAR(ref, real, epsilon * std::min(v[0], real));
}

TEST_F(testBeam, test_sigma_dt) {
    //putenv("FIXED_PARTICLES=1");
    std::vector<ftype> v;

    read_vector_from_file(v, statistics_params + "sigma_dt");
    ftype ref = v[0];
    ftype real = Beam->sigma_dt;
    ASSERT_NEAR(ref, real, epsilon * std::min(v[0], real));
}

TEST_F(testBeam, test_mean_dE) {
    //putenv("FIXED_PARTICLES=1");
    std::vector<ftype> v;

    read_vector_from_file(v, statistics_params + "mean_dE");
    ftype ref = v[0];
    ftype real = Beam->mean_dE;
    ASSERT_NEAR(ref, real, epsilon * std::min(v[0], real));
}

TEST_F(testBeam, test_mean_dt) {
    //putenv("FIXED_PARTICLES=1");
    std::vector<ftype> v;

    read_vector_from_file(v, statistics_params + "mean_dt");
    ftype ref = v[0];
    ftype real = Beam->mean_dt;
    ASSERT_NEAR(ref, real, epsilon * std::min(v[0], real));
}

TEST_F(testBeam, test_epsn_rms_l) {
    //putenv("FIXED_PARTICLES=1");
    std::vector<ftype> v;

    read_vector_from_file(v, statistics_params + "epsn_rms_l");
    ftype ref = v[0];
    ftype real = Beam->epsn_rms_l;
    ASSERT_NEAR(ref, real, epsilon * std::min(v[0], real));
}

TEST_F(testBeam, test_macroparticles_lost) {
    //putenv("FIXED_PARTICLES=1");
    std::vector<ftype> v;

    read_vector_from_file(v, statistics_params + "n_macroparticles_lost");
    ftype ref = v[0];
    ftype real = Beam->n_macroparticles_lost;
    ASSERT_NEAR(ref, real, epsilon * std::min(v[0], real));
}

TEST_F(testBeam, test_losses_long_cut) {

    Beam->losses_longitudinal_cut(Beam->dt, Beam->mean_dt,
                                  10 * fabs(Beam->mean_dt), Beam->id);

    std::vector<ftype> v;
    read_vector_from_file(v, long_cut_params + "id");
    for (unsigned int i = 0; i < v.size(); ++i)
    {
        ftype ref = v[i];
        ftype real = Beam->id[i];
        ASSERT_NEAR(ref, real, epsilon * std::min(ref, real));
    }
}

TEST_F(testBeam, test_losses_energy_cut) {

    Beam->losses_longitudinal_cut(Beam->dE, Beam->mean_dE,
                                  10 * fabs(Beam->mean_dE), Beam->id);

    std::vector<ftype> v;
    read_vector_from_file(v, energy_cut_params + "id");
    for (unsigned int i = 0; i < v.size(); ++i)
    {
        ftype ref = v[i];
        ftype real = Beam->id[i];
        ASSERT_NEAR(ref, real, epsilon * std::min(ref, real));
    }
}


// TODO continue with testing the cuts


int main(int ac, char* av[]) {
    ::testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}