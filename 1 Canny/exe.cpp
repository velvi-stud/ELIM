#include "opencv2/opencv.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <unistd.h>

using namespace cv;
using namespace std;


Mat nonMaximaSuppression(Mat magnitudo, Mat orientation){

    Mat dst = Mat::zeros(magnitudo.rows, magnitudo.cols, CV_8U);
    for (int i = 1; i < magnitudo.rows-1; i++)
    {
        for (int j = 1; j < magnitudo.cols-1; j++)
        {
            uchar mag = magnitudo.at<uchar>(i, j);
            float ang = orientation.at<float>(i, j);
            // si normalizza l'angolo poichè phase da solo valori positivi (ci sono anche negativi)
            ang = ang > 180 ? ang - 360 : ang;
            int dx = 0, dy = 0;
            // se la mag. del pixel i,j è maggiore degli opposti, lo prendo, altrimenti rimane 0
            // si controlla in che direzione sono i pixel opposti
            if (((ang > -22.5) && (ang <= 22.5)) || ((ang > 157.5) || (ang <= -157.5)))
            { // VERTICALE
                dx = 0;
                dy = -1;
            }
            else if (((ang > -67.5) && (ang <= -22.5)) || ((ang > 112.5) && (ang <= 157.5)))
            { //+45°
                dx = -1;
                dy = -1;
            }
            else if (((ang > -112.5) && (ang <= -67.5)) || ((ang > 67.5) && (ang <= 112.5)))
            { // ORIZZONTALE
                dx = -1;
                dy = 0;
            }
            else if (((ang > -157.5) && (ang <= -112.5)) || ((ang > 22.5) && (ang <= 67.5)))
            { //-45°
                dx = +1;
                dy = -1;
            }
            if (mag > magnitudo.at<uchar>(i + dx, j + dy) && mag > magnitudo.at<uchar>(i + (-1 * dx), j + (-1 * dy)))
                dst.at<uchar>(i, j) = mag;
        }
    }
    return dst;
}

Mat tresholdIsteresi(Mat nms, int lth, int hth)
{
    Mat dst = Mat::zeros(nms.rows, nms.cols, CV_8U);
    for (int i = 1; i < nms.rows; i++)
        for (int j = 1; j < nms.cols; j++)
            if (nms.at<uchar>(i, j) > hth) // Se il valore del pixel è maggiore della soglia alta diventa un edge forte
                dst.at<uchar>(i, j) = 255;
            else if (nms.at<uchar>(i, j) < lth) // Se il valore del pixel è minore della soglia bassa viene eliminato
                dst.at<uchar>(i, j) = 0;
            else if (nms.at<uchar>(i, j) >= lth && nms.at<uchar>(i, j) <= hth) // Il valore del pixel si trova tra le due soglie, quindi è un edge debole
                for (int x = -1; x <= 1; x++)
                    for (int y = -1; y <= 1; y++) // Controllo i valori nel suo intorno 3x3
                        if (nms.at<uchar>(i + x, j + y) > hth) // Se il loro valore è maggiore della soglia alta
                            dst.at<uchar>(i, j) = 255; // vengono promossi a punti di edge forti
    return dst;
}

void myCanny(Mat src, Mat &dst, int k_size, int TL, int TH)
{
    // passo 1 -> sfocare
    Mat gauss;
    GaussianBlur(src, gauss, Size(5, 5), 0);

    // passo 2 -> applicare filtro gx, gy (caso sobel)
    Mat sob_x, sob_y;
    Sobel(gauss, sob_x, CV_32F, 1, 0, k_size);
    Sobel(gauss, sob_y, CV_32F, 0, 1, k_size);

    // passo 3 -> calcolo magnitudo e normalizzare
    Mat magnitudo;
    magnitude(sob_x, sob_y, magnitudo);
    normalize(magnitudo, magnitudo, 0, 255, NORM_MINMAX, CV_8U);

    // passo 4 -> calcolo direzione (phase = atan() ad eccetto che ritorna solo valori positivi 0°,359°)
    Mat orientation;
    phase(sob_x, sob_y, orientation, true);

    // passo 5 -> non-maxima-suppression
    Mat nms = nonMaximaSuppression(magnitudo, orientation);

    // passo 6 -> tresholding con isteresi (doppo trashhold)
    Mat output = tresholdIsteresi(nms, TL, TH);

    // output
    dst = output;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        cout << "Usage: image_name, lth, hth " << endl;
        exit(-1);
    }
    int kernel_size = 3;
    Mat src = imread(argv[1], IMREAD_GRAYSCALE), dst;
    if (!src.data)
        return -1;
    int lth = atoi(argv[2]);
    int hth = atoi(argv[3]);
    myCanny(src, dst, kernel_size, lth, hth);
    imshow("input", src);
    imshow("output", dst);
    waitKey(0);
    return 0;
}