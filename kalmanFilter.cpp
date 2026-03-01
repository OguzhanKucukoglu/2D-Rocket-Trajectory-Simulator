#include "kalmanFilter.h"

KalmanFilter::KalmanFilter(float initialState, float initialError, float processNoise, float measurementNoise)
    : state(initialState)
    , uncertainty(initialError)
    , Q(processNoise)
    , R(measurementNoise)
{
}

void KalmanFilter::predict(float velocity, float dt)
{
    // Durum tahmini: Önceki konum + hız * zaman
    state = state + velocity * dt;

    // Belirsizlik artar (Çünkü zaman geçtikçe fizik modelinde hata birikir)
    uncertainty = uncertainty + Q;
}

void KalmanFilter::update(float measurement)
{
    // Kalman Kazancı (Kalman Gain): Sensöre mi daha çok güvenelim, modele mi?
    // Belirsizlik yüksekse (K -> 1), sensöre daha çok güveniriz.
    // Sensör gürültüsü (R) yüksekse (K -> 0), modele daha çok güveniriz.
    float K = uncertainty / (uncertainty + R);

    // Tahmini düzelt: Modele olan güvenimizle sensör ölçümünü harmanla
    state = state + K * (measurement - state);

    // Belirsizliği güncelle (Sensörden bilgi aldığımız için belirsizlik azalır)
    uncertainty = (1.0f - K) * uncertainty;
}

float KalmanFilter::getState() const
{
    return state;
}

float KalmanFilter::getUncertainty() const
{
    return uncertainty;
}
