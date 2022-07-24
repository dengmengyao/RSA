#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "Server.h"
#include "DesOpe.h"
#include"RsaOpe.h"
#include<math.h>
#pragma comment(lib,"ws2_32.lib")

extern Paraments m_cParament;

int runServer()
{
	//1.�����׽��ֿ�
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err) 
		printf("S: ����socket����ʧ��\n"); 
	else 
		printf("S: ����socket���ӳɹ�\n"); 
	//2.����һ���׽��ֹ�ʹ��
	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	//3.���׽��ְ󶨵����ص�ַ�Ͷ˿���
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6020);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(ServerSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR));
	//4.���׽�������Ϊ����ģʽ���Խ��տͻ�������
	err = listen(ServerSocket, 5);
	if (err)
		printf("S: δ�������Է��ͻ���\n"); 
	else 
		printf("S: �ɹ��������Է��ͻ���\n"); 
	//5.�ȴ������տͻ������󣬷����µ������׽���
	SOCKADDR_IN addr_out;
	int len = sizeof(SOCKADDR);
	SOCKET ClientSocket = accept(ServerSocket, (SOCKADDR*)&addr_out, &len);
	//6.������Կ	
	char desKey[10] = { '\n' };
	RsaParam rsaParam = RsaGetParam();
	m_cParament.d = rsaParam.d;
	m_cParament.e = rsaParam.e;
	m_cParament.n = rsaParam.n;
	PublicKey publicKey = GetPublicKey();
	char cpublicKey[100], sN[100], sE[100];
	sprintf(sN, "%lu", publicKey.nN);
	itoa(publicKey.nE, sE, 10);
	strcpy(cpublicKey, sN);
	int p1 = strlen(sN), p2 = strlen(sE);
	cpublicKey[p1] = ',';
	strncpy(cpublicKey + p1 + 1, sE, p2);
	cpublicKey[p1 + p2 + 1] = '\0';
	printf("S: ��ͻ��˷��͹�Կ�ͼ�����Կ:%s\n", cpublicKey);
	send(ClientSocket, cpublicKey, strlen(cpublicKey), 0);
	//���ռ��ܺ��DES��Կ
	char encryKey[300] = { '\0' };
	recv(ClientSocket, encryKey, 300, 0);//������Կ
	printf("S: �ӿͻ��˽��ռ��ܺ��DES��Կ:%s\n", encryKey);
	int k = 0;
	for (int i = 0; i < 4; i++) {
		char cencry[20] = { '\0' };
		int p = 0;
		while (encryKey[k++] != ',') 
			cencry[p++] = encryKey[k - 1];
		UINT64 encry = atoll(cencry);
		UINT64 decry = Decry(encry);
		desKey[i * 2] = decry >> 8;
		desKey[i * 2 + 1] = decry % 256;
	}
	printf("S: ��Կ���ܺ�:%s", desKey);
	op.MakeKey(desKey);
	while (1)
	{
		memset(op.plaintext, 0, sizeof(op.plaintext));//��ʼ������
		//���÷��ص��׽��ֺͿͻ���ͨ��
		char s[256] = { 0 };
		recv(ClientSocket, s, 256, 0);//��������
		int counts = strlen(s);
		int asc_recv = 0;
		int s1[32];
		for (int i = 0; i < counts; i++) {
			int sub = s[i] - 48;
			sub = sub * pow(2, (7 - i % 8));
			asc_recv += sub;
			if (i % 8 == 7) {
				s1[i / 8] = asc_recv;
				asc_recv = 0;
			}
		}
		printf("\nS: ���տͻ��˵�����:");
		for (int i = 0; i < counts / 8; i++)
			printf("%d,", s1[i]);
		printf("\n");
		//����յ��ļ�����Ϣ��תΪ����������
		op.groupCount = 0;
		//printf("%d\n", strlen(s));
		for (int i = 0; i < strlen(s); i++)
		{
			op.ciphArray[op.groupCount][i % 64] = s[i] - 48;
			if ((i + 1) % 64 == 0) 
				op.groupCount++; 
		}
		//�������ĵĽ���
		for (int i = 0; i < op.groupCount; i++)
			op.MakeCiph(op.ciphArray[i], i);
		//������ܺ������
		char time[64];
		strcpy(time, op.getTime());
		printf("S: [%s]�������ܺ������:", time);
		for (int i = 0; i < op.groupCount; i++)
			op.Bit2Char(op.textArray[i]);
		printf("%s\n", op.plaintext);
		if (strcmp(op.plaintext, "exit") == 0) 
			break;


		//����û���Ҫ����������Ϣ�����������
		char plaintext[255] = { 0 }, ciphtext[500] = { 0 };
		printf("S: ����������:");
		setbuf(stdin, NULL);
		scanf("%[^\n]s", plaintext);//ʹ�ÿ��д����ȡ��϶����ǿո�
		bool exit = false;
		if (strcmp(plaintext, "exit") == 0) 
		   exit = true; 
		op.MakeData(plaintext);
		int count = 0;
		strcpy(time, op.getTime());
		printf("S: [%s]��ͻ��˷�������:", time);
		for (int i = 0; i < op.groupCount; i++)
		{
			for (int j = 0; j < 64; j++)
				ciphtext[count++] = op.ciphArray[i][j] + 48;//Ҫ����48
		}
		ciphtext[count] = '\0';
		int ciphtexts[32];
		int asc = 0;
		for (int i = 0; i < count; i++) {
			int sub = ciphtext[i] - 48;
			sub = sub * pow(2, (7 - i % 8));
			asc += sub;
			if (i % 8 == 7) {
				ciphtexts[i / 8] = asc;
				asc = 0;
			}
		}
		for (int i = 0; i < count / 8; i++)
			printf("%d,", ciphtexts[i]);
		//�������ݸ�������
		send(ClientSocket, ciphtext, strlen(ciphtext), 0);
		if (exit) 
			break; 
	}
	printf("\nS: �˳�...");

	//�ر��׽���
	closesocket(ServerSocket);
	WSACleanup();
	return 0;
}