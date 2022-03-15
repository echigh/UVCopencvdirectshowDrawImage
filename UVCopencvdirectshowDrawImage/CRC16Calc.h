#pragma once
class CCRC16Calc
{
public:
	CCRC16Calc(void);

	~CCRC16Calc(void);

	unsigned short CRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen);
	unsigned short CRC16_CCITT_FALSE(unsigned char *puchMsg, unsigned int usDataLen);
	unsigned short CRC16_XMODEM(const char *puchMsg, unsigned int usDataLen);
	unsigned short CRC16_X25(unsigned char *puchMsg, unsigned int usDataLen);
	unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen);
	unsigned short CRC16_IBM(unsigned char *puchMsg, unsigned int usDataLen);
	unsigned short CRC16_MAXIM(unsigned char *puchMsg, unsigned int usDataLen);
	unsigned short CRC16_USB(unsigned char *puchMsg, unsigned int usDataLen);

	
private:
	void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf);
	void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf);
};

