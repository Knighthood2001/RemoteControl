#include <iostream>
#include <Windows.h>
#include <cstring>  // 用于 strlen 函数

#pragma comment(lib, "ws2_32.lib")
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
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.0.80");

    // 连接服务器
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        std::cout << "连接服务器失败\n" << std::endl;
        return 0;
    }
    std::cout << "连接服务器成功\n" << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////
    char buffer[1024] = {0};
    char receive_buffer[1024] = { 0 };  // 初始化数组，避免随机值;
    int count = 0;
    while (true) {
        count++;
        snprintf(buffer, sizeof(buffer), "%d", count);  // 格式化为字符串，自动加\0结束符
        Packet* packet = (Packet*)malloc(sizeof(PacketHeader) + 10);
        // 定义包头信息
        packet->header.magic = 0x55AA77CC;
        packet->header.cmd = 2000;
        packet->header.body_len = 10;
        // 将数据拷贝到要发送的packet中
        memcpy(packet->body, buffer, 10);
        int send_len = send(client_socket, (char*) & packet->header.magic, GetPacketLen(packet), 0);  // +1 携带字符串结束符 '\0'
        if (send_len == SOCKET_ERROR) { 
            std::cout << "发送数据失败" << std::endl;
            return 0;
        }
        std::cout << "client send data: " << buffer << std::endl;
        //std::cout << "客户端发送数据成功，发送长度：" << send_len << std::endl;
        free(packet);
        //// 等待接收数据
        //int receive_len = recv(client_socket, receive_buffer, sizeof(receive_buffer) - 1, 0);  // 预留1个位置给 '\0'
        //if (receive_len == SOCKET_ERROR) {
        //    std::cout << "接收数据失败，错误码：" << WSAGetLastError() << std::endl;
        //}
        //else if (receive_len > 0) {
        //    receive_buffer[receive_len] = '\0';  // 添加字符串结束符，避免乱码
        //    std::cout << "client receive data: " << receive_buffer << std::endl;
        //}
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
}