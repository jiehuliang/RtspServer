#ifndef RTCP_H
#define RTCP_H
#include "common.h"

#include <memory>

#define RTCP_TYPE_MAP_ENTRIES(XX) \
	XX(RTCP_FIR, 192) \
	XX(RTCP_NACK, 193) \
	XX(RTCP_SMPTETC, 194) \
	XX(RTCP_IJ, 195) \
	XX(RTCP_SR, 200) \
	XX(RTCP_RR, 201) \
	XX(RTCP_SDES, 202) \
	XX(RTCP_BYE, 203) \
	XX(RTCP_APP, 204) 

#define SDES_TYPE_MAP_ENTRIES(XX) \
	XX(RTCP_SDES_END, 0) \
	XX(RTCP_SDES_CNAME, 1) \
    XX(RTCP_SDES_NAME, 2) \
    XX(RTCP_SDES_EMAIL, 3) \
    XX(RTCP_SDES_PHONE, 4) \
    XX(RTCP_SDES_LOC, 5) \
    XX(RTCP_SDES_TOOL, 6) \
    XX(RTCP_SDES_NOTE, 7) \
    XX(RTCP_SDES_PRIVATE, 8) 
	
#define PSFB_TYPE_MAP(XX) \
	XX(RTCP_PSFB_PLI, 1) \
    XX(RTCP_PSFB_SLI, 2) \
    XX(RTCP_PSFB_RPSI, 3) \
    XX(RTCP_PSFB_FIR, 4) \
    XX(RTCP_PSFB_TSTR, 5) \
    XX(RTCP_PSFB_TSTN, 6) \
    XX(RTCP_PSFB_VBCM, 7) \
    XX(RTCP_PSFB_REMB, 15) 

#define RTPFB_TYPE_MAP(XX) \
    XX(RTCP_RTPFB_NACK, 1) \
    XX(RTCP_RTPFB_TMMBR, 3) \
    XX(RTCP_RTPFB_TMMBN, 4) \
    XX(RTCP_RTPFB_TWCC, 15) \


#define XX(name, value) name = value,
enum class RTCP_TYPE : uint8_t { RTCP_TYPE_MAP_ENTRIES(XX) };
#undef XX

#define XX(name, value) name = value,
enum class SDES_TYPE : uint8_t { SDES_TYPE_MAP_ENTRIES(XX) };
#undef XX

#define XX(name, value) name = value,
enum class PSFB_TYPE : uint8_t { PSFB_TYPE_MAP(XX) };
#undef XX

#define XX(name, value) name = value,
enum class RTPFB_TYPE : uint8_t { RTPFB_TYPE_MAP(XX) }; 
#undef XX


class RtcpHeader {
public:
#if __BYTE_ORDER == __BIG_ENDIAN
    uint32_t version : 2;
    uint32_t padding : 1;
    uint32_t report_count : 5;
#else
    uint32_t report_count : 5;
    uint32_t padding : 1;
    uint32_t version : 2;
#endif
    uint32_t pt : 8;

private:
    uint32_t length : 16;

public:
    static std::vector<RtcpHeader*> loadFromBytes(char* data, size_t size);

    static std::shared_ptr<Buffer> toBuffer(std::shared_ptr<RtcpHeader> rtcp);

    void setSize(size_t size);

    size_t getSize();

private:
    void net2Host(size_t size);
};

class ReportItem {
public:
    friend class RtcpRR;

    uint32_t ssrc;
    uint32_t fraction : 8;
    uint32_t cumulative : 24;
    uint16_t seq_cycles;
    uint16_t seq_max;
    uint32_t jitter;
    //Last SR timestamp, NTP timestamp,(ntpmsw & 0xFFFF) << 16  | (ntplsw >> 16) & 0xFFFF)
    uint32_t last_sr_stamp;
    //Delay since last SR timestamp,expressed in units of 1/65536 seconds
    uint32_t delay_since_last_sr;

private:
    void net2Host();
};

/*
 * 6.4.1 SR: Sender Report RTCP Packet

        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
header |V=2|P|    RC   |   PT=SR=200   |             length            |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                         SSRC of sender                        |
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
sender |              NTP timestamp, most significant word             |
info   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |             NTP timestamp, least significant word             |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                         RTP timestamp                         |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                     sender's packet count                     |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                      sender's octet count                     |
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
report |                 SSRC_1 (SSRC of first source)                 |
block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  1    | fraction lost |       cumulative number of packets lost       |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |           extended highest sequence number received           |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                      interarrival jitter                      |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                         last SR (LSR)                         |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                   delay since last SR (DLSR)                  |
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
report |                 SSRC_2 (SSRC of second source)                |
block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  2    :                               ...                             :
       +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
       |                  profile-specific extensions                  |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
 // sender report
class RtcpSR : public RtcpHeader {
public:
    uint32_t ssrc;
    // ntp timestamp MSW(in second)
    uint32_t ntpmsw;
    // ntp timestamp LSW(in picosecond)
    uint32_t ntplsw;
    uint32_t rtpts;
    uint32_t packet_count;
    uint32_t octet_count;
    ReportItem items;

public:
    static std::shared_ptr<RtcpSR> create(size_t item_count);

    void setNtpStamp(struct timeval tv);
    void setNtpStamp(uint64_t unix_stamp_ms);
};

class RtcpRR :public RtcpHeader {
public:
    friend class RtcpHeader;

    uint32_t ssrc;
    //may be more
    ReportItem items;

public:
    static std::shared_ptr<RtcpRR> create(size_t item_count);

    std::vector<ReportItem*> getItemList();
private:
    void net2Host(size_t size);
};

#endif //RTCP_H