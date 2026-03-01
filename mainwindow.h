#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QTimer>
#include "rocket.h"
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include "sensor.h"
#include "kalmanFilter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void simulationStep(); // her 16ms de çağrılacak
    void onStartClicked();
    void onStopClicked();
    void onResetClicked();
    void onAngleChanged(int value);
    void onSpeedChanged(int value);

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    Rocket rocket; // Pointer değil direk nesne
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *resetButton;
    QSlider *angleSlider;
    QSlider *speedSlider;
    QLabel *angleLabel;
    QLabel *speedLabel;
    QWidget *controlPanel;
    QSlider *windSlider;
    QLabel *windLabel;
    KalmanFilter *altitudeFilter;

    float simTime; // toplam geçen süre (saniye)
    float cameraY; // kameranın dikey pozisyonu (metre cinsinden, dünya koordinatı)
    float cameraX; // kameranın yatay pozisyonu (metre)
    float currentNoisyAltitude; // Ekranda göstermek için anlık gürültülü okuma

    float canvasHeight() const;
    void resetSimulation(); // helper fonksiyon

    // Koordinat dönüşümü için
    float metersToPixels(float meters) const;
    QPointF worldToScreen(const Vector2D &pos) const;

    void onWindChanged(int value);

    QVector<QPointF> trailPoints; // geçmiş konumlar
};
#endif // MAINWINDOW_H
