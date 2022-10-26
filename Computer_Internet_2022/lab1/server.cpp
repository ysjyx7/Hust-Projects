#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma once
#include"Config.h"
#include<sstream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

int main() {
    cout << "Please input the filepath:";
    cin >> Config::PATH;
    cout << "Please input the Server Address:";
    cin >> Config::SERVERADDRESS;
    cout << "Please input the Port:";
    cin >> Config::PORT;
    WSADATA wsaData;
    fd_set rfds;				//���ڼ��socket�Ƿ������ݵ����ĵ��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�������ݵ�����
    fd_set wfds;				//���ڼ��socket�Ƿ���Է��͵��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�����Է������ݣ�
    bool first_connetion = true;

    int nRc = WSAStartup(0x0202, &wsaData);

    if (nRc) {
        printf("Winsock  startup failed with error!\n");
    }

    if (wsaData.wVersion != 0x0202) {
        printf("Winsock version is not correct!\n");
        WSACleanup();
        return 0;
    }

    printf("Winsock  startup Ok!\n");


    SOCKET srvSocket;
    sockaddr_in addr, clientAddr;
    SOCKET sessionSocket;
    int addrLen;
    //create socket
    srvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSocket != INVALID_SOCKET)
        printf("Socket create Ok!\n");
    //set port and ip
    addr.sin_family = AF_INET;
    addr.sin_port = htons(Config::PORT);
    addr.sin_addr.S_un.S_addr =inet_addr(Config::SERVERADDRESS.c_str());
    //binding
    int rtn = bind(srvSocket, (LPSOCKADDR)&addr, sizeof(addr));
    if (rtn != SOCKET_ERROR)
        printf("Socket bind Ok!\n");
    else {
        cout << "Socket bind Error!" << WSAGetLastError() << endl;
        closesocket(srvSocket);
        WSACleanup();
        return 0;
    }
    //listen
    rtn = listen(srvSocket, 5);
    if (rtn != SOCKET_ERROR)
        printf("Socket listen Ok!\n");
    else {
        cout << "Socket listen Error!" << WSAGetLastError() << endl;
        closesocket(srvSocket);
        WSACleanup();
        return 0;
    }
    clientAddr.sin_family = AF_INET;
    addrLen = sizeof(clientAddr);
    char recvBuf[4096];

    u_long blockMode = 1;//��srvSock��Ϊ������ģʽ�Լ����ͻ���������

    if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO���������ֹ�׽ӿ�s�ķ�����ģʽ��
        cout << "ioctlsocket() failed with error!\n";
        return 0;
    }
    cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

    //���read,write����������rfds��wfds�����˳�ʼ����������FD_ZERO����գ��������FD_SET
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    //���õȴ��ͻ���������
    FD_SET(srvSocket, &rfds);

    while (true) {
        //���read,write������
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        //���õȴ��ͻ���������
        FD_SET(srvSocket, &rfds);

        if (!first_connetion) {
            //���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
            FD_SET(sessionSocket, &rfds);
            FD_SET(sessionSocket, &wfds);
        }

        //��ʼ�ȴ�
        int nTotal = select(0, &rfds, &wfds, NULL, NULL);

        //���srvSock�յ��������󣬽��ܿͻ���������
        if (FD_ISSET(srvSocket, &rfds)) {
            nTotal--;

            //�����ỰSOCKET
            sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
            if (sessionSocket != INVALID_SOCKET)
                printf("Socket listen one client request!\n");
            else {
                cout << "Socket listened is invalid" << endl;
            }

            //�ѻỰSOCKET��Ϊ������ģʽ
            if ((rtn = ioctlsocket(sessionSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO���������ֹ�׽ӿ�s�ķ�����ģʽ��
                cout << "ioctlsocket() failed with error!\n";
                return 0;
            }
            cout << "ioctlsocket() for session socket ok!	Waiting for client connection and data\n";

            //���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
            FD_SET(sessionSocket, &rfds);
            FD_SET(sessionSocket, &wfds);

            first_connetion = false;

        }

        //���ỰSOCKET�Ƿ������ݵ���
        if (nTotal > 0) {
            //����ỰSOCKET�����ݵ���������ܿͻ�������
            if (FD_ISSET(sessionSocket, &rfds)) {
                //receiving data from client
                memset(recvBuf, '\0', Config::BUFFERLENGTH);
                rtn = recv(sessionSocket, recvBuf, Config::BUFFERLENGTH, 0);
                if (rtn > 0) {

                    printf("Received %d bytes from client: %s\n", rtn, recvBuf);
                }
                else {
                    printf("Client leaving ...\n");
                    closesocket(sessionSocket);  //��Ȼclient�뿪�ˣ��͹ر�sessionSocket
                }

            }
        }
    }

}