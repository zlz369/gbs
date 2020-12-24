#include <gtest/gtest.h>
#include <gbs/transform.h>
#include <gbs/gbslib.h>
#include <gbs/bscbuild.h>

using namespace gbs;
const double tol = 1e-6;
const auto PI =acos(-1);

TEST(tests_transform, transform_point)
{
    {
        gbs::point<double, 3> x{0, 0, 0};
        gbs::translate(x, {1., 0., 0.});
        ASSERT_DOUBLE_EQ(x[0], 1.);
    }

    {
        gbs::point<double, 2> x{1, 0};
        gbs::rotate(x, PI / 2);
        ASSERT_NEAR(x[0], 0.,tol);
        ASSERT_NEAR(x[1], 1.,tol);
    }

    {
        gbs::point<double, 3> x{1, 0, 0};
        gbs::rotate(x, PI / 2, {0.,0.,1.});
        ASSERT_NEAR(x[0], 0.,tol);
        ASSERT_NEAR(x[1], 1.,tol);
        ASSERT_NEAR(x[2], 0.,tol);
    }

    {
        gbs::point<double, 3> x{1, 0, 0};
        gbs::rotate(x, PI / 2, {1.,0.,0.});
        ASSERT_NEAR(x[0], 1.,tol);
        ASSERT_NEAR(x[1], 0.,tol);
        ASSERT_NEAR(x[2], 0.,tol);
    }

    {
        gbs::point<double, 3> x{1, 0, 0};
        gbs::rotate(x, PI / 2, {0.,1.,0.});
        ASSERT_NEAR(x[0], 0.,tol);
        ASSERT_NEAR(x[1], 0.,tol);
        ASSERT_NEAR(x[2],-1.,tol);
    }

}


TEST(tests_transform, transform_BSCurve)
{
    {
        auto crv = gbs::build_segment<double,3>({0.,0.,0.},{1.,0.,0.});
        gbs::translate(crv,{1.,1.,0.});
        auto x = crv(0.);
        ASSERT_NEAR(x[0], 1.,tol);
        ASSERT_NEAR(x[1], 1.,tol);
        ASSERT_NEAR(x[2], 0.,tol);
        x = crv(1.);
        ASSERT_NEAR(x[0], 2.,tol);
        ASSERT_NEAR(x[1], 1.,tol);
        ASSERT_NEAR(x[2], 0.,tol);
    }

    {
        auto crv = gbs::build_segment<double,2>({0.,0.},{1.,0.});
        gbs::rotate(crv,PI / 2);
        auto x = crv(0.);
        ASSERT_NEAR(x[0], 0.,tol);
        ASSERT_NEAR(x[1], 0.,tol);
        x = crv(1.);
        ASSERT_NEAR(x[0], 0.,tol);
        ASSERT_NEAR(x[1], 1.,tol);
    }
}