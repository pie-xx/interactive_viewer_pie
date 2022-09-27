// filterDll1.cpp : DLL 用にエクスポートされる関数を定義します。
//
#include "pch.h"
#include "framework.h"
#include "ImgProcDll.h"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <io.h>
#include <stdio.h>

#include <codecvt>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <locale>

#include <system_error>
#include <vector>
#include <Windows.h>

#ifdef _DEBUG
#pragma comment(lib, "opencv_world460d.lib")
#else
#pragma comment(lib, "opencv_world460.lib")
#endif

std::vector<std::string>  ftbl = { "NON","BW","C1","C2","C3","Poster","Poster2","Poster3" };

void getlen(int* count) {
    *count = ftbl.size();
}

void getname(int* n, char* fname, int* size) {


    std::cout << "getname input " << n << " " << fname << " size=" << *size << std::endl;

    strncpy_s(fname, *size, ftbl.at(*n).data(), ftbl.at(*n).size());
    *size = ftbl.at(*n).size();

    std::cout << "getname output " << fname << " " << *size << std::endl;
}


#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/flann/flann.hpp>

using namespace cv;
using std::cout; using std::cin;
using std::endl; using std::ifstream;

/*
    UTF8文字列をwstringに変換する
*/
std::wstring Utf8ToWString
(
    std::string oUTF8Str
)
{
    // バッファサイズの取得
    int iBufferSize = ::MultiByteToWideChar(CP_UTF8, 0, oUTF8Str.c_str()
        , -1, (wchar_t*)NULL, 0);

    // バッファの取得
    wchar_t* wpBufWString = (wchar_t*)new wchar_t[iBufferSize];

    // UTF8 → wstring
    ::MultiByteToWideChar(CP_UTF8, 0, oUTF8Str.c_str(), -1, wpBufWString
        , iBufferSize);

    // wstringの生成
    std::wstring oRet(wpBufWString, wpBufWString + iBufferSize - 1);

    // バッファの破棄
    delete[] wpBufWString;

    // 変換結果を返す
    return(oRet);
}


Mat wimread(char* inpath) {
    setlocale(LC_ALL, "japanese");

    std::wstring  u16 = Utf8ToWString(inpath);

    FILE* fp;
    _wfopen_s(&fp, u16.data(), L"rb");
    if (fp == NULL) {
        std::cout << "BWcant open " << inpath << std::endl;
        return Mat();
    }

    long long int fsize = _filelengthi64(_fileno(fp));
    std::cout << "BWinput " << inpath << " size=" << fsize << std::endl;


    char* buff = (char*)malloc(fsize);

    fread(buff, fsize, 1, fp);
    fclose(fp);

    // Matへ変換
    std::vector<uchar> jpeg(buff, buff + fsize);
    cv::Mat img = cv::imdecode(jpeg, 1);

    free(buff);

    return img;
}

void wimwrite(char* outpath, Mat img) {
    setlocale(LC_ALL, "japanese");

    std::vector<uchar> buff2; //buffer for coding
    std::vector<int> param = std::vector<int>(2);
    param[0] = 1;
    param[1] = 95; //default(95) 0-100

    imencode(".jpg", img, buff2, param);

    std::wstring  u16 = Utf8ToWString(outpath);
    FILE* fp2;
    _wfopen_s(&fp2, u16.data(), L"wb");

    if (fp2 == NULL) {
        std::cout << "BWoutput cant open" << std::endl;
        return;
    }
    fwrite(buff2.data(), buff2.size(), 1, fp2);
    fclose(fp2);
}

std::string getPathDir(std::string outpath) {

    std::string outpathstr(outpath);
    int p = outpathstr.rfind('\\');
    if (p == -1) {
        p = outpathstr.rfind('/');
    }

    std::string outpathdir = outpathstr.substr(0, p);
    //std::cout << "pathdir " << outpathdir << " " << p << std::endl;

    return outpathdir;
}

////////////////////////////////////////////////////////////////////////////////////////

Mat filterOne(Mat img, int thd) {
    std::cout << "filterOne " << thd << std::endl;
    Mat outimg;
    adaptiveThreshold(img, outimg, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 51, thd);
    return outimg;
}

double calc_av(Mat* img, int x, int y, int w, int h) {
    double t = 0;
    for (int cy = y; cy < y + h; ++cy) {
        unsigned char* src = (*img).ptr<unsigned char>(cy);
        for (int cx = x; cx < x + w; ++cx) {
            t = t + double(src[cx]);
        }
    }
    return t / (w * h);
}

double calc_var(Mat* img, int x, int y, int w, int h, double a) {

    //double a = calc_av(img, x, y, w, h);

    double t = 0;
    for (int cy = y; cy < y + h; ++cy) {
        unsigned char* src = (*img).ptr<unsigned char>(cy);
        for (int cx = x; cx < x + w; ++cx) {
            double d = double(src[cx]) - a;
            t = t + d * d;
        }
    }
    return t / (w * h);
}

/*
void C3(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {

        Mat temp, img2, maskimg, maskimg2, maskimg3, ansimg;
        Mat img_hsv;
        std::vector<cv::Mat> planes;

        Mat img = wimread(inpath);
        if (img.size == 0) {
            return;
        }

        // 文字部分だけ黒く抜いたマクス作成
        cvtColor(img, temp, COLOR_RGB2GRAY);
        adaptiveThreshold(temp, maskimg, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 51, *thd);
        threshold(maskimg, maskimg3, 80, 255, THRESH_BINARY);
        cvtColor(maskimg3, maskimg, COLOR_GRAY2RGB);

        cvtColor(img, img_hsv, COLOR_BGR2HSV); //# 色空間をBGRからHSVに変換
        split(img_hsv, planes);

        convertScaleAbs(planes[1], planes[1], 3.0, 0);
        merge(planes, img);

        cvtColor(img, img2, COLOR_HSV2BGR); //# 色空間をHSVからBGRに変換

        //cvtColor(img,img2,COLOR_RGB2HSV);
        //convertScaleAbs ( img, img2, 1.5, -10 );
        //cvtColor(temp,img2,COLOR_HSV2RGB);
        //equalizeHist(img,img2);   // 落ちる

        bitwise_or(img2, maskimg, ansimg);

        wimwrite(outpath, ansimg);
}

void C3(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    std::cout << "c3 read " << inpath << " h " << img.rows << " x w " << img.cols << std::endl;

    for (int y = 0; y < img.rows; y = y + 64) {
        std::cout << " h " << y << std::endl;
        int d = 32;
        if (y + d > img.rows) {
            d = img.rows - y;
        }
        for (int dy = 0; dy < d; ++dy) {
            for (int x = 0; x < img.cols; x = x + 64) {
                int d2 = 32;
                if (x + d2 > img.cols) {
                    d2 = img.cols - x;
                }
                for (int dx = 0; dx < d2; ++dx) {
                    img.at<cv::Vec3b>( y+dy, x+dx )[2] = 255;
                }
            }
        }
    }
    std::cout << "c3 loop " << std::endl;

    wimwrite(outpath, img);
}
void C4(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {
    int smpwidth = 8;
    Mat temp, img2, maskimg, maskimg2, maskimg3, ansimg;
    Mat img_hsv;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }


    std::vector<cv::Mat> planes;
    split(img, planes);

    Mat bwimg;
    cvtColor(img, bwimg, COLOR_BGR2GRAY);

    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> batbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> gatbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> ratbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> avtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    Mat bctbl = Mat::zeros(0, 1, CV_32FC3);


    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            batbl.at(y).at(x) = (calc_av(&planes[0], x * smpwidth, y * smpwidth, smpwidth, smpwidth));
            gatbl.at(y).at(x) = (calc_av(&planes[1], x * smpwidth, y * smpwidth, smpwidth, smpwidth));
            ratbl.at(y).at(x) = (calc_av(&planes[2], x * smpwidth, y * smpwidth, smpwidth, smpwidth));
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            avtbl.at(y).at(x) = a;
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            t = t + s;
        }
    }
    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));

    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        for (int x = 0; x < int(img.cols / smpwidth); x++) {
            if (stdtbl.at(y).at(x) < ta / 2) {
                Mat dotc = Mat::zeros(1, 1, CV_32FC3);
                cv::Vec3f* dst = dotc.ptr<cv::Vec3f>(0);
                dst[0] =(batbl.at(y).at(x));
                dst[1]=(gatbl.at(y).at(x));
                dst[2]=(ratbl.at(y).at(x));
                bctbl.push_back(dotc);
            }
        }
    }

    int n_clusters = 4;

    std::cout << " bctbl.size()=" << bctbl.size() << std::endl;
    cv::Mat clusters = Mat::zeros(bctbl.rows, 1, CV_32FC3);
    //cv::Mat clusters;
    cv::Mat centers;

    //cv::theRNG() = 19771228; //クラスタリングの結果を固定する場合
    std::cout << "kmeans start" << std::endl;
    //std::vector<std::vector<int>> centers;
    cv::kmeans(bctbl,
        n_clusters,
        clusters,
        cv::TermCriteria(3, 100, 1.0),
        1,
        cv::KMEANS_PP_CENTERS,
        centers);
    std::cout << "kmeans end" << std::endl;
    //std::cout << " clusters=" << clusters << std::endl;
    for (int n = 0; n < clusters.rows; ++n) {
        cv::Vec3f* dst = bctbl.ptr<cv::Vec3f>(n);
        std::cout << " clusters=" << clusters.at<int>(n) << " " << dst[0] << std::endl;
    }
    std::cout << " centers=" << centers << std::endl;

    int hist[256];
    memset(hist, 0, sizeof(int)*256);

    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        for (int x = 0; x < int(img.cols / smpwidth); x++) {
            hist[avtbl.at(y).at(x)] = hist[avtbl.at(y).at(x)] + 1;
            if (stdtbl.at(y).at(x) > ta / 2) {
                cv::rectangle(img, cv::Point(x * smpwidth, y * smpwidth), cv::Point(x * smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                    cv::Scalar(0, 0, 255), 1, cv::LINE_4);
            }
            else {
                for (int i = 0; i < smpwidth; ++i) {
                    //cv::Vec3b* src = cupimg.ptr<cv::Vec3b>(y * smpwidth + i);
                    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y * smpwidth + i);
                    for (int j = 0; j < smpwidth; ++j) {
                        dst[x * smpwidth + j][0] = batbl.at(y).at(x);
                        dst[x * smpwidth + j][1] = gatbl.at(y).at(x);
                        dst[x * smpwidth + j][2] = ratbl.at(y).at(x);
                    }
                }
            }
        }
    }
    //for (int n = 0; n < 256; ++n) {
    //    std::cout << hist[n] << std::endl;
    //}
    std::cout << std::endl;

    wimwrite(outpath, img);
}

*/

void circle(char* inpath, char* outpath, int* r, int* px, int* py, int* cr, int* cg, int* cb) {
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }

    cv::circle(img, cv::Point(*px, *py), *r, cv::Scalar(*cr, *cg, *cb), -1, cv::LINE_4);


    wimwrite(outpath, img);
}

void Poster3(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {
    int smpwidth = 8;
    Mat temp, img2, maskimg, maskimg2, maskimg3, ansimg;
    Mat img_hsv;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }

    if (img.rows % 8 != 0 || img.cols % 8 != 0) {
        img = cv::Mat(img, cv::Rect(0, 0, img.cols - img.cols % 8, img.rows - img.rows % 8));
    }
    *width = img.cols;
    *height = img.rows;


    Mat bwimg;
    cvtColor(img, bwimg, COLOR_BGR2GRAY);

    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));

    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            t = t + s;
        }
    }

    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));

    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        for (int x = 0; x < int(img.cols / smpwidth); x++) {
            if (stdtbl.at(y).at(x) > ta / 2) {
                cv::rectangle(img, cv::Point(x * smpwidth, y * smpwidth), cv::Point(x * smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                    cv::Scalar(255, 255, 255), -1, cv::LINE_4);
            }
        }
    }









    cv::resize(img, img2, cv::Size(img.cols / 8, img.rows / 8));
    cv::cvtColor(img2, img2, COLOR_BGR2HSV);

    int origRows = img2.rows;
    std::cout << "original image is: " << img2.rows << "x" << img2.cols << endl;
    Mat colVec = img2.reshape(1, img2.rows * img2.cols); // change to a Nx3 column vector
    cout << "colVec is of size: " << colVec.rows << "x" << colVec.cols << endl;
    Mat colVecD, bestLabels, centers, clustered;
    int attempts = 5;
    int clusts = 8; //*thd;
    if (clusts < 2) {
        clusts = 2;
    }
    double eps = 0.001;
    colVec.convertTo(colVecD, CV_32FC3, 1.0 / 255.0); // convert to floating point

    double compactness = kmeans(colVecD, clusts, bestLabels,
        TermCriteria(3, attempts, eps),
        attempts, KMEANS_PP_CENTERS, centers);

    Mat labelsImg = bestLabels.reshape(1, origRows); // single channel image of labels
    std::cout << "Compactness = " << compactness << endl;

    Mat cencolors = Mat(clusts, 3, CV_8U);
    for (int r = 0; r < clusts; ++r) {
        for (int c = 0; c < 3; ++c) {
            cencolors.at<unsigned char>(r, c) = centers.at<float>(r, c) * 256;
        }
        if (cencolors.at<unsigned char>(r, 2) > *thd * 4 && cencolors.at<unsigned char>(r, 1) < 50) {
            cencolors.at<unsigned char>(r, 0) = 0;
            cencolors.at<unsigned char>(r, 1) = 0;
            cencolors.at<unsigned char>(r, 2) = 255;
        }
    }
    /*
    cencolors.at<unsigned char>(0, 0) = 0;
    cencolors.at<unsigned char>(0, 1) = 0;
    cencolors.at<unsigned char>(0, 2) = 0;
    cencolors.at<unsigned char>(1, 0) = 255;
    cencolors.at<unsigned char>(1, 1) = 0;
    cencolors.at<unsigned char>(1, 2) = 0;
    cencolors.at<unsigned char>(2, 0) = 0;
    cencolors.at<unsigned char>(2, 1) = 255;
    cencolors.at<unsigned char>(2, 2) = 0;
    cencolors.at<unsigned char>(3, 0) = 255;
    cencolors.at<unsigned char>(3, 1) = 255;
    cencolors.at<unsigned char>(3, 2) = 0;
    cencolors.at<unsigned char>(4, 0) = 0;
    cencolors.at<unsigned char>(4, 1) = 0;
    cencolors.at<unsigned char>(4, 2) = 255;
    cencolors.at<unsigned char>(5, 0) = 255;
    cencolors.at<unsigned char>(5, 1) = 0;
    cencolors.at<unsigned char>(5, 2) = 255;
    cencolors.at<unsigned char>(6, 0) = 0;
    cencolors.at<unsigned char>(6, 1) = 255;
    cencolors.at<unsigned char>(6, 2) = 255;
    cencolors.at<unsigned char>(7, 0) = 255;
    cencolors.at<unsigned char>(7, 1) = 255;
    cencolors.at<unsigned char>(7, 2) = 255;
    */
    std::vector<int>  ccount(clusts);
    for (int n = 0; n < clusts; ++n) {
        ccount.at(n) = 0;
    }
    cout << "ccount clear" << endl;

    for (int y = 0; y < img2.rows; y++) {
        cv::Vec3b* dst = img2.ptr<cv::Vec3b>(y);

        for (int x = 0; x < img2.cols; x++) {
            int r = labelsImg.at<int>(y, x);
            dst[x][0] = cencolors.at<unsigned char>(r, 0);
            dst[x][1] = cencolors.at<unsigned char>(r, 1);
            dst[x][2] = cencolors.at<unsigned char>(r, 2);
            ccount.at(r)++;
        }
    }
    cout << "set img2 done" << endl;

    for (int n = 0; n < clusts; ++n) {
        cout << "[" << int(cencolors.at<unsigned char>(n, 0))
            << " " << int(cencolors.at<unsigned char>(n, 1))
            << " " << int(cencolors.at<unsigned char>(n, 2))
            << "] " << ccount.at(n) << endl;
    }

    cv::cvtColor(img2, img2, COLOR_HSV2BGR);
    cout << "img.rows=" << img.rows << " img.cols=" << img.cols << " img.dims=" << img.dims << endl;
    cout << "img.size=" << img.size << endl;
    cout << "img2.rows=" << img2.rows << " img2.cols=" << img2.cols << " img2.dims=" << img2.dims << endl;
    cout << "img2.size=" << img2.size << endl;

    cv::resize(img2, img, cv::Size(img.cols, img.rows));

    wimwrite(outpath, img);
}


void Poster2(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {
    int smpwidth = 8;
    Mat temp, img2, maskimg, maskimg2, maskimg3, ansimg;
    Mat img_hsv;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    *width = img.cols;
    *height = img.rows;

    if (img.rows % 8 != 0 || img.cols % 8 != 0) {
        img = cv::Mat(img, cv::Rect(0, 0, img.cols - img.cols % 8, img.rows - img.rows % 8));
    }
    cv::resize(img, img2, cv::Size(img.cols / 8, img.rows / 8));
    cv::cvtColor(img2, img2, COLOR_BGR2HSV);

    int origRows = img2.rows;
    std::cout << "original image is: " << img2.rows << "x" << img2.cols << endl;
    Mat colVec = img2.reshape(1, img2.rows * img2.cols); // change to a Nx3 column vector
    cout << "colVec is of size: " << colVec.rows << "x" << colVec.cols << endl;
    Mat colVecD, bestLabels, centers, clustered;
    int attempts = 5;
    int clusts = *thd;
    if (clusts < 2) {
        clusts = 2;
    }
    double eps = 0.001;
    colVec.convertTo(colVecD, CV_32FC3, 1.0 / 255.0); // convert to floating point

    double compactness = kmeans(colVecD, clusts, bestLabels,
        TermCriteria(3, attempts, eps),
        attempts, KMEANS_PP_CENTERS, centers);

    Mat labelsImg = bestLabels.reshape(1, origRows); // single channel image of labels
    std::cout << "Compactness = " << compactness << endl;

    Mat cencolors = Mat(clusts, 3, CV_8U);
    for (int r = 0; r < clusts; ++r) {
        for (int c = 0; c < 3; ++c) {
            cencolors.at<unsigned char>(r, c) = centers.at<float>(r, c) * 256;
        }
    }

    std::vector<int>  ccount(clusts);
    for (int n = 0; n < clusts; ++n) {
        ccount.at(n) = 0;
    }
    cout << "ccount clear" << endl;

    for (int y = 0; y < img2.rows; y++) {
        cv::Vec3b* dst = img2.ptr<cv::Vec3b>(y);

        for (int x = 0; x < img2.cols; x++) {
            int r = labelsImg.at<int>(y, x);
            dst[x][0] = cencolors.at<unsigned char>(r, 0);
            dst[x][1] = cencolors.at<unsigned char>(r, 1);
            dst[x][2] = cencolors.at<unsigned char>(r, 2);
            ccount.at(r)++;
        }
    }
    cout << "set img2 done" << endl;

    for (int n = 0; n < clusts; ++n) {
        cout << "[" << int(cencolors.at<unsigned char>(n, 0))
            << " " << int(cencolors.at<unsigned char>(n, 1))
            << " " << int(cencolors.at<unsigned char>(n, 2))
            << "] " << ccount.at(n) << endl;
    }
    cv::cvtColor(img2, img2, COLOR_HSV2BGR);

    cv::resize(img2, img, cv::Size(img.cols, img.rows));

    wimwrite(outpath, img2);
}


void Poster(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {
    int smpwidth = 8;
    Mat temp, img2, maskimg, maskimg2, maskimg3, ansimg;
    Mat img_hsv;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    *width = img.cols;
    *height = img.rows;

    if (img.rows % 8 != 0 || img.cols % 8 != 0) {
        img = cv::Mat(img, cv::Rect(0, 0, img.cols - img.cols % 8, img.rows - img.rows % 8));
    }
    cv::resize(img, img2, cv::Size(img.cols / 8, img.rows / 8));
    //cv::cvtColor(img2, img2, COLOR_BGR2HSV);

    int origRows = img2.rows;
    std::cout << "original image is: " << img2.rows << "x" << img2.cols << endl;
    Mat colVec = img2.reshape(1, img2.rows * img2.cols); // change to a Nx3 column vector
    cout << "colVec is of size: " << colVec.rows << "x" << colVec.cols << endl;
    Mat colVecD, bestLabels, centers, clustered;
    int attempts = 5;
    int clusts = *thd;
    if (clusts < 2) {
        clusts = 2;
    }
    double eps = 0.001;
    colVec.convertTo(colVecD, CV_32FC3, 1.0 / 255.0); // convert to floating point

    double compactness = kmeans(colVecD, clusts, bestLabels,
        TermCriteria(3, attempts, eps),
        attempts, KMEANS_PP_CENTERS, centers);

    Mat labelsImg = bestLabels.reshape(1, origRows); // single channel image of labels
    std::cout << "Compactness = " << compactness << endl;

    Mat cencolors = Mat(clusts, 3, CV_8U);
    for (int r = 0; r < clusts; ++r) {
        for (int c = 0; c < 3; ++c) {
            cencolors.at<unsigned char>(r, c) = centers.at<float>(r, c) * 256;
        }
    }

    cout << "labelsImg is of size: " << labelsImg.rows << "x" << labelsImg.cols << endl;

    cout << centers << endl;
    cout << "img2.rows=" << img2.rows << " img2.cols=" << img2.cols << " img2.dims=" << img2.dims << endl;
    cout << "img2.size=" << img2.size << endl;

    cout << "bestLabels.rows=" << bestLabels.rows << " bestLabels.cols=" << bestLabels.cols << " bestLabels.dims=" << bestLabels.dims << endl;
    cout << "bestLabels.size=" << bestLabels.size << endl;
    cout << "labelsImg.rows=" << labelsImg.rows << " labelsImg.cols=" << labelsImg.cols << " labelsImg.dims=" << labelsImg.dims << endl;
    cout << "labelsImg.size=" << labelsImg.size << endl;
    cout << "centers.rows=" << centers.rows << " centers.cols=" << centers.cols << " centers.dims=" << centers.dims << endl;
    cout << "centers.size=" << centers.size << endl;


    cout << "cencolors.rows=" << cencolors.rows << " cencolors.cols=" << cencolors.cols << " cencolors.dims=" << cencolors.dims << endl;
    cout << "cencolors.size=" << cencolors.size << endl;


    //cv::resize(labelsImg, img2, cv::Size(img.cols, img.rows));

    std::vector<int>  ccount(clusts);
    for (int n = 0; n < clusts; ++n) {
        ccount.at(n) = 0;
    }
    cout << "ccount clear" << endl;

    for (int y = 0; y < img2.rows; y++) {
        cv::Vec3b* dst = img2.ptr<cv::Vec3b>(y);

        for (int x = 0; x < img2.cols; x++) {
            //cv::Vec3b* dst = img2.ptr<cv::Vec3b>(y * img2.cols + x);
            //cv::Vec3b* dst = img2.ptr<cv::Vec3b>(y,x);

            int r = labelsImg.at<int>(y, x);
            dst[x][0] = cencolors.at<unsigned char>(r, 0);
            dst[x][1] = cencolors.at<unsigned char>(r, 1);
            dst[x][2] = cencolors.at<unsigned char>(r, 2);
            ccount.at(r)++;
        }
    }
    cout << "set img2 done" << endl;

    for (int n = 0; n < clusts; ++n) {
        cout << "[" << int(cencolors.at<unsigned char>(n, 0))
            << " " << int(cencolors.at<unsigned char>(n, 1))
            << " " << int(cencolors.at<unsigned char>(n, 2))
            << "] " << ccount.at(n) << endl;
    }
    //cv::cvtColor(img2, img2, COLOR_HSV2BGR);
    cv::resize(img2, img, cv::Size(img.cols, img.rows));

    wimwrite(outpath, img);
}





void C3(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {
    int smpwidth = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    *width = img.cols;
    *height = img.rows;

    std::vector<cv::Mat> planes;
    split(img, planes);

    Mat bwimg;
    cvtColor(img, bwimg, COLOR_BGR2GRAY);

    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> batbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> gatbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> ratbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));

    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            batbl.at(y).at(x) = calc_av(&planes[0], x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            gatbl.at(y).at(x) = calc_av(&planes[1], x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            ratbl.at(y).at(x) = calc_av(&planes[2], x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            t = t + s;
        }
    }

    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));
    std::cout << "c3 ta = " << ta << std::endl;

    Mat cupimg, tximg;

    cvtColor(img, cupimg, COLOR_BGR2RGB);
    cvtColor(cupimg, cupimg, COLOR_RGB2BGR);
    /*
    convertScaleAbs(img, cupimg, 2.0, 0);
    cvtColor(img, cupimg, COLOR_BGR2HSV); //# 色空間をBGRからHSVに変換

    split(cupimg, planes);
    //convertScaleAbs(planes[1], planes[1], 3.0, 0); //# 彩度を上げる
    merge(planes, cupimg);
    cvtColor(cupimg, cupimg, COLOR_HSV2BGR); //# 色空間をBGRからHSVに変換
    convertScaleAbs(cupimg, cupimg, 3.0, 0);
    */

    planes[0] = filterOne(planes[0], *thd);
    planes[1] = filterOne(planes[1], *thd);
    planes[2] = filterOne(planes[2], *thd);

    merge(planes, tximg);

    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        for (int x = 0; x < int(img.cols / smpwidth); x++) {
            if (stdtbl.at(y).at(x) > ta / 2) {
                cv::rectangle(tximg, cv::Point(x * smpwidth, y * smpwidth), cv::Point(x * smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                    cv::Scalar(0, 0, 255), 1, cv::LINE_4);
                for (int i = 0; i < smpwidth; ++i) {
                    cv::Vec3b* src = tximg.ptr<cv::Vec3b>(y * smpwidth + i);
                    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y * smpwidth + i);
                    for (int j = 0; j < smpwidth; ++j) {
                        dst[x * smpwidth + j][0] = src[x * smpwidth + j][0];
                        dst[x * smpwidth + j][1] = src[x * smpwidth + j][1];
                        dst[x * smpwidth + j][2] = src[x * smpwidth + j][2];
                    }
                }

            }
            else {

                for (int i = 0; i < smpwidth; ++i) {
                    //cv::Vec3b* src = cupimg.ptr<cv::Vec3b>(y * smpwidth + i);
                    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y * smpwidth + i);
                    for (int j = 0; j < smpwidth; ++j) {
                        //dst[x * smpwidth + j][0] = src[x * smpwidth + j][0];
                        //dst[x * smpwidth + j][1] = src[x * smpwidth + j][1];
                        //dst[x * smpwidth + j][2] = src[x * smpwidth + j][2];
                        dst[x * smpwidth + j][0] = batbl.at(y).at(x);
                        dst[x * smpwidth + j][1] = gatbl.at(y).at(x);
                        dst[x * smpwidth + j][2] = ratbl.at(y).at(x);
                    }
                }
            }
        }
    }

    wimwrite(outpath, img);
}


void C2(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {
    Mat temp, img2, colimg, maskimg, maskimg2, maskimg3, ansimg;
    Mat img_hsv;
    std::vector<cv::Mat> planes;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    *width = img.cols;
    *height = img.rows;
    std::string outpathdir = getPathDir(outpath);

    std::cout << "c2 read " << inpath << " " << outpathdir << " " << *thd << std::endl;
    //wimwrite((char*)(outpathdir + "orgimg.jpg").data(), img);

    // 文字部分だけ黒く抜いたマスク作成
    cvtColor(img, temp, COLOR_RGB2GRAY);
    adaptiveThreshold(temp, maskimg, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 51, 20);
    threshold(maskimg, maskimg3, 80, 255, THRESH_BINARY);
    cvtColor(maskimg3, maskimg, COLOR_GRAY2RGB);

    //wimwrite((char*)(outpathdir + "charmask.jpg").data(), maskimg);
    // 色部分だけ抜いたマスク
    cvtColor(img, img_hsv, COLOR_BGR2HSV); //# 色空間をBGRからHSVに変換
    split(img_hsv, planes);
    std::cout << "c2 split " << std::endl;

    //convertScaleAbs(planes[1], planes[1], 5.0, 0);
    threshold(planes[1], planes[1], *thd * 8, 255, THRESH_BINARY);   // 色の付いた部分だけ残す
    cvtColor(planes[1], maskimg2, COLOR_GRAY2BGR); //# 色空間をHSVからBGRに変換 色部分だけのマスク

    //wimwrite((char*)(outpathdir + "colormask.jpg").data(), maskimg2);


    // 文字から色部分を抜く
    bitwise_or(maskimg, maskimg2, img2);
    //wimwrite((char*)(outpathdir + "onlychar.jpg").data(), img2);



    // 色部分だけ抜き出し
    bitwise_not(maskimg2, maskimg2);
    bitwise_or(img, maskimg2, colimg);
    //wimwrite((char*)(outpathdir + "onlycolor.jpg").data(), colimg);

    bitwise_and(img2, colimg, ansimg);


    std::cout << "c2 done " << std::endl;
    wimwrite(outpath, ansimg);
}

void C1(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {

    Mat temp, rimg, gimg, bimg;
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    *width = img.cols;
    *height = img.rows;

    std::vector<cv::Mat> planes;

    split(img, planes);

    planes[0] = filterOne(planes[0], *thd);
    planes[1] = filterOne(planes[1], *thd);
    planes[2] = filterOne(planes[2], *thd);

    merge(planes, img);

    //char pbuff[256];
    //sprintf(pbuff, "cols %d, rows %d", img.cols, img.rows);
    //cv::putText(img, pbuff, cv::Size(30, 30), 1, 1.5, 2, 2);

    //*width = img.cols;
    //*height = img.rows;

    wimwrite(outpath, img);
}

//std::u16string utf8_to_utf16(std::string const& src)
//{
//    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
//    return converter.from_bytes(src);
//}
//std::wstring utf8_to_wide_cppapi(std::string const& src)
//{
//    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
//    return converter.from_bytes(src);
//}

void BW(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    *width = img.cols;
    *height = img.rows;

    Mat temp;
    cvtColor(img, temp, COLOR_BGR2GRAY);

    temp = filterOne(temp, *thd);

    wimwrite(outpath, temp);
}

void NON(char* inpath, char* outpath, unsigned int* thd, int* width, int* height) {
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    *width = img.cols;
    *height = img.rows;


    wimwrite(outpath, img);
}
