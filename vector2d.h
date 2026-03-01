#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <cmath>

class Vector2D
{
public:
    float x;
    float y;

    // Constructor, başlangıç değerleriyle oluştur
    Vector2D(float x=0.0f, float y=0.0f);

    // Operatörler
    Vector2D operator+(const Vector2D &other) const;
    Vector2D operator-(const Vector2D &other) const;
    Vector2D operator*(float scalar) const; // vektör * sayı
    Vector2D& operator+=(const Vector2D &other);

    // Yardımcı fonksiyonlar
    float magnitude() const; // uzunluk (büyüklük)
    Vector2D normalized() const;  // birim vektör (uzunluğu 1)

};

#endif // VECTOR2D_H
