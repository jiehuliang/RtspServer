#include "RtspPlayerContext.h"
#include "Util/base64.h"


RtspPlayerContext::RtspPlayerContext(){
    _url = "";
    _schema = "rtsp";
    _host = "";
    _port = 554;
    _username = "";
    _password = "";
    _auth_type = "Basic";  
    
    _cseq = 0;
    _session_id = "";
    _content_base = "";
}

RtspPlayerContext::~RtspPlayerContext(){

}

ParseUrlError RtspPlayerContext::ParseUrl(const std::string& url){
    _url = url;
    ParseUrlError errorCode = ParseProtocol(url);
    if(ParseUrlError::kSuccess == errorCode){
        _content_base = _schema + "://" + _host + ":" + std::to_string(_port) + _app + "/" + _streamid + "/";
    }
    return errorCode;
}

ParseUrlError RtspPlayerContext::ParseProtocol(const std::string& part){
    size_t size = part.find_first_of("://");
    _schema = part.substr(0,size);
    if("rtsp" == _schema || "rtsps" == _schema){
        return ParseUserInfo(part.substr(size+3));
    }else{
        return ParseUrlError::kProtocolError;
    }
}

ParseUrlError RtspPlayerContext::ParseUserInfo(const std::string& part){
    size_t size = part.find("@");
    if(size != std::string::npos){
        std::string userinfo = part.substr(0,size);
        size_t usernamepos = userinfo.find_first_of(":");
        _username = userinfo.substr(0,usernamepos);
        _password = userinfo.substr(usernamepos+1);
        return ParseHostPort(part.substr(size+1));
    }
    return ParseHostPort(part);
}

ParseUrlError RtspPlayerContext::ParseHostPort(const std::string& part){
    size_t size = part.find_first_of("/");
    std::string ipportinfo;
    std::string pathpart;
    if(size != std::string::npos){
        ipportinfo = part.substr(0,size);
        pathpart = part.substr(size+1);
    }else{
        ipportinfo = part;
        pathpart = "";
    }

    size_t hostpos = ipportinfo.find_last_of(":");
    if(hostpos != std::string::npos){
        _host = ipportinfo.substr(0,hostpos);
        if(_host.empty()){
            return ParseUrlError::kMissingIp;
        }
        _port =static_cast<u_int16_t>(atoi(ipportinfo.substr(hostpos+1).c_str()));
    }else{
        _host = ipportinfo;
        _port = (_schema == "rtsp" ? 554 : 322);
    }
    return ParsePath(pathpart);

}

ParseUrlError RtspPlayerContext::ParsePath(const std::string& part){
    std::string tmp_part = part;
    if(tmp_part.empty()){
        _app = "/";
        _streamid = "";
        return ParseUrlError::kSuccess;
    }

    if('/' == part.back()){
        tmp_part.pop_back();
    }
    size_t last_slash = tmp_part.find_last_of("/");
    if(last_slash == std::string::npos){
        _app = "/";
        _streamid = tmp_part;
    }else{
        _app = "/" + tmp_part.substr(0,last_slash);
        _streamid = tmp_part.substr(last_slash+1);
    }
    return ParseUrlError::kSuccess;
}

std::string RtspPlayerContext::GetCurrentUrl() const{
    std::string url = _schema + "://";
    if(!_username.empty()){
        url += _username;
        if(!_password.empty()){
            url += ":" + _password;
        }
        url += "@";
    }
    url += _host;
    if(554 != _port){
        url += ":" + std::to_string(_port);
    }
    url += _app;
    if(!_streamid.empty()){
        url += "/" + _streamid;
    }
    return url;
}

std::string RtspPlayerContext::GetControlUrl(const std::string& control) const{
    if(control.empty()){
        return GetCurrentUrl();
    }
    if(control.find("rtsp://") == 0 || control.find("rtsps://") == 0){
        return control;
    }
    if(control[0] == '/'){
        return _schema + "://" + _host + ":" + std::to_string(_port) + control;
    }
    return _content_base + control;
}

int RtspPlayerContext::NextCSeq(){
    return ++_cseq;
}

void RtspPlayerContext::SetSessionId(const std::string& session){
    _session_id = session;
}
std::string RtspPlayerContext::GetSessionId() const{
    return _session_id;
}

void RtspPlayerContext::AddTrack(const Track::Ptr& track){
    _tracks.push_back(track);
}
Track::Ptr RtspPlayerContext::GetTrack(int idx) const{
    if(idx >= 0 && idx < static_cast<int>(_tracks.size())){
        return _tracks[idx];
    }
    return nullptr;
}

int RtspPlayerContext::GetTrackCount() const{
    return static_cast<int>(_tracks.size());
}
std::vector<Track::Ptr>& RtspPlayerContext::GetTracks(){
    return _tracks;
}

std::string RtspPlayerContext::GetAuthHeader(const std::string& method,const std::string& url){
    std::string authheader;
    if(_username.empty()){
        return "";
    }
    if("Basic" == _auth_type){
        authheader = "Authorization: Basic " + encodeBase64((_username+":"+_password));
    }else{

    }
    return authheader;
}