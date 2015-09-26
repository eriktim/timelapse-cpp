#include "cv.h"
#include "highgui.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
  int fps = 15;
  vector<Mat> frames;
  Size size(800, 600);

  // read input
  for (int i = 1; i < argc; i++) {
    Mat image = imread(argv[i], CV_LOAD_IMAGE_COLOR);
    if (!image.data) {
      cerr << "Ignoring image " << i << endl;
      continue;
    }
    Mat frame;
    resize(image, frame, size);
    frames.push_back(frame);
  }

  // blend
  Mat blackFrame(size, CV_8UC3, Scalar(0,0,0));
  frames.insert(frames.begin(), blackFrame);
  frames.push_back(blackFrame);
  vector<Mat> frames2;
  for (int i = 1; i < frames.size(); i++) {
    for (int k = 0; k < 20; k++) {
      float alpha = k / 20.0;
      Mat frame;
      addWeighted(frames[i], alpha, frames[i - 1], 1.0 - alpha, 0.0, frame);
      frames2.push_back(frame);
    }
  }

  // add black frames
  for (int i = 1; i < fps; i++) {
    frames2.push_back(blackFrame);
  }

  namedWindow("output", WINDOW_AUTOSIZE);
  int it = 0; // TODO iterator
  cout << "Looping " << frames2.size() << " frames" << endl;

  // show output
  while (true) {
    imshow("output", frames2[it]);
    if (waitKey(1000 / fps) >= 0) {
      break;
    }
    it++;
    if (it >= frames2.size()) {
      it = 0;
    }
  }
  return 0;
}
