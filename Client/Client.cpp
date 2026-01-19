#include <iostream>
#include <Windows.h>
#include <cstring>  // 用于 strlen 函数

#pragma comment(lib, "ws2_32.lib")
#define RECV_BUFFER_LEN 1024*1024*1
#pragma pack(push, 1) // 将这个结构体按照一字节对齐
struct PacketHeader {
    int magic; // 4字节包头
    int cmd; // 4字节命令
    int body_len; // 数据长度
};
#pragma pack(pop)
struct Packet {
    PacketHeader header; // 包头
    char body[]; // 包数据
};
int GetPacketLen(Packet* pck);
Packet* PackPacket(int cmd, char* buffer, int buffer_len);
Packet* ParsePacket(char* buffer, int len);
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
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.85.80");

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
            free(recv_pck);
        }
    }
    // 关闭客户端socket，避免资源泄漏
    closesocket(client_socket);
    WSACleanup();
    return 0;
}
int GetPacketLen(Packet* pck) {
    if (pck != nullptr) {
        return pck->header.body_len + sizeof(PacketHeader);
    }
    return 0; // 补充空指针返回值
}
Packet* PackPacket(int cmd, char* buffer, int buffer_len) {
    Packet* pck = (Packet*)malloc(sizeof(PacketHeader) + buffer_len);
    pck->header.magic = 0x55AA77CC;
    pck->header.cmd = cmd;
    pck->header.body_len = buffer_len;
    if (buffer_len > 0 && buffer != nullptr) {
        memcpy(pck->body, buffer, buffer_len);
    }
    return pck;
}
Packet* ParsePacket(char* buffer, int len) {
    // magic cmd body_len data
    Packet pck;
    Packet* ppck;
    int index = 0;
    // 找包头，4字节
    for (; index < len; index++) {
        if (*(int*)(buffer + index) == 0x55AA77CC) {
            pck.header.magic = *(int*)(buffer + index);
            index += 4;
            break;
        }
    }
    // 命令
    pck.header.cmd = *(int*)(buffer + index); index += 4;
    pck.header.body_len = *(int*)(buffer + index); index += 4;
    // 获取数据
    if (pck.header.body_len > 0) {
        // 创建接收缓存区
        ppck = (Packet*)malloc(sizeof(PacketHeader) + pck.header.body_len);
        // 拷贝用户数据
        memcpy(ppck->body, buffer + index, pck.header.body_len);
        // 拷贝包头
        memcpy(&ppck->header, &pck.header, sizeof(PacketHeader));
        return ppck;
    }
}