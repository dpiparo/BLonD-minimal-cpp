
#include <blond/globals.h>
#include <blond/utilities.h>
#include <blond/math_functions.h>
#include <omp.h>
#include <stdio.h>
#include <blond/input_parameters/GeneralParameters.h>
#include <blond/input_parameters/RfParameters.h>
#include <blond/beams/Beams.h>
#include <blond/beams/Slices.h>
#include <blond/beams/Distributions.h>
#include <blond/trackers/Tracker.h>
#include <blond/impedances/InducedVoltage.h>
#include <gtest/gtest.h>
#include <complex>


const std::string datafiles =
   "../demos/input_files/TC5_Wake_impedance/";


//RingAndRfSection *long_tracker;
Resonators *resonator;

class testInducedVoltage : public ::testing::Test {

protected:
   // Simulation parameters --------------------------------------------------------
// Bunch parameters
   const long int N_b = (long int) 1e10;                          // Intensity
   const ftype tau_0 = 2e-9;                       // Initial bunch length, 4 sigma [s]
// const particle_type particle = proton;
// Machine and RF parameters
   const ftype C = 6911.56;                        // Machine circumference [m]
   const ftype p_i = 25.92e9;                      // Synchronous momentum [eV/c]
//const ftype p_f = 460.005e9;                  // Synchronous momentum, final
   const long h = 4620;                            // Harmonic number
   const ftype V = 0.9e6;                          // RF voltage [V]
   const ftype dphi = 0;                           // Phase modulation/offset
   const ftype gamma_t = 1 / std::sqrt(0.00192);   // Transition gamma
   const ftype alpha = 1.0 / gamma_t / gamma_t;    // First order mom. comp. factor
   const int alpha_order = 1;
   const int n_sections = 1;
// Tracking details

   int N_t = 2;    // Number of turns to track
   int N_p = 5000000;         // Macro-particles

   int N_slices = 1 << 8; // = (2^8)

   virtual void SetUp()
   {

      omp_set_num_threads(Context::n_threads);

      f_vector_2d_t momentumVec(n_sections, f_vector_t(N_t + 1, p_i));

      f_vector_2d_t alphaVec(n_sections , f_vector_t(alpha_order+1, alpha));

      f_vector_t CVec(n_sections, C);

      f_vector_2d_t hVec(n_sections , f_vector_t(N_t + 1, h));

      f_vector_2d_t voltageVec(n_sections , f_vector_t(N_t + 1, V));

      f_vector_2d_t dphiVec(n_sections , f_vector_t(N_t + 1, dphi));

	   Context::GP = new GeneralParameters(N_t, CVec, alphaVec, alpha_order,
                                 momentumVec, proton);

	   Context::Beam = new Beams(N_p, N_b);

	   Context::RfP = new RfParameters(n_sections, hVec, voltageVec, dphiVec);


      //RingAndRfSection *long_tracker = new RingAndRfSection();

      longitudinal_bigaussian(tau_0 / 4, 0, -1, false);

	   Context::Slice = new Slices(N_slices, 0, 0, 2 * constant::pi, rad);
      //util::dump(Slice->bin_centers, 10, "bin_centers\n");

      std::vector<ftype> v;
      util::read_vector_from_file(v, datafiles +
                                  "TC5_new_HQ_table.dat");
      assert(v.size() % 3 == 0);

      std::vector<ftype> R_shunt, f_res, Q_factor;

      R_shunt.reserve(v.size() / 3);
      f_res.reserve(v.size() / 3);
      Q_factor.reserve(v.size() / 3);

      for (uint i = 0; i < v.size(); i += 3) {
         f_res.push_back(v[i] * 1e9);
         Q_factor.push_back(v[i + 1]);
         R_shunt.push_back(v[i + 2] * 1e6);
      }

      resonator = new Resonators(R_shunt, f_res, Q_factor);

   }


   virtual void TearDown()
   {
      // Code here will be called immediately after each test
      // (right before the destructor).
      delete Context::GP;
      delete Context::Beam;
      delete Context::RfP;
      delete Context::Slice;
      delete resonator;
      //delete long_tracker;
   }


};



TEST_F(testInducedVoltage, InducedVoltageTime_Constructor)
{

   std::vector<Intensity *> wakeSourceList({resonator});
   //wakeSourceList.push_back(resonator);
   InducedVoltageTime *indVoltTime = new InducedVoltageTime(wakeSourceList);

   std::string params = std::string("../unit-tests/references/Impedances/")
                        + "InducedVoltage/InducedVoltageTime/";

   std::vector<ftype> v;
   util::read_vector_from_file(v, params + "time_array.txt");

   ASSERT_EQ(v.size(), indVoltTime->fTimeArray.size());

   ftype epsilon = 1e-8;

   for (unsigned int i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = indVoltTime->fTimeArray[i];
      ASSERT_NEAR(ref, real, epsilon * std::max(fabs(ref), fabs(real)))
            << "Testing of indVoltTime->fTimeArray failed on i "
            << i << std::endl;
   }
   v.clear();

   util::read_vector_from_file(v, params + "total_wake.txt");
   ASSERT_EQ(v.size(), indVoltTime->fTotalWake.size());

   epsilon = 1e-8;
   for (unsigned int i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = indVoltTime->fTotalWake[i];
      ASSERT_NEAR(ref, real, epsilon * std::max(fabs(ref), fabs(real)))
            << "Testing of indVoltTime->fTotalWake failed on i "
            << i << std::endl;
   }
   v.clear();

   util::read_vector_from_file(v, params + "cut.txt");

   for (unsigned int i = 0; i < v.size(); ++i) {
      unsigned int ref = v[i];
      unsigned int real = indVoltTime->fCut;
      ASSERT_EQ(ref, real)
            << "Testing of indVoltTime->fCut failed on i "
            << i << std::endl;
   }
   v.clear();

   util::read_vector_from_file(v, params + "fshape.txt");

   for (unsigned int i = 0; i < v.size(); ++i) {
      unsigned int ref = v[i];
      unsigned int real = indVoltTime->fShape;
      ASSERT_EQ(ref, real)
            << "Testing of fShape failed on i "
            << i << std::endl;
   }
   v.clear();

   delete indVoltTime;

}

TEST_F(testInducedVoltage, InducedVoltageTimeReprocess)
{

   std::vector<Intensity *> wakeSourceList({resonator});
   //wakeSourceList.push_back(resonator);
   InducedVoltageTime *indVoltTime = new InducedVoltageTime(wakeSourceList);
   auto Slice = Context::Slice;
   Slice->track();

   for (uint i = 0; i < Slice->n_slices; i++)
      Slice->bin_centers[i] *= 1.1;

   indVoltTime->reprocess();

   std::string params = std::string("../unit-tests/references/Impedances/")
                        + "InducedVoltage/InducedVoltageTime/reprocess/";

   std::vector<ftype> v;
   util::read_vector_from_file(v, params + "time_array.txt");

   ASSERT_EQ(v.size(), indVoltTime->fTimeArray.size());

   ftype epsilon = 1e-8;

   for (unsigned int i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = indVoltTime->fTimeArray[i];
      ASSERT_NEAR(ref, real, epsilon * std::max(fabs(ref), fabs(real)))
            << "Testing of indVoltTime->fTimeArray failed on i "
            << i << std::endl;
   }
   v.clear();

   util::read_vector_from_file(v, params + "total_wake.txt");
   ASSERT_EQ(v.size(), indVoltTime->fTotalWake.size());

   epsilon = 1e-8;
   for (unsigned int i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = indVoltTime->fTotalWake[i];
      ASSERT_NEAR(ref, real, epsilon * std::max(fabs(ref), fabs(real)))
            << "Testing of indVoltTime->fTotalWake failed on i "
            << i << std::endl;
   }
   v.clear();

   util::read_vector_from_file(v, params + "cut.txt");

   for (unsigned int i = 0; i < v.size(); ++i) {
      unsigned int ref = v[i];
      unsigned int real = indVoltTime->fCut;
      ASSERT_EQ(ref, real)
            << "Testing of indVoltTime->fCut failed on i "
            << i << std::endl;
   }
   v.clear();

   util::read_vector_from_file(v, params + "fshape.txt");

   for (unsigned int i = 0; i < v.size(); ++i) {
      unsigned int ref = v[i];
      unsigned int real = indVoltTime->fShape;
      ASSERT_EQ(ref, real)
            << "Testing of fShape failed on i "
            << i << std::endl;
   }
   v.clear();

   delete indVoltTime;

}



TEST_F(testInducedVoltage, induced_voltage_generation)
{
	Context::Slice->track();

   std::vector<Intensity *> wakeSourceList({resonator});
   InducedVoltageTime *indVoltTime = new InducedVoltageTime(wakeSourceList);
   std::vector<ftype> res = indVoltTime->induced_voltage_generation();

   std::string params = std::string("../unit-tests/references/Impedances/")
                        + "InducedVoltage/InducedVoltageTime/";

   std::vector<ftype> v;
   util::read_vector_from_file(v, params + "induced_voltage.txt");

   ASSERT_EQ(v.size(), res.size());

   ftype max = *max_element(res.begin(), res.end(),
   [](ftype i, ftype j) {return fabs(i) < fabs(j);});
   ftype epsilon = 1e-9 * max;

   for (unsigned int i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = res[i];

      ASSERT_NEAR(ref, real, epsilon)
            << "Testing of indVoltTime->fInducedVoltage failed on i "
            << i << std::endl;

   }

   delete indVoltTime;


}


TEST_F(testInducedVoltage, induced_voltage_generation_convolution)
{
	Context::Slice->track();

   std::vector<Intensity *> wakeSourceList({resonator});
   InducedVoltageTime *indVoltTime = new InducedVoltageTime(wakeSourceList, time_or_freq::time_domain);
   std::vector<ftype> res = indVoltTime->induced_voltage_generation();

   std::string params = std::string("../unit-tests/references/Impedances/")
                        + "InducedVoltage/InducedVoltageTime/";

   std::vector<ftype> v;
   util::read_vector_from_file(v, params + "induced_voltage_with_convolution.txt");

   ASSERT_EQ(v.size(), res.size());

   ftype max = *max_element(res.begin(), res.end(),
   [](ftype i, ftype j) {return fabs(i) < fabs(j);});
   ftype epsilon = 1e-9 * max;

   for (unsigned int i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = res[i];

      ASSERT_NEAR(ref, real, epsilon)
            << "Testing of indVoltTime->fInducedVoltage failed on i "
            << i << std::endl;

   }
   delete indVoltTime;


}

TEST_F(testInducedVoltage, track)
{
	auto Beam = Context::Beam;
	Context::Slice->track();

   std::vector<Intensity *> wakeSourceList({resonator});
   InducedVoltageTime *indVoltTime = new InducedVoltageTime(wakeSourceList);
   //std::vector<ftype> res = indVoltTime->induced_voltage_generation();
   indVoltTime->track();
   std::string params = std::string("../unit-tests/references/Impedances/")
                        + "InducedVoltage/InducedVoltageTime/";

   std::vector<ftype> v;
   util::read_vector_from_file(v, params + "beam_dE.txt");
   // WARNING checking only the fist 100 elems
   std::vector<ftype> res(Beam->dE.data(), Beam->dE.data() + 100);
   ASSERT_EQ(v.size(), res.size());

   ftype epsilon = 1e-8;
   // warning checking only the first 100 elems
   for (unsigned int i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = res[i];
      ASSERT_NEAR(ref, real, epsilon * std::max(fabs(ref), fabs(real)))
            << "Testing of Beam->dE failed on i "
            << i << std::endl;
   }

   v.clear();
   delete indVoltTime;

}


TEST_F(testInducedVoltage, totalInducedVoltageSum)
{
	Context::Slice->track();

   std::vector<Intensity *> wakeSourceList({resonator});
   InducedVoltageTime *indVoltTime = new InducedVoltageTime(wakeSourceList);
   //std::vector<ftype> res = indVoltTime->induced_voltage_generation();
   //indVoltTime->track();
   std::vector<InducedVoltage *> indVoltList({indVoltTime});
   TotalInducedVoltage *totVol = new TotalInducedVoltage(indVoltList);

   std::vector<ftype> res = totVol->induced_voltage_sum(200);
   auto params = std::string("../unit-tests/references/Impedances/")
                 + "InducedVoltage/TotalInducedVoltage/";

   std::vector<ftype> v;
   util::read_vector_from_file(v, params + "extIndVolt.txt");
   // WARNING checking only the fist 100 elems
   //for (auto & :)
   ASSERT_EQ(v.size(), res.size());

   ftype max = *max_element(res.begin(), res.end(),
   [](ftype i, ftype j) {return fabs(i) < fabs(j);});
   ftype epsilon = 1e-9 * max;
   // warning checking only the first 100 elems
   for (unsigned i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = res[i];

      ASSERT_NEAR(ref, real, epsilon)
            << "Testing of extIndVolt failed on i "
            << i << std::endl;
   }

   v.clear();
   res.clear();

   res = totVol->fInducedVoltage;
   util::read_vector_from_file(v, params + "induced_voltage.txt");
   // WARNING checking only the fist 100 elems
   //for (auto & :)
   ASSERT_EQ(v.size(), res.size());

   max = *max_element(res.begin(), res.end(),
   [](ftype i, ftype j) {return fabs(i) < fabs(j);});
   epsilon = 1e-9 * max;
   // warning checking only the first 100 elems
   for (unsigned i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = res[i];

      ASSERT_NEAR(ref, real, epsilon)
            << "Testing of fInducedVoltage failed on i "
            << i << std::endl;
   }

   v.clear();
   delete indVoltTime;
   delete totVol;
}


TEST_F(testInducedVoltage, totalInducedVoltageTrack)
{

	auto Beam = Context::Beam;

   std::vector<Intensity *> wakeSourceList({resonator});
   InducedVoltageTime *indVoltTime = new InducedVoltageTime(wakeSourceList);
   std::vector<InducedVoltage *> indVoltList({indVoltTime});


   TotalInducedVoltage *totVol = new TotalInducedVoltage(indVoltList);

   totVol->track();
   //std::cout << "made it here\n";

   auto params = std::string("../unit-tests/references/Impedances/")
                 + "InducedVoltage/TotalInducedVoltage/";

   std::vector<ftype> v;
   util::read_vector_from_file(v, params + "beam_dE.txt");

   // WARNING checking only the fist 100 elems
   std::vector<ftype> res(Beam->dE.data(), Beam->dE.data() + 100);
   ASSERT_EQ(v.size(), res.size());

   ftype epsilon = 1e-8;
   // warning checking only the first 100 elems
   for (unsigned int i = 0; i < v.size(); ++i) {
      ftype ref = v[i];
      ftype real = res[i];

      ASSERT_NEAR(ref, real, epsilon * std::max(fabs(ref), fabs(real)))
            << "Testing of Beam->dE failed on i "
            << i << std::endl;
   }

   delete indVoltTime;
   delete totVol;
}


int main(int ac, char *av[])
{
   ::testing::InitGoogleTest(&ac, av);
   return RUN_ALL_TESTS();
}
