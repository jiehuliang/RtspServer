#ifndef RTSP_PLAYER_CONTEXT_H
#define RTSP_PLAYER_CONTEXT_H

#include "Rtsp.h"
#include <string>
#include <vector>

enum class ParseUrlError{
    kSuccess = 0,
    kProtocolError,
    kMissingIp,
    kAddressFormatError
};


class RtspPlayerContext{
public:
    RtspPlayerContext();
    ~RtspPlayerContext();

    ParseUrlError ParseUrl(const std::string& url);

    std::string GetCurrentUrl() const;
    std::string GetControlUrl(const std::string& control) const;

    int NextCSeq();

    void SetSessionId(const std::string& session);
    std::string GetSessionId() const;

    void AddTrack(const Track::Ptr& track);
    Track::Ptr GetTrack(int idx) const;
    int GetTrackCount() const;
    std::vector<Track::Ptr>& GetTracks();

    std::string GetAuthHeader(const std::string& method,const std::string& url);


private:
    ParseUrlError ParseProtocol(const std::string& urlparams);
    ParseUrlError ParseUserInfo(const std::string& urlparams);
    ParseUrlError ParseHostPort(const std::string& urlparams);
    ParseUrlError ParsePath(const std::string& urlparams);

public:
    std::string _url;
    std::string _schema;
    std::string _host;
    u_int16_t _port;
    std::string _username;
    std::string _password;
    std::string _app;
    std::string _streamid;
    std::string _auth_type;    

private:
    int _cseq;
    std::string _session_id;
    std::string _content_base;
    std::vector<Track::Ptr> _tracks;

};

inline const char* ErrorToString(ParseUrlError err) {
    switch (err) {
        case ParseUrlError::kSuccess: return "Success";
        case ParseUrlError::kProtocolError: return "Protocol error: unsupported schema";
        case ParseUrlError::kMissingIp: return "Missing IP address";
        case ParseUrlError::kAddressFormatError: return "Address format error";
        default: return "Unknown error";
    }
}


#endif //RTSP_PLAYER_CONTEXT_H