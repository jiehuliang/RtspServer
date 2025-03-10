#ifndef MEDIA_STREAM_H
#define MEDIA_STREAM_H

#include <string>
#include "H264Rtp.h"
#include "Timer/TimeStamp.h"

class RtspMediaStream {
public:
    RtspMediaStream(std::string url);
    ~RtspMediaStream();

    // Create a media stream from .264 or .h264 file
    bool createFromEs(int payload_type, int time_base);

    std::string h264_sdp_create(const char* sps, const int sps_len,
        const char* pps, const int pps_len, int payload_type, int time_base);

    Track::Ptr& getMediaTrack();

    void readFrame();

    std::string getSdp();

    void setEncoderSendCB(const std::function<void(const RtpPacket::Ptr& rtp)>& cb);

private:
    TimeStamp createTimeStamp_;

    std::string url_;

    std::string stream_;

    Track::Ptr _media_track;

    std::string sdp_;

    std::shared_ptr<H264RtpEncoder> encoder_;

private:
    const char* data_;
    size_t size_;

};

#endif // MEDIA_STREAM_H