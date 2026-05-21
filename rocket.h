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
    float temperature; // Roketin gövde sıcaklığı (°C)
    float heatFlux; // Anlık ısı akısı (W/m²)
    bool engineOn; // motor açık mı

    // Consturctor
    Rocket();

    // Fizik güncelleme
    void update(float dt); // dt = delta time (saniye cinsinden geçen süre)

    // Yardımcılar
    float totalMass() const;
    bool hasFuel() const;
    float machNumber() const; // Mach sayısı

    struct AtmosState {
        float temperature; // Kelvin
        float density;  // kg/m^3
        float soundSpeed; // m/s
    };

    AtmosState atmosphere(float altitude) const;

};

#endif // ROCKET_H
