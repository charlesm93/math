#include <gtest/gtest.h>
#include <stan/agrad/fvar.hpp>
#include <stan/math/functions/lmgamma.hpp>
#include <stan/agrad/var.hpp>
#include <test/agrad/util.hpp>

TEST(AgradFvar, lmgamma) {
  using stan::agrad::fvar;
  using stan::math::lmgamma;

  int x = 3;
  fvar<double> y(3.2,2.1);

  fvar<double> a = lmgamma(x, y);
  EXPECT_FLOAT_EQ(lmgamma(3, 3.2), a.val_);
  EXPECT_FLOAT_EQ(4.9138227, a.d_);
}

TEST(AgradFvarVar, lmgamma) {
  using stan::agrad::fvar;
  using stan::agrad::var;
  using stan::math::lmgamma;

  fvar<var> x(3.2,2.1);
  fvar<var> a = lmgamma(3, x);

  EXPECT_FLOAT_EQ(lmgamma(3,3.2), a.val_.val());
  EXPECT_FLOAT_EQ(4.9138227, a.d_.val());

  AVEC y = createAVEC(x.val_);
  VEC g;
  a.val_.grad(y,g);
  EXPECT_FLOAT_EQ(4.9138227 / 2.1, g[0]);
}

TEST(AgradFvarFvar, lmgamma) {
  using stan::agrad::fvar;
  using stan::math::lmgamma;

  fvar<fvar<double> > x;
  x.val_.val_ = 3.2;
  x.val_.d_ = 2.1;

  fvar<fvar<double> > a = lmgamma(3,x);

  EXPECT_FLOAT_EQ(lmgamma(3,3.2), a.val_.val_);
  EXPECT_FLOAT_EQ(4.9138227, a.val_.d_);
  EXPECT_FLOAT_EQ(0, a.d_.val_);
  EXPECT_FLOAT_EQ(0, a.d_.d_);

  fvar<fvar<double> > y;
  y.val_.val_ = 3.2;
  y.d_.val_ = 2.1;

  a = lmgamma(3,y);
  EXPECT_FLOAT_EQ(lmgamma(3,3.2), a.val_.val_);
  EXPECT_FLOAT_EQ(0, a.val_.d_);
  EXPECT_FLOAT_EQ(4.9138227, a.d_.val_);
  EXPECT_FLOAT_EQ(0, a.d_.d_);
}
