#if !defined __NT88API_H__
#define __NT88API_H__

#ifdef __cplusplus
extern "C"
{
#endif
 	 long  NTFindFirst(char appName[32]);

	 long  NTLogin(char * password);
	
	 long  NTGetHardwareID(char * hardwareID);

	 long  NTRead(long address, long length, unsigned char * pBuffer);

	 long  NTWrite(long address, long length, unsigned char * pBuffer);
	
	 //3DES
	 long  NT3DESCBCEncrypt(unsigned char iv[8], unsigned char * pDataBuffer, long buffLen);

	 long  NT3DESCBCDecrypt(unsigned char iv[8], unsigned char * pDataBuffer, long buffLen);
	
	 //License
	 long  NTCheckLicense(long licenseCode);
	
	 long  NTLogout();
	


#ifdef __cplusplus
}

#endif



#endif
