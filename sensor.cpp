#include "sensor.h"

std::mt19937& Sensor::generator()
{
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

float Sensor::gaussian(float mean, float stddev)
{
    std::normal_distribution<float> dist(mean, stddev);
    return dist(generator());
}

float Sensor::noisyAltitude(float trueAltitude)
{
    return trueAltitude + gaussian(0.0f, 20.0f);
}

float Sensor::noisySpeed(float trueSpeed)
{
    return trueSpeed + gaussian(0.0f, 3.0f);
}

Vector2D Sensor::noisyAcceleration(const Vector2D& trueAcceleration)
{
    return Vector2D(
        trueAcceleration.x + gaussian(0.0f, 0.5f),
        trueAcceleration.y + gaussian(0.0f, 0.5f));
}
