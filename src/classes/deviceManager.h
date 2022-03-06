#include <string>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

class deviceManager {

public:
  deviceManager();
  bool captureImage(string imageSaveTo);
  void startMotionDetection();
  void stopMotionDetection();
  bool setParameters(
    string deviceUrl,
    string deviceName,
    string frameResolution,
    int frameRotation,
    string snapshotPath,
    double fontScale,
    string externalCommand,
    string videoDirectory);

private:
  bool stopSignal = false;
  double fontScale = 1;
  int frameRate = 5;
  int frameRotation = -1;
  string deviceUrl = "";
  string deviceName = "";
  string frameResolution = "";   
  string snapshotPath = "";
  string externalCommand = "";
  string videoDirectory = "";
  string ffmpegCommand = "/usr/bin/ffmpeg -y -f rawvideo -pixel_format bgr24 -video_size widthxheight -framerate __framerate__ -i pipe:0 -vcodec libvpx-vp9 videoPath";
  
  string convertToString(char* a, int size);
  string getCurrentTimestamp();
  void overlayDatetime(Mat frame);
  void overlayChangeRate(Mat frame, float changeRate, int cooldown);
};