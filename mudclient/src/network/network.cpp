#include "stdafx.h"
#include "network.h"
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define IAC             255 // ff - in hex
#define DONT            254 // fe
#define DO              253 // fd
#define WONT            252 // fc
#define WILL            251 // fb
#define SB              250 // fa
#define SE              240 // f0
#define GA              249 // f9
#define COMPRESS        85  // 55
#define COMPRESS2       86  // 56

#ifdef _DEBUG
void OutputBytesBuffer(const void *data, int len, int maxlen, const char* label)
{
    OutputDebugStringA(label);
    char tmp[32]; sprintf(tmp, " len=%d\r\n", len);
    OutputDebugStringA(tmp);
    if (maxlen > len) maxlen = len;
    const unsigned char *bytes = (const unsigned char *)data;
    for (int i = 0; i < maxlen; ++i)
    {
        sprintf(tmp, "%.2x ", bytes[i]);
        OutputDebugStringA(tmp);
    }
    OutputDebugStringA("\r\n");
}
#define OUTPUT_BYTES(data, len, maxlen, label) OutputBytesBuffer(data, len, maxlen, label);
#else
#define OUTPUT_BYTES(data, len, maxlen, label)
#endif

Network::Network() : m_pMccpStream(NULL), m_mccp_on(false), m_totalReaded(0), m_totalDecompressed(0), m_double_iac_mode(true)
{
    m_input_buffer.alloc(1024);
    m_mccp_buffer.alloc(8192);
}

Network::~Network()
{
    close();
}

bool Network::connect(const NetworkConnectData& data)
{
    sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (sock == INVALID_SOCKET)
        return false;

    // non blocking mode
    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) != NO_ERROR)
        return false;

    sockaddr_in peer; 
    memset(&peer, 0, sizeof(sockaddr_in));
    peer.sin_family = AF_INET;
    peer.sin_port = htons( data.port );

    const std::string& a = data.address;
    if ( strspn( a.c_str(), "0123456789." ) != a.length() )
    {
        struct hostent *hp = gethostbyname(a.c_str());
        if (!hp)
            return false;
        memcpy((char*)&peer.sin_addr, hp->h_addr, sizeof(peer.sin_addr));
    }
    else
    {
       peer.sin_addr.s_addr = inet_addr( data.address.c_str() );
    }

    if (WSAAsyncSelect(sock, data.wndToNotify, data.notifyMsg, FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE) == SOCKET_ERROR)
    {
        close();
        return false;
    }

    if (WSAConnect( sock, (SOCKADDR*)&peer, sizeof(peer), NULL, NULL, NULL, NULL) == SOCKET_ERROR) 
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            close();
            return false;
        }
    }

    init_mccp();
    return true;
}

void Network::disconnect()
{
    close();
}

void Network::close()
{
    closesocket(sock);
    sock = NULL;
    close_mccp();

    m_input_data.clear();
    m_receive_data.clear();
    m_output_buffer.clear();
    m_send_data.clear();

    m_totalReaded = 0;
    m_totalDecompressed = 0;
}

void Network::getMccpRatio(MccpStatus* data)
{
    data->game_data_len = m_totalDecompressed;
    data->network_data_len = m_totalReaded;   
    data->status = m_mccp_on;
}

void Network::setSendDoubleIACmode(bool on)
{
    m_double_iac_mode = on;
}

NetworkEvents Network::processMsg(DWORD msg_lparam)
{
    WORD error = WSAGETSELECTERROR(msg_lparam);
    if (error)
    {   if (error == WSAECONNREFUSED || error == WSAETIMEDOUT)
            return NE_ERROR_CONNECT;
        if (error == WSAECONNABORTED)
            return NE_DISCONNECT;
        return NE_ERROR;
    }

    WORD event = WSAGETSELECTEVENT(msg_lparam);
    if (event == FD_READ)
    {
        int result = read_socket();
        if (result == -1)
            return NE_ERROR;
        if (result == -2)
            return NE_ERROR_MCCP;
        return (result != 0) ? NE_NEWDATA : NE_NOEVENT;
    }
    else if (event == FD_WRITE)
    {
        return (write_socket() == -1) ? NE_ERROR : NE_NOEVENT;
    }
    else if (event == FD_CLOSE)
    {
        return NE_DISCONNECT;
    }
    else if (event == FD_CONNECT)
    {
        return NE_CONNECT;
    }
    return NE_NOEVENT;
}

int Network::send(const tbyte* data, int len)
{
    const tbyte *b = data;
    const tbyte *e = b + len;

    for (const tbyte *p=b; p!=e; ++p)
    {
        if (*p == IAC)
        {
            m_output_buffer.write(b, (p-b)+1);
            b = p+1;
            if (m_double_iac_mode)
            {
                tbyte iac = IAC;
                m_output_buffer.write(&iac, 1);
            }
        }
    }
    if (b != e)
        m_output_buffer.write(b, e-b);

    int data_len = m_output_buffer.getSize();
    bool result = send_ex((tbyte*)m_output_buffer.getData(), data_len);
    m_output_buffer.truncate(data_len);
    return result ? len : -1;
}

DataQueue* Network::receive()
{
    return &m_receive_data;
}

int Network::read_socket()
{
    while (true)
    {
        WSABUF buffer;
        buffer.buf = m_input_buffer.getData();
        buffer.len = m_input_buffer.getSize();

        DWORD flags = 0;
        DWORD readed = 0;
        if (WSARecv(sock, &buffer, 1, &readed, &flags, NULL, NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                return -1;
        }
        //OUTPUT_BYTES(buffer.buf, readed, 8, "read socket");
        if (readed == 0)
            break;

        m_totalReaded += readed;
        if (!m_mccp_on)
        {
            if (readed == 0)
                return 0;
            m_input_data.write(m_input_buffer.getData(), readed);
            m_totalDecompressed += readed;
        }
        else
        {
            m_mccp_data.write(m_input_buffer.getData(), readed);
            if (m_mccp_data.getSize() == 0)
                return 0;
            if (!process_mccp())
                return -2;
        }
    }

    while(m_input_data.getSize() > 0)
    {
        const tbyte* in = (tbyte*)m_input_data.getData();
        int in_len = m_input_data.getSize();

        bool error = false;
        int processed = processing_data(in, in_len, &error);
        if (error)
            return -1;

        if (processed == 0)      // low data for processing
        {
            break;
        }
        else if (processed < 0)  // truncated system data
        {
            processed = -processed;
        }
        else
        {
            if (processed == 2 && in[0] == IAC && in[1] == IAC)
                m_receive_data.write(in, 1);
            else if (processed == 2 && in[0] == IAC && in[1] == GA)
            {
                char bytes[2] = { 0x1b, 0x5c };
                m_receive_data.write(bytes, 2);
            }
            else
                m_receive_data.write(in, processed);
        }
        m_input_data.truncate(processed);
    }
    return 1;
}

int Network::write_socket()
{
    if (m_send_data.getSize() == 0)
    {
         return 0;
    }

    WSABUF buffer;
    buffer.buf = (char*)m_send_data.getData();
    buffer.len = m_send_data.getSize();

    DWORD flags = 0;
    DWORD sent = 0;
    if (WSASend(sock, &buffer, 1, &sent, flags, NULL, NULL) == SOCKET_ERROR)
        return -1;
    //OUTPUT_BYTES(buffer.buf, sent, 8, "send socket");
    m_send_data.truncate(sent);
    return sent;
}

bool Network::send_ex(const tbyte* data, int len)
{
    assert(len >= 0);
    if (len < 0)
        return false;
    if (len == 0)
        return true;

    m_send_data.write(data, len);

    int sent = write_socket();
    if (sent == -1)
        return false;
    return true;
}

int Network::processing_data(const tbyte* buffer, int len, bool *error)
{
    const tbyte* b = buffer;
    const tbyte* e = b;
    if (*b != IAC && *b != 0)     // find iac symbol or zero
    {
        const tbyte* e = b + len;
        while(b != e)
        {
            if (*b == IAC || *b == 0)
                break;
            b++;
        }
        return len-(e-b);
    }

    if (*b == 0)                 // protect from incorrect data
        return -1;

    if (len < 2)
        return 0;

    e++;
    if (*e == IAC)               // double iac - continue
        return 2;

    if (*e == GA)                // IAC GA - continue
        return 2;

    if (len < 3)
        return 0;

    if (*e == DO)               // block IAC DO option from some muds (lpmud) ?
        return -3;
    
    if (e[0] == WILL)
    {
        if (e[1] == COMPRESS || e[1] == COMPRESS2)
        {
            // server send request for enable MCCP
            tbyte flag = (m_pMccpStream) ? DO : DONT;
            tbyte support[3] = { IAC, flag, e[1] };
            if (!send_ex(support, 3))
               { *error = true; return 0; }
        }
        else
        {
            // report, what client don't support other Telnet options
            tbyte not_support[3] = { IAC, DONT, e[1] };
            if (!send_ex(not_support, 3))
                { *error = true; return 0; }
        }
        return -3;
    }

    if (e[0] == WONT)
    {
        if (e[1] == COMPRESS || e[1] == COMPRESS2)
        {
            close_mccp();
            init_mccp();
        }
        return -3;
    }

    if (len < 5)
        return 0;

    if (e[0] == SB && (e[1] == COMPRESS || e[1] == COMPRESS2) && e[3] == SE)
    {
        if ((e[2] == IAC && e[1] == COMPRESS2) || (e[2] == WILL && e[1] == COMPRESS))
        { 
           m_totalDecompressed -= (len-5);
           m_mccp_data.write(e+4, len-5 );
           m_mccp_on = true;
           if (!process_mccp())
                { *error = true; return 0; }
           return -len;
        }
        return -5;
    }

    // other telnet options
    if (e[0] == SB)
    {
        int se = -1;
        for (int i = 1; i < len; ++i) { if (e[i] == SE) { se = i; break; }}
        if (se == -1) {
            return (len > 10) ? -len : 0;
        }
        return -(se+2);
    }
    return -1;      // skip error IAC
}

void Network::init_mccp()
{
    z_stream *zs = new z_stream();
    zs->next_in    =  NULL;
    zs->avail_in   =  0;
    zs->total_in   =  0;
    zs->next_out   =  NULL;
    zs->avail_out  =  0;
    zs->total_out  =  0;
    zs->zalloc     =  Z_NULL;
    zs->zfree      =  Z_NULL;
    zs->opaque     =  NULL;

    if (inflateInit(zs) != Z_OK)
    {
        delete zs;
        zs = NULL;
    }
    m_pMccpStream = zs;
}

bool Network::process_mccp()
{
    if (m_mccp_data.getSize() == 0)
        return true;
    m_pMccpStream->next_in = (Bytef*)m_mccp_data.getData();
    m_pMccpStream->avail_in = m_mccp_data.getSize();
    m_pMccpStream->next_out = (Bytef*)m_mccp_buffer.getData();
    m_pMccpStream->avail_out = m_mccp_buffer.getSize();

    int error = inflate(m_pMccpStream, Z_NO_FLUSH);
    if (error != Z_OK && error != Z_STREAM_END)
         return false;

    int size = m_mccp_buffer.getSize() - m_pMccpStream->avail_out;
    m_input_data.write(m_mccp_buffer.getData(), size);
    m_totalDecompressed += size;

    int processed = m_mccp_data.getSize() - m_pMccpStream->avail_in;
    m_mccp_data.truncate(processed);

    if (error == Z_STREAM_END)
    {
        int final_block = m_pMccpStream->avail_in;
        m_input_data.write(m_pMccpStream->next_in, final_block);
        m_mccp_data.truncate(final_block);
        close_mccp();
        init_mccp();
        m_totalDecompressed += final_block;
    }
    return true;
}

void Network::close_mccp()
{
    if (m_pMccpStream)
        inflateEnd(m_pMccpStream);
    delete m_pMccpStream;
    m_pMccpStream = NULL;
    m_mccp_on = false;
    m_mccp_data.clear();
}
