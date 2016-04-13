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

class detectedSign {
public:
    string name;
    Rect rect;
    int timeToLive;
    int timeFromCreate;
    detectedSign(const Rect &rect_, const string &name_) {
        name = name_;
        rect = rect_;
        timeToLive = MAX_TTL;
        timeFromCreate = 0;
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

};

class SignData {
    vector<detectedSign> objects;
    vector<int> answer;
    list<int> prev_answer;
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
        int mas[COUNT_ANSWER + 1] = {0, 0, 0, 0, 0, 0};
        for (auto iter = prev_answer.begin(); iter != prev_answer.end(); iter++) {
            mas[*iter]++;
        }
        int max = 0;
        for(int i = 0; i < COUNT_ANSWER; i++) {
            if (mas[i] > max)
                max = mas[i];
        }
        for(int i = 0; i < COUNT_ANSWER; i++) {
            if(max != 0 && mas[i] == max)
                answer[i] = 1;
            else
                answer[i] = 0;
        }
    }

public:
    SignData() {
        answer = vector<int>(COUNT_ANSWER);
        prev_answer = list<int>(COUNT_PREV_ANSWER,COUNT_ANSWER );
    }

    void process(const vector<Rect> &input_objects, const string &name_object) {
        for (int i = 0; i < input_objects.size(); i++) {
            int index = findBestFit(input_objects[i], name_object);
            if (index == -1) {
                objects.push_back(detectedSign(input_objects[i], name_object));
            } else {
                objects[index].updateRect(input_objects[i]);
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
        double max_size = 0;
        for (auto iter_object = objects.begin(); iter_object < objects.end(); iter_object++) {
            if (iter_object->timeFromCreate > MIN_TIME_FROM_CREATE) {
                if (square(iter_object->rect) > max_size) {
                    best_object = iter_object;
                    max_size = square(iter_object->rect);
                }
            }
        }

        prev_answer.pop_front();

        if (best_object != objects.end()) {
           // cout << best_object->name << ' ' << "TTL " << best_object->timeToLive << "TimeLive " << best_object->timeFromCreate << endl;
            if (best_object->name == "left")
                prev_answer.push_back(0);
            else if (best_object->name == "right")
                prev_answer.push_back(1);
            else if (best_object->name == "stop")
                prev_answer.push_back(2);
            else
                prev_answer.push_back(3);
        } else {
            prev_answer.push_back(COUNT_ANSWER);
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
