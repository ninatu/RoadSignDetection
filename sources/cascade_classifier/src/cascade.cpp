double square(const Rect &rect) {
    return rect.width * rect.height;
}

double dist(const Point &p, const Point &q) {
    return sqrt(pow((p.x - q.x),2) + pow((p.y - q.y), 2));
}

double measureRects(const Rect &rect1, const Rect &rect2) {
    double intersect_rect = square(rect1 & rect2);
    double union_rect = square(rect1) + square(rect2) - intersect_rect;
    return intersect_rect / union_rect;
}
void myfree(void *, void*) {

}


class CascadeDetector {
    string name;
    CascadeClassifier cascade;
    Scalar color;
    std::vector<Rect> detectRects;
    bool show;
public:
    void open(const string &name_xml,const string &name_sign, bool show_=false) {
        show = show_;
        name = name_sign;
        if( !cascade.load(name_xml) ){
            cout << " --(!)Error loading cascade xml:\n" << name_xml << endl;
                exit(1);
        }
        color = Scalar(rand() % 256, rand() % 256, rand() % 256);
    }
    std::vector<Rect> &detect(Mat& frame, Mat &frame_gray) {
        cascade.detectMultiScale( frame_gray,  detectRects, 1.1, 2, 0|CASCADE_SCALE_IMAGE,  Size(5, 5));
        if (show) {
            for ( size_t i = 0; i < detectRects.size(); i++ )
            {
                Point corner1(detectRects[i].x, detectRects[i].y);
                Point corner2(detectRects[i].x + detectRects[i].width, detectRects[i].y + detectRects[i].height);
                Point text_point(detectRects[i].x + detectRects[i].width/3, detectRects[i].y +  detectRects[i].height);
                rectangle(frame, corner1, corner2, color, 1, 8, 0 );
                putText(frame, name, text_point, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0 ,0), 1);
            }
       }
       return detectRects;
    }
};

class detectedSign {
public:
    string name;
    Rect rect;
    int timeToLive;
    int timeFromCreate;
    int traffic_light;
    detectedSign(const Rect &rect_, const string &name_) {
        name = name_;
        rect = rect_;
        timeToLive = MAX_TTL;
        timeFromCreate = 0;
        traffic_light = 0;
    }
    void updateRect(const Rect &new_rect) {
        rect = new_rect;
        if (timeFromCreate > MIN_TIME_FROM_CREATE)
            timeToLive = MAX_TTL;
        else
            timeToLive++;
    }
    void agged() {
        timeToLive--;
        timeFromCreate++;
    }
    void set_traffic_light(const Mat &img) {
        Size size_img = img.size();
        Mat top_img(img, Rect(0, 0, size_img.width, size_img.height / 2));
        Mat bottom_img(img, Rect(0, size_img.height / 2, size_img.width, size_img.height/2));
        cvtColor(top_img, top_img, CV_BGR2YUV);
        cvtColor(bottom_img, bottom_img, CV_BGR2YUV);
        Scalar mean_color_top = mean(top_img);
        Scalar mean_color_bottom = mean(bottom_img);
        double br_top = mean_color_top[0];
        double br_bottom = mean_color_bottom[0];
      //  cout << "br_top " << br_top << " br_bottom " << br_bottom << endl;
        if (br_top > br_bottom || abs(br_top - br_bottom) < 15 && br_bottom < 120)
            traffic_light = 1;
        else
            traffic_light = 0;
    }

};

class SignData {
    vector<detectedSign> objects;
    vector<int> answer;
    list<int> prev_answer;
    list<int> prev_trafic;
    int findBestFit(const Rect &rect, const string &name) { // return -2 if this error rect
        int index = -1;
        double max_intersect = 0;
        for (int i = 0; i < objects.size(); i++) {
            double intersect = measureRects(rect, objects[i].rect);
            if (intersect > RECT_THRESH && objects[i].name == name && intersect >= max_intersect){
                index = i;
                max_intersect = intersect;
            }
        }
        return index;
    }
    void computeAnswer() {
        int mas[COUNT_ANSWER] = {0, 0, 0, 0, 0};
        for (auto iter = prev_answer.begin(); iter != prev_answer.end(); iter++) {
            mas[*iter]++;
        }
        int max = 0;
        for(int i = 0; i < COUNT_ANSWER - 1; i++) {
            if (mas[i] > max)
                max = mas[i];
        }
        for(int i = 0; i < COUNT_ANSWER - 1; i++) {
            if(max != 0 && mas[i] == max)
                answer[i] = 1;
            else
                answer[i] = 0;
        }
        int sum_traffic = 0;
        for (auto iter = prev_trafic.begin(); iter != prev_trafic.end(); iter++) {
            sum_traffic+= *iter;
        }
       // cout << "SUM_TRAFFIC " << sum_traffic << endl;
        if (sum_traffic >= COUNT_PREV_ANSWER/2.0)
            answer[COUNT_ANSWER - 1] = 1;
        else
            answer[COUNT_ANSWER - 1] = 0;
    }

public:
    SignData() {
        answer = vector<int>(COUNT_ANSWER);
        prev_answer = list<int>(COUNT_PREV_ANSWER,COUNT_ANSWER);
        prev_trafic = list<int>(COUNT_PREV_ANSWER, 0);
    }

    void process(const Mat &frame, const vector<Rect> &input_objects, const string &name_object) {
        for (int i = 0; i < input_objects.size(); i++) {
            Mat img;
            cvtColor(Mat(frame, input_objects[i]), img, CV_BGR2HSV);
            if (name_object == string("stop")) {
                if (mean(img)[0] > HUE_THRESH)
                    continue;
            }
            else if (name_object != string("traffic_light")){
                if (mean(img)[0] < HUE_THRESH)
                    continue;
            }
            int index = findBestFit(input_objects[i], name_object);
            if (index == -1) {
                detectedSign new_sign =detectedSign(input_objects[i], name_object);
                if (new_sign.name == string("traffic_light")) {
                     new_sign.set_traffic_light(Mat(frame, input_objects[i]));
                }
                objects.push_back(new_sign);
            } else {
                objects[index].updateRect(input_objects[i]);
                if (objects[index].name == string("traffic_light")) {
                    objects[index].set_traffic_light(Mat(frame, input_objects[i]));
                }
            }

        }
    }
    void end_process() {
        for (auto iter_object = objects.begin(); iter_object < objects.end(); ) {
            iter_object->agged();
            if (iter_object->timeToLive < 0) {
                objects.erase(iter_object, iter_object+1);
            }
            iter_object++;
        }
    }
    const vector<int> &getCloseSign() {
        vector<detectedSign>::iterator best_object = objects.end();
        vector<detectedSign>::iterator best_traffic_light = objects.end();
        double max_size = 0;
        double max_size_traffic = 0;
        for (auto iter_object = objects.begin(); iter_object < objects.end(); iter_object++) {
            if (iter_object->timeFromCreate > MIN_TIME_FROM_CREATE) {
                if (square(iter_object->rect) > max_size  && iter_object->name != string("traffic_light")) {
                    best_object = iter_object;
                    max_size = square(iter_object->rect);
                }
                if (iter_object->name == string("traffic_light") && square(iter_object->rect) > max_size_traffic) {
                    best_traffic_light = iter_object;
                    max_size_traffic = square(iter_object->rect);
                }
            }


        }

        prev_answer.pop_front();
        prev_trafic.pop_front();

        if (best_object != objects.end()) {
            //cout << best_object->name << ' ' << "TTL " << best_object->timeToLive << "TimeLive " << best_object->timeFromCreate << " size " << square(best_object->rect) << endl;
            if (best_object->name == "left")
                prev_answer.push_back(0);
            else if (best_object->name == "stright")
                prev_answer.push_back(1);
            else if (best_object->name == "right")
                prev_answer.push_back(2);
            else
                prev_answer.push_back(3);
        } else {
            prev_answer.push_back(COUNT_ANSWER - 1);
        }

        if (best_traffic_light != objects.end()) {
          //  cout << "VALUE " << best_traffic_light->traffic_light << endl;
            prev_trafic.push_back(best_traffic_light->traffic_light);
        }
        else {
            prev_trafic.push_back(0);
        }
        computeAnswer();
        return answer;
    }
    void print() {
        int i = 0;
        for (auto iter_object = objects.begin(); iter_object < objects.end(); iter_object++) {
            cout << i++ << ' ' << iter_object->name << ' ' << square(iter_object->rect) << ' ' << "TTL" << iter_object->timeToLive << " TimeLive " << iter_object->timeFromCreate << endl;
        }
    }
};
