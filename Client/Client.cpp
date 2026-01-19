#include <iostream>
#include <Windows.h>
#include <cstring>  // 用于 strlen 函数
#include "PacketUtils.h"
#pragma comment(lib, "ws2_32.lib")
#define RECV_BUFFER_LEN 1024*1024*1

int main()
{
    // 初始化网络环境
    WSADATA wsadata;
    int wsa_ret = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (wsa_ret != 0) {
        std::cout << "初始化Winsock失败，错误码：" << wsa_ret << std::endl;
        return 0;
    }
    // 创建socket
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cout << "创建客户端socket失败\n" << std::endl;
        return 0;
    }
    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ntohs(9999);
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.2.6");

    // 连接服务器
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        std::cout << "连接服务器失败\n" << std::endl;
        return 0;
    }
    std::cout << "连接服务器成功\n" << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////
    char buffer[1024] = {0};
    char* recv_buffer = (char*)malloc(RECV_BUFFER_LEN);  
    int count = 0;
    while (true) {
        count++;
        //snprintf(buffer, sizeof(buffer), "%d", count);  
        std::cout << "请输入要发送的数据：";
        fgets(buffer, 1024, stdin);

        Packet* packet = PackPacket(2000, buffer, 10);
        int send_len = send(client_socket, (char*) & packet->header.magic, GetPacketLen(packet), 0); 
        if (send_len == SOCKET_ERROR) { 
            std::cout << "发送数据失败" << std::endl;
            return 0;
        }
        printf("client send data: %s\r\n", buffer);
        //std::cout << "客户端发送数据成功，发送长度：" << send_len << std::endl;
        free(packet);
        // 等待接收数据
        int recv_len = recv(client_socket, recv_buffer, RECV_BUFFER_LEN, 0); 
        if (recv_len == SOCKET_ERROR) {
            std::cout << "接收数据失败，错误码：" << WSAGetLastError() << std::endl;
        }
        else if (recv_len > 0) {
            Packet* recv_pck = ParsePacket(recv_buffer, recv_len);
            printf("client recv packet->body:%s\r\n", recv_pck->body);
            printf("client recv packet->header.magic:%x\r\n", recv_pck->header.magic);
            printf("client recv packet->header.cmd:%d\r\n", recv_pck->header.cmd);
            printf("client recv packet->header.body_len:%d\r\n", recv_pck->header.body_len);
            if (recv_pck->header.cmd == 1) {
            // 服务器返回屏幕数据，这里进行解析

            }
            free(recv_pck);
        }
    }
    // 关闭客户端socket，避免资源泄漏
    closesocket(client_socket);
    WSACleanup();
    return 0;
}