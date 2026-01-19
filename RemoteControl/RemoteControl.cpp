#include <iostream>
#include <Windows.h>
#include <cstring>  // 用于 strlen 函数

#pragma comment(lib, "ws2_32.lib")
#define RECV_BUFFER_SIZE 1024*1024*1
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
enum CMD {
    CMD_SCREEN = 1,
    CMD_MOUSE = 2,
    CMD_KEYBOARD = 4,
    CMD_TEST = 10,
};
Packet* ParsePacket(char* buffer, int len);
int GetPacketLen(Packet* pck);
Packet* PackPacket(int cmd, char* buffer, int buffer_len);
int HandleCommand(Packet* packet);
int HandleScreen(Packet* packet);
int HandleMouse(Packet* packet);
int HandleKeyBoard(Packet* packet);
int HandleTest(Packet* packet);

int main()
{
    // 初始化网络环境
    WSADATA wsadata;
    int wsa_ret = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (wsa_ret != 0) {
        std::cout << "初始化Winsock失败，错误码：" << wsa_ret << std::endl;
        return 0;
    }
    // 创建 socket(ip4,tcp,0)
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cout << "创建服务器 socket 失败\n" << std::endl;
        return 0;
    }
    // 给服务器绑定地址
    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ntohs(9999);
    server_addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(SOCKADDR_IN))== SOCKET_ERROR) {
        std::cout << "绑定服务器socket失败\n" << std::endl;
        return 0;
    }
    // 开启服务器监听(backlog是指允许三次握手的客户端数量)
    if (listen(server_socket, 1) == SOCKET_ERROR) {
        std::cout << "服务器socket监听失败\n" << std::endl;
        return 0;
    }
    // 等待客户端连接  accept， 返回客户端的socket，
    // 等待客户端连接，是会阻塞的
    SOCKADDR_IN client_addr;
    int client_addr_len = sizeof(SOCKADDR_IN);
    std::cout << "等待客户端连接\n" << std::endl;
    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
    // 等待客户端发送数据
    char* buffer = (char*)malloc(RECV_BUFFER_SIZE);
    // 记录缓冲区当前数据的长度
    static int index = 0;
    while (true) {
        //int recv_len = recv(client_socket, buffer, sizeof(buffer) - 1, 0); // 这样存在数据丢失的问题，需要有记录缓存区数据长度
        int recv_len = recv(client_socket, buffer+index, RECV_BUFFER_SIZE-index, 0); //缓冲区总大小为RECV_BUFFER_SIZE，已经使用了index个字节（前index个位置已存放上一次接收的有效数据），剩余可用空间为RECV_BUFFER_SIZE - index
        index += recv_len;
        if (recv_len == SOCKET_ERROR) {
            std::cout << "服务端接收数据失败" << std::endl;
        }
        else if (recv_len > 0) {
            Packet* packet = ParsePacket(buffer, index);
            index = index - GetPacketLen(packet);
            memmove(buffer, buffer + GetPacketLen(packet), index);//把已经读取的数据删了
            printf("server recv packet->body:%s\r\n", packet->body);
            printf("server recv packet->header.magic:%x\r\n", packet->header.magic);
            printf("server recv packet->header.cmd:%d\r\n", packet->header.cmd);
            printf("server recv packet->header.body_len:%d\r\n", packet->header.body_len);
            HandleCommand(packet);

            if (packet->header.cmd == 1) {
                // 发送数据给客户端
                Packet* pck = PackPacket(packet->header.cmd, packet->body, packet->header.body_len);
                free(packet);
                send(client_socket, (char*)&pck->header.magic, GetPacketLen(pck), 0);
                printf("server send packet->body:%s\r\n", pck->body);
            }
        }
        Sleep(200);
    }
    // 关闭Socket，避免资源泄漏（先关客户端连接Socket，再关监听Socket）
    closesocket(client_socket);
    closesocket(server_socket);
    // 清除网络环境
    WSACleanup();
    return 0;
}

Packet* ParsePacket(char* buffer, int len){
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

int HandleCommand(Packet* packet) {
    switch (packet->header.cmd)
    {
    case CMD_SCREEN:
        HandleScreen(packet);
        break;
    case CMD_MOUSE:
        HandleMouse(packet);
        break;
    case CMD_KEYBOARD:
        HandleKeyBoard(packet);
        break;
    case CMD_TEST:
        HandleTest(packet);
        break;
    default:
        break;
    }
    return 0;
}
int HandleScreen(Packet* packet) {

    return 0;
}
int HandleMouse(Packet* packet) {

    return 0;
}
int HandleKeyBoard(Packet* packet) {

    return 0;
}
int HandleTest(Packet* packet) {

    return 0;
}
