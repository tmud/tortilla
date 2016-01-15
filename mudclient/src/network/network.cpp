#include "stdafx.h"
#include "network.h"
#include <winsock2.h>
 #include <Mstcpip.h>
#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
void OutputBytesBuffer(const void *data, int len, int maxlen, const char* label)
{
    if (maxlen > len) maxlen = len;
    std::string l("["); l.append(label);
    char tmp[32]; sprintf(tmp, " len=%d,show=%d]:\r\n", len, maxlen); l.append(tmp);
    OutputDebugStringA(l.c_str());
    const unsigned char *bytes = (const unsigned char *)data;
    len = maxlen;
    const int show_len = 32;
    unsigned char *buffer = new unsigned char[show_len];

    while (len > 0)
    {
        int toshow = show_len;
        if (toshow > len) toshow = len;
        std::string hex;
        for (int i = 0; i < toshow; ++i)
        {
            sprintf(tmp, "%.2x ", bytes[i]);
            hex.append(tmp);
        }
        memcpy(buffer, bytes, toshow);
        for (int i=0; i<toshow; ++i) {
            if (buffer[i] < 32) buffer[i] = '.';
        }
        hex.append((const char*)buffer, toshow);
        OutputDebugStringA(hex.c_str());
        OutputDebugStringA("\r\n");
        bytes += toshow;
        len -= toshow;
    }
    delete []buffer;
}

void OutputTelnetOption(const void *data, const char* label)
{
    OutputDebugStringA(label);
    char tmp[32];
    unsigned char byte = *(const char*)data;
    sprintf(tmp, ": %d (%.2x)\r\n", byte, byte);
    OutputDebugStringA(tmp);
}
#endif

NetworkConnection::NetworkConnection() : m_connected(false)
{
    m_recive_buffer.alloc(1024);
}

NetworkConnection::~NetworkConnection()
{
    disconnect();
}

void NetworkConnection::connect(const NetworkConnectData& cdata)
{
    if (connected())
        return sendEvent(NE_ALREADY_CONNECTED);
    m_connection = cdata;
    if (!run())
        return sendEvent(NE_ERROR);
}

bool NetworkConnection::connected() const
{
    return m_connected;
}

void NetworkConnection::disconnect()
{
    if (!connected())
        return;
    stop();
    wait();
    m_send_data.clear();
    m_receive_data.clear();    
    m_connected = false;
}

bool NetworkConnection::send(const tbyte* data, int len)
{
    if (!connected())
        return false;
    CSectionLock lock(m_cs_send);
    m_send_data.write(data, len);
    return true;
}

bool NetworkConnection::receive(MemoryBuffer *data)
{
    if (!connected())
        return false;
    CSectionLock lock(m_cs_receive);
    data->alloc(m_receive_data.getSize());
    memcpy(data->getData(), m_receive_data.getData(), m_receive_data.getSize());
    m_receive_data.clear();
    return true;
}

void NetworkConnection::sendEvent(NetworkEvent e)
{
    PostMessage(m_connection.wndToNotify, m_connection.notifyMsg, 0, (LPARAM)e);
}

void NetworkConnection::threadProc()
{
    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        sendEvent(NE_ERROR_CONNECT);
        return;
    }

    class autoclose {
        SOCKET socket;
    public:
        autoclose(SOCKET s) : socket(s) {}
        ~autoclose() { if (socket != INVALID_SOCKET) closesocket(socket); socket = INVALID_SOCKET; }
    } ac(sock);

    // connecting
    sockaddr_in peer;
    memset(&peer, 0, sizeof(sockaddr_in));
    peer.sin_family = AF_INET;
    peer.sin_port = htons(m_connection.port);

    const std::string& a = m_connection.address;
    if (strspn(a.c_str(), "0123456789.") != a.length())
    {
        struct hostent *hp = gethostbyname(a.c_str());
        if (!hp)
        {
            sendEvent(NE_ERROR_CONNECT);
            return;
        }
        memcpy((char*)&peer.sin_addr, hp->h_addr, sizeof(peer.sin_addr));
    }
    else
    {
        peer.sin_addr.s_addr = inet_addr(m_connection.address.c_str());
    }

    /* keep alive option
    DWORD optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)(&optval), sizeof(DWORD)))
    {
        sendEvent(NE_ERROR_CONNECT);
        return;
    }
    tcp_keepalive alive;
    alive.onoff = 1;
	alive.keepalivetime = 5000;    // <- ����� ����� ��������� keep-alive (��)
	alive.keepaliveinterval = 500; // <- ����� ����� ��������� ��� ��������� ������
    DWORD nSize = 0;
    if  (WSAIoctl(sock, SIO_KEEPALIVE_VALS, &alive, sizeof(alive), NULL, 0, &nSize,NULL,NULL) == SOCKET_ERROR)
    {
        sendEvent(NE_ERROR_CONNECT);
        return;
    }*/

    sendEvent(NE_CONNECTING);
    if (::connect(sock, (sockaddr*)&peer, sizeof(peer)) == SOCKET_ERROR)
    {
        sendEvent(NE_ERROR_CONNECT);
        return;
        /*if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            sendEvent(NE_ERROR_CONNECT);
            return;
        }*/
    }
    m_connected = true;
    sendEvent(NE_CONNECT);    

    fd_set set;
    FD_ZERO(&set);
    FD_SET(sock, &set);

    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;

    while (!needStop())
    {
        int n = ::select(0, &set, NULL, NULL, &tv);
        if (n == SOCKET_ERROR)
        {
            sendEvent(NE_ERROR);
            m_connected = false;
            break;
        }

        if (FD_ISSET(sock, &set))
        {
             // receive new data
             int received = ::recv(sock, m_recive_buffer.getData(), m_recive_buffer.getSize(), 0);
             if (received == SOCKET_ERROR)
             {
                 if (WSAGetLastError() == WSAEWOULDBLOCK) {}    // ��� ������
                 else
                 {
                    sendEvent(NE_DISCONNECT);
                    break;
                 }
             }
             if (received > 0)
             {
                 CSectionLock lock(m_cs_receive);
                 //OUTPUT_BYTES(m_recive_buffer.getData(), received, received, "received from net");
                 m_receive_data.write(m_recive_buffer.getData(), received);
             }
             if (received > 0)
                 sendEvent(NE_NEWDATA);
        }

        // send exist data
        if (m_send_data.getSize() > 0)
        {
            CSectionLock lock(m_cs_send);
            //OUTPUT_BYTES(m_send_data.getData(), sended, 32, "sended to net");
            int sended = ::send(sock, (const char*)m_send_data.getData(), m_send_data.getSize(), 0);
            if (sended == SOCKET_ERROR)
            {
                sendEvent(NE_DISCONNECT);
                break;
            }            
            m_send_data.truncate(sended);
        }
    }
    FD_CLR(sock, &set);
    shutdown(sock, 2);
}

Network::Network() : m_pMccpStream(NULL), m_mccp_on(false), m_totalReaded(0), m_totalDecompressed(0), 
m_double_iac_mode(true), m_utf8_encoding(false), m_mtts_step(-1), m_msdp_on(false)
{
    //m_input_buffer.alloc(1024);
    m_mccp_buffer.alloc(8192);
    m_output_buffer.setBufferSize(1024);
}

Network::~Network()
{
}

void Network::connect(const NetworkConnectData& data)
{
    m_connection.connect(data);
}

void Network::disconnect()
{
    m_connection.disconnect();

    close_mccp();
    close_mtts();
    close_msdp();

    m_input_data.clear();
    m_receive_data.clear();
    m_output_buffer.clear();
    m_send_data.clear();

    m_totalReaded = 0;
    m_totalDecompressed = 0;
}

NetworkEvent Network::translateEvent(LPARAM event)
{
    NetworkEvent e = (NetworkEvent)event;
   /* if (e == NE_NEWDATA)
    {
        read_data();        
    }*/
    return e;

    /*
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
    else if (event ==FD_ADDRESS_LIST_CHANGE )
    {
        MessageBox(NULL, L"Address list change", L"network", MB_OK);
    }
    else if (event ==FD_ROUTING_INTERFACE_CHANGE )
    {
        MessageBox(NULL, L"Routing interface change", L"network", MB_OK);
    }*/
    return NE_NOEVENT;
}

bool Network::send(const tbyte* data, int len)
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

    bool result = m_connection.send((tbyte*)m_output_buffer.getData(), m_output_buffer.getSize());
    m_output_buffer.clear();
    return result;
}

bool Network::sendplain(const tbyte* data, int len)
{
    return m_connection.send(data, len);
}

bool Network::receive(DataQueue* data)
{
    int result = read_data();
    if (result == -1)
        return NE_ERROR;
    if (result == -2)
        return NE_ERROR_MCCP;
    return (result != 0) ? NE_NEWDATA : NE_NOEVENT;


    return m_connection.receive(data);
}

DataQueue* Network::receiveMsdp()
{
    return &m_msdp_data;
}

int Network::read_data()
{
    if (!m_connection.receive(&m_input_buffer))
        return -1;
    int readed = m_input_buffer.getSize();

/*        WSABUF buffer;
        buffer.buf = m_input_buffer.getData();
        buffer.len = m_input_buffer.getSize();

        DWORD flags = 0;
        DWORD readed = 0;

        if (WSARecv(sock, &buffer, 1, &readed, &flags, NULL, NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                return -1;
        }
        if (readed == 0)
            break;*/

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

    while(m_input_data.getSize() > 0)
    {
        const tbyte* in = (tbyte*)m_input_data.getData();
        int in_len = m_input_data.getSize();

        bool error = false;
        int processed = processing_data(in, in_len, &error);
        if (error)
            return -1;

        if (processed == 0)      // low/no data for processing
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
                unsigned char bytes[2] = { 0x1b, 0x5c };
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
         return 0;

    WSABUF buffer;
    buffer.buf = (char*)m_send_data.getData();
    buffer.len = m_send_data.getSize();

    DWORD flags = 0;
    DWORD sent = 0;
    if (WSASend(sock, &buffer, 1, &sent, flags, NULL, NULL) == SOCKET_ERROR)
        return -1;

    m_send_data.truncate(sent);
    return sent;
}

bool Network::send_ex(const tbyte* data, int len)
{
    assert(data && len >= 0);
    if (len < 0)
        return false;
    if (len > 0)
        m_send_data.write(data, len);
    int sent = write_socket();
    return (sent == -1) ? false : true;
}

int Network::processing_data(const tbyte* buffer, int len, bool *error)
{
    const tbyte* b = buffer;
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

    const tbyte* e = b + 1;

    if (*e == IAC)               // double IAC - continue
        return 2;

    if (*e == GA)                // IAC GA - continue
        return 2;

    if (len < 3)
        return 0;

    if (e[0] == DO)               // some command via IAC DO option for negotiation
    {
        OUTPUT_OPTION(&e[1], "IAC DO");
        if (e[1] == TTYPE)
        {
            tbyte support[3] = { IAC, WILL, e[1] };
            if (!send_ex(support, 3))
                {  *error = true; return 0; }
            init_mtts();
        }
        else
        {
            // report, what client don't support other Telnet options
            tbyte not_support[3] = { IAC, WONT, e[1] };
            if (!send_ex(not_support, 3))
                {  *error = true; return 0; }
        }
        return -3;
    }

    if (e[0] == DONT)
    {
        OUTPUT_OPTION(&e[1], "IAC DONT");
        if (e[1] == TTYPE)
            close_mtts();
        return -3;
    }

    if (e[0] == WILL)
    {
        OUTPUT_OPTION(&e[1], "IAC WILL");
        if (e[1] == COMPRESS || e[1] == COMPRESS2)
        {
            tbyte flag = (m_pMccpStream) ? DO : DONT;
            tbyte support[3] = { IAC, flag, e[1] };
            if (!send_ex(support, 3))
               { *error = true; return 0; }
        }
        else if (e[1] == MSDP)
        {
            tbyte support[3] = { IAC, DO, e[1] };
            if (!send_ex(support, 3))
               { *error = true; return 0; }
            init_msdp();
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
        OUTPUT_OPTION(&e[1], "IAC WONT");
        if (e[1] == COMPRESS || e[1] == COMPRESS2)
        {
            close_mccp();
            init_mccp();
        }
        if (e[1] == MSDP)
        {
            close_msdp();
        }
        return -3;
    }

    if (len < 5)
        return 0;

    if (e[0] == SB && (e[1] == COMPRESS || e[1] == COMPRESS2) && e[3] == SE)
    {
        if ((e[2] == IAC && e[1] == COMPRESS2) || (e[2] == WILL && e[1] == COMPRESS))
        {
          if (m_mccp_on)
               { *error = true; return 0; }
           m_totalDecompressed -= (len-5);
           m_mccp_data.write(e+4, len-5);
           m_mccp_on = true;
           if (!process_mccp())
                { *error = true; return 0; }
           return -len;
        }
        return -5;
    }

    if (e[0] == SB && e[1] == MSDP)
    {
        for (int i = 3; i < len; ++i) {
            if (e[i] == SE && e[i-1] == IAC && e[i-2] != IAC)
            {
                // finished msdp data block
                process_msdp(b, i+2);
                return -(i+2);
            }
        }
        return 0;
    }

    if (len < 6)
        return 0;

    if (e[0] == SB && e[1] == TTYPE && e[2] == TTYPE_SEND && e[3] == IAC && e[4] == SE)
    {
        if (!process_mtts())
             { *error = true; return 0; }
        return -6;
    }

    // other SB options
    if (e[0] == SB)
    {
        for (int i = 2; i < len; ++i) {
            if (e[i] == SE && e[i-1] == IAC && e[i-2] != IAC) return -(i+2);
        }
        return 0;
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
    while (m_mccp_data.getSize() > 0)
    {
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

void Network::init_mtts()
{
    m_mtts_step = 0;
}

void Network::close_mtts()
{
    m_mtts_step = -1;
}

// ref: http://tintin.sourceforge.net/mtts/
bool Network::process_mtts()
{
    if (m_mtts_step == -1) 
        return true;
    std::string str("MTTS 41");
    if (m_mtts_step == 0)
        str.assign("TORTILLA");
    else if (m_mtts_step == 1)
        str.assign("ANSI-256COLOR");
    else if (m_utf8_encoding)
        str.assign("MTTS 45");
    m_mtts_step++;

    int len = str.length() + 6;
    tbyte *tosend = new tbyte[len];
    tosend[0] = IAC;
    tosend[1] = SB;
    tosend[2] = TTYPE;
    tosend[3] = TTYPE_IS;
    memcpy(&tosend[4], str.c_str(), str.length());
    tosend[len-2] = IAC;
    tosend[len-1] = SE;
    bool result = send_ex(tosend, len);
    delete tosend;
    return result;
}

void Network::init_msdp()
{
    tbyte turnon[3] = { IAC, DO, MSDP };
    m_msdp_data.write(turnon, 3);
    m_msdp_on = true;
}

void Network::close_msdp()
{
    if (m_connection.) 
    {
        tbyte turnoff[3] = { IAC, DONT, MSDP };
        m_msdp_data.write(turnoff, 3);
    }
    m_msdp_on = false;
}

void Network::process_msdp(const tbyte* buffer, int len)
{
    if (m_msdp_on)
        m_msdp_data.write(buffer, len);
}

void Network::getMccpStatus(MccpStatus* data)
{
    data->game_data_len = m_totalDecompressed;
    data->network_data_len = m_totalReaded;
    data->status = m_mccp_on;
}

void Network::setSendDoubleIACmode(bool on)
{
    m_double_iac_mode = on;
}

void Network::setUtf8Encoding(bool flag)
{
    m_utf8_encoding = flag;
}
