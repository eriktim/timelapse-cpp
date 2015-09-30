#include <algorithm>
#include <iterator>
#include <stdio.h>
#include <utility>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"

using namespace std;
using namespace cv;

int fps = 15;
int hold = 1; // sec

bool add_black_frames(vector<Mat>& frames)
{
  Mat black(frames[0].size(), CV_8UC3, Scalar(0,0,0));
  for (int k = 0; k < fps; k++) {
    frames.push_back(black);
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
    for (int k = 0; k < 3 * fps; k++) {
      float alpha = k / 3.0 / fps;
      Mat frame;
      addWeighted(frames0[i], alpha, frames0[i - 1], 1.0 - alpha, 0.0, frame);
      frames.push_back(frame);
    }
    for (int j = 1; j < fps * hold; j++) {
      frames.push_back(frames0[i]);
    }
  }
}

bool stabilize_frames(vector<Mat>& frames)
{
  vector<Mat> frames0; // TODO method
  vector<Mat> grayscales;
  frames0.reserve(frames.size());
  for (vector<Mat>::iterator it = frames.begin();
       it != frames.end();
       it = frames.erase(it)) {
    frames0.push_back(*it);
    Mat gray;
    cvtColor(*it, gray, CV_BGR2GRAY);
    grayscales.push_back(gray);
  }
  SurfFeatureDetector detector(400);
  SurfDescriptorExtractor extractor;
Mat T = Mat::eye(3, 3, CV_64F);
frames.push_back(frames0[0]);
  for (int ii = 1; ii < frames0.size(); ii++) {
    Mat img_object = grayscales[ii];
    Mat img_scene = grayscales[ii-1];
////////////////////
//-- Step 1: Detect the keypoints using SURF Detector
  int minHessian = 400;

  SurfFeatureDetector detector( minHessian , 8, 3, true, true ); // ???

  std::vector<KeyPoint> keypoints_object, keypoints_scene;

  detector.detect( img_object, keypoints_object );
  detector.detect( img_scene, keypoints_scene );

  //-- Step 2: Calculate descriptors (feature vectors)
  SurfDescriptorExtractor extractor;

  Mat descriptors_object, descriptors_scene;

  extractor.compute( img_object, keypoints_object, descriptors_object );
  extractor.compute( img_scene, keypoints_scene, descriptors_scene );

  //-- Step 3: Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  std::vector< DMatch > matches;
  matcher.match( descriptors_object, descriptors_scene, matches );

  double max_dist = 0; double min_dist = 100;

  //-- Quick calculation of max and min distances between keypoints
  for( int i = 0; i < descriptors_object.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

  printf("-- Max dist : %f \n", max_dist );
  printf("-- Min dist : %f \n", min_dist );

  //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
  std::vector< DMatch > good_matches;

  for( int i = 0; i < descriptors_object.rows; i++ ) { 
    if( matches[i].distance < 3*min_dist ) {
      good_matches.push_back( matches[i]);
    }
  }

  Mat img_matches;
  drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

  //-- Localize the object
  std::vector<Point2f> obj;
  std::vector<Point2f> scene;

  for( int i = 0; i < good_matches.size(); i++ )
  {
    //-- Get the keypoints from the good matches
    obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
    scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
  }

  Mat H = findHomography( obj, scene, CV_RANSAC , 2);
  T = T * H;
  //T = H;

  Mat frame;
  warpPerspective( frames0[ii] , frame, T, frames0[ii].size());
  // write stabilized frames
  ostringstream oss;
  oss << "/tmp/timelapse/" << ii << ".jpg";
  imwrite(oss.str(), frame); 

////////////////////
//frames.push_back(img_matches);
frames.push_back(frame);
  }
}

int main(int argc, char** argv)
{
  vector<Mat> frames;
  int lines = 720;
  Size size;

  // read input
  for (int i = 1; i < argc; i++) {
    Mat image = imread(argv[i], CV_LOAD_IMAGE_COLOR);
    if (!image.data) {
      cerr << "Ignoring image " << i << endl;
      continue;
    }
    if (i == 1) {
      Size orgSize = image.size();
      float ratio = orgSize.width / 1.0 / orgSize.height;
      size = Size(lines * ratio, lines);
      cout << "Input size: " << orgSize << endl;
      cout << "Output size: " << size << endl;
    }
    Mat frame;
    resize(image, frame, size);
    frames.push_back(frame);
  }

  stabilize_frames(frames);
  blend_frames(frames);
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

  /*
  // write output
  VideoWriter video("out.avi", CV_FOURCC('X','V','I','D'), fps, size, true);
  for (vector<Mat>::iterator it = frames.begin();
       it != frames.end();
       it = frames.erase(it)) {
    video.write(*it);
  }*/

  return 0;
}
