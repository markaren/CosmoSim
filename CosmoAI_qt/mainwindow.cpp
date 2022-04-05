#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <random>
#include <string>
#include <iostream>
#define _USE_MATH_DEFINES // for C++
#include <math.h>
#include <cmath>
#include <QtCore>
#include <QString>
#include <QPainter>
#include <QDebug>
#define PI 3.14159265358979323846



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    grid = true;
    markers = true;

    init_values();

    imgActual = QImage(wSize, wSize, QImage::Format_RGB32);
    imgApparent = QImage(2*wSize, wSize, QImage::Format_RGB32);
    imgDistorted = QImage(2*wSize, wSize, QImage::Format_RGB32);
    rocket = QPixmap(":/new/prefix1/Tintin.png");

    // Set max/min values for UI elements
    ui->einsteinSlider->setMaximum(0.1*wSize);
    ui->einsteinSpinbox->setMaximum(0.1*wSize);
    ui->srcSizeSlider->setMaximum(0.1*wSize);
    ui->srcSizeSpinbox->setMaximum(0.1*wSize);
    ui->lensDistSlider->setMaximum(100);
    ui->lensDistSpinbox->setMaximum(100);
    ui->lensDistSlider->setMinimum(30);
    ui->lensDistSpinbox->setMinimum(30);
    ui->xSlider->setMaximum(wSize/2);
    ui->xSpinbox->setMaximum(wSize/2);
    ui->xSlider->setMinimum(-wSize/2);
    ui->xSpinbox->setMinimum(-wSize/2);
    ui->ySlider->setMaximum(wSize/2);
    ui->ySpinbox->setMaximum(wSize/2);
    ui->ySlider->setMinimum(-wSize/2);
    ui->ySpinbox->setMinimum(-wSize/2);

    // Connect sliders and spinboxes
    connect(ui->einsteinSlider, SIGNAL(valueChanged(int)), ui->einsteinSpinbox, SLOT(setValue(int)));
    connect(ui->einsteinSpinbox, SIGNAL(valueChanged(int)), ui->einsteinSlider, SLOT(setValue(int)));
    connect(ui->srcSizeSpinbox, SIGNAL(valueChanged(int)), ui->srcSizeSlider, SLOT(setValue(int)));
    connect(ui->srcSizeSlider, SIGNAL(valueChanged(int)), ui->srcSizeSpinbox, SLOT(setValue(int)));
    connect(ui->lensDistSpinbox, SIGNAL(valueChanged(int)), ui->lensDistSlider, SLOT(setValue(int)));
    connect(ui->lensDistSlider, SIGNAL(valueChanged(int)), ui->lensDistSpinbox, SLOT(setValue(int)));
    connect(ui->xSpinbox, SIGNAL(valueChanged(int)), ui->xSlider, SLOT(setValue(int)));
    connect(ui->xSlider, SIGNAL(valueChanged(int)), ui->xSpinbox, SLOT(setValue(int)));
    connect(ui->ySpinbox, SIGNAL(valueChanged(int)), ui->ySlider, SLOT(setValue(int)));
    connect(ui->ySlider, SIGNAL(valueChanged(int)), ui->ySpinbox, SLOT(setValue(int)));

    updateImg();
}

void MainWindow::init_values() {

    wSize = 600;
    einsteinR = wSize/20;
    srcSize = wSize/20;
    KL = 0.65;
    xPos = 0;
    yPos = 0;
    source = ui->srcTypeComboBox->currentText();

    // Set initial values for UI elements
    ui->einsteinSpinbox->setValue(einsteinR);
    ui->einsteinSlider->setSliderPosition(einsteinR);
    ui->srcSizeSpinbox->setValue(srcSize);
    ui->srcSizeSlider->setSliderPosition(srcSize);
    ui->lensDistSpinbox->setValue(KL*100);
    ui->lensDistSlider->setSliderPosition(KL*100);
    ui->xSpinbox->setValue(xPos);
    ui->xSlider->setSliderPosition(xPos);
    ui->ySpinbox->setValue(yPos);
    ui->ySlider->setSliderPosition(yPos);
    ui->gridBox->setChecked(grid);
    ui->markerBox->setChecked(markers);
}

void MainWindow::drawSource(int begin, int end, QImage& img, double xPos, double yPos) {
    int rows = img.height();
    int cols = img.width();
    for (int row = begin; row < end; row++) {
        for (int col = 0; col < cols; col++) {
            double x = col - xPos - cols/2;
            double y = -yPos - row + rows/2;
            int val;
            if (source == "Gauss"){
                val = round(255 * exp((-x * x - y * y) / (2.0*srcSize*srcSize)));
                img.setPixel(col, row, qRgb(val, val, val));
            }
            else if (source == "Circle"){
                val = 255 * (x*x + y*y < srcSize*srcSize);
                img.setPixel(col, row, qRgb(val, val, val));
            }
            else if (source == "Square"){
                val = 255*(abs(x) < srcSize && abs(y) < srcSize);
                img.setPixel(col, row, qRgb(val, val, val));
            }
        }
    }
}

void MainWindow::distort(int begin, int end, QImage imgApparent, QImage& imgDistorted, double R, double apparentPos, double KL) {
    int rows = imgDistorted.height();
    int cols = imgDistorted.width();
    // Evaluate each point in imgDistorted plane ~ lens plane
    for (int row = begin; row < end; row++) {
        for (int col = 0; col <= cols; col++) { // <= ???????????????????????????????????????

            // Set coordinate system with origin at x=R
            double x = (col - apparentPos - cols/2.0) * KL;
            double y = (rows/2.0 - row) * KL;

            // Calculate distance and angle of the point evaluated relative to center of lens (origin)
            double r = sqrt(x*x + y*y);
            double theta = atan2(y, x);

            // Point mass lens equation
            double frac = (einsteinR * einsteinR * r) / (r * r + R * R + 2 * r * R * cos(theta));
            double x_ = (x + frac * (r / R + cos(theta))) / KL;
            double y_ = (y - frac * sin(theta)) / KL;

            // Translate to array index
            int row_ = (int)round(rows/2.0 - y_);
            int col_ = (int)round(apparentPos + cols/2.0 + x_);


            // If (x', y') within source, copy value to imgDistorted
            if (row_ < rows && col_ < cols && row_ > 0 && col_ >= 0) {
            imgDistorted.setPixel(col, row, imgApparent.pixel(col_, row_));
            }
        }
    }
}

void MainWindow::drawSourceThreaded(QImage& img, double xPos, double yPos){
    unsigned int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads_vec;
    for (unsigned int k = 0; k < num_threads; k++) {
        unsigned int thread_begin = (img.height() / num_threads) * k;
        unsigned int thread_end = (img.height() / num_threads) * (k + 1);
        std::thread t(&MainWindow::drawSource, this, thread_begin, thread_end, std::ref(img), xPos, yPos);
        threads_vec.push_back(std::move(t));
    }
    for (auto& thread : threads_vec) {
        thread.join();
    }
}

// Split the image into (number of threads available) pieces and distort the pieces in parallel
void MainWindow::distortThreaded(double R, double apparentPos, QImage& imgApparent, QImage& imgDistorted, double KL) {
    unsigned int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads_vec;
    for (unsigned int k = 0; k < num_threads; k++) {
        unsigned int thread_begin = (imgDistorted.height() / num_threads) * k;
        unsigned int thread_end = (imgDistorted.height() / num_threads) * (k + 1);
        std::thread t(&MainWindow::distort, this, thread_begin, thread_end, imgApparent, std::ref(imgDistorted), R, apparentPos, KL);
        threads_vec.push_back(std::move(t));
    }
    for (auto& thread : threads_vec) {
        thread.join();
    }
}

QPixmap MainWindow::rotate(QPixmap src, double angle,int x, int y){
    QPixmap r(src.size());
//    QSize s = src.size();
    r.fill(QColor::fromRgb(Qt::black));
    QPainter m(&r);
    m.setRenderHint(QPainter::SmoothPixmapTransform);
    m.translate(src.width()/2 + x, src.height()/2 + y);
    m.rotate(angle*180/PI);
    m.translate(-src.width()/2 - x, -src.height()/2 - y);
    m.drawPixmap(0,0, src);
    return r;
}

void MainWindow::drawRadius(QPixmap& src){
    QPointF center(src.width()/2, src.height()/2);
    QPainter painter(&src);
    QPen pen(Qt::gray, 2, Qt::DashLine);
    painter.setPen(pen);
    painter.setOpacity(0.3);
    painter.drawEllipse(center, (int)round(einsteinR/KL), (int)round(einsteinR/KL));
}

void MainWindow::drawGrid(QPixmap& img){
    QPainter painter(&img);
    QPen pen(Qt::gray, 2, Qt::DashLine);
    painter.setPen(pen);
    painter.setOpacity(0.3);

    QLineF lineVert(wSize/2, 0, wSize/2, wSize);
    QLineF lineHor(0, wSize/2, wSize, wSize/2);
    painter.drawLine(lineVert);
    painter.drawLine(lineHor);
}

void MainWindow::drawMarker(QPixmap& src, int x, int y, QColor color){
    QPointF point(wSize/2 + x, wSize/2 - y);
    QPainter painter(&src);
    QPen pen(color, 10);
    painter.setPen(pen);
    painter.setOpacity(0.4);
    painter.drawPoint(point);
}



void MainWindow::updateImg() {

    imgApparent.fill(Qt::black);
    imgActual.fill(Qt::black);
    imgDistorted.fill(Qt::black);

    phi = atan2(yPos, xPos);
    double actualPos = sqrt(xPos*xPos + yPos*yPos);
    double apparentPos = (actualPos + sqrt(actualPos*actualPos + 4 / (KL*KL) * einsteinR*einsteinR)) / 2.0;
    double apparentPos2 = (int)round((actualPos - sqrt(actualPos*actualPos + 4 / (KL*KL) * einsteinR*einsteinR)) / 2.0);
    double R = apparentPos * KL;
    int actualX = (int)round(actualPos*cos(phi));
    int actualY = (int)round(actualPos*sin(phi));

    if (source == "Rocket"){
        QPixmap rocket1 = rocket.scaled(3*srcSize, 3*srcSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPainter p1(&imgApparent);
        p1.drawPixmap(apparentPos + wSize - rocket1.width()/2, wSize/2 - rocket1.height()/2, rocket1);
        QPainter p2(&imgActual);
        p2.drawPixmap(actualX + wSize/2 - rocket1.width()/2, wSize/2 - actualY - rocket1.height()/2, rocket1);
    }

    else{
        drawSourceThreaded(imgApparent, apparentPos, 0);
        drawSourceThreaded(imgActual, actualX, actualY);
    }


    // Convert image to pixmap
    QPixmap imgAppPixDisp = QPixmap::fromImage(imgApparent);

    // Pixmap with pre-rotation
    auto imgAppPix = rotate(imgAppPixDisp, phi, apparentPos, 0);

    // Crop pixmap to correct display size
    QRect rect2(wSize/2, 0, wSize, wSize);
    imgAppPixDisp = imgAppPixDisp.copy(rect2);

    // Convert pre-rotated pixmap to image and distort the image
    imgApparent = imgAppPix.toImage();
    distortThreaded(R, apparentPos, imgApparent, imgDistorted, KL);

    // Convert distorted image to pixmap, rotate and crop
    QPixmap imgDistPix = QPixmap::fromImage(imgDistorted);
    imgDistPix = rotate(imgDistPix, -phi, 0, 0);
    QRect rect(wSize/2, 0, wSize, wSize);
    imgDistPix = imgDistPix.copy(rect);

//    QString sizeString = QString("(%1,%2)").arg(distRot2.width()).arg(distRot2.height());
//    qDebug(qUtf8Printable(sizeString));

    int apparentX = (int)round(apparentPos*cos(phi));
    int apparentY = (int)round(apparentPos*sin(phi));
    int apparentX2 = (int)round(apparentPos2*cos(phi));
    int apparentY2 = (int)round(apparentPos2*sin(phi));

    auto imgActPix = QPixmap::fromImage(imgActual);
    if (grid == true) {
        drawGrid(imgActPix);
        drawGrid(imgAppPixDisp);
        drawGrid(imgDistPix);
        drawRadius(imgDistPix);
    }

    if (markers == true) {
        drawMarker(imgDistPix, apparentX, apparentY, Qt::blue);
        drawMarker(imgDistPix, apparentX2, apparentY2, Qt::blue);
        drawMarker(imgDistPix, actualX, actualY, Qt::red);
    }

    // Draw pixmaps on QLabels
    ui->actLabel->setPixmap(imgActPix);
    ui->appLabel->setPixmap(imgAppPixDisp);
    ui->distLabel->setPixmap(imgDistPix);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_einsteinSpinbox_valueChanged()
{
    // Set variables to current spinbox values
    einsteinR = ui->einsteinSpinbox->value();
    updateImg();
}


void MainWindow::on_srcSizeSpinbox_valueChanged()
{
    srcSize = ui->srcSizeSpinbox->value();
    updateImg();
}


void MainWindow::on_lensDistSpinbox_valueChanged(int arg1)
{
    KL = arg1/100.0;
//    KL_percent = ui->lensDistSpinbox->value();
    updateImg();
}


void MainWindow::on_xSpinbox_valueChanged()
{
    xPos = ui->xSpinbox->value();
    updateImg();
}


void MainWindow::on_ySpinbox_valueChanged()
{
    yPos = ui->ySpinbox->value();
    updateImg();
}


void MainWindow::on_gridBox_stateChanged(int arg1)
{
    grid = arg1;
    updateImg();
}


void MainWindow::on_markerBox_stateChanged(int arg1)
{
    markers = arg1;
    updateImg();
}


void MainWindow::on_pushButton_clicked()
{
    init_values();
    updateImg();
}


void MainWindow::on_srcTypeComboBox_currentTextChanged(const QString &arg1)
{
    source = arg1;
    updateImg();
}

