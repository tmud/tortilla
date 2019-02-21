﻿#pragma once

#include "zlib.h"
#include "../common/tempThread.h"

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
//other symbols
#define TAB              9

enum NetworkEvent
{
    NE_NOEVENT = 0,
    NE_NEWDATA,
    NE_ALREADY_CONNECTED,
    NE_CONNECTING,
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

class NetworkConnection : private TempThread<false>
{
public:
    NetworkConnection(int receive_buffer);
    ~NetworkConnection();
    void connect(const NetworkConnectData& cdata);
    bool connected();
    void disconnect();
    void send(const tbyte* data, int len);
    int  receive(MemoryBuffer *data);
private:
    void threadProc();
    void sendEvent(NetworkEvent e);
    NetworkConnectData m_connection;
    CriticalSection m_cs_connect;
    CriticalSection m_cs_send;
    CriticalSection m_cs_receive;
    DataQueue m_send_data;
    DataQueue m_receive_data;
    MemoryBuffer m_recive_buffer;
    bool m_connected;
    bool m_connecting;
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
    NetworkEvent translateEvent(LPARAM event);
    void connect(const NetworkConnectData& data, bool disable_mccp);
    void disconnect();
    void send(const tbyte* data, int len);      // send with iacs processing
    void sendplain(const tbyte* data, int len); // send data directly
    DataQueue& received();
    DataQueue& receivedMsdp();
    void getMccpStatus(MccpStatus* data);
    void setSendDoubleIACmode(bool on);
    void setUtf8Encoding(bool flag);
private:
    int  read_data();
    int  processing_data(const tbyte* buffer, int len, bool *error);
    void init_mccp();
    bool process_mccp();
    void close_mccp();
    void init_mtts();
    void process_mtts();
    void close_mtts();
    void init_msdp();
    void process_msdp(const tbyte* buffer, int len);
    void close_msdp();

    NetworkConnection m_connection;

    MemoryBuffer m_input_buffer;            // to receive data from network    
    DataQueue m_mccp_data;                  // accamulated MCCP data from network
    MemoryBuffer m_mccp_buffer;             // to decompress MCCP data   
    DataQueue m_input_data;                 // accamulated data from network
    DataQueue m_receive_data;               // ready to get by app

    DataQueue m_output_buffer;              // buffer to accumulate output data
    DataQueue m_msdp_data;                  // data of msdp protocol

    z_stream *m_pMccpStream;
    bool m_mccp_on;
    bool m_disable_mccp;

    int  m_totalReaded;
    int  m_totalDecompressed;
    bool m_double_iac_mode;
    bool m_utf8_encoding;

    int  m_mtts_step;
    bool m_msdp_on;
};
