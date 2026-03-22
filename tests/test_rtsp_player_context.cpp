#include "Rtsp/RtspPlayerContext.h"
#include <iostream>
#include <cassert>

void TestBasicUrl() {
    std::cout << "=== TestBasicUrl ===\n";
    RtspPlayerContext ctx;

    ParseUrlError err = ctx.ParseUrl("rtsp://192.168.1.100:8554/live/stream");

    assert(err == ParseUrlError::kSuccess);
    assert(ctx._host == "192.168.1.100");
    assert(ctx._port == 8554);
    assert(ctx._app == "/live");
    assert(ctx._streamid == "stream");

    std::cout << "TestBasicUrl PASSED\n\n";
}

void TestUrlWithAuth() {
    std::cout << "=== TestUrlWithAuth ===\n";
    RtspPlayerContext ctx;

    ctx.ParseUrl("rtsp://admin:123456@192.168.1.100/live/stream");

    assert(ctx._username == "admin");
    assert(ctx._password == "123456");

    std::cout << "TestUrlWithAuth PASSED\n\n";
}

void TestDefaultPort() {
    std::cout << "=== TestDefaultPort ===\n";
    RtspPlayerContext ctx;

    ctx.ParseUrl("rtsp://192.168.1.100/live/stream");

    assert(ctx._port == 554);

    std::cout << "TestDefaultPort PASSED\n\n";
}

void TestGetCurrentUrl() {
    std::cout << "=== TestGetCurrentUrl ===\n";
    RtspPlayerContext ctx;

    ctx.ParseUrl("rtsp://admin:123456@192.168.1.100:8554/live/stream");
    std::string url = ctx.GetCurrentUrl();

    assert(url == "rtsp://admin:123456@192.168.1.100:8554/live/stream");

    std::cout << "TestGetCurrentUrl PASSED\n\n";
}

void TestGetControlUrl() {
    std::cout << "=== TestGetControlUrl ===\n";
    RtspPlayerContext ctx;

    ctx.ParseUrl("rtsp://192.168.1.100:8554/live/stream");

    std::string control1 = ctx.GetControlUrl("trackID=0");
    assert(control1 == "rtsp://192.168.1.100:8554/live/stream/trackID=0");

    std::string control2 = ctx.GetControlUrl("rtsp://192.168.1.100:8554/live/stream/trackID=1");
    assert(control2 == "rtsp://192.168.1.100:8554/live/stream/trackID=1");

    std::cout << "TestGetControlUrl PASSED\n\n";
}

void TestCSeq() {
    std::cout << "=== TestCSeq ===\n";
    RtspPlayerContext ctx;

    assert(ctx.NextCSeq() == 1);
    assert(ctx.NextCSeq() == 2);
    assert(ctx.NextCSeq() == 3);

    std::cout << "TestCSeq PASSED\n\n";
}

void TestGetAuthHeader() {
    std::cout << "=== TestGetAuthHeader ===\n";
    RtspPlayerContext ctx;

    ctx.ParseUrl("rtsp://admin:123456@192.168.1.100/live/stream");
    std::string auth = ctx.GetAuthHeader("DESCRIBE", "/live/stream");

    assert(!auth.empty());
    assert(auth.find("Authorization: Basic ") == 0);

    std::cout << "TestGetAuthHeader PASSED\n\n";
}

void TestNoAuth() {
    std::cout << "=== TestNoAuth ===\n";
    RtspPlayerContext ctx;

    ctx.ParseUrl("rtsp://192.168.1.100/live/stream");
    std::string auth = ctx.GetAuthHeader("DESCRIBE", "/live/stream");

    assert(auth.empty());

    std::cout << "TestNoAuth PASSED\n\n";
}

int main() {
    std::cout << "========== RtspPlayerContext Tests ==========\n\n";

    TestBasicUrl();
    TestUrlWithAuth();
    TestDefaultPort();
    TestGetCurrentUrl();
    TestGetControlUrl();
    TestCSeq();
    TestGetAuthHeader();
    TestNoAuth();

    std::cout << "========== All Tests PASSED! ==========\n";

    return 0;
}
