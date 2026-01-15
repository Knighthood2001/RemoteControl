#include <iostream>
#include <Windows.h>
#include <cstring>  // 用于 strlen 函数

#pragma comment(lib, "ws2_32.lib")
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
    char buffer[1024] = { 0 };  // 初始化数组，避免随机值;
    while (true) {
        int recv_len = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (recv_len == SOCKET_ERROR) {
            std::cout << "客户端接收数据失败" << std::endl;
        }
        else if (recv_len > 0) {
            buffer[recv_len] = '\0';  // 添加字符串结束符，避免乱码
            std::cout << "server recv data: " << buffer << std::endl;

            // 发送数据给客户端
            int send_len = send(client_socket, buffer, strlen(buffer) + 1, 0);
            if (send_len == SOCKET_ERROR) {
                std::cout << "服务端回发数据失败" << std::endl;
            }
            std::cout << "send recv data: " << buffer << std::endl;
        }
    }
    // 关闭Socket，避免资源泄漏（先关客户端连接Socket，再关监听Socket）
    closesocket(client_socket);
    closesocket(server_socket);
    // 清除网络环境
    WSACleanup();
    return 0;
}