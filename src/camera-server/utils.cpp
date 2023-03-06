#include "utils.h"

// This exec()/exec_async() model is inspired by:
//  https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
string exec(const string& cmd) {
    /*
    string redirectedCmd = cmd + " 2>&1";
    string result;
    array<char, 128> buffer;
    unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(redirectedCmd.c_str(), "r"), pclose);
    if (!pipe) {
        spdlog::error("popen({}) failed, errno: {}", cmd, errno);
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;*/
    array<char, 128> buffer;
    string output = "";

    string redirectedCmd = cmd + " 2>&1";
    FILE* pipe = popen(redirectedCmd.c_str(), "r");
    
    if (!pipe) {
        spdlog::error("popen({}) failed, errno: {}", cmd, errno);
        return output;
    }
    
    while (!feof(pipe)) {
        if (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            output += buffer.data();
    }
    int rc = pclose(pipe);
    if (rc != EXIT_SUCCESS) {
        spdlog::warn("external command returns non-zero! (rc: {})", rc);
    }
    return output;
}

void exec_async(void* This, const string& cmd, exec_cb cb) {
    auto f = [](void* This, string cmd, exec_cb cb) {
        string result = exec(cmd.c_str());
        cb(This, result);
    };    
    thread th_exec(f, This, cmd, cb);
    th_exec.detach();
}
