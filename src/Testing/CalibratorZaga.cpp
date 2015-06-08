#include "../File/Fake.h"
#include "../Util.h"
#include "../ParameterFile.h"
#include "../Calibrator/Zaga.h"
#include <gtest/gtest.h>

namespace {
   class TestCalibratorZaga : public ::testing::Test {
      protected:
         TestCalibratorZaga() {
         }
         virtual ~TestCalibratorZaga() {
         }
         virtual void SetUp() {
         }
         virtual void TearDown() {
         }
         void test(CalibratorZaga& cal, File& file, float i0, float i1, float i2, float e0, float e1, float e2) {
            FieldPtr field  = file.getField(Variable::Precip, 0);
            (*field)(0,0,0) = i0;
            (*field)(0,0,1) = i1;
            (*field)(0,0,2) = i2;
            cal.calibrate(file);
            const FieldPtr after  = file.getField(Variable::Precip, 0);

            EXPECT_FLOAT_EQ(e0, (*after)(0,0,0));
            EXPECT_FLOAT_EQ(e1, (*after)(0,0,1));
            EXPECT_FLOAT_EQ(e2, (*after)(0,0,2));
         }
         void testP0(float ensMean, float ensFrac, float par4, float par5, float par6, float par7, float expected) {
            std::vector<float> parValues(8, 0);
            parValues[4] = par4;
            parValues[5] = par5;
            parValues[6] = par6;
            parValues[7] = par7;
            Parameters par(parValues);
            float p0 = CalibratorZaga::getP0(ensMean, ensFrac, par);
            EXPECT_FLOAT_EQ(expected, p0);
         }
         void testInvCdf(float iQuantile, float iEnsMean, float iEnsFrac, Parameters iParameters, float iExpected) {
            float invCdf = CalibratorZaga::getInvCdf(iQuantile, iEnsMean, iEnsFrac, iParameters);
            EXPECT_FLOAT_EQ(iExpected, invCdf);
         }
         Parameters getParameters(float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8) {
            std::vector<float> parValues(8, 0);
            parValues[0] = a1;
            parValues[1] = a2;
            parValues[2] = a3;
            parValues[3] = a4;
            parValues[4] = a5;
            parValues[5] = a6;
            parValues[6] = a7;
            parValues[7] = a8;
            return Parameters (parValues);
         }
         ParameterFile getParameterFile(float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8) {
            ParameterFile parFile("testing/files/parameters.txt");
            parFile.setParameters(getParameters(a1, a2, a3, a4, a5, a6, a7, a8), 0);
            return parFile;
         }
         CalibratorZaga getCalibrator(ParameterFile* parFile, const Options& iOptions=Options("")) {
            return CalibratorZaga(parFile, Variable::Precip, iOptions);
         }
   };

   TEST_F(TestCalibratorZaga, small) {
      // Set up file
      FileFake file(1, 1, 3, 1);
      // Set up calibrator
      ParameterFile parFile = getParameterFile(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      CalibratorZaga cal = getCalibrator(&parFile, Options("fracThreshold=0.5"));

      // High precip case: mean = 5.667 frac(x <= 0.5) = 0
      test(cal, file, 12, 1, 4, 6.6280001, 1.0158194, 3.0753615);
      // Low precip case: mean = 0.3333 frac(x <= 0.5) = 0.3333
      test(cal, file, 0.1, 0, 0.9, 0, 0, 0.59979528);

      test(cal, file, 0, 0.5, 0, 0,0.13598996,0); // P0 = 94%

   }
   TEST_F(TestCalibratorZaga, maxEnsMean) {
      // Set up file
      FileFake file(1, 1, 3, 1);
      // Set up calibrator
      ParameterFile parFile = getParameterFile(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      CalibratorZaga cal = getCalibrator(&parFile, Options("fracThreshold=0.5 maxEnsMean=4"));
      // High precip case: Mean is at capped at 4mm
      test(cal, file, 12, 1, 4, 5.0678229, 0.50604898, 2.1251636);
      // Low precip case: Mean isn't capped
      test(cal, file, 0.1, 0, 0.9, 0, 0, 0.59979528);
   }
   TEST_F(TestCalibratorZaga, missingEnsemble) {
      // Set up file
      FileFake file(1, 1, 3, 1);
      // Set up calibrator
      ParameterFile parFile = getParameterFile(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      CalibratorZaga cal = getCalibrator(&parFile, Options("fracThreshold=0.5"));

      test(cal, file, Util::MV, 0.5, 0.5, Util::MV,0.5,0.5);
      test(cal, file, Util::MV, Util::MV, Util::MV, Util::MV,Util::MV,Util::MV);
   }
   TEST_F(TestCalibratorZaga, missingParameters) {
      // Set up file
      FileFake file(1, 1, 3, 1);
      // Set up calibrator
      ParameterFile parFile = getParameterFile(-1.1,1.4,0.05,-0.05, 2.03, -0.05, Util::MV, -2.71);
      CalibratorZaga cal = getCalibrator(&parFile, Options("fracThreshold=0.5"));

      test(cal, file, 12, 0.5, 0.5, 12,0.5,0.5);
      test(cal, file, Util::MV, 0.5, 0.5, Util::MV,0.5,0.5);
      test(cal, file, Util::MV, Util::MV, Util::MV, Util::MV,Util::MV,Util::MV);
   }
   TEST_F(TestCalibratorZaga, description) {
      CalibratorZaga::description();
   }
   TEST_F(TestCalibratorZaga, p0) {
      testP0(5, 0, 0, 0, 0, 0, 0.5);
      testP0(5, 0.5, 0, 0, 0, 0, 0.5);
   }
   TEST_F(TestCalibratorZaga, getInvCdf) {
      Parameters par = getParameters(1,0,1,0,0,0,0,0);
      for(int i = 0; i <= 5; i++) {
         testInvCdf((float) i / 10, 0, 0, par, 0);
      }
   }
   TEST_F(TestCalibratorZaga, getFracThreshold) {
      ParameterFile parFile = getParameterFile(-1.1,1.4,0.05,-0.05, 2.03, -0.05, Util::MV, -2.71);
      CalibratorZaga cal = getCalibrator(&parFile);
      // Check that default is valid
      EXPECT_GE(cal.getFracThreshold(), 0);
   }
   TEST_F(TestCalibratorZaga, getInvCdfComplicated) {
      Parameters par = getParameters(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      // High precip case
      testInvCdf(0.003, 3, 0.7, par, 0);
      testInvCdf(0.188, 3, 0.7, par, 0);
      testInvCdf(0.19, 3, 0.7, par, 0.0068841949);
      testInvCdf(0.5, 3, 0.7, par, 1.3596177);
      testInvCdf(0.8, 3, 0.7, par, 3.4923909);
      testInvCdf(0.99999, 3, 0.7, par, 24.551832);

      // Low precip case
      testInvCdf(0,     0.4, 0.1, par, 0);
      testInvCdf(0.52,  0.4, 0.1, par, 0);
      testInvCdf(0.732, 0.4, 0.1, par, 0.5198217);
      testInvCdf(0.8,   0.4, 0.1, par, 0.797209);

      // No precip case
      testInvCdf(0,     0, 0, par, 0);
      testInvCdf(0.5,   0, 0, par, 0);
      testInvCdf(0.732, 0, 0, par, 0);
      testInvCdf(0.88,  0, 0, par, 0);
      testInvCdf(0.95,  0, 0, par, 0.27228063);
   }
   TEST_F(TestCalibratorZaga, getInvCdfInvalid) {
      Parameters par = getParameters(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(Util::MV, 15, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(Util::MV, 15, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(0.5, Util::MV, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(0.5, 15, Util::MV, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(0.5, -1, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(0.5, 15, -0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(0.5, 15, 1.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(-1, 15, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(1, 15, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(1.01, 15, 0.3, par));
      par = getParameters(-1.1,1.4,0.05,-0.05, 2.03, Util::MV, 0.82, -2.71);
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(0, 15, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getInvCdf(0.5, 15, 0.3, par));
   }
   /* This would be nice, but not yet implemented
   TEST_F(TestCalibratorZaga, getInvCdfExtreme) {
      // Extremes should produce valid values
      Parameters par = getParameters(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      EXPECT_NE(Util::MV, CalibratorZaga::getInvCdf(0.5, 100, 0.1, par));
      EXPECT_NE(Util::MV, CalibratorZaga::getInvCdf(0.5, 123981, 0.1, par));
      EXPECT_NE(Util::MV, CalibratorZaga::getInvCdf(0.1, 123981, 1, par));
      EXPECT_NE(Util::MV, CalibratorZaga::getInvCdf(0.99999, 123981, 1, par));
   }
   */
   TEST_F(TestCalibratorZaga, getP0Invalid) {
      Parameters par = getParameters(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getP0(Util::MV, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getP0(15, Util::MV, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getP0(-1, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getP0(15, -0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getP0(15, 1.3, par));
      par = getParameters(-1.1,Util::MV,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getP0(15, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getP0(0, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getP0(0, 0, par));
   }
   TEST_F(TestCalibratorZaga, outputPop) {
      // Set up file
      FileFake file(2, 2, 1, 1);
      // Set up calibrator
      ParameterFile parFile = getParameterFile(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      FieldPtr precip  = file.getField(Variable::Precip, 0);
      // In the neighbourhood around point (0,0):
      // Frac(precip <= 0.4) = 0.75 mean = 0.2
      (*precip)(0,0,0) = 0.1;
      (*precip)(0,1,0) = 0;
      (*precip)(1,0,0) = 0.5;
      (*precip)(1,1,0) = 0.2;
      CalibratorZaga cal(&parFile, Variable::Precip, Options("outputPop=1 neighbourhoodSize=1 fracThreshold=0.4 popThreshold=0.5"));
      cal.calibrate(file);
      FieldPtr pop  = file.getField(Variable::Pop, 0);
      // mu    = exp(-1.1 + 1.4 * (0.2)^0.33333) = 0.754824
      // sigma = exp(0.05 + -0.05 * 0.2) = 1.040811
      // nu    = inv.logit(2.03 -0.05*0.2 + 0.82 * 0.75 -2.71*(0.2)^0.33333) = 0.7408083
      // Compute 1-CDF at 0.5 mm using R:
      // require(gamlss)
      // require(boot)
      // 1-pZAGA(0.5, 0.754824, 1.040811, 0.7408083)
      // 0.1306302
      EXPECT_FLOAT_EQ(0.13062906, (*pop)(0,0,0));
   }
   TEST_F(TestCalibratorZaga, outputPop6h) {
      // Set up file
      FileFake file(2, 2, 1, 6);
      // Set up calibrator
      ParameterFile parFile = getParameterFile(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      // In the neighbourhood around point (0,0):
      // Frac(precip <= 0.4) = 0.75 mean = 0.2
      for(int t = 0; t < 6; t++) {
         FieldPtr precip  = file.getField(Variable::Precip, t);
         (*precip)(0,0,0) = 0.1/6;
         (*precip)(0,1,0) = 0;
         (*precip)(1,0,0) = 0.5/6;
         (*precip)(1,1,0) = 0.2/6;
      }
      CalibratorZaga cal(&parFile, Variable::Precip, Options("outputPop=1 neighbourhoodSize=1 fracThreshold=0.4 popThreshold=0.5 6h=1"));
      cal.calibrate(file);
      FieldPtr pop  = file.getField(Variable::Pop6h, 5);
      // mu    = exp(-1.1 + 1.4 * (0.2)^0.33333) = 0.754824
      // sigma = exp(0.05 + -0.05 * 0.2) = 1.040811
      // nu    = inv.logit(2.03 -0.05*0.2 + 0.82 * 0.75 -2.71*(0.2)^0.33333) = 0.7408083
      // Compute 1-CDF at 0.5 mm using R:
      // require(gamlss)
      // require(boot)
      // 1-pZAGA(0.5, 0.754824, 1.040811, 0.7408083)
      // 0.1306302
      EXPECT_FLOAT_EQ(0.13062906, (*pop)(0,0,0));
      pop  = file.getField(Variable::Pop6h, 0);
      EXPECT_FLOAT_EQ(Util::MV, (*pop)(0,0,0));
   }
   TEST_F(TestCalibratorZaga, invalid) {
      ::testing::FLAGS_gtest_death_test_style = "threadsafe";
      Util::setShowError(false);
      ParameterFile parFile = getParameterFile(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      EXPECT_DEATH(CalibratorZaga(&parFile, Variable::Precip, Options("neighbourhoodSize=-1")), ".*");
   }
   TEST_F(TestCalibratorZaga, valid) {
      ParameterFile parFile = getParameterFile(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      // Negative thresholds is fine
      CalibratorZaga(&parFile, Variable::Precip, Options("fracThreshold=-1"));
      CalibratorZaga(&parFile, Variable::Precip, Options("popThreshold=-1"));
   }
   TEST_F(TestCalibratorZaga, getCdf) {
      Parameters par = getParameters(-1.1,1.4,0.05,-0.05, 2.03, -0.05, 0.82, -2.71);
      // CDF for negative values should be 0
      EXPECT_FLOAT_EQ(0, CalibratorZaga::getCdf(-1, 0.4, 0.3, par));

      // CDF at 0 should equal getP0
      EXPECT_FLOAT_EQ(CalibratorZaga::getP0(0.4, 0.3, par), CalibratorZaga::getCdf(0,  0.4, 0.3, par));
      EXPECT_FLOAT_EQ(CalibratorZaga::getP0(0.2, 0.1, par), CalibratorZaga::getCdf(0,  0.2, 0.1, par));

      // General CDF check
      // mean = 0.4; frac=0.3;
      // mu = exp(-1.1 + 1.4 * (mean)^0.33333);
      // sigma = exp(0.05 + -0.05 * mean);
      // nu = inv.logit(2.03 -0.05*mean + 0.82 * frac -2.71*(mean)^0.33333);
      // pZAGA(0.5, mean, sigma, nu)
      EXPECT_FLOAT_EQ(0.74966329, CalibratorZaga::getCdf(0.5, 0.4, 0.3, par));
      EXPECT_FLOAT_EQ(0.9982819,  CalibratorZaga::getCdf(1.2, 0, 1, par));
      EXPECT_FLOAT_EQ(0.4181222,  CalibratorZaga::getCdf(0.2, 1, 0, par));

      // Missing values
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(Util::MV, 0.4, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(1, Util::MV, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(1, 0.4, Util::MV, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(Util::MV, Util::MV, Util::MV, par));

      // Invalid ensemble mean
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(1,        -0.1, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(-0.1,     -0.1, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(Util::MV, -0.1, 0.3, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(Util::MV, -0.1, -0.1, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(1,        -0.1, -0.1, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(1,        -0.1, 1.1, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(1,        -0.1, Util::MV, par));

      // Invalid ensemble fraction
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(1,        0.4, 1.1, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(-0.1,     0.4, 1.1, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(Util::MV, 0.4, 1.1, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(1,        Util::MV, 1.1, par));
      EXPECT_FLOAT_EQ(Util::MV, CalibratorZaga::getCdf(1,        0.4, -1.1, par));
   }
   // Testing that options are set in constructor Done in Calibrator.cpp:
}
int main(int argc, char **argv) {
     ::testing::InitGoogleTest(&argc, argv);
       return RUN_ALL_TESTS();
}
