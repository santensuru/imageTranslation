#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QFileDialog>
#include <vector>
#include <QDateTime>
#include <math.h>
#include <iostream>

typedef struct Pixel {
    int x = 0;
    int y = 0;
    QColor color;

} Pixel;

namespace Ui {

class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QImage *imageObject1, *imageObject2;
    QPixmap image1, image2;
    QGraphicsScene *scene1, *scene2;

    std::vector<Pixel> locations;
    void getRandomLocations();

    std::vector<Pixel> pairLocations;
    void searchSimilarPixels();

    float countScale();
    float countRotation();

private slots:
    void handleBtnImg1();
    void handleBtnImg2();
    void handleBtnExec();

};

#endif // MAINWINDOW_H
