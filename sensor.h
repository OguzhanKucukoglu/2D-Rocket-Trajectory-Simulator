#ifndef SENSOR_H
#define SENSOR_H

#include "vector2d.h"
#include <random>

class Sensor
{
public:
    // Gürültülü ölçümler — gerçek değere rastgele sapma ekleme
    static float noisyAltitude(float trueAltitude);
    static float noisySpeed(float trueSpeed);
    static Vector2D noisyAcceleration(const Vector2D& trueAcceleration);

private:
    // Gaussian (Normal) dağılım ile rastgele sayı üretir
    static float gaussian(float mean, float stddev);

    // Rastgele sayı üreteci — tüm çağrılarda ortak kullanılır
    static std::mt19937& generator();
};

#endif // SENSOR_H
