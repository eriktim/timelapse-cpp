#include <algorithm>
#include <iterator>
#include <stdio.h>
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

#define RATIO_WIDESCREEN (16.0 / 9)

int fps = 25;
double ratio = -1;
int hold = 1; // sec
double crop = 0.1;
int lines = 720;
Size size;
Size outputSize;

bool add_black_frames(vector<Mat>& frames)
{
  Mat black(frames[0].size(), CV_8UC3, Scalar(0,0,0));
  for (int k = 0; k < fps; k++) {
    frames.push_back(black);
  }
}

bool crop_frames(vector<Mat>& frames)
{
  vector<Mat> frames0; // TODO method
  frames0.reserve(frames.size());
  for (vector<Mat>::iterator it = frames.begin();
       it != frames.end();
       it = frames.erase(it)) {
    frames0.push_back(*it);
  }
  int dx = (size.width - outputSize.width) / 2;
  int dy = (size.height - outputSize.height) / 2;
  Rect roi(dx, dy, size.width - dx, size.height - dy);
  for (vector<Mat>::iterator it = frames0.begin();
       it != frames0.end();
       it++) {
    Mat frame = *it;
    frames.push_back(frame(roi));
  }
}

bool blend_frames(vector<Mat>& frames)
{
  vector<Mat> frames0; // TODO method
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
    for (int k = 0; k < fps; k++) {
      float alpha = k / 1.0 / fps;
      Mat frame;
      addWeighted(frames0[i], alpha, frames0[i - 1], 1.0 - alpha, 0.0, frame);
      frames.push_back(frame);
    }
    for (int j = 1; j < fps * hold; j++) {
      frames.push_back(frames0[i]);
    }
  }
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
    return elems;
}

int toInt(string str) {
  return atoi(str.c_str());
}

int main(int argc, char** argv)
{
  vector<Mat> frames;
  ifstream config("METADATA_F.txt");
  if (!config.is_open()) {
    cerr << "Failed opening config file" << endl;
  }

  // read input
  string line;
  string prefix;
  Size orgSize;

  while (getline(config, line)) {
    cout << "Parsing " << line << endl;
    vector<string> p = split(line, ' ');
    if (p[0][0] == '#') {
      prefix = p[1];
      orgSize.width = toInt(p[2]);
      orgSize.height = toInt(p[3]);
      size.width = orgSize.width;
      size.height = orgSize.height;
      outputSize.width = orgSize.width;
      outputSize.height = orgSize.height;
      continue;
    }
    Mat image2 = imread(prefix + p[1], CV_LOAD_IMAGE_COLOR);
    cout << "Images " << prefix << "{" << p[0] << "," << p[1] << "}" << endl;
    if (!image2.data) {
      cerr << "Ignoring image " << p[1] << endl;
      continue;
    }
    vector<string> match1 = split(p[2], '>');
    vector<string> match2 = split(p[3], '>');
    vector<string> match3 = split(p[4], '>');
    vector<string> a1 = split(match1[0], ',');
    vector<string> a2 = split(match2[0], ',');
    vector<string> a3 = split(match3[0], ',');
    vector<string> b1 = split(match1[1], ',');
    vector<string> b2 = split(match2[1], ',');
    vector<string> b3 = split(match3[1], ',');
    vector<Point2f> A, B;
    A.push_back( Point2f(toInt(a1[0]), toInt(a1[1])) );
    A.push_back( Point2d(toInt(a2[0]), toInt(a2[1])) );
    A.push_back( Point2d(toInt(a3[0]), toInt(a3[1])) );
    B.push_back( Point2d(toInt(b1[0]), toInt(b1[1])) );
    B.push_back( Point2d(toInt(b2[0]), toInt(b2[1])) );
    B.push_back( Point2d(toInt(b3[0]), toInt(b3[1])) );

    //circle(image1, A[0], 20, Scalar(0,0,255), 5, CV_AA);
    //circle(image1, A[1], 20, Scalar(0,0,255), 5, CV_AA);
    //circle(image1, A[2], 20, Scalar(0,0,255), 5, CV_AA);
    //circle(image2, B[0], 20, Scalar(0,0,255), 5, CV_AA);
    //circle(image2, B[1], 20, Scalar(0,0,255), 5, CV_AA);
    //circle(image2, B[2], 20, Scalar(0,0,255), 5, CV_AA);

    Mat warped;
    Mat H = getAffineTransform(B, A);
    warpAffine(image2, warped, H, size, INTER_CUBIC);

    //circle(warped, A[0], 20, Scalar(0,255,0), 5, CV_AA);
    //circle(warped, A[1], 20, Scalar(0,255,0), 5, CV_AA);
    //circle(warped, A[2], 20, Scalar(0,255,0), 5, CV_AA);

    /*if (i == 1) {
      float orgRatio = orgSize.width / 1.0 / orgSize.height;
      if (!(ratio > 0)) {
        ratio = orgRatio;
      }
      outputSize = Size(lines * ratio, lines);
      if (orgRatio > ratio) {
        size.width = (1.0 + crop) * outputSize.width * orgRatio;
        size.height = (1.0 + crop) * outputSize.width;
      } else {
        size.width = (1.0 + crop) * outputSize.width;
        size.height = (1.0 + crop) * outputSize.width / orgRatio;
      }
      size.width += size.width % 2;
      size.height += size.height % 2;
      cout << "Input size: " << orgSize << endl;
      cout << "Process size: " << size << endl;
      cout << "Output size: " << outputSize << endl;
    }*/
    //Mat frame;
    //resize(image, frame, size);
    //frames.push_back(frame);
    if (!frames.size()) {
      Mat image1 = imread(prefix + p[0], CV_LOAD_IMAGE_COLOR);
      cout << image1.size() << endl;
      cout << "Adding " << p[0] << endl;
      frames.push_back(image1);
    }
    cout << "Adding " << p[1] << endl;
    frames.push_back(warped);
  }
  cout << "Finished stabilization" << endl;

  vector<Mat> frames2;
  Size videoSize(800, 600);
  for (int q = 0; q < frames.size(); q++) {
    Mat out;
    resize(frames[q], out, videoSize);
    frames2.push_back(out);
  }

  //normalize_frames(frames);
  blend_frames(frames2);
  cout << "Finished blending" << endl;
  add_black_frames(frames2);
  //crop_frames(frames);

  /*namedWindow("output", WINDOW_AUTOSIZE);
  int i = 0; // TODO iterator
  cout << "Looping " << frames.size() << " frames" << endl;

  // show output
  Mat out;
  while (true) {
    resize(frames[i], out, Size(800, 600));
    imshow("output", out);
    if (waitKey(1000 / fps) >= 0) {
      break;
    }
    i++;
    if (i >= frames.size()) {
      i = 0;
    }
  }*/

  cout << "Writing output" << endl;

  // write output
  VideoWriter video("out.avi", CV_FOURCC('X','V','I','D'), fps, videoSize, true);
  int iii = 0;
  //for (vector<Mat>::iterator it = frames.begin();
  //     it != frames.end();
  //     it++) {
  while (iii < frames2.size()) {
    //video.write(*it);
    video.write(frames2[iii++]);
    //ostringstream oss2;
    //oss2 << "/tmp/timelapse/out_" << iii << ".jpg";
    //cout << oss2.str() << endl;
    //if (!imwrite(oss2.str(), frames[iii++])) {
    //  cout << "Faied writing " << (iii - 1) << "!" << endl;
    //}
  }

  config.close();

  return 0;
}
