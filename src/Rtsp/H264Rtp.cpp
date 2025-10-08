#include "H264Rtp.h"
#include <arpa/inet.h>
#include "HooLog/HooLog.h"


int H264RtpDecoder::inputRtp(const RtpPacket::Ptr& rtp) {
	auto seq = rtp->getSeq();
	auto ret = decodeRtp(rtp);
}

int H264RtpDecoder::decodeRtp(const RtpPacket::Ptr& rtp) {
	
}


H264RtpEncoder::H264RtpEncoder(uint32_t ssrc, uint8_t interleaved, uint8_t pt, uint32_t sample_rate, size_t mtu_size) {
    _pt = pt;
    _interleaved = interleaved;
    _ssrc = ssrc;
    _sample_rate = sample_rate;
    _mtu_size = mtu_size;
}

void H264RtpEncoder::insertConfigFrame(uint32_t pts) {
    if (!sps_ || !pps_) {
        return;
    }
    //gop�����sps��ʼ��sps��pps���滹��ʱ�����ͬ�Ĺؼ�֡������mark bitΪfalse
    packRtp(sps_->buffer.c_str(), sps_->len, pts, false, true);
    packRtp(pps_->buffer.c_str(), sps_->len, pts, false, false);
}

int H264RtpEncoder::inputFrame(const H264Nalu::Ptr& nalu) {
    switch (nalu->nal_unit_type) {
    case H264Nalu::H264NaluType::NAL_SPS: {
        sps_ = std::move(nalu);
        return true;
    }
    case H264Nalu::H264NaluType::NAL_PPS: {
        pps_ = std::move(nalu);
        return true;
    }
    default: break;
    }
    if(nalu->dropAble()) {
        return true;
    }
    if (last_nalu_) {
        //���ʱ��������˱仯����ômarkbit����true
        inputFrame_l(last_nalu_, last_nalu_->_pts != nalu->_pts);
    }
    last_nalu_ = std::move(nalu);
}

bool H264RtpEncoder::inputFrame_l(const H264Nalu::Ptr& nalu, bool is_mark){
    if (nalu->keyFrame()) {
        //��֤ÿһ���ؼ�֡ǰ����SPS��PPS
        insertConfigFrame(nalu->_pts);
    }
    packRtp(nalu->buffer.c_str(), nalu->len, nalu->_pts, is_mark, false);
    return true;
}

void H264RtpEncoder::packRtp(const char* data, size_t len, uint32_t pts, bool is_mark, bool gop_pos){
    if (len + 3 <= _mtu_size - RtpPacket::RtpHeaderSize) {
        packRtpStapA(data, len, pts, is_mark, gop_pos);
    } else {
        packRtpFu(data, len, pts, is_mark, gop_pos);
    }
}

void H264RtpEncoder::packRtpStapA(const char* ptr, size_t len, uint32_t pts, bool is_mark, bool gop_pos) {
    auto rtp = makeRtp(0, nullptr, len + 3, is_mark, pts);
    uint8_t* playload = rtp->getPayload();
    playload[0] = (ptr[0] & (~0x1F)) | 24;
    playload[1] = (len >> 8) & 0xFF;
    playload[2] = len & 0xFF;
    memcpy(playload + 3 , (uint8_t*)ptr, len);
    send_cb_(rtp);
}

void H264RtpEncoder::packRtpFu(const char* ptr, size_t len, uint32_t pts, bool is_mark, bool gop_pos) {
    auto packet_size = _mtu_size - RtpPacket::RtpHeaderSize - 2;

    auto fu_flag_0 = (ptr[0] & (~0x1F)) | 28;
    auto fu_flag_1 = (ptr[0] & 0x1F);

    //FU-A start bit
    fu_flag_1 |= 0x80;

    size_t offset = 1;
    while (!(fu_flag_1 & 0x40)) {
        if (!(fu_flag_1 & 0x80) && len <= offset + packet_size) {
            //FU-A end bit
            fu_flag_1 |= 0x40;
            packet_size = len - offset;
        }

        auto rtp = makeRtp(0, nullptr, packet_size + 2, (fu_flag_1 & 0x40) && is_mark, pts);
        uint8_t* playload = rtp->getPayload();
        playload[0] = fu_flag_0;
        playload[1] = fu_flag_1;
        memcpy(playload + 2, ptr + offset, packet_size);
        send_cb_(rtp);

        offset += packet_size;
        fu_flag_1 &= (~0x80);
    }
}


RtpPacket::Ptr H264RtpEncoder::makeRtp(int type,const char* data,size_t len,bool mark,uint32_t stamp) {
    uint16_t playload_len = (uint16_t)(len + RtpPacket::RtpHeaderSize);
    RtpPacket::Ptr rtp = RtpPacket::CreateRtp();
    rtp->sample_rate = _sample_rate;
    //rtsp over tcp ͷ
    auto pack = rtp->getData();
    std::string capacity(RtpPacket::RtpTcpHeaderSize + playload_len, 0);
    pack->Append(capacity.c_str(), RtpPacket::RtpTcpHeaderSize + playload_len);
    auto ptr = pack->Peek();
    ptr[0] = '$';
    ptr[1] = _interleaved;
    ptr[2] = playload_len >> 8;
    ptr[3] = playload_len & 0xFF;

    auto header = rtp->getHeader();
    header->version = RtpPacket::RtpVersion;
    header->padding = 0;
    header->ext = 0;
    header->csrc = 0;
    header->mark = mark;
    header->pt = _pt;
    header->seq = htons(_seq);
    ++_seq;
    header->timestamp = htonl(uint64_t(stamp));
    header->ssrc = htonl(_ssrc);

    //��Ч����
    if (data) {
        memcpy(&ptr[RtpPacket::RtpHeaderSize + RtpPacket::RtpTcpHeaderSize], data, len);
    }
    return rtp;
}

void H264RtpEncoder::set_send_cb(const std::function<void(const RtpPacket::Ptr& rtp)>& send_cb) {
    send_cb_ = send_cb;
}

