#include <iostream>
#include "opencv2/opencv.hpp"
#include <vector>
#include <cstdlib>

using namespace std;
using namespace cv;

void save_detected(string dir, string filename, vector<Mat>& eyes);

int main()
{
    Mat img, gray;

    CascadeClassifier fist("my_handfist.xml");

    vector<string>result;

    glob("teste\\images\\fist_test\\", result, false);

    string save_dir = "teste\\classificador_detections\\VP\\";
    string dir = "teste\\images\\fist_test\\";

    cout << "size: " << (int) result.size() << endl;

    namedWindow("Frame", WINDOW_NORMAL);
    resizeWindow("Frame", 500, 500);

    for (int i = 0; i < (int)result.size(); i++) {
        cout << "Image " << i + 1 << ": " << result[i] << endl;
        img = imread(result[i]);
        cvtColor(img, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, gray);

        vector<Rect>point1;

        fist.detectMultiScale(gray, point1, 1.09, 30, CASCADE_FIND_BIGGEST_OBJECT, Size(110, 110));

        vector<Mat>fists;

        string filename = "";
        for(int j = dir.size(); j < (int)result[i].size(); j++) {
            if (result[i][j] == '.')
                break;
            filename+=result[i][j];
        }


        for (int i = 0; i < (int)point1.size(); i++) {
            fists.push_back(img(point1[i]));
            rectangle(img, Point(point1[i].x, point1[i].y), Point(point1[i].x + point1[i].width, point1[i].y + point1[i].height), Scalar(0, 0, 255), 2);
        }


         //save_detected(save_dir,filename, fists);

         imshow("Frame", img);

         waitKey(0);
    }
    return 0;
}


void save_detected(string dir, string filename, vector<Mat>& eyes) {
    for (int i = 0; i < (int)eyes.size(); i++) {
        imwrite(dir+filename+to_string(i)+".png", eyes[i]);
    }
}
