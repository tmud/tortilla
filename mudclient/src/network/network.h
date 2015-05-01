#pragma once

#include "zlib.h"

#ifdef _DEBUG
#define OUTPUT_BYTES(data, len, maxlen, label) OutputBytesBuffer(data, len, maxlen, label);
#define OUTPUT_OPTION(data, label) OutputTelnetOption(data, label);
void OutputBytesBuffer(const void *data, int len, int maxlen, const char* label);
void OutputTelnetOption(const void *data, const char* label);
#else
#define OUTPUT_BYTES(data, len, maxlen, label)
#define OUTPUT_OPTION(data, label)
#endif

#define IAC             255 // ff - in hex
#define DONT            254 // fe
#define DO              253 // fd
#define WONT            252 // fc
#define WILL            251 // fb
#define SB              250 // fa
#define SE              240 // f0
#define GA              249 // f9
//MCCP
#define COMPRESS        85  // 55
#define COMPRESS2       86  // 56
//MTTS (terminal type)
#define TTYPE           24  // 18
#define TTYPE_IS        0
#define TTYPE_SEND      1
//MSDP
#define MSDP            69  // 45
#define MSDP_VAR         1
#define MSDP_VAL         2
#define MSDP_TABLE_OPEN  3
#define MSDP_TABLE_CLOSE 4
#define MSDP_ARRAY_OPEN  5
#define MSDP_ARRAY_CLOSE 6
//other (not supported yet)
#define TELOPT_MSSP      70 // 46
#define TELOPT_MSP       90 // 5a
#define TELOPT_MXP       91 // 5b
#define TELOPT_ATCP     200 // c8
#define TELOPT_GMCP     201 // c9
#define TELOPT_NAWS      31 // 1f
#define TELOPT_CHARSET   42 // 2a

enum NetworkEvents
{
    NE_NOEVENT = 0,
    NE_NEWDATA,
    NE_CONNECT,
    NE_DISCONNECT,
    NE_ERROR_CONNECT,
    NE_ERROR,
    NE_ERROR_MCCP
};

struct NetworkConnectData
{
    std::string address;
    int port;
    HWND wndToNotify;
    UINT notifyMsg;
};

struct MccpStatus
{
    MccpStatus() : network_data_len(0), game_data_len(0), status(0) {}
    int network_data_len;
    int game_data_len;
    int status;
};

class Network
{
public:
    Network();
    ~Network();
    bool connect(const NetworkConnectData& data);
    NetworkEvents processMsg(DWORD msg_lparam);
    DataQueue* receive();
    DataQueue* receive_msdp();
    bool send(const tbyte* data, int len);
    bool sendplain(const tbyte* data, int len); // send data directly
    void disconnect();
    void getMccpRatio(MccpStatus* data);
    void setSendDoubleIACmode(bool on);
    void setUtf8Encoding(bool flag);

private:
    bool send_ex(const tbyte* data, int len);
    int read_socket();
    int write_socket();
    void close();
    int processing_data(const tbyte* buffer, int len, bool *error);
    void init_mccp();
    bool process_mccp();
    void close_mccp();
    void init_mtts();
    bool process_mtts();
    void close_mtts();
    void init_msdp();
    void process_msdp(const tbyte* buffer, int len);
    void close_msdp();

    MemoryBuffer m_input_buffer;            // to receive data from network    
    DataQueue m_mccp_data;                  // accamulated MCCP data from network
    MemoryBuffer m_mccp_buffer;             // to decompress data   
    DataQueue m_input_data;                 // accamulated data from network
    DataQueue m_receive_data;               // ready to receive by app
    DataQueue m_output_buffer;              // buffer to compile output data
    DataQueue m_send_data;                  // data for send to server
    DataQueue m_msdp_data;                  // data of msdp protocol

    SOCKET sock;
    z_stream *m_pMccpStream;
    bool m_mccp_on;

    int  m_totalReaded;
    int  m_totalDecompressed;
    bool m_double_iac_mode;
    bool m_utf8_encoding;

    int  m_mtts_step;

    bool m_msdp_on;
};
