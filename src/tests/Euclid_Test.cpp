#include "BaseTestCase.h"

#include <cmath>
#include "tools/euclid.h"

/*static inline int _Vector3_cmp(Vector3 v1, Vector3 v2)
{
    return fabs(v1.x - v2.z) >= 0.00000000001 && fabs(v1.y - v2.y) >= 0.00000000001 && fabs(v1.z - v2.z) >= 0.00000000001;
}

static inline void _Vector3_str(Vector3 v, char* buffer, int length)
{
    snprintf(buffer, length, "(%f,%f,%f)", v.x, v.y, v.z);
}
DEFINE_COMPARE_ASSERT(Vector3, _Vector3_cmp, _Vector3_str);*/

TEST(Euclid, get2DAngle)
{
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.0, 0.0), 0.0);

    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.1, 0.0), 0.0);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(1.0, 0.0), 0.0);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(2.0, 0.0), 0.0);

    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.0, 0.1), M_PI_2);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.0, 1.0), M_PI_2);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.0, 2.0), M_PI_2);

    EXPECT_DOUBLE_EQ(euclidGet2DAngle(-0.1, 0.0), M_PI);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(-1.0, 0.0), M_PI);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(-2.0, 0.0), M_PI);

    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.0, -0.1), 3.0 * M_PI_2);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.0, -1.0), 3.0 * M_PI_2);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.0, -2.0), 3.0 * M_PI_2);

    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.5, 0.5), M_PI_4);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(0.5, -0.5), 7.0 * M_PI_4);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(-0.5, 0.5), 3.0 * M_PI_4);
    EXPECT_DOUBLE_EQ(euclidGet2DAngle(-0.5, -0.5), 5.0 * M_PI_4);
}

/*TEST(Euclid, Vector3)
{
    Vector3 v1, v2, v3;

    // Test scaling
    v1.x = 0.0;
    v1.y = -4.3;
    v1.z = 8.2;
    v2.x = 0.0;
    v2.y = -8.6;
    v2.z = 16.4;
    v3.x = 0.0;
    v3.y = 2.15;
    v3.z = -4.1;
    EXPECT_EQ(v3Scale(v1, 2.0), v2);
    EXPECT_EQ(v3Scale(v1, -0.5), v3);
}*/

TEST(Euclid, VectorSpherical)
{
    Vector3 v1;
    VectorSpherical v2;

    /* Test conversion to spherical */
    v1.x = 0.0;
    v1.y = 0.0;
    v1.z = 0.0;
    v2 = v3ToSpherical(v1);
    EXPECT_DOUBLE_EQ(v2.r, 0.0);
    EXPECT_DOUBLE_EQ(v2.phi, 0.0);
    EXPECT_DOUBLE_EQ(v2.theta, 0.0);

    v1.x = 1.0;
    v1.y = 0.0;
    v1.z = 0.0;
    v2 = v3ToSpherical(v1);
    EXPECT_DOUBLE_EQ(v2.r, 1.0);
    EXPECT_DOUBLE_EQ(v2.phi, 0.0);
    EXPECT_DOUBLE_EQ(v2.theta, 0.0);

    v1.x = -1.0;
    v1.y = 0.0;
    v1.z = 0.0;
    v2 = v3ToSpherical(v1);
    EXPECT_DOUBLE_EQ(v2.r, 1.0);
    EXPECT_DOUBLE_EQ(v2.phi, M_PI);
    EXPECT_DOUBLE_EQ(v2.theta, 0.0);

    v1.x = 0.0;
    v1.y = 1.0;
    v1.z = 0.0;
    v2 = v3ToSpherical(v1);
    EXPECT_DOUBLE_EQ(v2.r, 1.0);
    EXPECT_DOUBLE_EQ(v2.phi, 0.0);
    EXPECT_DOUBLE_EQ(v2.theta, M_PI_2);

    v1.x = 0.0;
    v1.y = -1.0;
    v1.z = 0.0;
    v2 = v3ToSpherical(v1);
    EXPECT_DOUBLE_EQ(v2.r, 1.0);
    EXPECT_DOUBLE_EQ(v2.phi, 0.0);
    EXPECT_DOUBLE_EQ(v2.theta, -M_PI_2);

    v1.x = 0.0;
    v1.y = 0.0;
    v1.z = 1.0;
    v2 = v3ToSpherical(v1);
    EXPECT_DOUBLE_EQ(v2.r, 1.0);
    EXPECT_DOUBLE_EQ(v2.phi, 3.0 * M_PI_2);
    EXPECT_DOUBLE_EQ(v2.theta, 0.0);

    v1.x = 0.0;
    v1.y = 0.0;
    v1.z = -1.0;
    v2 = v3ToSpherical(v1);
    EXPECT_DOUBLE_EQ(v2.r, 1.0);
    EXPECT_DOUBLE_EQ(v2.phi, M_PI_2);
    EXPECT_DOUBLE_EQ(v2.theta, 0.0);

    v1.x = v1.y = v1.z = 0.5;
    v2 = v3ToSpherical(v1);
    EXPECT_DOUBLE_EQ(v2.r, sqrt(0.5 * 0.5 + 0.5 * 0.5 + 0.5 * 0.5));
    EXPECT_DOUBLE_EQ(v2.phi, 7.0 * M_PI_4);
    EXPECT_DOUBLE_EQ(v2.theta, M_PI_2 - 0.955316618125);
}
