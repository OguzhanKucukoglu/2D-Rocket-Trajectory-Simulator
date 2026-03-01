#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , simTime(0.0f)
    , cameraY(0.0f)
    , cameraX(0.0f)
{
    ui->setupUi(this);
    setWindowTitle("2D Rocket Simulator");
    showMaximized();

    altitudeFilter = new KalmanFilter(0.0f, 1.0f, 0.1f, 20.0f);
    // Başlangıç Yüksekliği: 0
    // Başlangıç Belirsizliği: 1.0
    // Süreç (Model) Gürültüsü (Q): 0.1 (Fiziğimize çok güveniyoruz)
    // Ölçüm (Sensör) Gürültüsü (R): 20.0 (Sensörümüz ±20m sapıyor)

    // Ana layout
    QWidget *outerContainer = new QWidget(this);
    QVBoxLayout *outerLayout = new QVBoxLayout(outerContainer);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // Kontrol paneli
    QWidget *controlPanel = new QWidget(outerContainer);
    controlPanel->setFixedHeight(100);
    controlPanel->setStyleSheet("background-color: #1a1a2e;");

    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);

    // Slider satırı
    QHBoxLayout *slidersRow = new QHBoxLayout();

    angleLabel = new QLabel("Pitch Kick: 1000 m", controlPanel);
    angleLabel->setStyleSheet("color: white;");
    angleLabel->setFixedWidth(150);

    angleSlider = new QSlider(Qt::Horizontal, controlPanel);
    angleSlider->setMinimum(200);
    angleSlider->setMaximum(5000);
    angleSlider->setValue(1000);

    speedLabel = new QLabel("Itki: 15000 N", controlPanel);
    speedLabel->setStyleSheet("color: white;");
    speedLabel->setFixedWidth(140);

    speedSlider = new QSlider(Qt::Horizontal, controlPanel);
    speedSlider->setMinimum(5000);
    speedSlider->setMaximum(50000);
    speedSlider->setValue(15000);

    windLabel = new QLabel("Ruzgar: 0 N", controlPanel);
    windLabel->setStyleSheet("color: white;");
    windLabel->setFixedWidth(120);

    windSlider = new QSlider(Qt::Horizontal, controlPanel);
    windSlider->setMinimum(-500);   // sola esen rüzgar
    windSlider->setMaximum(500);    // sağa esen rüzgar
    windSlider->setValue(0);        // başlangıçta rüzgar yok

    slidersRow->addWidget(angleLabel);
    slidersRow->addWidget(angleSlider);
    slidersRow->addSpacing(20);
    slidersRow->addWidget(speedLabel);
    slidersRow->addWidget(speedSlider);
    slidersRow->addSpacing(20);
    slidersRow->addWidget(windLabel);
    slidersRow->addWidget(windSlider);

    // Buton satırı
    QHBoxLayout *buttonsRow = new QHBoxLayout();

    startButton = new QPushButton("Baslat", controlPanel);
    stopButton  = new QPushButton("Durdur", controlPanel);
    resetButton = new QPushButton("Sifirla", controlPanel);

    QString btnStyle = "QPushButton { color: white; padding: 5px 20px; "
                       "border-radius: 4px; font-size: 13px; }";
    startButton->setStyleSheet(btnStyle + "QPushButton { background: #27ae60; }");
    stopButton->setStyleSheet(btnStyle  + "QPushButton { background: #c0392b; }");
    resetButton->setStyleSheet(btnStyle + "QPushButton { background: #2980b9; }");

    buttonsRow->addStretch();
    buttonsRow->addWidget(startButton);
    buttonsRow->addWidget(stopButton);
    buttonsRow->addWidget(resetButton);
    buttonsRow->addStretch();

    controlLayout->addLayout(slidersRow);
    controlLayout->addLayout(buttonsRow);

    outerLayout->addStretch(1);
    outerLayout->addWidget(controlPanel);

    setCentralWidget(outerContainer);

    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(angleSlider, &QSlider::valueChanged, this, &MainWindow::onAngleChanged);
    connect(speedSlider, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    connect(windSlider, &QSlider::valueChanged, this, &MainWindow::onWindChanged);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::simulationStep);

    resetSimulation();
}

MainWindow::~MainWindow()
{
    delete ui;
}

float MainWindow::canvasHeight() const
{
    return height() - 100.0f;  // pencere yüksekliği - kontrol paneli
}

float MainWindow::metersToPixels(float meters) const
{
    return meters * 0.5f;  // 1 metre = 0.5 piksel
}

QPointF MainWindow::worldToScreen(const Vector2D &pos) const
{
    float screenX = width() / 2.0f + metersToPixels(pos.x - cameraX);
    float screenY = canvasHeight() * 0.7f + metersToPixels(pos.y - cameraY);
    return QPointF(screenX, screenY);
}

void MainWindow::simulationStep()
{
    float dt = 0.016f;
    simTime += dt;
    rocket.update(dt);
    cameraY += (rocket.position.y - cameraY) * 0.05f;
    cameraX += (rocket.position.x - cameraX) * 0.05f;

    trailPoints.append(QPointF(rocket.position.x, rocket.position.y)); // dünya koordinatı
    if (trailPoints.size() > 500)
        trailPoints.removeFirst();

    float trueAltitude = -rocket.position.y;
    if(trueAltitude < 0) trueAltitude = 0;

    // Sensörden GÜRÜLTÜLÜ bir okuma al (Roketin tek görebildiği bu)
    currentNoisyAltitude = Sensor::noisyAltitude(trueAltitude);

    // Kalman Filtresi Döngüsü:
    // a) Tahmin (Predict): şu hızla (dikey) gidiyorum, demek ki şuraya çıktım
    altitudeFilter->predict(-rocket.velocity.y, dt);

    // b) Düzeltme (Update): sensör de x metredeyim diyor, dengeyi bul
    altitudeFilter->update(currentNoisyAltitude);

    update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(0, 0, width(), (int)canvasHeight(), QColor(15, 15, 30));

    // Yer çizgisi
    float groundScreenY = worldToScreen(Vector2D(0.0f,0.0f)).y();
    painter.setPen(QPen(QColor(100, 200, 100), 2));
    painter.drawLine(0, groundScreenY, width(), groundScreenY);

    // Yörünge izi
    if (trailPoints.size() > 1) {
        painter.setPen(QPen(QColor(255, 80, 0, 150), 2));
        for(int i = 1; i < trailPoints.size(); i++)
        {
            Vector2D p1(trailPoints[i-1].x(), trailPoints[i-1].y());
            Vector2D p2(trailPoints[i].x(), trailPoints[i].y());
            painter.drawLine(worldToScreen(p1), worldToScreen(p2));
        }
    }

    // Roketi çiz
    QPointF rocketScreen = worldToScreen(rocket.position);

    // Roketin hareket açısını hesapla
    float angle = std::atan2(rocket.velocity.y, rocket.velocity.x);
    // atan2 → hız vektöründen açı çıkarır (radyan)

    painter.save();  // mevcut transform'u kaydet

    // Çizim merkezini roketin konumuna taşı, sonra döndür
    painter.translate(rocketScreen);
    painter.rotate(angle * 180.0f / M_PI + 90.0f);
    // +90 derece: Qt'de 0° sağ yön, rokette 0° yukarı olsun

    // Gövde (dikdörtgen)
    painter.setBrush(QBrush(QColor(200, 200, 210)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(-6, -20, 12, 35);   // (x, y, genişlik, yükseklik)

    // Burun (üçgen)
    QPolygonF nose;
    nose << QPointF(0, -30)   // tepe
         << QPointF(-6, -20)  // sol alt
         << QPointF(6, -20);  // sağ alt
    painter.setBrush(QBrush(QColor(220, 50, 50)));
    painter.drawPolygon(nose);

    // Kanatlar (sol ve sağ)
    QPolygonF leftFin;
    leftFin << QPointF(-6, 10)
            << QPointF(-14, 20)
            << QPointF(-6, 15);
    painter.setBrush(QBrush(QColor(150, 150, 160)));
    painter.drawPolygon(leftFin);

    QPolygonF rightFin;
    rightFin << QPointF(6, 10)
             << QPointF(14, 20)
             << QPointF(6, 15);
    painter.drawPolygon(rightFin);

    // Motor alevi (motor açıksa)
    if (rocket.engineOn && rocket.hasFuel()) {
        painter.setBrush(QBrush(QColor(255, 150, 0, 200)));
        QPolygonF flame;
        flame << QPointF(-4, 15)
              << QPointF(4, 15)
              << QPointF(0, 30);   // alevin ucu
        painter.drawPolygon(flame);
    }

    painter.restore();  // eski transform'a geri dön

    // Gerçek Yükseklik (Bembeyaz)
    painter.setPen(Qt::white);
    painter.drawText(10, 140, QString("Gercek Alt: %1 m").arg(-rocket.position.y, 0, 'f', 0));

    // Sensörün Okuduğu Gürültülü Yükseklik (Kırmızı - Çünkü Güvenilmez)
    painter.setPen(QColor(255, 100, 100)); // Açık kırmızı
    painter.drawText(10, 160, QString("Sensor Alt: %1 m").arg(currentNoisyAltitude, 0, 'f', 0));

    // Kalman'ın Tahmin Ettiği Yükseklik (Mavi/Camgöbeği - Akıllı Algoritma)
    painter.setPen(QColor(100, 200, 255)); // Açık Mavi
    painter.drawText(10, 180, QString("Kalman Alt: %1 m").arg(altitudeFilter->getState(), 0, 'f', 0));

    // Bilgi yazısı
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    painter.drawText(10, 20, QString("Sure: %1 s").arg(simTime, 0, 'f', 1));
    painter.drawText(10, 40, QString("Yukseklik: %1 m").arg(-rocket.position.y, 0, 'f', 0));
    painter.drawText(10, 60, QString("Hiz: %1 m/s").arg(rocket.velocity.magnitude(), 0, 'f', 0));
    painter.drawText(10, 80, QString("Yakit: %1 kg").arg(rocket.fuelMass, 0, 'f', 0));
    painter.drawText(10, 100, QString("Mach: %1").arg(rocket.machNumber(), 0, 'f', 2));
    painter.drawText(10, 120, QString("G: %1").arg(rocket.acceleration.magnitude() / 9.81f, 0, 'f', 1));
}

void MainWindow::resetSimulation()
{
    timer->stop();
    trailPoints.clear();
    simTime = 0.0f;
    cameraX = 0.0f;
    cameraY = 0.0f;

    rocket = Rocket();
    rocket.velocity = Vector2D(0.0f, 0.0f);
    rocket.engineOn = true;

    if (altitudeFilter) delete altitudeFilter;
    altitudeFilter = new KalmanFilter(0.0f, 1.0f, 0.1f, 20.0f);
    currentNoisyAltitude = 0.0f;

    update();
}

void MainWindow::onStartClicked()  { timer->start(16); }
void MainWindow::onStopClicked()   { timer->stop(); }
void MainWindow::onResetClicked()  { resetSimulation(); }

void MainWindow::onAngleChanged(int value)
{
    rocket.pitchKickAlt = (float)value;
    angleLabel->setText(QString("Pitch Kick: %1 m").arg(value));
}

void MainWindow::onSpeedChanged(int value)
{
    rocket.thrustForce = (float)value;
    speedLabel->setText(QString("Itki: %1 N").arg(value));
}

void MainWindow::onWindChanged(int value)
{
    rocket.windForce = Vector2D((float)value, 0.0f);
    windLabel->setText(QString("Ruzgar: %1 N").arg(value));
}
