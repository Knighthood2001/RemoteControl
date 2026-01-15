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
    // 创建socket
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cout << "创建客户端socket失败\n" << std::endl;
        return 0;
    }
    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ntohs(9999);
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.212.80");

    // 连接服务器
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        std::cout << "连接服务器失败\n" << std::endl;
        return 0;
    }
    std::cout << "连接服务器成功\n" << std::endl;
    char buffer[1024] = "hello world!";
    int send_len = send(client_socket, buffer, strlen(buffer)+1, 0);  // +1 携带字符串结束符 '\0'
    if (send_len == SOCKET_ERROR) {
        std::cout << "发送数据失败" << std::endl;
        return 0;
    }
    std::cout << "客户端发送数据成功，发送长度：" << send_len << std::endl;
    // 等待接收数据
    char receive_buffer[1024] = { 0 };  // 初始化数组，避免随机值;
    int receive_len = recv(client_socket, receive_buffer, sizeof(receive_buffer) - 1, 0);  // 预留1个位置给 '\0'
    if (receive_len == SOCKET_ERROR) {
        std::cout << "接收数据失败，错误码：" << WSAGetLastError() << std::endl;
    }else if (receive_len > 0) {
        receive_buffer[receive_len] = '\0';  // 添加字符串结束符，避免乱码
        std::cout << "client receive data: " << receive_buffer << std::endl;
    }
    WSACleanup();
    return 0;
}