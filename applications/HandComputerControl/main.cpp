#include <iostream>
#include <opencv2/opencv.hpp>
#include <windows.h>

using namespace std;
using namespace cv;

void cancel_keys_except(const string& key);
void press_key(const string& key);
void mouse_movement(const string& direction);

// method used to detect the hand using HAAR cascade Classifier
vector<Rect> detect_hand(const Mat& image);
// method used to get only the detected hands, that is, to take the ROIs
void draw_rect_hand(const vector<Rect>& points, Mat& originalFrame, Mat& image);


// global variables to manipulate the game
Mat sprite_front, screen;

// 500 is going to be the size of the screen game, 2.2 is the number that could place
// the sprite in the middle of the screen when it comes to the vertical direction
int current_sprite_position_horizontal = 7;
// this variables specifies the sprite position in the horizontal direction
// 1.5 is the number the places the sprite in some good position of the screen game
int current_sprite_position_vertical = 500 / 1.5;

// game methods
void load_game();
void move_sprite(string direction);


int main()
{
    VideoCapture cap(0);

    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    Mat frame;

    /*
    namedWindow("FrameLeft", WINDOW_NORMAL);
    resizeWindow("FrameLeft", Size(160, 160));
    namedWindow("FrameRight", WINDOW_NORMAL);
    resizeWindow("FrameRight", Size(160, 160));
    namedWindow("FrameUp", WINDOW_NORMAL);
    resizeWindow("FrameUp", Size(160, 160));
    namedWindow("FrameDown", WINDOW_NORMAL);
    resizeWindow("FrameDown", Size(160, 160));
    namedWindow("FrameEnter", WINDOW_NORMAL);
    resizeWindow("FrameEnter", Size(160, 160));
    */

    while(true) {

        cap.read(frame);
        flip(frame, frame, 1);

         // obtaining the points where the hands were found
        vector<Rect>points = detect_hand(frame);

        // drawing in clone frame
        Mat cloneFrame = frame.clone();

        // obtaining the ROIs containing only the detected hand (painting the part where the hand is)
        draw_rect_hand(points, frame, cloneFrame);

        // define left side
        rectangle(frame, Point(320, 260), Point(400, 330), Scalar(0, 255, 255), 4);

        // define down side
        rectangle(frame, Point(420, 340), Point(500, 410), Scalar(0, 255, 255), 4);

        // define right side
        rectangle(frame, Point(520, 260), Point(600, 330), Scalar(0, 255, 255), 4);

        // define up side
        rectangle(frame, Point(420, 180), Point(500, 250), Scalar(0, 255, 255), 4);

        // define enter key
        //rectangle(frame, Point(40, 260), Point(120, 330), Scalar(0, 255, 255), 4);

        // getting rectangle area to analysis correctly each button
        Mat roiImageLeft = cloneFrame(Rect(Point(320, 260), Point(400, 330)));
        Mat roiImageRight = cloneFrame(Rect(Point(520, 260), Point(600, 330)));
        Mat roiImageUp = cloneFrame(Rect(Point(420, 180), Point(500, 250)));
        Mat roiImageDown = cloneFrame(Rect(Point(420, 340), Point(500, 410)));
        //Mat roiImageEnter = cloneFrame(Rect(Point(40, 260), Point(120, 330)));

        // checking color for each rectangle (the detection is done using RGB, because what is important is the detection of the hand
        // the rectangles ware drawn using BGR (RGB format)
        Mat maskLeft, maskRight, maskUp, MaskDown, maskEnter;
        inRange(roiImageLeft, Scalar(255, 0, 0), Scalar(255, 0, 0), maskLeft);
        inRange(roiImageRight, Scalar(255, 0, 0), Scalar(255, 0, 0), maskRight);
        inRange(roiImageUp, Scalar(255, 0, 0), Scalar(255, 0, 0), maskUp);
        inRange(roiImageDown, Scalar(255, 0, 0), Scalar(255, 0, 0), MaskDown);
        //inRange(roiImageEnter, Scalar(255, 0, 0), Scalar(255, 0, 0), maskEnter);

        // obtaining amount of color for each square.
        int contLeft = countNonZero(maskLeft);
        int contRight = countNonZero(maskRight);
        int contUp = countNonZero(maskUp);
        int contDown = countNonZero(MaskDown);
        int contEnter = countNonZero(maskEnter);

        load_game();

        // 3000 pixels were found to be a good default value to determine a direction
        if (contLeft > contRight && contLeft > contUp && contLeft > contDown && contLeft > contEnter && contLeft > 3000) {

            cout << "Left pressed: " << contLeft << endl;
            move_sprite("left");
            // cancel_keys_except("left");
            // press_key("left");

        } else if (contRight > contLeft && contRight > contUp && contRight > contDown && contRight > contEnter && contRight > 3000) {

            cout << "Right pressed: " << contRight << endl;
            move_sprite("right");
            // cancel_keys_except("right");
            // press_key("right");

        } else if (contUp > contRight && contUp > contLeft && contUp > contDown && contUp > contEnter && contUp > 3000) {

            cout << "Up pressed: " << contUp << endl;
            move_sprite("up");
            // cancel_keys_except("up");
            // press_key("up");

        } else if (contDown > contRight && contDown > contUp && contDown > contLeft && contDown > contEnter && contDown > 3000) {

            cout << "Down pressed: " << contDown << endl;
            move_sprite("down");
            // cancel_keys_except("down");
            // press_key("down");

        } else if (contEnter > contRight && contEnter > contUp && contEnter > contLeft && contEnter > contDown && contEnter > 3000) {

            cout << "Enter pressed: " << contEnter << endl;
            // move_sprite("enter");
            // cancel_keys_except("enter");
            // press_key("enter");

        } else {
            cancel_keys_except("");
        }


        imshow("Frame", frame);

        /*
        imshow("FrameLeft", maskLeft);
        imshow("FrameRight", maskRight);
        imshow("FrameUp", maskUp);
        imshow("FrameDown", MaskDown);
        imshow("FrameEnter", maskEnter);
        */

         // if click ESC, program stops
        if(waitKey(1) == 27)
            break;
    }

    return 0;
}

// this method performs the cascade detection of the hand and return the rectangle point
vector<Rect> detect_hand(const Mat& image) {

     // loading cascade file to detect eyes
     CascadeClassifier hand("my_handfist.xml");

     vector<Rect>points;

     // need to convert image to gray in order to run the detect multi scale
     Mat gray;
     cvtColor(image, gray, COLOR_BGR2GRAY);
     equalizeHist(gray, gray);

     // Flag CASCADE_FIND_BIGGEST_OBJECT is used to return only the last object found, hence the number of objects returned will be one or none
     // Size(70,70) - minimum possible object size. Objects smaller than that are ignored.
     hand.detectMultiScale(gray, points, 1.09, 30, CASCADE_FIND_BIGGEST_OBJECT, Size(110, 110));

     return points;
}

// this method gets the points returned when detecting hands and draw the color of interest (blue)
void draw_rect_hand(const vector<Rect>& points, Mat& originalFrame, Mat& image) {

    // running through each of the points got from detect multiScale
    for (Rect point : points){
        // drawing a rectangle on the image to specify that in that area it was found a fist
        rectangle(originalFrame, Point(point.x, point.y), Point(point.x + point.width, point.y + point.height), Scalar(0, 255, 0), 8);
        // drawing a rectangle on the image to specify that in that area it was found a fist (blue color in order to specify the direction)
        rectangle(image, Point(point.x, point.y), Point(point.x + point.width, point.y + point.height), Scalar(255, 0, 0), -1);
    }
}



// this method is used to move the mouse
void mouse_movement(const string& direction) {
    POINT pos;
    GetCursorPos(&pos);
    if (direction == "up") {
        pos.y-=20;
    } else if (direction == "left") {
        pos.x-=20;
    } else if (direction == "right") {
        pos.x+=20;
    } else if (direction == "down") {
        pos.y+=20;
    }

    SetCursorPos(pos.x, pos.y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, pos.x, pos.y, 0, 0);
}

// method used to pass the arrow key to be pressed
void press_key(const string& key) {
    if (key == "up") {
        keybd_event(VK_UP, 0, 0, 0);
    } else if (key == "left") {
        keybd_event(VK_LEFT, 0, 0, 0);
    } else if (key == "right") {
        keybd_event(VK_RIGHT, 0, 0, 0);
    } else if (key == "down") {
        keybd_event(VK_DOWN, 0, 0, 0);
    }  else if (key == "enter") {
        keybd_event(VK_RETURN, 0, 0, 0);
    }
}

// method used to unpressed other keys, if the user wants to press some keys, others must be unpressed
void cancel_keys_except(const string& key) {
    if (key == "up") {
        keybd_event(VK_DOWN, 0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_LEFT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RIGHT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);
    } else if (key == "left") {
        keybd_event(VK_DOWN, 0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_UP,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RIGHT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);
    } else if (key == "right") {
        keybd_event(VK_DOWN, 0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_UP,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_LEFT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);
    } else if (key == "down") {
        keybd_event(VK_UP,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_LEFT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RIGHT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);
    } else if (key == "enter") {
        keybd_event(VK_UP,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_LEFT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RIGHT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_DOWN,0,KEYEVENTF_KEYUP,0);
    } else if (key == "") {
        keybd_event(VK_DOWN, 0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_UP,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_LEFT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RIGHT,0,KEYEVENTF_KEYUP,0);
        keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);
    }
}




// method used to initialize the screen of the game and also load the sprites and current position of the sprite
void load_game() {
    // character sprites
    sprite_front = imread("imagens/sprite.png", IMREAD_COLOR);
    // game screen
    screen = imread("imagens/background_game.png", IMREAD_COLOR);

    // resize screen to be 500x500
    resize(screen, screen, Size(500, 500));

    // pasting the sprite in almost the middle of the screen
    sprite_front.copyTo(screen.rowRange(current_sprite_position_vertical, current_sprite_position_vertical + sprite_front.rows)
                        .colRange(current_sprite_position_horizontal, current_sprite_position_horizontal + sprite_front.cols));

    imshow("screen", screen);
}

// method used to move the sprite to the left and to the right according to the eyes movement
void move_sprite(string direction) {
    // cancel_keys_except(direction);
    // press_key(direction);

    if (direction == "left") {
        // if current position of the sprite is not extrapolating the wall on the left, then it's possible move to the left
        if (current_sprite_position_horizontal >= 20) {
            current_sprite_position_horizontal-=20;
        }
    } else if (direction == "right") {
        // if current position of the sprite is not extrapolating the wall on the right, then it's possible move to the right
        if (current_sprite_position_horizontal < (screen.cols - 80)) {
            current_sprite_position_horizontal+=20;
        }
    } else if (direction == "up") {
        if (current_sprite_position_vertical > 90) {
            current_sprite_position_vertical-=20;
        }
    } else if (direction == "down") {
        if (current_sprite_position_vertical < screen.rows - 180)
            current_sprite_position_vertical+=20;
    }

    // mouse_movement(direction);

    // loading background again and insert sprite in the new position
    load_game();
}


