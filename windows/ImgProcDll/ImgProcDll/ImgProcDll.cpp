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

//const int Vertical_writing = 1;
//const int Horizontal_writing = 0;

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/flann/flann.hpp>

using namespace cv;
using std::cout; using std::cin;
using std::endl; using std::ifstream;

#include "ImgProcDll.h"



cv::Mat wimread(char* inpath) {
    setlocale(LC_ALL, "japanese");

    int size = ::MultiByteToWideChar(CP_UTF8, 0, inpath, -1, (wchar_t*)NULL, 0);
    wchar_t* winpath = (wchar_t*)new wchar_t[size];
    ::MultiByteToWideChar(CP_UTF8, 0, inpath, -1, winpath, size);

    FILE* fp;
    _wfopen_s(&fp, winpath, L"rb");
    delete[] winpath;
    if (fp == NULL) {
        std::cout << "cant open " << inpath << std::endl;
        return Mat();
    }

    long long int fsize = _filelengthi64(_fileno(fp));
    std::cout << "input " << inpath << " size=" << fsize << std::endl;

    unsigned char* buff = new unsigned char[fsize];

    fread(buff, fsize, 1, fp);
    fclose(fp);

    // Matへ変換
    std::vector<uchar> jpeg(buff, buff + fsize);
    cv::Mat img = cv::imdecode(jpeg, 1);

    delete[] buff;

    return img;
}

void wimwrite(char* outpath, Mat img) {
    setlocale(LC_ALL, "japanese");

    std::vector<uchar> buff2; //buffer for coding
    std::vector<int> param = std::vector<int>(2);
    param[0] = 1;
    param[1] = 95; //default(95) 0-100

    imencode(".jpg", img, buff2, param);

    int size = ::MultiByteToWideChar(CP_UTF8, 0, outpath, -1, (wchar_t*)NULL, 0);
    wchar_t* woutpath = (wchar_t*)new wchar_t[size];
    ::MultiByteToWideChar(CP_UTF8, 0, outpath, -1, woutpath, size);

    FILE* fp2;
    _wfopen_s(&fp2, woutpath, L"wb");
    delete[] woutpath;
    if (fp2 == NULL) {
        std::cout << "output cant open" << std::endl;
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
