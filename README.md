# Camera server

A C++17 project inspired by [Motion](https://github.com/Motion-Project/motion).

There are two main functions of this program:

1. Detect image change (i.e., "motion") from a video feed and record videos
during the periods when a motion event occurs;
1. Provide various inter-process communication methods for downstream
programs to consume live images from different types of cameras
so that downstream programs don't have to implement their own version of video
feed handling repetitively. The following methods are currently supported:

    1. File;
    1. HTTP;
    1. POSIX Shared Memory;
    1. ZeroMQ Pub/Sub mode.

## Dependencies

* [Crow HTTP library](https://github.com/CrowCpp/Crow) for HTTP service support
  * `Asio`, an  asynchronous mode used by Crow:  `apt install libasio-dev`
  * `OpenSSL`, for SSL support: `apt-get install libssl-dev`
* `nlohmann-json3 (>= 3.9)`, JSON support: `apt install nlohmann-json3-dev`
* `opencv`, for frame manipulation: `apt install libopencv-dev`
* `spdlog` for logging: `apt install libspdlog-dev`
* `v4l-utils`: for manually examining and manipulating local video devices.
* `ZeroMQ` for message queue, `apt install libzmq3-dev`
* `FFmpeg`: the back-end used by `OpenCV` to decode/encode videos

  * **No GPU route**  
    * If you don't have an Nvidia GPU, simply issue `apt install ffmpeg` should
    be enough--we will use FFmpeg's default configuration and use the CPU to do
    all the heavy-lifting things.

  * **Nvidia GPU route**

    * With an Nvidia GPU, it is going to be much more complicated. Check
    build notes [here](./helper/build-notes.md).

## Build and deployment

* 
```bash
mkdir ./build
cmake ../
make -j2
```
* Copy `./configs/camera-server.jsonc` to `~/.configs/ak-studio`.

* The program is tested on Debian and should work on other distributions
or POSIX-compliant OSes. However, given that it uses quite a few POSIX APIs,
it is unlikely that it could run on Windows without significant porting effort.

## Quality assurance

* Instead of `cmake ../`, run `cmake .. -DBUILD_ASAN=ON` /
`cmake .. -DBUILD_UBSAN=ON ` to test memory/undefiend behavior error with
AddressSanitizer / UndefinedBehaviorSanitizer.
* The repo is also tested with `Valgrind` from time to time:
`valgrind --leak-check=yes --log-file=valgrind.rpt ./build/cs`.
