#include "iostream"
#include "DesOpe.h"
#include "Client.h"
#include "StaticArray.h"
#include "RsaOpe.h"
using namespace std;
extern Paraments m_cParament;
int main() {
	char mode[10] = { 0 };
	printf("Client:\n");
	while (1) {
		runClient();
		break;
	}
	system("pause");
	return 1;
}