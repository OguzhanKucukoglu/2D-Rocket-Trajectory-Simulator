#ifndef KALMANFILTER_H
#define KALMANFILTER_H

class KalmanFilter
{
public:
    // Başlangıç değerleri
    KalmanFilter(float initialState, float initialError, float processNoise, float measurementNoise);

    // Fizik modeline göre tahminde bulun (Predict)
    void predict(float velocity, float dt);

    // Sensörden gelen gürültülü ölçümle tahmini düzelt (Update)
    void update(float measurement);

    // Filtrelenmiş en iyi tahmini al
    float getState() const;

    // Tahmindeki belirsizliği (hata payını) al
    float getUncertainty() const;

private:
    float state; // x (Tahmin edilen yükseklik)
    float uncertainty; // P (Tahminin belirsizliği / varyansı)
    float Q; // Süreç gürültüsü (Modelimize ne kadar güveniyoruz?)
    float R; // Ölçüm gürültüsü (Sensöre ne kadar güveniyoruz?)
};

#endif // KALMANFILTER_H
