#include "rocket.h"
#include <cmath>

Rocket::Rocket()
    : position(0.0f, 0.0f)
    , velocity(0.0f, 0.0f)
    , acceleration(0.0f, 0.0f)
    , mass(500.0f)
    , fuelMass(3000.0f)
    , thrustForce(15000.0f)
    , dragCoefficient(0.3f)
    , crossSection(1.0f)
    , engineOn(false)
    , isp(300.0f) // orta seviye
    , windForce(0.0f,0.0f)
    , thrustAngle(M_PI / 2.0f) // 90° = dik yukarı (radyan cinsinden)
    , pitchKickAlt(1000.0f) // 1000 metrede pitch kick başlasın
    , temperature(15.0f) // Başlangıçta 15 derece
    , heatFlux(0.0f)
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

    AtmosState atmos = atmosphere(altitude);
    float airDensity = atmos.density; // hava yoğunluğu (kg/m^3)

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

    // Atmosferik Isınma (motordan bağımsız — her zaman hesaplanır)
    if (airDensity > 0.001f && speed > 100.0f) {
        heatFlux = 0.5f * airDensity * std::pow(speed, 3) * 1e-8f;
        temperature += heatFlux * dt;
    } else {
        heatFlux = 0.0f;
    }

    temperature -= (temperature - 15.0f) * 0.01f * dt;

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


float Rocket::machNumber() const
{
    float alt = -position.y;
    if (alt < 0) alt = 0;

    AtmosState atmos = atmosphere(alt);

    if (atmos.soundSpeed <= 0) return 0;
    return velocity.magnitude() / atmos.soundSpeed;
}


Rocket::AtmosState Rocket::atmosphere(float alt) const
{
    const float altitude = (alt <0.0f) ? 0.0f : alt;

    AtmosState s;
    const float g0 = 9.80665f;
    const float Rair = 287.05f;
    const float gam = 1.4f;

    if (altitude < 11000.0f)
    {
        // Troposfer: -6.5 K/km
        s.temperature = 288.15f - 0.0065f * altitude; // Her metre için 6.5°C/km düşüş → ISA tanımı
        s.density = 1.225f * std::pow(s.temperature / 288.15f, 4.256f);
    }
    else if (altitude < 20000.0f)
    {
        // Alt Stratosfer: izotermal (216.65 K)
        s.temperature = 216.65f; // sabit, ozon tabakası
        s.density = 0.3639f * std::exp(-0.0001577f * (altitude - 11000.0f));
    }
    else if (altitude < 32000.0f)
    {
        // Üst Stratosfer: +1.0 K/km
        s.temperature = 216.65f + 0.001f * (altitude - 20000.0f); // Ozon ısıtıyor, sıcaklık artmaya başlar
        s.density = 0.0880f * std::pow(s.temperature / 216.65f, -35.16f); // Negatif üs → sıcaklık artarken yoğunluk hâlâ düşüyor
    }
    else if (altitude < 47000.0f)
    {
        // Stratopoz: +2.8 K/km
        s.temperature = 228.65f + 0.0028f * (altitude - 32000.0f);
        s.density     = 0.01322f * std::pow(s.temperature / 228.65f, -13.20f);
    }
    else if (altitude < 51000.0f)
    {
        // Mezosfer alt: izotermal (270.65 K)
        s.temperature = 270.65f;
        s.density     = 0.001427f * std::exp(-0.0001262f * (altitude - 47000.0f));
    }
    else if (altitude < 71000.0f)
    {
        // Mezosfer orta: -2.8 K/km
        s.temperature = 270.65f - 0.0028f * (altitude - 51000.0f);
        s.density     = 0.000861f * std::pow(s.temperature / 270.65f, 11.20f);
    }
    else if (altitude < 86000.0f)
    {
        // Mezosfer üst: -2.0 K/km
        s.temperature = 214.65f - 0.002f  * (altitude - 71000.0f);
        s.density     = 0.0000642f * std::pow(s.temperature / 214.65f, 16.08f);
    }
    else
    {
        // Termosfer: ISA burada biter — üstel azalış
        // 100 km'de (Kármán hattı) yoğunluk neredeyse sıfır
        s.temperature = 186.87f;
        s.density     = 0.00000696f * std::exp(-0.0001654f * (altitude - 86000.0f));
    }

    s.soundSpeed = std::sqrt(gam * Rair * s.temperature);
    return s;
}

