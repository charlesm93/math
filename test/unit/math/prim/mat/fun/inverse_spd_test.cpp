#include <stan/math/prim/mat.hpp>
#include <gtest/gtest.h>

TEST(MathMatrix, inverse_spd_exception) {
  using stan::math::inverse_spd;

  stan::math::matrix_d m1(2,3);
  
  // non-square
  m1 << 1, 2, 3, 4, 5, 6;
  EXPECT_THROW(inverse_spd(m1),std::invalid_argument);

  stan::math::matrix_d m2(3,3);
  
  // non-symmetric
  m2 << 1, 2, 3, 4, 5, 6, 7, 8, 9;  
  EXPECT_THROW(inverse_spd(m1),std::invalid_argument);

  // not positive definite
  m2 << 1, 2, 3,
        2, 4, 5,
        3, 5, 6;
  EXPECT_THROW(inverse_spd(m1),std::invalid_argument);
}
