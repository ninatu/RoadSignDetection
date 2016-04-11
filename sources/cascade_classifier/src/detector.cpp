#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay( Mat frame );

/** Global variables */
class CascadeDetector {
    string name;
    CascadeClassifier cascade;
    Scalar color;
    std::vector<Rect> detectRects;
public:
    void open(const string &name_xml,const string &name_sign) {
        name = name_sign;
        if( !cascade.load(name_xml) ){
            cout << " --(!)Error loading cascade xml:\n" << name_xml << endl;
                exit(1);
        }
        color = Scalar(rand() % 256, rand() % 256, rand() % 256);
    }
    std::vector<Rect> &detect(Mat& frame, Mat &frame_gray) {
        cascade.detectMultiScale( frame_gray,  detectRects, 1.1, 2, 0|CASCADE_SCALE_IMAGE,  Size(30, 30));
        for ( size_t i = 0; i < detectRects.size(); i++ )
        {
            Point corner1(detectRects[i].x, detectRects[i].y);
            Point corner2(detectRects[i].x + detectRects[i].width, detectRects[i].y + detectRects[i].height);
            Point text_point(detectRects[i].x + detectRects[i].width/3, detectRects[i].y +  detectRects[i].height);
            rectangle(frame, corner1, corner2, color, 2, 8, 0 );
            putText(frame, name, text_point, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0 ,0), 2);
        }
        return detectRects;
    }
};

int MAX_TTL = 20;
int MIN_TIME_FROM_CREATE = 10;

class detectedSign {
public:
    string name;
    Rect rect;
    Point center;
    Size size;
    int timeToLive;
    int timeFromCreate;
    detectedSign(const Rect &rect_, const string &name_) {
        name = name_;
        rect = rect_;
        timeToLive = MAX_TTL;
        timeFromCreate = 0;
    }
    void renew(const Rect &new_rect) {
        rect = new_rect;
        timeToLive++;
    }
    void agged() {
        timeToLive--;
        timeFromCreate++;
        return timeToLive;
    }

};
double dist(const Point &p, const Point &q) {
    return sqrt(pow((p.x - q.x),2) + pow((p.y - q.y), 2));
}

double measureRects(Rect rect1, Rect rect2) {
    double intersect_rect = square(rect1 | rect2);
    double union_rect = square(rect1) + square(rect2) - intersect_rect;
    return intersect_rect / union_rect;
}

double square(Rect rect) {
    return rect.width * rect.height;
}

class SingData {
    vector<detectedSign> objects;
    double CENTER_THRESH = 10;
    double RECT_THRESH = 0.5;
    int findBestFit(Rect &rect, const string &name) {
        int index = -1;
        double max_intersect = 0;
        for (int i = 0; i < objects.size(); i++) {
            double intersect= measureRects(rect, objects[i]);
            if (intersect > RECT_THRESH && objects[i].name == name && intersect >= max_intersect){
                index = i;
                max_intersect = intersect;
            }
        }
        return index;
    }

public:

    void process(vector<Rect> &input_objects, const string &name_object) {
        for (int i = 0; i < input_objects.size(); i++) {
            int index = findBestFit(input_objects[i]);
            if (index == -1) {
                objects.push_back(detectedSign(input_objects[i], name_object));
            } else {
                objects[index].renew(input_objects[i]);
            }
        }
    }
    void end_process() {
        for (int i = 0; i < objects.size(); i++) {
            objects[i].agged(); // написать удаление
        }
    }
    string getCloseSign() {
        string name_close_sign = "";
        for (int i = 0; i < objects.size; i++) {

        }
    }

};

CascadeDetector left_detector;
CascadeDetector right_detector;
CascadeDetector stop_detector;
CascadeDetector stright_detector;

string left_arrow_cascade_name =  "./data/haarcascade_left_arrow17.xml";
String right_arrow_cascade_name =  "./data/haarcascade_right_arrow7.xml";
String stop_arrow_cascade_name =  "./data/haarcascade_stop_sign7.xml";
String stright_arrow_cascade_name =  "./data/haarcascade_left_arrow4.xml";

String window_name = "Sign detection";
/** @function main */
int main( void )
{
    VideoCapture capture;
    Mat frame;

    left_detector.open(left_arrow_cascade_name, string("left"));
    right_detector.open(right_arrow_cascade_name, string("right"));
    stop_detector.open(stop_arrow_cascade_name, string("stop"));
//    stringt_detector.open(stright_arrow_cascade_name, string("stright"));

    //Read the video stream
    capture.open(0);
    if ( ! capture.isOpened() ) { printf("--(!)Error opening video capture\n"); return -1; }

    while ( capture.read(frame) )
    {
        if( frame.empty() )
        {
            printf(" --(!) No captured frame -- Break!");
            break;
        }

        //-- 3. Apply the classifier to the frame
        detectAndDisplay( frame );
        int c = waitKey(10);
        if( (char)c == 27 ) { break; } // escape
    }
    return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
    std::vector<Rect> left_arrows;
    std::vector<Rect> rigth_arrows;
    std::vector<Rect> stop_arrows;
    std::vector<Rect> stright_arrows;

    Mat frame_gray;

    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );
    left_detector.detect(frame, frame_gray);
    right_detector.detect(frame, frame_gray);
    stop_detector.detect(frame, frame_gray);
//    stright_detector.detect(frame, frame_gray);

    //-- Show what you got
    imshow( window_name, frame );
}
