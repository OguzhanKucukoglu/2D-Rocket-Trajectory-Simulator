#include "rocket.h"
#include <cmath>

Rocket::Rocket()
    : position(0.0f, 0.0f)
    , velocity(0.0f, 0.0f)
    , acceleration(0.0f, 0.0f)
    , mass(500.0f)
    , fuelMass(300.0f)
    , thrustForce(15000.0f)
    , dragCoefficient(0.3f)
    , crossSection(1.0f)
    , engineOn(false)
    , isp(300.0f) // orta seviye
    , windForce(0.0f,0.0f)
    , thrustAngle(M_PI / 2.0f) // 90° = dik yukarı (radyan cinsinden)
    , pitchKickAlt(1000.0f) // 1000 metrede pitch kick başlasın
{
}

float Rocket::totalMass() const
{
    return mass + fuelMass;
}


bool Rocket::hasFuel() const
{
    return fuelMass > 0.0f;
}

void Rocket::update(float dt)
{
    float altitude = -position.y;
    if ( altitude < 0 ) altitude = 0; // yerin altına inmesin
    float airDensity = 1.225f * std::exp(-altitude / 8500.0f); // hava yoğunluğu (kg/m^3)

    const float R_earth = 6371000.0f; // Dünya yarıçapı (metre)
    float g = 9.81f * std::pow(R_earth / (R_earth + altitude) , 2); // Yer çekimi ivmesi

    // 1) Yerçekimi kuvveti (her zaman aşağı)
    Vector2D gravity(0.0f, totalMass() * g);

    // 2) İtki kuvveti (motor açıksa ve yakıt varsa, yukarı)
    Vector2D thrust(0.0f, 0.0f);
    if (engineOn && hasFuel())
    {

        float altitude = - position.y;

        if (altitude < pitchKickAlt)
        {
            thrustAngle = M_PI / 2.0f; // 90°
        }
        else
        {
            float speed = velocity.magnitude();
            if(speed > 0.1f)
            {
                // atan2(-vy, vx) → hız vektörünün açısı
                // -velocity.y çünkü bizim y eksenimiz ters (yukarı = negatif)
                float targetAngle = std::atan2(-velocity.y, velocity.x);
                float minAngle = 85.0f * M_PI / 180.0f;

                thrustAngle = std::fmax(0.0f, fmin(targetAngle, minAngle));
            }
        }

        // İtki vektörü
        float tx = std::cos(thrustAngle) * thrustForce;
        float ty = -std::sin(thrustAngle) * thrustForce;
        thrust = Vector2D(tx, ty);

        float fuelBornRate = thrustForce / (isp * g); // kg / s
        fuelMass -= fuelBornRate * dt;
        if ( fuelMass < 0.0f) fuelMass = 0.0f;
    }

    // 3) Sürükleme kuvveti (hıza zıt yönde)
    float speed = velocity.magnitude();
    Vector2D drag(0.0f,0.0f);

    if (speed > 0.0f)
    {
        float dragMag = 0.5f * airDensity * dragCoefficient * crossSection * speed * speed;
        drag = velocity.normalized() * (-dragMag); // hızın tersi yönünde
    }

    // 4) Net kuvvet -> ivme ( F = ma -> a = F/m )
    float airRatio = airDensity / 1.225f; // deniz seviyesine göre oran
    Vector2D netForce = gravity + thrust + drag + windForce * airRatio;
    acceleration = netForce * (1.0f / totalMass());

    // 5) Euler entegrasyonu
    velocity += acceleration * dt;
    position += velocity * dt;

    // 6) Yere çarpma kontrolü ( y pozitif = aşağı, yer = y 0)
    if(position.y > 0.0f && velocity.y > 0.0f)
    {
        position.y = 0.0f;
        velocity = Vector2D(0.0f,0.0f);
        acceleration = Vector2D(0.0f,0.0f);
        engineOn= false;
    }
}

float Rocket::soundSpeed(float altitude) const
{
    if (altitude < 11000.0f) return 340.3f - 0.004f * altitude; // Troposfer: 340 → 296 (Sıcaklık düşüyor)
    else if (altitude < 25000.0f) return 295.0f; // Stratosfer alt: sabit (Sıcaklık sabit)
    else return 295.0f + 0.001f * (altitude - 25000.0f); // Stratosfer üst: hafif artış (Ozon - sıcaklık artıyor)
}


float Rocket::machNumber() const
{
    float alt = -position.y;
    if (alt < 0) alt = 0;

    float speed = velocity.magnitude();
    float ss = soundSpeed(alt);
    if (ss <= 0) return 0;
    return speed / ss;
}

