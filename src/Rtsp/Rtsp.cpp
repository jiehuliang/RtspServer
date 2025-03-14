#include "Rtsp.h"
#include <cstring>
#include <iostream>
#include <arpa/inet.h>

#define AV_RB16(x) ((((const uint8_t *)(x))[0] << 8) | ((const uint8_t *)(x))[1])

size_t RtpHeader::getCsrcSize() const {
	//each csrc occupies 4 bytes
	return csrc << 2;
}

uint8_t* RtpHeader::getCsrcData() {
	if (!csrc) {
		return nullptr;
	}
	return &payload;
}

size_t RtpHeader::getExtSize() const {
	//rtp has ext
	if (!ext) {
		return 0;
	}
	auto ext_ptr = &payload + getCsrcSize();
	//uint16_t reserved = AV_RB16(ext_ptr);
	//each ext occupies 4 bytes
	return AV_RB16(ext_ptr + 2) << 2;
}
uint16_t RtpHeader::getExtReserved() const{
	//rtp has ext
	if (!ext) {
		return 0;
	}
	auto ext_ptr = &payload + getCsrcSize();
	return AV_RB16(ext_ptr);
}

uint8_t* RtpHeader::getExtData() {
	if (!ext) {
		return nullptr;
	}
	auto ext_ptr = &payload + getCsrcSize();
	//the extra 4 bytes is reserved , ext_len
	return ext_ptr + 4;
}

size_t RtpHeader::getPayloadOffset() const {
	//when there is ext ,you also need to ignore the reserved, ext_len 4 bytes
	return getCsrcSize() + (ext ? (4 + getExtSize()) : 0);
}

uint8_t* RtpHeader::getPayloadData() {
	return &payload + getPayloadOffset();
}

size_t RtpHeader::getPaddingSize(size_t rtp_size) const {
	if (!padding) {
		return 0;
	}
	auto end = (uint8_t*)this + rtp_size - 1;
	return *end;
}

ssize_t RtpHeader::getPayloadSize(size_t rtp_size) const {
	auto invalid_size = getPayloadOffset() + getPaddingSize(rtp_size);
	return (ssize_t)rtp_size - invalid_size - RtpPacket::RtpHeaderSize;
}


RtpHeader* RtpPacket::getHeader() {
	//need to remove rtp over tcp 4 bytes length
	return (RtpHeader*)(data_->Peek() + RtpPacket::RtpTcpHeaderSize);
}

const RtpHeader* RtpPacket::getHeader() const {
	return (RtpHeader*)(data_->Peek() + RtpPacket::RtpTcpHeaderSize);
}

RtpPacket::Ptr RtpPacket::CreateRtp() {
	auto rtp = Ptr(new RtpPacket);
	rtp->data_ = std::shared_ptr<Buffer>(new Buffer);
	return rtp;
}

uint16_t RtpPacket::getSeq() const {
	return ntohs(getHeader()->seq);
}

uint32_t RtpPacket::getStamp() const {
	return ntohl(getHeader()->timestamp);
}

uint64_t RtpPacket::getStampMS(bool ntp) const {

}

uint32_t RtpPacket::getSSRC() const {
	return ntohl(getHeader()->ssrc);
}

uint8_t* RtpPacket::getPayload() {
	return getHeader()->getPayloadData();
}

size_t RtpPacket::getPayloadSize() const {
	return (size_t)getHeader()->getPayloadSize(data_->readablebytes());
}

std::shared_ptr<Buffer> RtpPacket::getData() {
	return data_;
}


std::string Track::getSSRC() {
	char tmp[9] = { 0 };
	auto ui32Ssrc = htonl(_ssrc);
	uint8_t* pSsrc = (uint8_t*)&ui32Ssrc;
	for (int i = 0; i < 4; i++) {
		sprintf(tmp + 2 * i, "%02X", pSsrc[i]);
	}
	return tmp;
}
