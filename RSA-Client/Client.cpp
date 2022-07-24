#include <windows.h>
#include <stdio.h>
#include "Client.h"
#include"RsaOpe.h"
#include "DesOpe.h"
#include<math.h>
#pragma comment(lib,"ws2_32.lib")


int runClient()
{
	//1.�����׽��ֿ�
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err) 
		printf("C: ����socket����ʧ��\n"); 
	else 
		printf("C: ����socket���ӳɹ�\n"); 
	//2.����һ���׽��ֹ�ʹ��
	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	//3.�������������������
	SOCKADDR_IN socksin;//��¼�������˵�ַ
	socksin.sin_family = AF_INET;
	socksin.sin_port = htons(6020);
	socksin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int rf = connect(ServerSocket, (SOCKADDR*)&socksin, sizeof(socksin));
	if (rf == SOCKET_ERROR)
		printf("C: �����������ʧ��\n");
	else
	{
		printf("C: ����������ӳɹ�\n");

		char publicKey[100] = { 0 };
		recv(ServerSocket, publicKey, 100, 0);
		printf("C: �ӷ��������յ���Կ:%s\n", publicKey);
		char sN[100], sE[100];
		int pN = 0, pE = 0;
		bool divide = false;
		for (int i = 0; i < strlen(publicKey); i++)
		{
			if (publicKey[i] == ',') { divide = true; continue; }
			if (!divide) 
				sN[pN++] = publicKey[i];
			else 
				sE[pE++] = publicKey[i];
		}
		PublicKey rsaPublic;
		rsaPublic.nE = atoi(sE);
		rsaPublic.nN = atoll(sN);
		char encryKey[300];

		char desKey[8] = { 0 }, plaintext[255], ciphtext[500] = { 0 };
		GenerateDesKey(desKey);
		printf("C: �������DES��Կ:");
		for (int i = 0; i < 8; i++) {
			printf("%c", desKey[i]);
		}
		printf("\n");
		//scanf("%s", desKey);
		char tempK[8];
		strcpy(tempK, desKey);
		op.MakeKey(tempK);
		int p = 0;
		for (int i = 0; i < 4; i++) {
			int p1 = i * 2, p2 = i * 2 + 1;
			int num1 = int(desKey[p1]);
			int num2 = int(desKey[p2]);
			UINT64 curNum = (num1 << 8) + num2;
			UINT64 encry = Encry(curNum, rsaPublic);
			char cencry[20];
			sprintf(cencry, "%lu", encry);
			strncpy(encryKey + p, cencry, strlen(cencry));
			p += strlen(cencry);
			char divide = ',';
			encryKey[p] = divide;
			p += 1;
		}
		encryKey[p] = '\0';
		//���ͼ��ܵ�DES��Կ����������
		printf("C: ����������ͼ��ܵ�DES��Կ:%s\n", encryKey);
		send(ServerSocket, encryKey, p, 0);


		while (1)
		{  //����������Ϣ
			printf("C: ����������:");
			setbuf(stdin, NULL);
			scanf("%[^\n]s", plaintext);//ʹ�ÿ��д����ȡ��϶����ǿո�
			bool exit = false;
			if (strcmp(plaintext, "exit") == 0) { exit = true; }
			op.MakeData(plaintext);
			int count = 0;
			char time[64];
			strcpy(time, op.getTime());
			printf("C: [%s]���������������:", time);
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
			send(ServerSocket, ciphtext, strlen(ciphtext), 0);
			if (exit) 
				break; 
			//���÷��ص��׽��ֺͷ�����ͨ�ţ����ռ�����Ϣ
			char s[256] = { 0 };
			recv(ServerSocket, s, 256, 0);
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
			printf("\nC: ���շ�����������:");
			for(int i=0;i<counts/8;i++)
				printf("%d,", s1[i]);
			printf("\n");
			memset(op.plaintext, 0, sizeof(op.plaintext));//��ʼ������
			//�յ�������Ϣ�󣬽��н���
			op.groupCount = 0;
			for (int i = 0; i < strlen(s); i++)//����յ��ļ�����Ϣ��תΪ����������
			{
				op.ciphArray[op.groupCount][i % 64] = s[i] - 48;
				if ((i + 1) % 64 == 0)
					op.groupCount++;
			}
			for (int i = 0; i < op.groupCount; i++)
				op.MakeCiph(op.ciphArray[i], i);
			//������ܺ������
			strcpy(time, op.getTime());
			printf("C: [%s]�������ܺ������:", time);
			for (int i = 0; i < op.groupCount; i++)
				op.Bit2Char(op.textArray[i]);
			printf("%s\n", op.plaintext);
			if (strcmp(op.plaintext, "exit") == 0) 
				break; 
		}
		//����û�ѡ���˳�����������������˳�����
		printf("\nC: �˳�...");
	}
	closesocket(ServerSocket);
	WSACleanup();
	return 0;
}