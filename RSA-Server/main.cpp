#include "iostream"
#include "DesOpe.h"
#include "Server.h"
#include "StaticArray.h"
#include "RsaOpe.h"
using namespace std;
extern Paraments m_cParament;
int main() {
	char mode[10] = { 0 };
	printf("Server:\n");
	while (1) {
		runServer();
		break;
	}
	system("pause");
	return 1;
}