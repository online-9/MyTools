#ifndef __MYTOOLS_ALGORITHM_RC4_H__
#define __MYTOOLS_ALGORITHM_RC4_H__

#include <time.h>
#include <fstream>
#include "ToolsPublic.h"
#include <vector>

class RC4 {
public:
	/*
	���캯��������Ϊ��Կ����
	*/
	RC4(LPCSTR pszKey)
	{
		for (UINT i = 0; i < strlen(pszKey); ++i)
			K.push_back(pszKey[i]);
		keylen = strlen(pszKey);
	}

	void GetKeyStream(CHAR* puszKeyStream, UINT uKeyStreamSize)
	{
		keyStream(uKeyStreamSize);
		for (UINT i = 0; i < k.size(); ++i)
			puszKeyStream[i] = k.at(i);
	}

	void GetEncrypText(LPCSTR strPlainText, CHAR* pszEnrypText)
	{
		// �������ĳ��� ������Կ��
		auto uLen = strlen(strPlainText);
		keyStream(uLen);
		for (UINT i = 0; i < uLen; ++i)
			pszEnrypText[i] = strPlainText[i] ^ k[i];

	}

private:
	unsigned char S[256]; //״̬��������256�ֽ�
	unsigned char T[256]; //��ʱ��������256�ֽ�
	int keylen;		//��Կ���ȣ�keylen���ֽڣ�ȡֵ��ΧΪ1-256
	std::vector<char> K;	  //�ɱ䳤����Կ
	std::vector<char> k;	  //��Կ��

	/*
	��ʼ��״̬����S����ʱ����T����keyStream��������
	*/
	void initial() {
		for (int i = 0; i<256; ++i){
			S[i] = i;
			T[i] = K[i%keylen];
		}
	}
	/*
	��ʼ����״̬����S����keyStream��������
	*/
	void rangeS() {
		int j = 0;
		for (int i = 0; i<256; ++i){
			j = (j + S[i] + T[i]) % 256;
			//cout<<"j="<<j<<endl;
			S[i] = S[i] + S[j];
			S[j] = S[i] - S[j];
			S[i] = S[i] - S[j];
		}
	}
	/*
	������Կ��
	len:����Ϊlen���ֽ�
	*/
	void keyStream(int len){
		initial();
		rangeS();

		int i = 0, j = 0, t;
		while (len--){
			i = (i + 1) % 256;
			j = (j + S[i]) % 256;

			S[i] = S[i] + S[j];
			S[j] = S[i] - S[j];
			S[i] = S[i] - S[j];

			t = (S[i] + S[j]) % 256;
			k.push_back(S[t]);
		}
	}

};


class RC4_decryption{
public:
	RC4_decryption(LPCSTR puszEncrypText, UINT uEncrypSize, CHAR* pszKey) 
	{
		m_puszEncrypText = puszEncrypText;
		m_uEncrypSize = uEncrypSize;
		m_pszKey = pszKey;
	}
	void GetDecrypText(CHAR* puszDecrypText)
	{
		for (UINT i = 0; i < m_uEncrypSize; ++i)
		{
			puszDecrypText[i] = (CHAR)(m_puszEncrypText[i] ^ m_pszKey[i]);
		}
		puszDecrypText[m_uEncrypSize] = '\0';
	}

private:
	LPCSTR m_puszEncrypText;
	UINT m_uEncrypSize;
	CHAR* m_pszKey;
};


#endif