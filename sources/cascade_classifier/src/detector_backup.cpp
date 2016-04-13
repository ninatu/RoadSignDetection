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


/** Function Headers */
void detectAndDisplay( Mat &frame, socket_t &socket);
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

string left_arrow_cascade_name =  "./data/haarcascade_left_arrow18.xml";
String right_arrow_cascade_name =  "./data/haarcascade_right_arrow8.xml";
String stop_arrow_cascade_name =  "./data/haarcascade_stop_sign7.xml";
String stright_arrow_cascade_name =  "./data/haarcascade_stright_arrow10.xml";
//String stop_arrow_cascade_name =  "./data/haarcascade_stop_sign10.xml";
//String stright_arrow_cascade_name =  "./data/haarcascade_stright_arrow500.xml";


SignData historySigns;

String window_name = "Sign detection";
/** @function main */
int main( void )
{
    context_t context(1);
    socket_t socket(context, ZMQ_PUB);
    socket.bind("tcp://*:4444");
    VideoCapture capture;
    Mat frame;

    left_detector.open(left_arrow_cascade_name, string("left"));
    right_detector.open(right_arrow_cascade_name, string("right"));
    stop_detector.open(stop_arrow_cascade_name, string("stop"));
    stright_detector.open(stright_arrow_cascade_name, string("stright"));

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
        detectAndDisplay( frame, socket);
        int c = waitKey(10);
        if( (char)c == 27 ) { break; } // escape
    }
    return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat &frame, socket_t &socket )
{
    Mat frame_gray;
    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );
    historySigns.process(left_detector.detect(frame, frame_gray), "left");
    historySigns.process(right_detector.detect(frame, frame_gray), "right");
    historySigns.process(stop_detector.detect(frame, frame_gray), "stop");
    historySigns.process(stright_detector.detect(frame, frame_gray), "stright");

    historySigns.end_process();
   // historySigns.print();
    const vector<int> &closesign =  historySigns.getCloseSign();
    message_t msg((void *)(closesign.data()), COUNT_ANSWER * sizeof(int), myfree);
    socket.send(msg);
    if (closesign[0])
        cout << "left ";
    if (closesign[1])
        cout << "right ";
    if (closesign[2])
        cout << "stop ";
    if (closesign[3])
        cout << "stright ";
    if (closesign[0] + closesign[1] + closesign[2] + closesign[3] != 0)
        cout << endl;

    imshow( window_name, frame );
}

