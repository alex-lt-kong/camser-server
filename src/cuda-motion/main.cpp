#include "device_manager.h"
#include "global_vars.h"
#include "utils.h"

#include <cxxopts.hpp>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/drogon.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <iostream>
#include <vector>

using namespace drogon;
using namespace std;
using json = nlohmann::json;

vector<unique_ptr<DeviceManager>> myDevices;
string configPath =
    string(getenv("HOME")) + "/.config/ak-studio/cuda-motion.jsonc";

void initialize_http_service(std::string host, int port) {

  app()
      .setLogPath("./")
      .setLogLevel(trantor::Logger::kWarn)
      .addListener(host, port)
      .setThreadNum(16)
      .disableSigtermHandling()
      .registerHandler(
          "/live_image/?deviceId={deviceId}",
          [](const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback,
             const size_t deviceId) {
            drogon::HttpResponsePtr resp;
            if (deviceId < myDevices.size()) {
              resp = HttpResponse::newHttpResponse(HttpStatusCode::k200OK,
                                                   ContentType::CT_IMAGE_JPG);
              std::vector<uint8_t> encodedImg;
              myDevices[deviceId]->getLiveImage(encodedImg);
              resp->setBody(
                  String((const char *)encodedImg.data(), encodedImg.size()));
            } else {
              Json::Value json;
              json["result"] = "error";
              json["reason"] =
                  fmt::format("deviceId {} out of range", deviceId);
              resp = HttpResponse::newHttpJsonResponse(json);
            }
            callback(resp);
          },
          {Get, "Realtime CCTV"})
      .run();
}

int main(int argc, char *argv[]) {
  cxxopts::Options options(argv[0], "video feed handler that uses CUDA");
  // clang-format off
  options.add_options()
    ("h,help", "print help message")
    ("c,config-path", "path of the config file", cxxopts::value<string>()->default_value(configPath));
  // clang-format on
  auto result = options.parse(argc, argv);
  if (result.count("help") || !result.count("config-path")) {
    std::cout << options.help() << "\n";
    return 0;
  }
  configPath = result["config-path"].as<std::string>();

  // Doc: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  // Including microseconds is handy for naive profiling
  spdlog::set_pattern("%Y-%m-%dT%T.%f%z|%5t|%8l| %v");
  spdlog::info("Cuda Motion started (git commit: {})", GIT_COMMIT_HASH);
  install_signal_handler();

  spdlog::info("cv::getBuildInformation(): {}",
               string(cv::getBuildInformation()));

  spdlog::info("Loading json settings from {}", configPath);
  ifstream is(configPath);
  settings = json::parse(is, nullptr, true, true);
  size_t deviceCount = settings["devices"].size();
  if (deviceCount == 0) {
    throw logic_error("No devices are defined.");
  }
  myDevices = vector<unique_ptr<DeviceManager>>();
  for (size_t i = 0; i < deviceCount; ++i) {
    myDevices.emplace_back(make_unique<DeviceManager>(i));
    myDevices[i]->StartEv();
  }
  initialize_http_service(settings["httpService"]["interface"].get<string>(),
                          settings["httpService"]["port"].get<int>());

  spdlog::info("Drogon exited");

  for (size_t i = 0; i < myDevices.size(); ++i) {
    myDevices[i]->JoinEv();
    spdlog::info("{}-th device event loop thread exited gracefully", i);
  }
  // stop_http_service();
  spdlog::info("All device event loop threads exited gracefully");

  return 0;
}
