#include "Rtcp.h"
#include "Logging.h"
#include "Buffer.h"
#include <arpa/inet.h>

void RtcpHeader::setSize(size_t size) {
    length = htons((uint16_t)((size >> 2) - 1));
}

size_t RtcpHeader::getSize(){
    //加上rtcp头长度
    return (1 + ntohs(length)) << 2;
}

void RtcpHeader::net2Host(size_t len) {
    switch ((RTCP_TYPE)pt) {
        case RTCP_TYPE::RTCP_SR: {
            RtcpSR* sr = (RtcpSR*)this;
            sr->net2Host(len);
            break;
        }
        case RTCP_TYPE::RTCP_RR:{
            RtcpRR* rr = (RtcpRR*)this;
            rr->net2Host(len);
            break;
        }
    }
}

std::vector<RtcpHeader*> RtcpHeader::loadFromBytes(char* data, size_t len) {
    std::vector<RtcpHeader*> ret;
    ssize_t remain = len;
    char* ptr = data;
    while (remain > (ssize_t)sizeof(RtcpHeader)) {
        RtcpHeader* rtcp = (RtcpHeader*)ptr;
        auto rtcp_len = rtcp->getSize();
        if (remain < (ssize_t)rtcp_len) {
            LOG_WARN << "非法的rtcp包,声明的长度超过实际数据长度";
            break;
        }
        try {
            rtcp->net2Host(len);
            ret.emplace_back(rtcp);
        }
        catch (std::exception &ex) {
            LOG_WARN << ex.what() << ",长度为:" << rtcp_len;
        }
        ptr += rtcp_len;
        remain -= rtcp_len;
    }
    return ret;
}

std::shared_ptr<Buffer> RtcpHeader::toBuffer(std::shared_ptr<RtcpHeader> rtcp){
    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
    buffer->Append((char*)rtcp.get(), rtcp->getSize());
    return buffer;
}

static size_t alignSize(size_t bytes) {
    return (size_t) ((bytes + 3) >> 2) << 2;
}

static void setupHeader(RtcpHeader *rtcp, RTCP_TYPE type, size_t report_count, size_t total_bytes){
    rtcp->version = 2;
    rtcp->padding = 0;
    if (report_count > 0x1F){
        LOG_INFO << "rtcp report_count max 31 ,now is: " << report_count;
    }
    //items total counts
    rtcp->report_count = report_count;
    rtcp->pt = (uint8_t) type;
    rtcp->setSize(total_bytes);
}

static void setupPadding(RtcpHeader* rtcp, size_t padding_size) {
    if (padding_size) {
        rtcp->padding = 1;
        ((uint8_t*)rtcp)[rtcp->getSize() - 1] = padding_size & 0xFF;
    }
    else {
        rtcp->padding = 0;
    }
}

#define CHECK_MIN_SIZE(size, kMinSize) \
if (size < kMinSize) { \
    throw std::out_of_range(" rtcp 包长度不足:" ); \
}

#define CHECK_REPORT_COUNT(item_count) \
/*修正个数，防止getItemList时内存越界*/ \
if (report_count != item_count) { \
    LOG_WARN << std::to_string((int)(RTCP_TYPE)pt) << " report_count 字段不正确,已修正为:" << (int)report_count << " -> " << item_count; \
    report_count = item_count; \
}

std::shared_ptr<RtcpSR> RtcpSR::create(size_t item_count){
    auto real_size = sizeof(RtcpSR) - sizeof(ReportItem) + item_count * sizeof(ReportItem);
    auto bytes = alignSize(real_size);
    auto ptr = (RtcpSR *)new char[bytes];
    setupHeader(ptr, RTCP_TYPE::RTCP_SR, item_count, bytes);
    setupPadding(ptr, bytes - real_size);
    return std::shared_ptr<RtcpSR>(ptr, [](RtcpSR* ptr) {
        delete[](char*) ptr;
        });
}

void RtcpSR::setNtpStamp(struct timeval tv) {
    ntpmsw = htonl(tv.tv_sec + 0x83AA7E80); /* 0x83AA7E80 is the number of seconds from 1900 to 1970 */
    ntplsw = htonl((uint32_t)((double)tv.tv_usec * (double)(((uint64_t)1) << 32) * 1.0e-6));
}

void RtcpSR::setNtpStamp(uint64_t unix_stamp_ms) {
    struct timeval tv;
    tv.tv_sec = unix_stamp_ms / 1000;
    tv.tv_usec = (unix_stamp_ms % 1000) * 1000;
    setNtpStamp(tv);
}

void ReportItem::net2Host() {
    ssrc = ntohl(ssrc);
    cumulative = ntohl(cumulative) >> 8;
    seq_cycles = ntohs(seq_cycles);
    seq_max = ntohs(seq_max);
    jitter = ntohl(jitter);
    last_sr_stamp = ntohl(last_sr_stamp);
    delay_since_last_sr = ntohl(delay_since_last_sr);
}

std::vector<ReportItem*> RtcpRR::getItemList() {
    std::vector<ReportItem*> ret;
    ReportItem* ptr = &items;
    for (int i = 0; i < (int)report_count; ++i) {
        ret.emplace_back(ptr);
        ++ptr;
    }
    return ret;
}

void RtcpRR::net2Host(size_t size) {
    static const size_t kMinSize = sizeof(RtcpRR) - sizeof(items);
    CHECK_MIN_SIZE(size, kMinSize);
    ssrc = ntohl(ssrc);

    ReportItem* ptr = &items;
    int item_count = 0;
    for (int i = 0; i < (int)report_count && (char*)(ptr)+sizeof(ReportItem) <= (char*)(this) + size; ++i) {
        ptr->net2Host();
        ++ptr;
        ++item_count;
    }

    CHECK_REPORT_COUNT(item_count);
}

size_t SdesChunk::totalBytes() const {
    return alignSize(minSize() + txt_len);
}

size_t SdesChunk::minSize() {
    return sizeof(SdesChunk) - sizeof(text);
}

void SdesChunk::net2Host() {
    ssrc = ntohl(ssrc);
}

std::shared_ptr<RtcpSdes> RtcpSdes::create(const std::vector<std::string>& item_text) {
    size_t item_total_size = 0;
    for (auto& text : item_text) {
        //统计所有SdesChunk对象占用的空间
        item_total_size += alignSize(SdesChunk::minSize() + (0xFF & text.size()));
    }
    auto real_size = sizeof(RtcpSdes) - sizeof(SdesChunk) + item_total_size;
    auto bytes = alignSize(real_size);
    auto ptr = (RtcpSdes*)new char[bytes];
    memset(ptr, 0x00, bytes);
    auto item_ptr = &ptr->chunks;
    for (auto& text : item_text) {
        item_ptr->txt_len = (text.size() & 0xFF);
        //确保赋值\0为RTCP_SDES_END
        memcpy(item_ptr->text, text.data(), item_ptr->txt_len + 1);
        item_ptr = (SdesChunk*)((char*)item_ptr + item_ptr->totalBytes());
    }

    setupHeader(ptr, RTCP_TYPE::RTCP_SDES, item_text.size(), bytes);
    setupPadding(ptr, bytes - real_size);
    return std::shared_ptr<RtcpSdes>(ptr, [](RtcpSdes* ptr) {
        delete[](char*) ptr;
    });
}

void RtcpSdes::net2Host(size_t size) {
    static const size_t kMinSize = sizeof(RtcpSdes) - sizeof(chunks);
    CHECK_MIN_SIZE(size, kMinSize);
    SdesChunk* ptr = &chunks;
    int item_count = 0;
    for (int i = 0; i < (int)report_count && (char*)(ptr)+SdesChunk::minSize() <= (char*)(this) + size; ++i) {
        ptr->net2Host();
        ptr = (SdesChunk*)((char*)ptr + ptr->totalBytes());
        ++item_count;
    }
    CHECK_REPORT_COUNT(item_count);
}

std::vector<SdesChunk*> RtcpSdes::getChunkList() {
    std::vector<SdesChunk*> ret;
    SdesChunk* ptr = &chunks;
    for (int i = 0; i < (int)report_count; ++i) {
        ret.emplace_back(ptr);
        ptr = (SdesChunk*)((char*)ptr + ptr->totalBytes());
    }
    return ret;
}
