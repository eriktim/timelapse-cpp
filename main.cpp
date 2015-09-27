#include <algorithm>
#include <iterator>
#include <utility>
#include "cv.h"
#include "highgui.h"

using namespace std;
using namespace cv;

int fps = 15;

bool add_black_frames(vector<Mat>& frames)
{
  Mat black(frames[0].size(), CV_8UC3, Scalar(0,0,0));
  for (int k = 0; k < fps; k++) {
    frames.push_back(black);
  }
}

bool blend(vector<Mat>& frames)
{
  vector<Mat> frames0;
  frames0.reserve(frames.size() + 2);
  Mat black(frames[0].size(), CV_8UC3, Scalar(0,0,0));
  frames0.push_back(black);
  for (vector<Mat>::iterator it = frames.begin();
       it != frames.end();
       it = frames.erase(it)) {
    frames0.push_back(*it);
  }
  frames0.push_back(black);
  for (int i = 1; i < frames0.size(); i++) {
    for (int k = 0; k < 20; k++) {
      float alpha = k / 20.0;
      Mat frame;
      addWeighted(frames0[i], alpha, frames0[i - 1], 1.0 - alpha, 0.0, frame);
      frames.push_back(frame);
    }
  }
}

int main(int argc, char** argv)
{
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

  blend(frames);
  add_black_frames(frames);

  namedWindow("output", WINDOW_AUTOSIZE);
  int i = 0; // TODO iterator
  cout << "Looping " << frames.size() << " frames" << endl;

  // show output
  while (true) {
    imshow("output", frames[i]);
    if (waitKey(1000 / fps) >= 0) {
      break;
    }
    i++;
    if (i >= frames.size()) {
      i = 0;
    }
  }
  return 0;
}
