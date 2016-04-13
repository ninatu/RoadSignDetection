#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <zmq.hpp>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>

using namespace std;
using namespace cv;
using namespace zmq;


int MAX_TTL = 7;
int MIN_TIME_FROM_CREATE = 10;
double CENTER_THRESH = 10;
double RECT_THRESH = 0.5;
int COUNT_PREV_ANSWER = 5;
int COUNT_ANSWER = 5;
int HUE_THRESH = 55;


/** Function Headers */
void detectAndDisplay( Mat &frame, socket_t &socket, bool show=false);
double measureRects(const Rect &rect1, const Rect &rect2);
double dist(const Point &p, const Point &q);
double square(const Rect &rect) ;
void myfree(void *, void*);
#include "cascade.cpp"
class detectedSign;
class SignData;
class CascadeDetector;


/** Global variables */

CascadeDetector left_detector;
CascadeDetector right_detector;
CascadeDetector stop_detector;
CascadeDetector stright_detector;
CascadeDetector trafic_light;

//string left_arrow_cascade_name =  "./data/haarcascade_left_arrow18.xml";
//String right_arrow_cascade_name =  "./data/haarcascade_right_arrow8.xml";
//String stop_arrow_cascade_name =  "./data/haarcascade_stop_sign7.xml";
//String stright_arrow_cascade_name =  "./data/haarcascade_stright_arrow10.xml";
//string left_arrow_cascade_name =  "./data/haarcascade_left_arrow18.xml";
//String right_arrow_cascade_name =  "./data/haarcascade_right_arrow8.xml";
//String stop_arrow_cascade_name =  "./data/haarcascade_stop_sign11.xml";
//String stright_arrow_cascade_name =  "./data/haarcascade_stright_arrow10.xml";

string left_arrow_cascade_name =  "./data/haarcascade_left_arrow18.xml";
string right_arrow_cascade_name =  "./data/haarcascade_right_arrow8.xml";
string stop_arrow_cascade_name =  "./data/haarcascade_stop_sign11.xml";
string stright_arrow_cascade_name =  "./data/haarcascade_stright_arrow10.xml";
string trafic_light_cascade_name =  "./data/haarcascade_traffic_light.xml";


SignData historySigns;

String window_name = "Sign detection";
/** @function main */
int main(int argc, char**argv )
{
    bool show = false;
    if (argc > 1 && string(argv[1]) == string("--show"))
        show = true;
    else if (argc > 1 && string(argv[1])  == string("--help")) {
        cout << "Use --show to create window" << endl;
        return 0;
    }

    context_t context(1);
    socket_t socket(context, ZMQ_PUB);
    socket.bind("tcp://*:4444");
    VideoCapture capture;
    Mat frame;

    left_detector.open(left_arrow_cascade_name, string("left"), show);
    right_detector.open(right_arrow_cascade_name, string("right"), show);
    stop_detector.open(stop_arrow_cascade_name, string("stop"), show);
    stright_detector.open(stright_arrow_cascade_name, string("stright"), show);    
    trafic_light.open(trafic_light_cascade_name, string("trafic_light"), show);

    //Read the video stream
    capture.open(0);
    if ( ! capture.isOpened() ) { printf("--(!)Error opening video capture\n"); return -1; }
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    while ( capture.read(frame) )
    {
        if( frame.empty() )
        {
            printf(" --(!) No captured frame -- Break!");
            break;
        }

        //Apply the classifier to the frame
        detectAndDisplay( frame, socket, show);
        int c = waitKey(10);
        if( (char)c == 27 ) { break; } // escape
    }
    return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat &frame, socket_t &socket, bool show)
{
    Mat frame_gray;
    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );
    historySigns.process(frame, left_detector.detect(frame, frame_gray), "left");
    historySigns.process(frame, right_detector.detect(frame, frame_gray), "right");
    historySigns.process(frame, stop_detector.detect(frame, frame_gray), "stop");
    historySigns.process(frame, stright_detector.detect(frame, frame_gray), "stright");
    historySigns.process(frame, trafic_light.detect(frame, frame_gray), "traffic_light");

    historySigns.end_process();
   // historySigns.print();
    const vector<int> &closesign =  historySigns.getCloseSign();
    message_t msg((void *)(closesign.data()), COUNT_ANSWER * sizeof(int), myfree);
    socket.send(msg);
    if (show) {
        if (closesign[0])
            cout << "left ";
        if (closesign[1])
            cout << "stright ";
        if (closesign[2])
            cout << "right ";
        if (closesign[3])
            cout << "stop ";
        if (closesign[4])
            cout << "red ";
        if (closesign[0] + closesign[1] + closesign[2] + closesign[3]  + closesign[4]!= 0)
            cout << endl;
        imshow( window_name, frame );
    }
}

