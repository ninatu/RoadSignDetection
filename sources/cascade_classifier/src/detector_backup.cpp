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
    String name;
    CascadeClassifier cascade;
    Scalar color;
    std::vector<Rect> detectRects;
public:
    void open(const String &name_xml,const String &name_sign) {
        name = name_sign;
        if( !cascade.load(name_xml) ){
            cout << "--(!)Error loading cascade xml:\n" << name_xml << endl;
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
            rectangle(frame, corner1, corner2, color, 2, 8, 0 );
        }
        return detectRects;
    }
};

CascadeDetector left_detector;

String left_arrow_cascade_name =  "./data/haarcascade_right_arrow400.xml";
String right_arrow_cascade_name =  "./data/haarcascade_right_arrow17.xml";
String stop_arrow_cascade_name =  "./data/haarcascade_stop_sign6s.xml";
String stright_arrow_cascade_name =  "./data/haarcascade_left_arrow4.xml";
CascadeClassifier left_cascade;
CascadeClassifier right_cascade;
CascadeClassifier stop_cascade;
CascadeClassifier stright_cascade;

Scalar left_color = Scalar(255, 0, 0);
Scalar right_color = Scalar(0, 255, 0);
Scalar stop_color = Scalar(255, 255, 0);
Scalar stright_color = Scalar(0, 0, 255);

String window_name = "Sign detection";
/** @function main */
int main( void )
{
    VideoCapture capture;
    Mat frame;

    left_detector.open(left_arrow_cascade_name, String("left"));
    if( !left_cascade.load( left_arrow_cascade_name) ){ printf("--(!)Error loading left cascade\n"); return -1; };
//    if( !right_cascade.load( right_arrow_cascade_name) ){ printf("--(!)Error loading right cascade\n"); return -1; };
//    if( !stop_cascade.load( stop_arrow_cascade_name) ){ printf("--(!)Error loading stop cascade\n"); return -1; };
//    if( !stright_cascade.load( stright_arrow_cascade_name) ){ printf("--(!)Error loading stright cascade\n"); return -1; };

    //-- 2. Read the video stream
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
//    left_cascade.detectMultiScale( frame_gray,  left_arrows, 1.1, 2, 0|CASCADE_SCALE_IMAGE,  Size(30, 30));
//    right_cascade.detectMultiScale( frame_gray,  right_arrows, 1.1, 2, 0|CASCADE_SCALE_IMAGE,  Size(30, 30));
//    stop_cascade.detectMultiScale( frame_gray,  stop_arrows, 1.1, 2, 0|CASCADE_SCALE_IMAGE,  Size(30, 30));
//    stright_cascade.detectMultiScale( frame_gray,  stright_arrows, 1.1, 2, 0|CASCADE_SCALE_IMAGE,  Size(30, 30));
    left_detector.detect(frame, frame_gray);
//    for ( size_t i = 0; i < left_arrows.size(); i++ )
//    {
//        Point corner1(left_arrows[i].x, left_arrows[i].y);
//        Point corner2(left_arrows[i].x + left_arrows[i].width, left_arrows[i].y + left_arrows[i].height);
//        rectangle(frame, corner1, corner2, left_color, 2, 8, 0 );
//    }
    //-- Show what you got
    imshow( window_name, frame );
}
