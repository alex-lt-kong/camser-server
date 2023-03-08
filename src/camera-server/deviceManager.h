#include <string>
#include <queue>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include <nlohmann/json.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "utils.h"

using namespace std;
using namespace cv;
using njson = nlohmann::json;


// This multithreading model is inspired by:
// https://stackoverflow.com/questions/1151582/pthread-function-from-a-class
class MyEventLoopThread {
public:
    MyEventLoopThread() {}
    virtual ~MyEventLoopThread() {}

    /** Returns true if the thread was successfully started, false if there was an error starting the thread */
    void StartInternalEventLoopThread() {
        if (_internalThreadShouldQuit == false) {
            throw runtime_error("StartInternalEventLoopThread() is called "
                "when the internal thread is still running");
        }
        _internalThreadShouldQuit = false;
        if (pthread_create(&_thread, NULL,
            InternalThreadEntryFunc, this) != 0) {
            throw runtime_error("pthread_create() failed, errno: " +
                to_string(errno));
        }
    }

    /**
     * @brief set the _internalThreadShouldQuit. Users should check this
     * signal periodically and quit the event loop timely based on the signal.
    */
    void StopInternalEventLoopThread() {
        _internalThreadShouldQuit = true;
    }

    void WaitForInternalEventLoopThreadToExit() {
        pthread_join(_thread, NULL);
    }

    /**
     * @brief One should either WaitForInternalEventLoopThreadToExit() or
     * DetachInternalEventLoopThread()
    */
    void DetachInternalEventLoopThread() {
        if (pthread_detach(_thread) == 0) {
            throw runtime_error("failed to pthread_detach() a thread, errno: " +
                to_string(errno));
        }
    }



protected:
    /** Implement this method in your subclass with the code you want your thread to run. */
    virtual void InternalThreadEntry() = 0;
    bool _internalThreadShouldQuit = true;

private:
    static void * InternalThreadEntryFunc(void * This) {
        ((MyEventLoopThread *)This)->InternalThreadEntry();
        return NULL;
    }
    pthread_t _thread = 0;
};


class deviceManager : public MyEventLoopThread {

public:
    deviceManager();
    ~deviceManager();
    void setParameters(const size_t deviceIndex, const njson& defaultConf,
        njson& overrideConf);
    void getLiveImage(vector<uint8_t>& pl);

protected:
    void InternalThreadEntry();


private:
    pthread_mutex_t mutexLiveImage;
    vector<uint8_t> encodedJpgImage;
    njson conf;
    size_t deviceIndex = 0;
    string deviceName;
    double fontScale = 1;

    // motionDetection variables
    enum MotionDetectionMode motionDetectionMode;
    double frameDiffPercentageUpperLimit = 0;
    double frameDiffPercentageLowerLimit = 0;
    double pixelDiffAbsThreshold = 0;    
    uint64_t diffEveryNthFrame = 1;
    bool drawContours;

    // videoRecording variables   
    bool encoderUseExternal; 
    string pipeRawVideoTo;
    uint32_t maxFramesPerVideo;

    int snapshotFrameInterval = 1;
    float throttleFpsIfHigherThan;
    int frameIntervalInMs = 24;
    string snapshotPath;
    queue<long long int> frameTimestamps;

    volatile sig_atomic_t* done;
    
    void updateVideoCooldownAndVideoFrameCount(int64_t& cooldown,
        uint32_t& videoFrameCount);
    bool shouldFrameBeThrottled();
    string fillinVariables(basic_string<char> originalString);
    string convertToString(char* a, int size);
    string getCurrentTimestamp();
    void startOrKeepVideoRecording(FILE*& ffmpegPipe, VideoWriter& vwriter,
        int64_t& cooldown, string& timestampOnVideoStarts);
    void stopVideoRecording(FILE*& ffmpegPipe, VideoWriter& vwriter,
        uint32_t& videoFrameCount, string& timestampOnVideoStarts, int cooldown);
    void overlayDatetime(Mat frame);
    void overlayDeviceName(Mat frame);
    void overlayContours(Mat dispFrame, Mat diffFrame);
    void overlayChangeRate(Mat frame, float changeRate, int cooldown, long long int videoFrameCount);
    float getFrameChanges(Mat prevFrame, Mat currFrame, Mat* diffFrame);
    void generateBlankFrame(Mat& currFrame);
};
