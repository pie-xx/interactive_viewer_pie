// filterDll1.cpp : DLL 用にエクスポートされる関数を定義します。
//
#include "pch.h"
#include "framework.h"

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

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/flann/flann.hpp>

using namespace cv;
using std::cout; using std::cin;
using std::endl; using std::ifstream;

#include "ImgProcDll.h"


std::vector<std::string>  ftbl = { "NON","BW","C1","C3","VAR","VAR2" ,"VAR3","VAR4","VAR4box","VAR5" };

//void getlen(int* count) {
//    *count = ftbl.size();
//}
int getlen() {
    return ftbl.size();
}
/*
void getname(int* n, char* fname, int* size) {


    std::cout << "getname input " << n << " " << fname << " size=" << *size << std::endl;

    strncpy_s(fname, *size, ftbl.at(*n).data(), ftbl.at(*n).size());
    *size = ftbl.at(*n).size();

    std::cout << "getname output " << fname << " " << *size << std::endl;
}
*/
void getname(int n, char* fname, int* size) {


    std::cout << "getname input " << n << " " << fname << " size=" << *size << std::endl;

    strncpy_s(fname, *size, ftbl.at(n).data(), ftbl.at(n).size());
    *size = ftbl.at(n).size();

    std::cout << "getname output " << fname << " " << *size << std::endl;
}
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


void circle(char* inpath, char* outpath, int* r, int* px, int* py, int* cr, int* cg, int* cb) {
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }

    cv::circle(img, cv::Point(*px, *py), *r, cv::Scalar(*cr, *cg, *cb), -1, cv::LINE_4);


    wimwrite(outpath, img);
}

void rectangle(char* inpath, char* outpath, int* inpara, int* outpara) {
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }

    int x = inpara[0];
    int y = inpara[1];
    int w = inpara[2];
    int h = inpara[3];
    int cr = inpara[4];
    int cg = inpara[5];
    int cb = inpara[6];
    int thic = inpara[7];

    cout << "rectangle x="<< x << " y="<< y <<" w="<<w<<" h="<<h << " r="<<cr<<" g="<<cg<<" b="<<cb<<" th="<<thic << endl;
    cv::rectangle(img, cv::Rect(x,y,w,h), cv::Scalar(cr, cg, cb), thic);

    wimwrite(outpath, img);
}

void Poster3(char* inpath, char* outpath, int* inpara, int* outpara) {
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
    outpara[0] = img.cols;
    outpara[1] = img.rows;

    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

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
    double thd = ta * 1.0;

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
        if (cencolors.at<unsigned char>(r, 2) > thd * 4 && cencolors.at<unsigned char>(r, 1) < 50) {
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

void dispTap(char* inpath, char* outpath, unsigned int* ipara, int* outpara) {
    const int chartWidth = 1024;
    const int chartHeight = 256;
    const int margin = 20;
    const int sn = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }

    int x = ipara[0];
    int y = ipara[1];

    cv::line(img, Point(x, 0), Point(x, img.rows), cv::Scalar(0, 0, 255), 1, cv::LINE_4);
    cv::line(img, Point(0, y), Point(img.cols, y), cv::Scalar(0, 0, 255), 1, cv::LINE_4);

    wimwrite(outpath, img);

}


void LongSpaceChart(char* inpath, char* outpath1, char* outpath2, unsigned int* ipara, int* outpara) {
    const int smpwidth = 8;
    const int chartWidth = 1024;
    const int chartHeight = 256;
    const int margin = 20;
    const int sn = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

    int tapx = ipara[0];
    int tapy = ipara[1];

    // 分散表の作成
    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    double maxt = 0;
    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            if (maxt < s) {
                maxt = s;
            }
            t = t + s;
        }
    }
    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));
    double thd = ta * 0.75;

    Mat chartimg = cv::Mat::zeros(chartHeight + margin * 2, chartWidth + margin * 2, CV_8UC3);
    cv::rectangle(chartimg, Rect(margin, margin, chartWidth, chartHeight), cv::Scalar(255, 255, 255), -1, cv::LINE_4);

    cout << "cx vector" << std::endl;


    for (int cx = 0; cx < int(bwimg.cols / smpwidth); ++cx) {

        int maxsp = 0;
        int splen = 0;
        bool osp = TRUE;
        for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
            if (stdtbl.at(y).at(cx) < thd) {
                if (osp) {
                    splen++;
                    if (maxsp < splen) {
                        maxsp = splen;
                    }
                }
                osp = TRUE;
            }
            else {
                osp = FALSE;
                splen = 0;
            }
        }
        cout << "cx="<< cx << " maxsp="<< maxsp << std::endl;

        cv::rectangle(chartimg, Rect((cx-1) * smpwidth * chartWidth / img.cols + margin, margin,
                                        smpwidth * chartWidth / img.cols + margin, maxsp* smpwidth* chartHeight / img.rows),
            Scalar(0, 0, 255), -1, cv::LINE_4);
    }
    wimwrite(outpath1, chartimg);

    const int chart2Width = 256;
    const int chart2Height = 1024;

    Mat chart2img = cv::Mat::zeros(chart2Height + margin * 2, chart2Width + margin * 2, CV_8UC3);
    cv::rectangle(chart2img, Rect(margin, margin, chart2Width, chart2Height), cv::Scalar(255, 255, 255), -1, cv::LINE_4);

    int ox, oy;
    ox = oy = 0;
    for (int cy = 0; cy < int(bwimg.rows / smpwidth); ++cy) {

        int maxsp = 0;
        int splen = 0;
        bool osp = TRUE;
        for (int cx = 0; cx < int(bwimg.cols / smpwidth); cx++) {
            if (stdtbl.at(cy).at(cx) < thd) {
                if (osp) {
                    splen++;
                    if (maxsp < splen) {
                        maxsp = splen;
                    }
                }
                osp = TRUE;
            }
            else {
                osp = FALSE;
                splen = 0;
            }
        }
        cout << "cy=" << cy << " maxsp=" << maxsp << std::endl;

        cv::rectangle(chart2img, Rect( margin, (cy - 1) * smpwidth * chart2Height / img.rows + margin,
            maxsp * smpwidth * chart2Width / img.cols, smpwidth * chart2Height / img.rows),
            Scalar(0, 0, 255), -1, cv::LINE_4);
        cout << "*cy=" << cy * smpwidth * chart2Height / img.rows << " *maxsp=" << maxsp * smpwidth * chart2Width / img.cols + margin << std::endl;
    }


    wimwrite(outpath2, chart2img);
}


void XYVarChart(char* inpath, char* outpath1, char* outpath2, unsigned int* ipara, int* outpara) {
    const int smpwidth = 8;
    const int chartWidth = 1024;
    const int chartHeight = 256;
    const int margin = 20;
    const int sn = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

    int tapx = ipara[0];
    int tapy = ipara[1];

    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));

    double maxt = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            if (maxt < s) {
                maxt = s;
            }
        }
    }

    Mat chartimg = cv::Mat::zeros(chartHeight + margin * 2, chartWidth + margin * 2, CV_8UC3);
    cv::rectangle(chartimg, Rect(margin, margin, chartWidth, chartHeight), cv::Scalar(255, 255, 255), -1, cv::LINE_4);

    cout << "cx vector" << std::endl;

    int ox = 0, oy = 0;
    for (int cx = 0; cx < img.cols; ++cx) {
        int val = stdtbl.at(tapy / smpwidth).at(cx / smpwidth) * chartHeight / maxt;

        cv::line(chartimg, Point(ox * chartWidth / img.cols + margin, oy + margin), Point(cx * chartWidth / img.cols + margin, val + margin),
            Scalar(0, 0, 255), 1, cv::LINE_4);
        ox = cx; oy = val;
    }
    wimwrite(outpath1, chartimg);

    const int chart2Width = 256;
    const int chart2Height = 1024;

    Mat chart2img = cv::Mat::zeros(chart2Height + margin * 2, chart2Width + margin * 2, CV_8UC3);
    cv::rectangle(chart2img, Rect(margin, margin, chart2Width, chart2Height), cv::Scalar(255, 255, 255), -1, cv::LINE_4);

    ox = 0; oy = 0;
    for (int cy = 0; cy < img.rows; ++cy) {
        int val = stdtbl.at(cy / smpwidth).at(tapx / smpwidth) * chart2Width / maxt;

        cv::line(chart2img, Point(ox + margin, oy * chart2Height / img.rows + margin), Point(val + margin, cy * chart2Height / img.rows + margin),
            Scalar(0, 0, 255), 1, cv::LINE_4);
        ox = val; oy = cy;
    }


    wimwrite(outpath2, chart2img);
}


/*
* XY方向の明度の折れ線チャート作成
*/
void XYGrayChart(char* inpath, char* outpath1, char* outpath2, unsigned int* ipara, int* outpara) {
    const int chartWidth = 1024;
    const int chartHeight = 256;
    const int margin = 20;
    const int sn = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

    int x = ipara[0];
    int y = ipara[1];

    Mat chartimg = cv::Mat::zeros(chartHeight + margin * 2, chartWidth + margin * 2, CV_8UC3);
    cv::rectangle(chartimg, Rect(margin, margin, chartWidth, chartHeight), cv::Scalar(255, 255, 255), -1, cv::LINE_4);

    cout << "cx vector" << std::endl;
    std::vector<unsigned char*> srcs(sn);
    for (int n = 0; n < sn; ++n) {
        srcs.at(n) = bwimg.ptr<unsigned char>(y - n - sn / 2);
    }
    int ox = 0, oy = 0;
    for (int cx = 0; cx < img.cols; ++cx) {
        int val = 0;
        for (int n = 0; n < srcs.size(); ++n) {
            val = val + srcs[n][cx];
        }
        val = val / sn;

        cv::line(chartimg, Point(ox * chartWidth / img.cols + margin, oy + margin), Point(cx * chartWidth / img.cols + margin, val + margin),
            Scalar(0, 0, 255), 1, cv::LINE_4);
        ox = cx; oy = val;
    }
    wimwrite(outpath1, chartimg);

    const int chart2Width = 256;
    const int chart2Height = 1024;

    Mat chart2img = cv::Mat::zeros(chart2Height + margin * 2, chart2Width + margin * 2, CV_8UC3);
    cv::rectangle(chart2img, Rect(margin, margin, chart2Width, chart2Height), cv::Scalar(255, 255, 255), -1, cv::LINE_4);

    ox = 0; oy = 0;
    for (int cy = 0; cy < img.rows; ++cy) {
        unsigned char* src = bwimg.ptr<unsigned char>(cy);
        int val = 0;
        for (int n = 0; n < sn; ++n) {
            val = val + src[x + n - sn / 2];
        }
        val = val / sn;
        cv::line(chart2img, Point(ox + margin, oy * chart2Height / img.rows + margin), Point(val + margin, cy * chart2Height / img.rows + margin),
            Scalar(0, 0, 255), 1, cv::LINE_4);
        ox = val; oy = cy;
    }


    wimwrite(outpath2, chart2img);
}

void makeHistgram(Mat img, int* hist) {
    //cout << "makeHistgram " << std::endl;
    //cout << "img.cols =" << img.cols << std::endl;
    //cout << "img.rows =" << img.rows << std::endl;
    //cout << "img.type()=" << img.type() << std::endl;
    if (img.type() != CV_8U) {
        return;
    }
    for (int n = 0; n < 256; ++n) {
        hist[n] = 0;
    }
    for (int y = 0; y < img.rows; ++y) {
        for (int x = 0; x < img.cols; ++x) {
            hist[img.data[y * img.cols + x]]++;
        }
    }
    //for (int n = 0; n < 256; ++n) {
    //    cout << n << " = " << hist[n] << std::endl;
    //}
}

void makeHistgramList(char* inpath, int* hist ) {

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    Mat temp;
    cv::cvtColor(img, temp, COLOR_BGR2GRAY);

    makeHistgram(temp, hist);
}

void makeLocalHistgramList(char* inpath, int* ipara, int* hist) {
    //cout << "makeLocalHistgramList " << std::endl;
    //cout << "px=" << ipara[0] << std::endl;
    //cout << "py=" << ipara[1] << std::endl;
    const int smpwidth = 8;
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    Mat temp;
    cv::cvtColor(img, temp, COLOR_BGR2GRAY);

    int x = ipara[0];
    int y = ipara[1];

    Mat locimg = cv::Mat(temp, cv::Rect(x-x% smpwidth, y-y% smpwidth, smpwidth, smpwidth));

    makeHistgram(locimg, hist);
}


void makeHistgramChart( char* outpath, int* inpara, int* hist ) {

    double tv = 0;
    int maxh = 0;
    int tn = 0;
    for (int n = 0; n < 256; ++n) {
        tv = tv + n * hist[n];
        tn = tn + hist[n];
        if (maxh < hist[n]) {
            maxh = hist[n];
        }
    }
    double avr = tv / tn;

    int px = inpara[0];
    int py = inpara[1];
    int width = inpara[2];
    int height = inpara[3];

    //Mat img = wimread(outpath);
    //if (img.size == 0) {
    Mat    img = cv::Mat::zeros(height+py, width+px, CV_8UC3);
    //}
    cv::rectangle(img, Rect(px, py, width, height), cv::Scalar(255, 255, 255), -1, cv::LINE_4);
    cv::rectangle(img, Rect(px, py, width, height), cv::Scalar(0, 0, 0), 1, cv::LINE_4);

    int ohx = 0;
    for (int n = 0; n < 256; ++n) {
        int hy = height * hist[n] / maxh;
        int hx = width * (n+1) / 256;

        int pw = hx - ohx;
        if (pw == 0) {
            pw = 1;
        }
        cv::rectangle(img, Rect(px+ohx, py+(height-hy), pw, hy), cv::Scalar(255, 0, 0), -1, cv::LINE_4);
        ohx = hx;
    }

    int ax = width * (avr) / 256;
    cv::rectangle(img, Rect(px + ax, py, 1, height), cv::Scalar(0, 0, 255), -1, cv::LINE_4);

    wimwrite(outpath, img);
}

/*
* 指定位置の色情報を返す
*/
void getPosColor(char* inpath, char* outpath, unsigned int* ipara, int* outpara) {
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    int x = ipara[0];
    int y = ipara[1];

    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y);

    outpara[0] = dst[x][0];
    outpara[1] = dst[x][1];
    outpara[2] = dst[x][2];
}
/*
* 縦書き横書きの判定
* return Horizontal_writing:横書き　Vertical_writing :縦書き
*/
WRITING_DIR _isHorV(Mat bwimg, int tapx, int tapy) {
    const int chksize = 64;
    const int hfcsize = chksize / 2;
    cout << "tapx=" << tapx << std::endl;
    cout << "tapy=" << tapy << std::endl;

    if (tapx < hfcsize) {
        tapx = hfcsize;
    }
    else
        if (tapx > bwimg.cols - hfcsize) {
            tapx = bwimg.cols - hfcsize;

        }
    if (tapy < hfcsize) {
        tapy = hfcsize;
    }
    else
        if (tapy > bwimg.rows - hfcsize) {
            tapy = bwimg.rows - hfcsize;

        }
    //cout << "tapx=" << tapx << std::endl;
    //cout << "tapy=" << tapy << std::endl;

    std::vector<double> stdHtbl(chksize);
    std::vector<double> stdVtbl(chksize);
    double ha = 0;
    //double va = 0;
    for (int n = 0; n < chksize; ++n) {
        double tx = 0;
        double ty = 0;
        unsigned char* src = bwimg.ptr<unsigned char>(tapy + n - hfcsize);
        for (int m = 0; m < chksize; ++m) {
            tx = tx + src[tapx + m - hfcsize];
            unsigned char* srcx = bwimg.ptr<unsigned char>(tapy + m - hfcsize);
            ty = ty + srcx[tapx + n - hfcsize];
        }
        //cout << std::endl;
        stdHtbl.at(n) = tx / chksize;
        stdVtbl.at(n) = ty / chksize;
        ha = ha + tx / chksize;
        //va = va + ty / chksize;
    }
    ha = ha / chksize;
    //va = va / chksize;
    cout << "ha=" << ha << std::endl;
    //cout << "va=" << va << std::endl;

    double hast = 0;
    double vast = 0;
    for (int n = 0; n < chksize; ++n) {
        hast = hast + (stdHtbl.at(n) - ha) * (stdHtbl.at(n) - ha);
        vast = vast + (stdVtbl.at(n) - ha) * (stdVtbl.at(n) - ha);
    }
    cout << "hast=" << hast << std::endl;
    cout << "vast=" << vast << std::endl;

    //outpara[0] = (hast > vast) ? 0 : 1; // 0: H横書き 1: V縦書き

    return (hast > vast) ? Horizontal_writing : Vertical_writing; // 0: H横書き 1: V縦書き

}

/*
* 縦書き横書きの判定 stdtbl
* return Horizontal_writing:横書き　Vertical_writing :縦書き
*/
WRITING_DIR _isHorV2(Mat stdtbl, int x, int y) {
    const int chksize = 8;
    const int hfcsize = chksize / 2;

    int tapx = x / 8;
    int tapy = y / 8;

    if (tapx < hfcsize) {
        tapx = hfcsize;
    }
    else
        if (tapx > stdtbl.cols - hfcsize) {
            tapx = stdtbl.cols - hfcsize;

        }
    if (tapy < hfcsize) {
        tapy = hfcsize;
    }
    else
        if (tapy > stdtbl.rows - hfcsize) {
            tapy = stdtbl.rows - hfcsize;

        }
    //cout << "tapx=" << tapx << std::endl;
    //cout << "tapy=" << tapy << std::endl;

    std::vector<double> stdHtbl(chksize);
    std::vector<double> stdVtbl(chksize);
    double ha = 0;
    //double va = 0;
    for (int n = 0; n < chksize; ++n) {
        double tx = 0;
        double ty = 0;
        unsigned char* src = stdtbl.ptr<unsigned char>(tapy + n - hfcsize);
        for (int m = 0; m < chksize; ++m) {
            tx = tx + src[tapx + m - hfcsize];
            unsigned char* srcx = stdtbl.ptr<unsigned char>(tapy + m - hfcsize);
            ty = ty + srcx[tapx + n - hfcsize];
        }
        //cout << std::endl;
        stdHtbl.at(n) = tx / chksize;
        stdVtbl.at(n) = ty / chksize;
        ha = ha + tx / chksize;
        //va = va + ty / chksize;
    }
    ha = ha / chksize;
    //va = va / chksize;
    cout << "ha=" << ha << std::endl;
    //cout << "va=" << va << std::endl;

    double hast = 0;
    double vast = 0;
    for (int n = 0; n < chksize; ++n) {
        hast = hast + (stdHtbl.at(n) - ha) * (stdHtbl.at(n) - ha);
        vast = vast + (stdVtbl.at(n) - ha) * (stdVtbl.at(n) - ha);
    }
    cout << "hast=" << hast << std::endl;
    cout << "vast=" << vast << std::endl;

    //outpara[0] = (hast > vast) ? 0 : 1; // 0: H横書き 1: V縦書き

    return (hast > vast) ? Horizontal_writing : Vertical_writing; // 0: H横書き 1: V縦書き

}

void markCharMap(char* inpath, char* outpath, int* ipara, int* outpara) {
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    Mat grayimg;
    cv::cvtColor(img, grayimg, COLOR_BGR2GRAY);
    Mat resultimg;
    cv::cvtColor(img, resultimg, COLOR_BGR2GRAY);

    for (int y = 0; y < img.rows; ++y) {
        unsigned char* src = resultimg.ptr<unsigned char>(y);
        for (int x = 0; x < img.cols; ++x) {
            if (_isHorV(grayimg, x, y) == Horizontal_writing) {
                src[x] = 255;
            }
            else {
                src[x] = 0;
            }
        }
    }

    wimwrite(outpath, resultimg);
}

void markChar(char* inpath, char* outpath, int* ipara, int* outpara) {
    int smpwidth = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    Mat grayimg;
    cv::cvtColor(img, grayimg, COLOR_BGR2GRAY);

    WRITING_DIR dir = _isHorV(grayimg, ipara[0], ipara[1]);
    cout << "dir=" << dir << std::endl;

    std::vector<cv::Mat> planes;
    cv::split(img, planes);


    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));

    double t = 0;
    for (int y = 0; y < int(grayimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(grayimg.cols / smpwidth); x++) {
            double a = calc_av(&grayimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            double s = calc_var(&grayimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            t = t + s;
        }
    }

    double ta = t / (int(grayimg.rows / smpwidth) * int(grayimg.cols / smpwidth));
    std::cout << "markChar ta = " << ta << std::endl;

    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        for (int x = 0; x < int(img.cols / smpwidth) - 1; x++) {
            if (stdtbl.at(y).at(x) > ta / 2) {
                cv::rectangle(img, cv::Point(x * smpwidth, y * smpwidth), cv::Point((x + 1) * smpwidth - 1, y * smpwidth + smpwidth - 1),
                    cv::Scalar(0, 0, 255), 1, cv::LINE_4);
            }
        }
    }

    //縦方向連続空白サーチ
    int wc = 0; int bc = 0;

    cv::rectangle(grayimg, cv::Point(0, 0), cv::Point(img.cols, img.rows),
        cv::Scalar(0, 0, 0), -1, cv::LINE_4);

    if (dir == Vertical_writing) {
        //縦方向連続空白サーチ
        for (int x = 0; x < int(img.cols / smpwidth) - 1; x++) {
            wc = bc = 0;
            for (int y = 0; y < int(img.rows / smpwidth); y++) {
                if (stdtbl.at(y).at(x) > ta / 2) {
                    if (wc > 12) {
                        cv::rectangle(grayimg, cv::Point(x * smpwidth, (y - wc) * smpwidth), cv::Point((x + 1) * smpwidth - 1, y * smpwidth + smpwidth - 1),
                            cv::Scalar(255, 255, 255), -1, cv::LINE_4);
                    }
                    if (wc > 0) {
                        bc = wc = 0;
                    }
                    bc++;
                }
                else {
                    if (bc > 0) {
                        bc = wc = 0;
                    }
                    wc++;
                }
            }
            if (wc > 12) {
                cv::rectangle(grayimg, cv::Point(x * smpwidth, (int(img.rows / smpwidth) - wc) * smpwidth), cv::Point((x + 1) * smpwidth - 1, int(img.rows / smpwidth) * smpwidth + smpwidth - 1),
                    cv::Scalar(0, 255, 0), -1, cv::LINE_4);
            }
        }
    }
    else {
        //横方向連続空白サーチ
        for (int y = 0; y < int(img.rows / smpwidth); y++) {
            wc = bc = 0;
            for (int x = 0; x < int(img.cols / smpwidth) - 1; x++) {
                if (stdtbl.at(y).at(x) > ta / 2) {
                    if (wc > 12) {
                        cv::rectangle(grayimg, cv::Point((x - wc) * smpwidth, (y)*smpwidth), cv::Point((x + 1) * smpwidth - 1, (y + 1) * smpwidth + smpwidth - 1),
                            cv::Scalar(255, 255, 255), -1, cv::LINE_4);
                    }
                    if (wc > 0) {
                        bc = wc = 0;
                    }
                    bc++;
                }
                else {
                    if (bc > 0) {
                        bc = wc = 0;
                    }
                    wc++;
                }
            }
            if (wc > 12) {
                cv::rectangle(grayimg, cv::Point((int(img.cols / smpwidth) - wc) * smpwidth, (y)*smpwidth), cv::Point((int(img.cols / smpwidth) + 1) * smpwidth - 1, (y + 1) * smpwidth + smpwidth - 1),
                    cv::Scalar(0, 255, 0), -1, cv::LINE_4);
            }
        }
    }

    Mat bwimg;
    cv::threshold(grayimg, bwimg, 50, 255, cv::THRESH_BINARY);

    Mat labelImage(img.size(), CV_32S);;
    int labels_n = cv::connectedComponents(bwimg, labelImage, 8, CV_32S);
    cout << "labels_n " << labels_n << std::endl;
    std::vector<Vec3b> colors(labels_n);
    colors[0] = Vec3b(0, 0, 0);//background
    for (int label = 1; label < labels_n; ++label) {
        colors[label] = Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
    }
    Mat dst(img.size(), CV_8UC3);
    for (int r = 0; r < dst.rows; ++r) {
        for (int c = 0; c < dst.cols; ++c) {
            int label = labelImage.at<int>(r, c);
            Vec3b& pixel = dst.at<Vec3b>(r, c);
            pixel = colors[label];
        }
    }
    wimwrite(outpath, dst);

    /*
    Mat labels;
    Mat stats;
    Mat centroids;
    cv::connectedComponentsWithStats(bwimg, labels, stats, centroids);

    //std::cout << labels << std::endl;
    std::cout << "stats.size()=" << stats.size() << std::endl;
    //std::cout << centroids << std::endl;

    for (int i = 0; i < stats.rows; i++)
    {
        int x = stats.at<int>(Point(0, i));
        int y = stats.at<int>(Point(1, i));
        int w = stats.at<int>(Point(2, i));
        int h = stats.at<int>(Point(3, i));

        std::cout << "x=" << x << " y=" << y << " w=" << w << " h=" << h << std::endl;

        Scalar color(255, 0, 0);
        Rect rect(x, y, w, h);
        cv::rectangle(bwimg, rect, color);
    }
    wimwrite(outpath, bwimg);
    */
}


void isHorV(char* inpath, char* outpath, int* ipara, int* outpara) {


    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    int tapx = ipara[0];
    int tapy = ipara[1];
    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

    outpara[0] = _isHorV(bwimg, tapx, tapy);
}

/*
* 文字部分を消去し、背景だけ残す
*/
void C3(char* inpath, char* outpath, int* inpara, int* outpara) {
    int smpwidth = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    outpara[0] = img.cols;
    outpara[1] = img.rows;

    std::vector<cv::Mat> planes;
    cv::split(img, planes);

    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

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
    //planes[0] = filterOne(planes[0], *thd);
    //planes[1] = filterOne(planes[1], *thd);
    //planes[2] = filterOne(planes[2], *thd);

    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        int r = 0;
        for (int x = 1; x < int(img.cols / smpwidth) - 1; x++) {
            if (stdtbl.at(y).at(x) > ta / 2) {
                r++;
            }
            else {
                if (r > 0) {
                    double bs = batbl.at(y).at(x);
                    double be = batbl.at(y).at(x - r - 1);
                    double bd = (be - bs) / r;
                    double gs = gatbl.at(y).at(x);
                    double ge = gatbl.at(y).at(x - r - 1);
                    double gd = (ge - gs) / r;
                    double rs = ratbl.at(y).at(x);
                    double re = ratbl.at(y).at(x - r - 1);
                    double rd = (re - rs) / r;
                    for (int n = 0; n < r; ++n) {
                        int dx = x - n - 1;
                        //cv::rectangle(img, cv::Point(dx * smpwidth, y * smpwidth), cv::Point(dx * smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                        //    cv::Scalar(batbl.at(y).at(x)+bd*n, gatbl.at(y).at(x)+gd*n, ratbl.at(y).at(x)+rd*n), -1, cv::LINE_4);
                        batbl.at(y).at(x - n - 1) = batbl.at(y).at(x) + bd * n;
                        gatbl.at(y).at(x - n - 1) = gatbl.at(y).at(x) + gd * n;
                        ratbl.at(y).at(x - n - 1) = ratbl.at(y).at(x) + rd * n;
                    }
                    //cv::rectangle(img, cv::Point(x * smpwidth - smpwidth * r, y * smpwidth), cv::Point(x * smpwidth - 1, y * smpwidth + smpwidth - 1),
                    //    cv::Scalar(255, 0, 0), 1, cv::LINE_4);
                    //cv::rectangle(img, cv::Point((x - r -1) * smpwidth, y * smpwidth), cv::Point((x -r- 1) * smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                    //    cv::Scalar(0, 0, 255), 1, cv::LINE_4);
                    //cv::rectangle(img, cv::Point((x) * smpwidth, y * smpwidth), cv::Point((x) * smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                    //    cv::Scalar(0, 0, 255), 1, cv::LINE_4);
                }
                r = 0;
            }
        }
    }

    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        for (int x = 1; x < int(img.cols / smpwidth) - 1; x++) {
            cv::rectangle(img, cv::Point((x - 1) * smpwidth, y * smpwidth), cv::Point(x * smpwidth - 1, y * smpwidth + smpwidth - 1),
                cv::Scalar(batbl.at(y).at(x), gatbl.at(y).at(x), ratbl.at(y).at(x)), -1, cv::LINE_4);
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
    cv::cvtColor(img, temp, COLOR_RGB2GRAY);
    adaptiveThreshold(temp, maskimg, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 51, 20);
    cv::threshold(maskimg, maskimg3, 80, 255, THRESH_BINARY);
    cv::cvtColor(maskimg3, maskimg, COLOR_GRAY2RGB);

    //wimwrite((char*)(outpathdir + "charmask.jpg").data(), maskimg);
    // 色部分だけ抜いたマスク
    cv::cvtColor(img, img_hsv, COLOR_BGR2HSV); //# 色空間をBGRからHSVに変換
    cv::split(img_hsv, planes);
    std::cout << "c2 split " << std::endl;

    //convertScaleAbs(planes[1], planes[1], 5.0, 0);
    cv::threshold(planes[1], planes[1], *thd * 8, 255, THRESH_BINARY);   // 色の付いた部分だけ残す
    cv::cvtColor(planes[1], maskimg2, COLOR_GRAY2BGR); //# 色空間をHSVからBGRに変換 色部分だけのマスク

    //wimwrite((char*)(outpathdir + "colormask.jpg").data(), maskimg2);


    // 文字から色部分を抜く
    cv::bitwise_or(maskimg, maskimg2, img2);
    //wimwrite((char*)(outpathdir + "onlychar.jpg").data(), img2);



    // 色部分だけ抜き出し
    cv::bitwise_not(maskimg2, maskimg2);
    cv::bitwise_or(img, maskimg2, colimg);
    //wimwrite((char*)(outpathdir + "onlycolor.jpg").data(), colimg);

    cv::bitwise_and(img2, colimg, ansimg);


    std::cout << "c2 done " << std::endl;
    wimwrite(outpath, ansimg);
}
////////////////////////////////////////////////////////////////////////////////////////

void C1(char* inpath, char* outpath, int* inpara, int* outpara) {

    Mat temp, rimg, gimg, bimg;
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    outpara[0] = img.cols;
    outpara[1] = img.rows;

    std::vector<cv::Mat> planes;

    cv::split(img, planes);

    planes[0] = filterOne(planes[0], inpara[0]);
    planes[1] = filterOne(planes[1], inpara[0]);
    planes[2] = filterOne(planes[2], inpara[0]);

    merge(planes, img);

    wimwrite(outpath, img);
}

void mkLUTtable(Mat locimg, int* lut, int a, int* pwlim, int* pblim ) {
    int lochist[256];

    makeHistgram(locimg, lochist);

    int wnumthd = locimg.rows * locimg.cols / 2;

    int dotcount = 0;
    int wlim = a;
    for (int n = 255; n > a; --n) {
        dotcount = dotcount + lochist[n];
        //if (lochist[n] != 0) {
        if (dotcount > wnumthd) {
            wlim = n;
            break;
        }
    }
    wlim = wlim * 1.0;
    int blim = a * 0.8;
    for (int n = 0; n < 256; ++n) {
        if (lochist[n] != 0) {
            blim = n * 0.7;
            break;
        }
    }
    if (blim > wlim) {
        wlim = blim * 1.2;
    }

    for (int n = 0; n < blim; ++n) {
        lut[n] = 0;
    }
    for (int n = blim; n < wlim; ++n) {
        lut[n] = 256 * (n - blim) / (wlim - blim);
    }
    for (int n = wlim; n < 256; ++n) {
        lut[n] = 255;
    }
    *pwlim = wlim;
    *pblim = blim;
}

void VAR5(char* inpath, char* outpath, int* inpara, int* outpara) {
    const int smpwidth = 8;
    const int chartWidth = 1024;
    const int chartHeight = 256;
    const int margin = 20;
    const int sn = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    outpara[0] = img.cols;
    outpara[1] = img.rows;
    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> avrtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> wltbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> bltbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));

    double maxt = 0;
    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            avrtbl.at(y).at(x) = a;
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            if (maxt < s) {
                maxt = s;
            }
            t = t + s;
        }
    }

    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));
    double thd = ta * 1.0;

    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        int wlim = 256;
        int blim = 0;
        int lut[256];
        for (int x = 1; x < int(img.cols / smpwidth) - 1; x++) {
            if (stdtbl.at(y).at(x) > thd) {
                int lochist[256];

                Mat locimg = cv::Mat(bwimg, cv::Rect(x * smpwidth, y * smpwidth, smpwidth, smpwidth));

                mkLUTtable(locimg, lut, avrtbl.at(y).at(x), &wlim, &blim);
                /*
                makeHistgram(locimg, lochist);

                
                int a = avrtbl.at(y).at(x);
                int wlim = a;
                //int maxv = 0;
                /*
                for (int n = 255; n > a; --n) {
                    if (lochist[n] > maxv) {
                        maxv = lochist[n];
                        wlim = n;
                    }
                }
                //wlim = wlim * 0.9;
                //wlim = 256;

                blim = a * 0.9;
                //maxv = 0;
                for (int n = 0; n < a ; ++n) {
                    if (lochist[n] > 0) {
                        //maxv = lochist[n];
                        blim = n *0.9;
                        break;
                    }
                }

                wltbl.at(y).at(x) = wlim;
                bltbl.at(y).at(x) = blim;

                for (int n = 0; n < blim; ++n) {
                    lut[n] = 0;
                }
                for (int n = blim; n < wlim; ++n) {
                    lut[n] = 256 * (n - blim) / (wlim - blim);
                }
                for (int n = wlim; n < 256; ++n) {
                    lut[n] = 255;
                }
                */
                for (int dy = 0; dy < smpwidth; ++dy) {
                    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y * smpwidth + dy);
                    for (int d = 0; d < smpwidth; ++d) {
                        dst[x * smpwidth + d][0] = lut[dst[x * smpwidth + d][0]];
                        dst[x * smpwidth + d][1] = lut[dst[x * smpwidth + d][1]];
                        dst[x * smpwidth + d][2] = lut[dst[x * smpwidth + d][2]];
                    }
                }
            }
            else {
                //if (avrtbl.at(y).at(x) > 80) {
                //    cv::rectangle(img, cv::Point((x)*smpwidth, y * smpwidth), cv::Point((x)*smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                //        cv::Scalar(255, 255, 255), -1, cv::LINE_4);
                //}
                
                int a = avrtbl.at(y).at(x);
                int wl = a;
                int bl = blim;
                if (wl > 256) {
                    std::cout << "wl > 256" << std::endl;
                    wl = 256;
                }
                if (bl > wl) {
                    std::cout << "bl > wl" << std::endl;
                    bl = wl;
                }
                for (int n = 0; n < bl; ++n) {
                    lut[n] = 0;
                }
                for (int n = bl; n < wl; ++n) {
                    lut[n] = 256 * (n - bl) / (wl - bl);
                }
                for (int n = wl; n < 256; ++n) {
                    lut[n] = 255;
                }

                for (int dy = 0; dy < smpwidth; ++dy) {
                    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y * smpwidth + dy);
                    for (int d = 0; d < smpwidth; ++d) {
                        dst[x * smpwidth + d][0] = lut[dst[x * smpwidth + d][0]];
                        dst[x * smpwidth + d][1] = lut[dst[x * smpwidth + d][1]];
                        dst[x * smpwidth + d][2] = lut[dst[x * smpwidth + d][2]];
                    }
                }
            }
        }
    }

    wimwrite(outpath, img);

}

void _VAR4(char* inpath, char* outpath, int* inpara, int* outpara, bool showbox ) {
    const int smpwidth = 8;
    const int chartWidth = 1024;
    const int chartHeight = 256;
    const int margin = 20;
    const int sn = 8;
    std::cout << "VAR4" << std::endl;
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }

    outpara[0] = img.cols;
    outpara[1] = img.rows;
    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> stdtbl2(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> avrtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> mintbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> maxtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::cout << "VAR4 vector def" << std::endl;

    double maxt = 0;
    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            avrtbl.at(y).at(x) = a;
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            if (maxt < s) {
                maxt = s;
            }
            t = t + s;
        }
    }
    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));
    double thd = ta * 1.0;
    std::cout << "VAR4 stdtbl done" << std::endl;

    for (int x = 0; x<int(bwimg.cols / smpwidth); ++x) {
        stdtbl2.at(0).at(x) = 0;
        stdtbl2.at(int(bwimg.rows / smpwidth)-1).at(x) = 0;
    }
    std::cout << "VAR4 stdtbl2 s1 bwimg.rows="<< int(bwimg.rows / smpwidth) - 1 << " bwimg.cols=" << int(bwimg.cols / smpwidth) - 1 << std::endl;
    for (int y = 1; y < int(bwimg.rows / smpwidth)-1; y++) {
        stdtbl2.at(y).at(0) = 0;
        stdtbl2.at(y).at(int(bwimg.cols / smpwidth) - 1) = 0;
        for (int x = 1; x < int(bwimg.cols / smpwidth)-1; x++) {
            if (stdtbl.at(y).at(x) >= thd) {
                if (stdtbl.at(y-1).at(x) < thd && stdtbl.at(y + 1).at(x) < thd && stdtbl.at(y).at(x-1) < thd && stdtbl.at(y).at(x + 1) < thd) {
                    stdtbl2.at(y).at(x) = 0;
                }
                else {
                    stdtbl2.at(y).at(x) = stdtbl.at(y).at(x);
                }
            }
            else {
                stdtbl2.at(y).at(x) = 0;
            }
        }
    }
    std::cout << "VAR4 stdtbl2 s2" << std::endl;
    for (int y = 2; y < int(bwimg.rows / smpwidth) - 2; y++) {
        for (int x = 2; x < int(bwimg.cols / smpwidth) - 2; x++) {
            if (stdtbl.at(y).at(x) < thd) {
                if (stdtbl.at(y - 1).at(x) > thd) {
                    stdtbl2.at(y).at(x) = stdtbl.at(y-1).at(x);
                    avrtbl.at(y).at(x) = avrtbl.at(y-1).at(x);
                }else
                if (stdtbl.at(y + 1).at(x) > thd) {
                    stdtbl2.at(y).at(x) = stdtbl.at(y + 1).at(x);
                    avrtbl.at(y).at(x) = avrtbl.at(y+1).at(x);
                }else
                if (stdtbl.at(y).at(x - 1) > thd) {
                    stdtbl2.at(y).at(x) = stdtbl.at(y).at(x-1);
                    avrtbl.at(y).at(x) = avrtbl.at(y).at(x-1);
                }else
                if(stdtbl.at(y).at(x + 1) > thd) {
                    stdtbl2.at(y).at(x) = stdtbl.at(y).at(x+1);
                    avrtbl.at(y).at(x) = avrtbl.at(y).at(x+1);
                }else
                if (stdtbl.at(y - 1).at(x - 1) > thd) {
                    stdtbl2.at(y).at(x) = stdtbl.at(y - 1).at(x - 1);
                    avrtbl.at(y).at(x) = avrtbl.at(y - 1).at(x - 1);
                }
                else
                if (stdtbl.at(y + 1).at(x - 1) > thd) {
                    stdtbl2.at(y).at(x) = stdtbl.at(y + 1).at(x - 1);
                    avrtbl.at(y).at(x) = avrtbl.at(y + 1).at(x - 1);
                }
                else
                if (stdtbl.at(y+1).at(x - 1) > thd) {
                    stdtbl2.at(y).at(x) = stdtbl.at(y + 1).at(x - 1);
                    avrtbl.at(y).at(x) = avrtbl.at(y + 1).at(x - 1);
                }
                else
                if (stdtbl.at(y + 1).at(x + 1) > thd) {
                    stdtbl2.at(y).at(x) = stdtbl.at(y + 1).at(x + 1);
                    avrtbl.at(y).at(x) = avrtbl.at(y + 1).at(x + 1);
                }else
                if (stdtbl.at(y - 2).at(x) > thd) {
                    stdtbl2.at(y).at(x) = stdtbl.at(y - 2).at(x);
                    avrtbl.at(y).at(x) = avrtbl.at(y - 2).at(x);
                }
                else
                    if (stdtbl.at(y + 2).at(x) > thd) {
                        stdtbl2.at(y).at(x) = stdtbl.at(y + 2).at(x);
                        avrtbl.at(y).at(x) = avrtbl.at(y + 2).at(x);
                    }
                    else
                        if (stdtbl.at(y).at(x - 2) > thd) {
                            stdtbl2.at(y).at(x) = stdtbl.at(y).at(x - 2);
                            avrtbl.at(y).at(x) = avrtbl.at(y).at(x - 2);
                        }
                        else
                            if (stdtbl.at(y).at(x + 2) > thd) {
                                stdtbl2.at(y).at(x) = stdtbl.at(y).at(x + 2);
                                avrtbl.at(y).at(x) = avrtbl.at(y).at(x + 2);
                            }
                            else
                                if (stdtbl.at(y - 2).at(x - 1) > thd) {
                                    stdtbl2.at(y).at(x) = stdtbl.at(y - 2).at(x - 1);
                                    avrtbl.at(y).at(x) = avrtbl.at(y - 2).at(x - 1);
                                }
                                else
                                    if (stdtbl.at(y + 2).at(x - 1) > thd) {
                                        stdtbl2.at(y).at(x) = stdtbl.at(y + 2).at(x - 1);
                                        avrtbl.at(y).at(x) = avrtbl.at(y + 2).at(x - 1);
                                    }
                                    else
                                        if (stdtbl.at(y + 1).at(x - 2) > thd) {
                                            stdtbl2.at(y).at(x) = stdtbl.at(y + 1).at(x - 2);
                                            avrtbl.at(y).at(x) = avrtbl.at(y + 1).at(x - 2);
                                        }
                                        else
                                            if (stdtbl.at(y + 1).at(x + 2) > thd) {
                                                stdtbl2.at(y).at(x) = stdtbl.at(y + 1).at(x + 2);
                                                avrtbl.at(y).at(x) = avrtbl.at(y + 1).at(x + 2);
                                            }
            }
            else {
                stdtbl2.at(y).at(x) = stdtbl.at(y).at(x);
            }
        }
    }
    std::cout << "VAR4 stdtbl2 done" << std::endl;
    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        int wlim = 256;
        int blim = 0;
        int lut[256];
        for (int x = 1; x < int(img.cols / smpwidth) - 1; x++) {
            if (stdtbl2.at(y).at(x) > thd) {
                int lochist[256];

                Mat locimg = cv::Mat(bwimg, cv::Rect(x * smpwidth, y * smpwidth, smpwidth, smpwidth));
                mkLUTtable(locimg, lut, avrtbl.at(y).at(x), &wlim, &blim);
                /*
                makeHistgram(locimg, lochist);


                int wnumthd = smpwidth * smpwidth / 2;
                int a = avrtbl.at(y).at(x);
                wlim = a;
                int dotcount=0;
                for (int n = 255; n > a; --n) {
                    dotcount = dotcount + lochist[n];
                    //if (lochist[n] != 0) {
                    if (dotcount > wnumthd) {
                            wlim = n;
                        break;
                    }
                }
                wlim = wlim * 1.0;
                blim = a * 0.8;
                for (int n = 0; n < 256; ++n) {
                    if (lochist[n] != 0) {
                        blim = n * 0.8;
                        break;
                    }
                }
                if (blim > wlim) {
                    wlim = blim * 1.2;
                }

                for (int n = 0; n < blim; ++n) {
                    lut[n] = 0;
                }
                for (int n = blim; n < wlim; ++n) {
                    lut[n] = 256 * (n - blim) / (wlim - blim);
                }
                for (int n = wlim; n < 256; ++n) {
                    lut[n] = 255;
                }

                /**/
                for (int dy = 0; dy < smpwidth; ++dy) {
                    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y * smpwidth + dy);
                    for (int d = 0; d < smpwidth; ++d) {
                        dst[x * smpwidth + d][0] = lut[dst[x * smpwidth + d][0]];
                        dst[x * smpwidth + d][1] = lut[dst[x * smpwidth + d][1]];
                        dst[x * smpwidth + d][2] = lut[dst[x * smpwidth + d][2]];
                    }
                }
                if (showbox) {
                    cv::rectangle(img, cv::Point((x)*smpwidth, y * smpwidth), cv::Point((x)*smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                        cv::Scalar(0, 0, 255), 1, cv::LINE_4);
                }

            }
            else {
                if (avrtbl.at(y).at(x) > 80) {
                    cv::rectangle(img, cv::Point((x)*smpwidth, y * smpwidth), cv::Point((x)*smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                        cv::Scalar(255, 255, 255), -1, cv::LINE_4);
                }
                /*
                int a = avrtbl.at(y).at(x);
                int wl = a;
                int bl = blim;
                if (wl > 256) {
                    std::cout << "wl > 256" << std::endl;
                    wl = 256;
                }
                if (bl > wl) {
                    std::cout << "bl > wl" << std::endl;
                    bl = wl;
                }
                for (int n = 0; n < bl; ++n) {
                    lut[n] = 0;
                }
                for (int n = bl; n < wl; ++n) {
                    lut[n] = 256 * (n - bl) / (wl - bl);
                }
                for (int n = wl; n < 256; ++n) {
                    lut[n] = 255;
                }
                for (int dy = 0; dy < smpwidth; ++dy) {
                    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y * smpwidth + dy);
                    for (int d = 0; d < smpwidth; ++d) {
                        dst[x * smpwidth + d][0] = lut[dst[x * smpwidth + d][0]];
                        dst[x * smpwidth + d][1] = lut[dst[x * smpwidth + d][1]];
                        dst[x * smpwidth + d][2] = lut[dst[x * smpwidth + d][2]];
                    }
                }
                */
            }
        }
    }
    std::cout << "VAR4 done" << std::endl;

    wimwrite(outpath, img);

}
void VAR4(char* inpath, char* outpath, int* inpara, int* outpara) {
    _VAR4(inpath, outpath, inpara, outpara, FALSE);
}

void VAR4box(char* inpath, char* outpath, int* inpara, int* outpara) {
    _VAR4(inpath, outpath, inpara, outpara, TRUE);
}



void VAR3(char* inpath, char* outpath, int* inpara, int* outpara) {
    const int smpwidth = 8;
    const int chartWidth = 1024;
    const int chartHeight = 256;
    const int margin = 20;
    const int sn = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    outpara[0] = img.cols;
    outpara[1] = img.rows;
    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> avrtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));

    double maxt = 0;
    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            avrtbl.at(y).at(x) = a;
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            if (maxt < s) {
                maxt = s;
            }
            t = t + s;
        }
    }

    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));
    double thd = ta * 1.0;

    for (int y = 0; y < int(img.rows / smpwidth); y++) {

        for (int x = 1; x < int(img.cols / smpwidth) - 1; x++) {
            if (stdtbl.at(y).at(x) > thd) {
                int lochist[256];

                Mat locimg = cv::Mat(bwimg, cv::Rect(x *smpwidth, y * smpwidth, smpwidth, smpwidth));
                int wlim, blim;
                int lut[256];

                mkLUTtable(locimg, lut, avrtbl.at(y).at(x), &wlim, &blim);
                /*
                makeHistgram(locimg, lochist);


                int lut[256];
                int a = avrtbl.at(y).at(x);
                int wlim = a;
                int maxv = 0;
                for (int n = 255; n > a; --n) {
                    if (lochist[n] > maxv) {
                        maxv = lochist[n];
                        wlim = n;
                    }
                }
                wlim = wlim * 0.9;
                
                int blim = a * 0.9;
                maxv = 0;
                for (int n = 0; n < a * 0.8; ++n) {
                    if (lochist[n] > maxv) {
                        maxv = lochist[n];
                        blim = n;
                    }
                }

                for (int n = 0; n < blim; ++n) {
                    lut[n] = 0;
                }
                for (int n = blim; n < wlim; ++n) {
                    lut[n] = 256 * (n - blim) / (wlim - blim);
                }
                for (int n = wlim; n < 256; ++n) {
                    lut[n] = 255;
                }
                */
                for (int dy = 0; dy < smpwidth; ++dy) {
                    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y * smpwidth + dy);
                    for (int d = 0; d < smpwidth; ++d) {
                        dst[x * smpwidth + d][0] = lut[dst[x * smpwidth + d][0]];
                        dst[x * smpwidth + d][1] = lut[dst[x * smpwidth + d][1]];
                        dst[x * smpwidth + d][2] = lut[dst[x * smpwidth + d][2]];
                    }
                }
            }
            else {
                if (avrtbl.at(y).at(x) > 80) {
                    cv::rectangle(img, cv::Point((x)*smpwidth, y * smpwidth), cv::Point((x)*smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                        cv::Scalar(255, 255, 255), -1, cv::LINE_4);
                }
            }
        }
    }

    wimwrite(outpath, img);

}


void createVarTbl(Mat bwimg, double* thd, std::vector<std::vector<int>> stdtbl, std::vector<std::vector<int>> avrtbl ) {
    const int smpwidth = 8;
    double maxt = 0;
    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            avrtbl.at(y).at(x) = a;
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            if (maxt < s) {
                maxt = s;
            }
            t = t + s;
        }
    }

    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));
    *thd = ta * 0.75;

    return;
}

void VAR2(char* inpath, char* outpath, int* inpara, int* outpara) {
    const int smpwidth = 8;
    const int chartWidth = 1024;
    const int chartHeight = 256;
    const int margin = 20;
    const int sn = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    outpara[0] = img.cols;
    outpara[1] = img.rows;
    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));
    std::vector<std::vector<int>> avrtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));

    double maxt = 0;
    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            avrtbl.at(y).at(x) = a;
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            if (maxt < s) {
                maxt = s;
            }
            t = t + s;
        }
    }

    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));
    double thd = ta * 0.75;

    for (int y = 0; y < int(img.rows / smpwidth); y++) {

        for (int x = 1; x < int(img.cols / smpwidth) - 1; x++) {
            if (stdtbl.at(y).at(x) > thd) {
                //cv::rectangle(img, cv::Point((x) * smpwidth, y * smpwidth), cv::Point((x) * smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                //    cv::Scalar(0, 0, 255), 1, cv::LINE_4);
                //Mat dst = cv::Mat(img, cv::Rect(x*smpwidth, y * smpwidth, smpwidth, smpwidth));
                int lut[256];
                int a = avrtbl.at(y).at(x);
                //int wlim = (256 + a) / 2;
                int wlim = a+4;
                int blim = a * 0.9;
                for (int n = 0; n < blim; ++n) {
                    lut[n] = 0;
                }
                for (int n = blim; n < wlim; ++n) {
                    lut[n] = 256*(n-blim)/(wlim-blim) ;
                }
                for (int n = wlim; n < 256; ++n) {
                    lut[n] = 255;
                }
                for (int dy = 0; dy < smpwidth; ++dy) {
                    cv::Vec3b* dst = img.ptr<cv::Vec3b>(y * smpwidth+dy);
                    for (int d = 0; d < smpwidth; ++d) {
                        dst[x * smpwidth+d][0] = lut[dst[x * smpwidth + d][0]];
                        dst[x * smpwidth + d][1] = lut[dst[x * smpwidth + d][1]];
                        dst[x * smpwidth + d][2] = lut[dst[x * smpwidth + d][2]];
                    }
                }
            }
            else {
                if (avrtbl.at(y).at(x) > 80) {
                    cv::rectangle(img, cv::Point((x)*smpwidth, y * smpwidth), cv::Point((x)*smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                        cv::Scalar(255, 255, 255), -1, cv::LINE_4);
                }
            }
        }
    }

    wimwrite(outpath, img);

}
//
// 文字のありそうなところを分散値から見つけて赤枠で囲む
//
void VAR(char* inpath, char* outpath, int* inpara, int* outpara) {
    const int smpwidth = 8;
    const int chartWidth = 1024;
    const int chartHeight = 256;
    const int margin = 20;
    const int sn = 8;

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    outpara[0] = img.cols;
    outpara[1] = img.rows;
    Mat bwimg;
    cv::cvtColor(img, bwimg, COLOR_BGR2GRAY);

    std::vector<std::vector<int>> stdtbl(int(img.rows / smpwidth), std::vector<int>(int(img.cols / smpwidth)));

    double maxt = 0;
    double t = 0;
    for (int y = 0; y < int(bwimg.rows / smpwidth); y++) {
        for (int x = 0; x < int(bwimg.cols / smpwidth); x++) {
            double a = calc_av(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth);
            double s = calc_var(&bwimg, x * smpwidth, y * smpwidth, smpwidth, smpwidth, a);
            stdtbl.at(y).at(x) = s;
            if (maxt < s) {
                maxt = s;
            }
            t = t + s;
        }
    }

    double ta = t / (int(bwimg.rows / smpwidth) * int(bwimg.cols / smpwidth));
    double thd = ta * 0.75;

    for (int y = 0; y < int(img.rows / smpwidth); y++) {
        for (int x = 1; x < int(img.cols / smpwidth) - 1; x++) {
            if (stdtbl.at(y).at(x) > thd) {
                cv::rectangle(img, cv::Point((x) * smpwidth, y * smpwidth), cv::Point((x) * smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                    cv::Scalar(0, 0, 255), 1, cv::LINE_4);
            }
            else {
                //cv::rectangle(img, cv::Point((x)*smpwidth, y * smpwidth), cv::Point((x)*smpwidth + smpwidth - 1, y * smpwidth + smpwidth - 1),
                //    cv::Scalar(255, 255, 255), -1, cv::LINE_4);
            }
        }
    }

    wimwrite(outpath, img);

}

void BW2(char* inpath, char* outpath, int* inpara, int* outpara) {

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    int blim = inpara[0];
    int wlim = inpara[1];
    outpara[0] = img.cols;
    outpara[1] = img.rows;

    Mat temp;
    cv::cvtColor(img, temp, COLOR_BGR2GRAY);

    cv::Mat lut = cv::Mat(1, 256, CV_8U);
    for (int i = 0; i < blim; i++) {
        lut.at<uchar>(0, i) = 0;
    }
    for (int i = blim; i < wlim; ++i) {
        lut.at<uchar>(0, i) = 256 * (i - blim) / (wlim - blim);
    }
    for (int i = wlim; i < 256; i++) {
        lut.at<uchar>(0, i) = 255;
    }


    cv::LUT(img, lut, temp);

    wimwrite(outpath, temp);
}

void BW(char* inpath, char* outpath, int* inpara, int* outpara) {

    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    outpara[0] = img.cols;
    outpara[1] = img.rows;

    Mat temp;
    cv::cvtColor(img, temp, COLOR_BGR2GRAY);

    temp = filterOne(temp, inpara[0]);

    wimwrite(outpath, temp);
}

void NON(char* inpath, char* outpath, int* inpara, int* outpara) {
    Mat img = wimread(inpath);
    if (img.size == 0) {
        return;
    }
    outpara[0] = img.cols;
    outpara[1] = img.rows;

    wimwrite(outpath, img);
}
