#ifndef GUARD_PMAC2Turbo_h
#define GUARD_PMAC2Turbo_h

#include <string>


#define ETHERNETCMDSIZE 8
#define ETHERNET_DATA_SIZE 1492

#define MAX_BUFFER_SIZE 2097152
#define INPUT_SIZE        (ETHERNET_DATA_SIZE+1)  /* +1 to allow space to add terminating ACK */
#define STX   '\2'
#define CTRLB '\2'
#define CTRLC '\3'
#define CTRLD  0x04
#define ACK   '\6'
#define CTRLF '\6'
#define BELL  '\7'
#define CTRLG '\7'
#define CTRLK  0X0B
#define CTRLP '\16'
#define CTRLV '\22'
#define CTRLX '\24'

// Ethernet command structure.  The pragma is to foce the compiler to
// pack bytes without bufferring
#pragma pack(1)
typedef struct tagEthernetCmd
{
    unsigned char RequestType;
    unsigned char Request;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength; /* length of bData */
    unsigned char bData[ETHERNET_DATA_SIZE];
} ETHERNETCMD,*PETHERNETCMD;
#pragma pack()

#define ETHERNET_CMD_HEADER ( sizeof(ETHERNETCMD) - ETHERNET_DATA_SIZE )

// PMAC RequestType fields
#define VR_UPLOAD   0xC0
#define VR_DOWNLOAD 0x40

// PMAC Request fields
#define VR_PMAC_SENDLINE      0xB0
#define VR_PMAC_GETLINE       0xB1
#define VR_PMAC_FLUSH         0xB3
#define VR_PMAC_GETMEM        0xB4
#define VR_PMAC_SETMEM        0xB5
#define VR_PMAC_SETBIT        0xBA
#define VR_PMAC_SETBITS       0xBB
#define VR_PMAC_PORT          0xBE
#define VR_PMAC_GETRESPONSE   0xBF
#define VR_PMAC_READREADY     0xC2
#define VR_CTRL_RESPONSE      0xC4
#define VR_PMAC_GETBUFFER     0xC5
#define VR_PMAC_WRITEBUFFER   0xC6
#define VR_PMAC_WRITEERROR    0xC7
#define VR_FWDOWNLOAD         0xCB
#define VR_IPADDRESS          0xE0






class PMAC2Turbo
{
  public:
    PMAC2Turbo ();
    PMAC2Turbo (std::string const& IP, int const PORT = 1025);
    ~PMAC2Turbo ();


    void Connect (std::string const& IP, int const PORT = 1025);
    void Disconnect ();

    void Terminal ();

    void Flush ();
    void IPAddress (std::string const& IP = "");
    void SendCTRLK ();
    void SendLine (std::string const& Line);
    void WriteBuffer (std::string const& Buffer);
    void GetBuffer (std::string const& OutFileName = "");
    void ListGather (std::string const& OutFileName = "");


  private:
    int fSocket;
    ETHERNETCMD fEthCmd;
    char fData[1401];
    std::string fDataSend;


};






#endif
