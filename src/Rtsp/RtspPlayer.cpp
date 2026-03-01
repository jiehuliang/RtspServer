#include "RtspPlayer.h"

RtspPlayer::RtspPlayer() {
}

RtspPlayer::~RtspPlayer() {
}

void RtspPlayer::setConnWeakPtr(const std::weak_ptr<TcpConnection>& conn_weak) {
	conn_weak_self = conn_weak;
}

void RtspPlayer::onPlay(){
    sendOptions();
}



void RtspPlayer::sendOptions(){

}
