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

void MainWindow::getRandomLocations()
{
    locations.clear();

    int w = imageObject1->width();
    int h = imageObject1->height();

    qsrand(QDateTime::currentMSecsSinceEpoch() / 1000);

    int vertexs = 2;
    for (int i=0; i<vertexs; i++)
    {
        int x = qrand() % w;
        int y = qrand() % h;

        if (imageObject1->pixelColor(x, y).alpha() == 0)
        {
            i--;
            continue;
        }

        Pixel pixel;
        pixel.x = x;
        pixel.y = y;
        pixel.color = imageObject1->pixelColor(x, y);

        if (helperFind(&pixel))
        {
            i--;
            continue;
        }
        else
        {
            locations.push_back(pixel);
        }
    }
}

void MainWindow::searchSimilarPixels()
{
    pairLocations.clear();

    int w = imageObject2->width();
    int h = imageObject2->height();

    size_t k = 0;
    for (int i=0; i<w; i++)
    {
        for (int j=0; j<h; j++)
        {
            if (k == locations.size())
            {
                break;
            }

            if (imageObject2->pixelColor(i, j) == locations.at(k).color)
            {
                Pixel pixel;
                pixel.x = i;
                pixel.y = j;
                pixel.color = imageObject1->pixelColor(i, j);

                pairLocations.push_back(pixel);

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

        float d1 = ( sqrt(point1.x*point1.x + point1.y*point1.y) * sqrt(based1.x*based1.x + based1.y*based1.y) );
        float d2 = ( sqrt(point2.x*point2.x + point2.y*point2.y) * sqrt(based2.x*based2.x + based2.y*based2.y) );

        float s = d1/d2;
        countScale += s;
    }

    return countScale/locations.size();
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
        countDegree += acos(s);
    }

    return countDegree/locations.size();
}

void MainWindow::handleBtnExec()
{
    getRandomLocations();
    searchSimilarPixels();

    QString result = "Scale: " + QString::number(countScale()) + "\n";
    result += "Rotation: " + QString::number(countRotation()) + " radian\n";

    ui->result->document()->setPlainText(result);
    std::cout << result.toStdString();
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
