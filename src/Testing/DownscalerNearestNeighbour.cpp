#include "../Util.h"
#include "../File/File.h"
#include "../Downscaler/Downscaler.h"
#include <gtest/gtest.h>

namespace {
   class TestDownscalerNearestNeighbour : public ::testing::Test {
      public:
         vec2 makeVec2(int nLat, int nLon, const std::vector<float>& values) {
            vec2 grid;
            grid.resize(nLat);
            for(int i = 0; i < nLat; i++) {
               grid[i].resize(nLon);
               for(int j = 0; j < nLon; j++) {
                  int index = i*nLon + j;
                  grid[i][j] = values[index];
               }
            }
            return grid;
         };
         void setLatLon(FileFake& iFile, float iLat[], float iLon[]) {
            vec2 lat;
            vec2 lon;
            int nLat = iFile.getNumLat(); 
            int nLon = iFile.getNumLon();
            lat.resize(nLat);
            lon.resize(nLat);
            for(int i = 0; i < nLat; i++) {
               lat[i].resize(nLon);
               lon[i].resize(nLon);
               for(int j = 0; j < nLon; j++) {
                  lat[i][j] = iLat[i];
                  lon[i][j] = iLon[j];
               }
            }
            iFile.setLats(lat);
            iFile.setLons(lon);
         };
      protected:
   };

   TEST_F(TestDownscalerNearestNeighbour, isValid) {
      FileFake from(3,2,1,1);
      FileFake to(2,2,1,1);
      setLatLon(from, (float[]) {50,55,60}, (float[]){0,10});
      setLatLon(to,   (float[]) {40, 54.99},   (float[]){-1,9.99});

      vec2Int I, J, If, Jf;
      DownscalerNearestNeighbour::getNearestNeighbour(from, to, I, J);
      DownscalerNearestNeighbour::getNearestNeighbourFast(from, to, If, Jf);
      EXPECT_EQ(I, If);
      EXPECT_EQ(J, Jf);

      ASSERT_EQ(2, I.size());
      ASSERT_EQ(2, I[0].size());

      EXPECT_EQ(0, I[0][0]);
      EXPECT_EQ(0, J[0][0]);
      EXPECT_EQ(0, I[0][1]);
      EXPECT_EQ(1, J[0][1]);
      EXPECT_EQ(1, I[1][0]);
      EXPECT_EQ(0, J[1][0]);
      EXPECT_EQ(1, I[1][1]);
      EXPECT_EQ(1, J[1][1]);
   }
   TEST_F(TestDownscalerNearestNeighbour, missingLatLon) {
      FileFake from(3,2,1,1);
      FileFake to(2,2,1,1);
      setLatLon(from, (float[]) {50,Util::MV,60}, (float[]){0,Util::MV});
      setLatLon(to,   (float[]) {40, 54},   (float[]){-1,Util::MV});

      vec2Int I, J, If, Jf;
      DownscalerNearestNeighbour::getNearestNeighbour(from, to, I, J);
      DownscalerNearestNeighbour::getNearestNeighbourFast(from, to, If, Jf);
      EXPECT_EQ(I, If);
      EXPECT_EQ(J, Jf);

      ASSERT_EQ(2, I.size());
      ASSERT_EQ(2, I[0].size());

      EXPECT_EQ(0, I[0][0]);
      EXPECT_EQ(0, J[0][0]);
      EXPECT_EQ(Util::MV, I[0][1]);
      EXPECT_EQ(Util::MV, J[0][1]);
      EXPECT_EQ(0, I[1][0]);
      EXPECT_EQ(0, J[1][0]);
      EXPECT_EQ(Util::MV, I[1][1]);
      EXPECT_EQ(Util::MV, J[1][1]);
   }
   TEST_F(TestDownscalerNearestNeighbour, identicalGrid) {
      int nLat = 3;
      int nLon = 4;
      FileFake from(nLat,nLon,1,1);
      FileFake to(nLat,nLon,1,1);
      setLatLon(from, (float[]) {50,55,60}, (float[]){0,3,5,10});
      setLatLon(to,   (float[]) {50,55,60}, (float[]){0,3,5,10});

      vec2Int I, J, If, Jf;
      DownscalerNearestNeighbour::getNearestNeighbour(from, to, I, J);
      DownscalerNearestNeighbour::getNearestNeighbourFast(from, to, If, Jf);
      EXPECT_EQ(I, If);
      EXPECT_EQ(J, Jf);

      ASSERT_EQ(nLat, I.size());
      ASSERT_EQ(nLon, I[0].size());

      for(int i = 0; i < nLat; i++) {
         for(int j = 0; j < nLon; j++) {
            EXPECT_EQ(i, I[i][j]);
            EXPECT_EQ(j, J[i][j]);
         }
      }
   }
   TEST_F(TestDownscalerNearestNeighbour, unsortedGrid) {
      FileFake from(3,2,1,1);
      FileFake to(2,2,1,1);
      setLatLon(from, (float[]) {60,50,55}, (float[]){5,4});
      setLatLon(to,   (float[]) {56,49},    (float[]){3,4.6});

      vec2Int I, J, If, Jf;
      DownscalerNearestNeighbour::getNearestNeighbour(from, to, I, J);
      DownscalerNearestNeighbour::getNearestNeighbourFast(from, to, If, Jf);
      EXPECT_EQ(I, If);
      EXPECT_EQ(J, Jf);

      ASSERT_EQ(2, I.size());
      ASSERT_EQ(2, I[0].size());

      EXPECT_EQ(2, I[0][0]);
      EXPECT_EQ(1, J[0][0]);
      EXPECT_EQ(2, I[0][0]);
      EXPECT_EQ(0, J[0][1]);
      EXPECT_EQ(1, I[1][0]);
      EXPECT_EQ(1, J[1][0]);
      EXPECT_EQ(1, I[1][0]);
      EXPECT_EQ(0, J[1][1]);
   }
   TEST_F(TestDownscalerNearestNeighbour, description) {
      DownscalerNearestNeighbour::description();
   }
   TEST_F(TestDownscalerNearestNeighbour, downscale) {
      DownscalerNearestNeighbour d(Variable::T);
      FileFake from(3,2,1,1);
      FileFake to(2,2,1,1);
      setLatLon(from, (float[]) {60,50,55}, (float[]){5,4});
      setLatLon(to,   (float[]) {56,49},    (float[]){3,4.6});
      d.downscale(from, to);
      const Field& fromT = *from.getField(Variable::T, 0);
      const Field& toT   = *to.getField(Variable::T, 0);
      EXPECT_FLOAT_EQ(fromT[2][1][0], toT[0][0][0]);
      EXPECT_FLOAT_EQ(fromT[2][0][0], toT[0][1][0]);
      EXPECT_FLOAT_EQ(fromT[1][1][0], toT[1][0][0]);
      EXPECT_FLOAT_EQ(fromT[1][0][0], toT[1][1][0]);
   }
   TEST_F(TestDownscalerNearestNeighbour, 10x10) {
      DownscalerNearestNeighbour d(Variable::T);
      FileArome from("testing/files/10x10.nc");
      const Field& fromT  = *from.getField(Variable::T, 0);
      FileFake to(2,2,1,1);
      setLatLon(to,   (float[]) {5,11},    (float[]){2,3});
      d.downscale(from, to);
      const Field& toT   = *to.getField(Variable::T, 0);
      ASSERT_EQ(2, toT.size());
      ASSERT_EQ(2, toT[0].size());
      EXPECT_FLOAT_EQ(301, toT[0][0][0]);
      EXPECT_FLOAT_EQ(302, toT[0][1][0]);
      EXPECT_FLOAT_EQ(309, toT[1][0][0]);
      EXPECT_FLOAT_EQ(301, toT[1][1][0]);
      vec2Int I, J;
      d.getNearestNeighbour(from, to, I, J);
      EXPECT_EQ(9, I[1][1]);
      EXPECT_EQ(3, J[1][1]);
      EXPECT_FLOAT_EQ(301, fromT[9][3][0]);
   }
}
int main(int argc, char **argv) {
     ::testing::InitGoogleTest(&argc, argv);
       return RUN_ALL_TESTS();
}