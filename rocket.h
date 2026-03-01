#ifndef ROCKET_H
#define ROCKET_H

#include "vector2d.h"

class Rocket
{
public:
    // Fiziksel Özellikler
    Vector2D position; // konum (metre)
    Vector2D velocity; // hız (metre/saniye)
    Vector2D acceleration; // ivme (metre/saniye²)
    Vector2D windForce; // rüzgar kuvveti (Newton)

    float mass; // toplam kütle (kg) — yakıt dahil
    float fuelMass; // yakıt kütlesi (kg)
    float thrustForce; // motor itki kuvveti (Newton)
    float dragCoefficient; // sürükleme katsayısı (Cd)
    float crossSection; // roketin kesit alanı (m²)
    float isp;  // Özgül itki (saniye)
    float thrustAngle; // Roketin bakış açısı (radyan cinsinden)
    float pitchKickAlt; // Pitch Kick yapılacak irtifa (metre)

    bool engineOn; // motor açık mı

    // Consturctor
    Rocket();

    // Fizik güncelleme
    void update(float dt); // dt = delta time (saniye cinsinden geçen süre)

    // Yardımcılar
    float totalMass() const;
    bool hasFuel() const;
    float soundSpeed(float altitude) const; // irtifaya bağlı ses hızı
    float machNumber() const; // Mach sayısı

};

#endif // ROCKET_H
