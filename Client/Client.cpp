#include <iostream>
#include <Windows.h>
#include <cstring>  // 用于 strlen 函数
#include <atlimage.h> 
#include "PacketUtils.h"
#pragma comment(lib, "ws2_32.lib")
#define RECV_BUFFER_LEN 1024*1024*1
// 定义两个全局变量
SOCKET client_socket;
SOCKADDR_IN server_addr;
HWND g_hwnd = NULL;
CImage g_image;

SOCKET InitClientSocket(const char* serverIp, u_short serverPort);
int InitWindow(HINSTANCE hInstance, int nCmdShow);
DWORD WINAPI SendScreenCallBack(LPVOID lpThreadParameter);

LRESULT CALLBACK winProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg)
    {
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
        break;
    }
    return 0;
}
// 创建窗口的主程序
int WINAPI WinMain(
    HINSTANCE hInstance,  // 当前实例句柄
    HINSTANCE hPreventInstance, // 前一个实例句柄
    PSTR pCmdLine,  // 命令行参数
    int nCmdShow  // 窗口显示方式
){
    // 初始化窗口
    InitWindow(hInstance, nCmdShow);
    // 连接服务器
    SOCKET client_socket = InitClientSocket("192.168.0.80", 9999);
    // 连接服务器
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        std::cout << "连接服务器失败\n" << std::endl;
        closesocket(client_socket); // 失败时关闭socket
        WSACleanup(); // 清理WSA资源
    }
    std::cout << "连接服务器成功\n" << std::endl;
    // 创建一个线程
    unsigned long send_screen_thread_id = 0;
    HANDLE handle_send_screen = CreateThread(NULL,0, SendScreenCallBack, NULL, 0, &send_screen_thread_id);
    // 创建消息循环队列
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        // 翻译消息
        TranslateMessage(&msg);
        // 分发消息
        DispatchMessage(&msg);

    }

}


//int main(){
//    SOCKET client_socket = InitClientSocket("192.168.0.80", 9999);
//    if (client_socket == INVALID_SOCKET) {
//        std::cout << "客户端初始化失败，程序退出" << std::endl;
//        return 1; // 初始化失败，非0退出码更规范
//    }
//    //////////////////////////////////////////////////////////////////////////////////////
//    char buffer[1024] = {0};
//    char* recv_buffer = (char*)malloc(RECV_BUFFER_LEN);  
//    int count = 0;
//    while (true) {
//        count++;
//        //snprintf(buffer, sizeof(buffer), "%d", count);  
//        std::cout << "请输入要发送的数据：";
//        fgets(buffer, 1024, stdin);
//
//        Packet* packet = PackPacket(1, buffer, 10);
//        int send_len = send(client_socket, (char*) & packet->header.magic, GetPacketLen(packet), 0); 
//        if (send_len == SOCKET_ERROR) { 
//            std::cout << "发送数据失败" << std::endl;
//            return 0;
//        }
//        printf("client send data: %s\r\n", buffer);
//        //std::cout << "客户端发送数据成功，发送长度：" << send_len << std::endl;
//        free(packet);
//        // 等待接收数据
//        int recv_len = recv(client_socket, recv_buffer, RECV_BUFFER_LEN, 0); 
//        if (recv_len == SOCKET_ERROR) {
//            std::cout << "接收数据失败，错误码：" << WSAGetLastError() << std::endl;
//        }
//        else if (recv_len > 0) {
//            Packet* recv_pck = ParsePacket(recv_buffer, recv_len);
//            printf("client recv packet->body:%s\r\n", recv_pck->body);
//            printf("client recv packet->header.magic:%x\r\n", recv_pck->header.magic);
//            printf("client recv packet->header.cmd:%d\r\n", recv_pck->header.cmd);
//            printf("client recv packet->header.body_len:%d\r\n", recv_pck->header.body_len);
//            if (recv_pck->header.cmd == 1) {
//            // 服务器返回屏幕数据，这里进行解析
//
//            }
//            free(recv_pck);
//        }
//    }
//    // 关闭客户端socket，避免资源泄漏
//    closesocket(client_socket);
//    WSACleanup();
//    return 0;
//}
//

// 封装客户端初始化和连接服务器的函数
// 参数：serverIp - 服务器IP地址，serverPort - 服务器端口
// 返回值：成功返回客户端socket句柄，失败返回INVALID_SOCKET
SOCKET InitClientSocket(const char* serverIp, u_short serverPort) {
    // 初始化网络环境
    WSADATA wsadata;
    int wsa_ret = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (wsa_ret != 0) {
        std::cout << "初始化Winsock失败，错误码：" << wsa_ret << std::endl;
        return INVALID_SOCKET;
    }
    // 创建socket
    /*SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);*/
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cout << "创建客户端socket失败\n" << std::endl;
        WSACleanup(); // 失败时清理WSA资源
        return INVALID_SOCKET;
    }
    /*SOCKADDR_IN server_addr = {0};*/
    server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ntohs(serverPort);
    server_addr.sin_addr.S_un.S_addr = inet_addr(serverIp);

    return client_socket;
}
// 创建窗口
int InitWindow(HINSTANCE hInstance, int nCmdShow) {
    // 1 注册一个窗口类
    WNDCLASS ws = { 0 };
    LPCSTR CLASS_NAME = "MainWindow";
    ws.lpfnWndProc = winProc;
    ws.hInstance = hInstance;
    ws.lpszClassName = CLASS_NAME;
    ws.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    ws.hCursor = LoadCursor(NULL, IDC_ARROW);
    ws.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    ws.style = CS_HREDRAW | CS_VREDRAW;
    if (!RegisterClass(&ws)) {
        MessageBox(NULL, "窗口注册失败", "错误", MB_OK | MB_ICONERROR);
        return 0;
    }
    // 2 创建窗口
    //CreateWindowA(lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
    g_hwnd = CreateWindow(
        CLASS_NAME,
        "远程控制",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        600, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    if (g_hwnd == NULL) {
        MessageBox(NULL, "窗口创建失败", "错误", MB_OK | MB_ICONERROR);
        return 0;
    }
    // 3显示窗口
    ShowWindow(g_hwnd, nCmdShow);
    // 4 更新窗口
    UpdateWindow(g_hwnd);
    return 0;
}
//DWORD(WINAPI* PTHREAD_START_ROUTINE)(
//LPVOID lpThreadParameter
//)
// 返回值 调用约定 函数名(参数列表){函数体}
DWORD WINAPI SendScreenCallBack(LPVOID lpThreadParameter) {
    char* recv_buffer = (char*)malloc(RECV_BUFFER_LEN);
    while (true) {
        Packet* pack = PackPacket(CMD_SCREEN, NULL, 0);
        // 发送获取屏幕数据
        send(client_socket, (char*)&pack->header.magic, GetPacketLen(pack), 0);
        free(pack);
        int len = recv(client_socket, recv_buffer, RECV_BUFFER_LEN, 0);
        if (len > 0) {
            Packet* pack = ParsePacket(recv_buffer, len);
            if (pack != NULL) {
                // 拿到图片数据，进行 绘制
                HGLOBAL hMen = GlobalAlloc(GMEM_MOVEABLE, 0);
                if (hMen == NULL) {
                    continue;
                }
                IStream* pStream = NULL;
                HRESULT ret = CreateStreamOnHGlobal(hMen, true, &pStream);
                if (ret == S_OK) {
                    ULONG lenght = 0;
                    pStream->Write(pack->body, pack->header.body_len, &lenght);
                    free(pack);
                    // 将指针移到末尾
                    LARGE_INTEGER lg = { 0 };
                    pStream->Seek(lg, STREAM_SEEK_SET, NULL);
                    // 数据移动到缓存中
                    
                    g_image.Load(pStream);
                    // 通知UI线程重绘
                    InvalidateRect(g_hwnd, NULL, FALSE);
                    UpdateWindow(g_hwnd);

                }
            }
        }
    }
}