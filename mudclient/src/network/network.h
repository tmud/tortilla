#pragma once

#include "zlib.h"

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

struct MccpData
{
    MccpData() : network_data_len(0), game_data_len(0) {}
    int network_data_len;
    int game_data_len;
};

class Network
{
public:
    Network();
    ~Network();
    bool connect(const NetworkConnectData& data);    
    NetworkEvents processMsg(DWORD msg_lparam);
    DataQueue* receive();
    int send(const tbyte* data, int len);    
    void disconnect();
    void getMccpRatio(MccpData* data);
    void setSendDoubleIACmode(bool on);
    
private:
    bool send_ex(const tbyte* data, int len);
    int read_socket();
    int write_socket();
    void close();
    int processing_data(const tbyte* buffer, int len, bool *error);
    void init_mccp();
    bool process_mccp();
    void close_mccp();

    MemoryBuffer m_input_buffer;            // to receive data from network    
    DataQueue m_mccp_data;                  // accamulated MCCP data from network
    MemoryBuffer m_mccp_buffer;             // to decompress data   
    DataQueue m_input_data;                 // accamulated data from network
    DataQueue m_receive_data;               // ready to receive by app
    DataQueue m_output_buffer;              // buffer to compile output data
    DataQueue m_send_data;                  // data for send to server

    SOCKET sock;
    z_stream *m_pMccpStream;
    bool m_mccp_on;

    int  m_totalReaded;
    int  m_totalDecompressed;
    bool m_double_iac_mode;
};
