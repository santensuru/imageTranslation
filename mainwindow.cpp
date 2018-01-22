#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->btnExec->setEnabled(false);

    // Connect button signal to appropriate slot
    connect(ui->btnImg1, SIGNAL (released()), this, SLOT (handleBtnImg1()));
    connect(ui->btnImg2, SIGNAL (released()), this, SLOT (handleBtnImg2()));
    connect(ui->btnExec, SIGNAL (released()), this, SLOT (handleBtnExec()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleBtnImg1()
{
    QString imagePath = QFileDialog::getOpenFileName(
                this,
                tr("Open File Image 1"),
                "",
                tr("PNG (*.png);;JPEG (*.jpg *.jpeg)" )
                );

    if (imagePath.isEmpty())
        return;

    imageObject1 = new QImage();
    imageObject1->load(imagePath);

    image1 = QPixmap::fromImage(*imageObject1);

    scene1 = new QGraphicsScene(this);
    scene1->addPixmap(image1);
    scene1->setSceneRect(image1.rect());
    ui->img1->setScene(scene1);

    ui->btnImg1->setEnabled(false);

    if (ui->btnImg2->isEnabled() == false)
    {
        ui->btnExec->setEnabled(true);
    }
}

void MainWindow::handleBtnImg2()
{
    QString imagePath = QFileDialog::getOpenFileName(
                this,
                tr("Open File Image 2"),
                "",
                tr("PNG (*.png);;JPEG (*.jpg *.jpeg)" )
                );

    if (imagePath.isEmpty())
        return;

    imageObject2 = new QImage();
    imageObject2->load(imagePath);

    image2 = QPixmap::fromImage(*imageObject2);

    scene2 = new QGraphicsScene(this);
    scene2->addPixmap(image2);
    scene2->setSceneRect(image2.rect());
    ui->img2->setScene(scene2);

    ui->btnImg2->setEnabled(false);

    if (ui->btnImg1->isEnabled() == false)
    {
        ui->btnExec->setEnabled(true);
    }
}

void MainWindow::convertToGrayscale(QImage *imageObject)
{
    for (int ii = 0; ii < (*imageObject).height(); ii++) {
        uchar* scan = (*imageObject).scanLine(ii);
        int depth =4;
        for (int jj = 0; jj < (*imageObject).width(); jj++) {

            QRgb* rgbpixel = reinterpret_cast<QRgb*>(scan + jj*depth);
            int gray = qGray(*rgbpixel);
            *rgbpixel = QColor(gray, gray, gray).rgba();
        }
    }
}

void MainWindow::getRandomLocations()
{
    locations.clear();

    int w = imageObject1->width() - 4;
    int h = imageObject1->height() - 4;

    qsrand(QDateTime::currentMSecsSinceEpoch() / 1000);

    int vertexs = 2;
    for (int i=0; i<vertexs; i++)
    {
        int x = qrand() % w + 2;
        int y = qrand() % h + 2;

        //std::cout<< qAlpha(imageObject1->pixel(0, 0)) << std::endl;
        //std::cout<< qAlpha(imageObject1->pixel(w/2, h/2)) << std::endl;

        if (qAlpha(imageObject1->pixel(x, y)) == 0)
        {
            i--;
            continue;
        }

        Pixel pixel;
        pixel.x = x;
        pixel.y = y;
        pixel.color = imageObject1->pixelColor(x, y);

        float neightbour = 0.0f;
        for (int j=x-2; j<x+3; j++)
        {
            for (int k=y-2; k<y+3; k++)
            {
                if (j != x && k != y)
                {
                    neightbour += qGray(imageObject1->pixel(j, k));
                }
            }
        }

        pixel.neightbour = neightbour / 24.0f;

        if (helperFind(&pixel))
        {
            i--;
            continue;
        }
        else
        {
            locations.push_back(pixel);
            ui->img1->scene()->addEllipse(pixel.x - 2, pixel.y - 2, 4, 4, QPen(Qt::red, 2));
        }
    }
}

void MainWindow::searchSimilarPixels()
{
    pairLocations.clear();

    int w = imageObject2->width();
    int h = imageObject2->height();

    size_t k = 0;
    for (int i=2; i<w-3; i++)
    {
        for (int j=2; j<h-3; j++)
        {
            if (k == locations.size())
            {
                break;
            }

            float neightbour = 0.0f;
            for (int l=i-2; l<i+3; l++)
            {
                for (int m=j-2; m<j+3; m++)
                {
                    if (l != i && m != j)
                    {
                        neightbour += qGray(imageObject2->pixel(l, m));
                    }
                }
            }

            Pixel pixel;
            pixel.x = i;
            pixel.y = j;
            pixel.color = imageObject2->pixelColor(i, j);
            pixel.neightbour = neightbour / 24.0f;

            //std::cout<< pixel.neightbour <<  " " << locations.at(k).neightbour << std::endl;

            if (pixel.color == locations.at(k).color &&
                    (pixel.neightbour <= APPROXIMATION + locations.at(k).neightbour &&
                     pixel.neightbour >= locations.at(k).neightbour - APPROXIMATION))
            {
                pairLocations.push_back(pixel);
                ui->img2->scene()->addEllipse(pixel.x - 2, pixel.y - 2, 4, 4, QPen(Qt::red, 2));

                k++;
            }
        }
    }
}

float MainWindow::countScale()
{
    // scale -> 1) count distance 2 point related in 1 and 2 picture
    //          2) distance 1 / distance 2 = scale

    float countScale = 0.0f;
    for (size_t i=1; i<locations.size(); i++)
    {
        Pixel based1 = locations.at(i-1);
        Pixel based2 = pairLocations.at(i-1);

        Pixel point1 = locations.at(i);
        Pixel point2 = pairLocations.at(i);

        float d1 = sqrt((point1.x-based1.x) * (point1.x-based1.x) + (point1.y-based1.y) * (point1.y-based1.y));
        float d2 = sqrt((point2.x-based2.x) * (point2.x-based2.x) + (point2.y-based2.y) * (point2.y-based2.y));

        float s = d2/d1;
        countScale += s;
    }

    return countScale/(locations.size()-1);
}

float MainWindow::countRotation()
{
    // https://stackoverflow.com/questions/4294638/how-to-calculate-angle-between-two-direction-vectors-that-form-a-closed-open-sha
    // cos( alpha ) = (x1 * x2 + y1 * y2) / ( sqrt(x1*x1 + y1*y1) * sqrt(x2*x2 + y2*y2) )

    // must in same base -> normalisation

    float countDegree = 0.0f;
    for (size_t i=1; i<locations.size(); i++)
    {
        Pixel based1 = locations.at(i-1);
        Pixel based2 = pairLocations.at(i-1);

        Pixel point1 = locations.at(i);
        Pixel point2 = pairLocations.at(i);

        int x1 = point1.x-based1.x;
        int y1 = point1.y-based1.y;

        int x2 = point2.x-based2.x;
        int y2 = point2.y-based2.y;

        float s = (x1*x2 + y1*y2) / ( sqrt(x1*x1 + y1*y1) * sqrt(x2*x2 + y2*y2) );
        countDegree -= acos(s);
    }

    return countDegree/(locations.size()-1);
}

void MainWindow::handleBtnExec()
{
    QString result = "";

    while(true){

        ui->img1->scene()->clear();
        ui->img2->scene()->clear();

        scene1->addPixmap(image1);
        scene1->setSceneRect(image1.rect());
        ui->img1->setScene(scene1);

        scene2->addPixmap(image2);
        scene2->setSceneRect(image2.rect());
        ui->img2->setScene(scene2);

    //convert to grayscale
    //convertToGrayscale(&(*imageObject1));
    //convertToGrayscale(&(*imageObject2));

    getRandomLocations();
    searchSimilarPixels();

    float scale = countScale();
    float radian = countRotation();

    result += "Scale: " + QString::number(scale) + "\n";
    result += "Rotation: " + QString::number(radian) + " radian";

    float degree = radian * 180.0f * 7.0f / 22.0f;

    result += " = ~" + QString::number(degree) + " degree\n";

    ui->result->document()->setPlainText(result);
    std::cout << result.toStdString();

    //if (scale < 1)
        break;
    }
}

// how to solve
/*
 * find homography point, 2 is enought
 * https://kislayabhi.github.io/Merging-images_using_homography/
 *
 * or random generate point
 * and find the related (need homography)
 *
 * then...
 *
 * count scale
 * count rotate
 *
 */

bool MainWindow::helperFind(Pixel *pixel)
{
    for (size_t i=0; i<locations.size(); i++)
    {
        if (locations.at(i).x == (*pixel).x && locations.at(i).y == (*pixel).y)
        {
            return true;
        }
    }

    return false;
}
