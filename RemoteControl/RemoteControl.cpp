#include <iostream>
#include <Windows.h>
#include <cstring>  // 用于 strlen 函数
#include <atlimage.h>
#include "PacketUtils.h"
#pragma comment(lib, "ws2_32.lib")
#define RECV_BUFFER_SIZE 1024*1024*1
SOCKET client_socket;

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
    client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
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
            std::cout << "服务端接收数据成功，开始解析数据" << std::endl;
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
                send(client_socket, (char*)&pck->header.magic, GetPacketLen(pck), 0);
                printf("server send packet->body:%s\r\n", pck->body);
            }
            free(packet);
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


int HandleCommand(Packet* packet) {
    int ret = 0;
    switch (packet->header.cmd)
    {
    case CMD_SCREEN:
        ret = HandleScreen(packet);
        break;
    case CMD_MOUSE:
        ret = HandleMouse(packet);
        break;
    case CMD_KEYBOARD:
        ret = HandleKeyBoard(packet);
        break;
    case CMD_TEST:
        ret = HandleTest(packet);
        break;
    default:
        break;
    }
    return ret;
}
int HandleScreen(Packet* packet) {
    CImage image;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    // 1. 获取主显示器的分辨率（像素宽、像素高）
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);    // SM_CXSCREEN：获取主屏幕的水平分辨率（宽度）
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);   // SM_CYSCREEN：获取主屏幕的垂直分辨率（高度）
    std::cout << "width: " << screenWidth << ", height: " << screenHeight << std::endl;
    // 2. 获取主显示器的色深（每像素位数，如32位真彩色）
    // 获取桌面设备上下文（DC），对应主显示器
    HDC hDesktopDC = GetDC(NULL);
    // 获取每像素的颜色位数
    int colorDepth = GetDeviceCaps(hDesktopDC, BITSPIXEL);
    image.Create(screenWidth, screenHeight, colorDepth);
    BitBlt(image.GetDC(), 0, 0, screenWidth, screenHeight, hDesktopDC, 0, 0, SRCCOPY);
    // 释放设备上下文，避免资源泄漏
    ReleaseDC(NULL, hDesktopDC);
    //image.Save(L"test.png", ::Gdiplus::ImageFormatPNG);  //如果确定项目一直用 Unicode 字符集，可以直接用 L 前缀声明宽字符
    //image.ReleaseDC();
    HGLOBAL hMen = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMen == NULL) {
        return -1; // 分配失败
    }
    // 创建一个内存流
    IStream* pStream = NULL;
    HRESULT ret = CreateStreamOnHGlobal(hMen, true, &pStream);
    if (ret == S_OK) {
        image.Save(pStream, ::Gdiplus::ImageFormatPNG);// 将文件保存到内存流中
        // 将流指针放到开头
        LARGE_INTEGER lg = {0}; 
        pStream->Seek(lg, STREAM_SEEK_SET, NULL);
        // 获取这个流指针
        char* pdata = (char*)GlobalLock(hMen);
        int len = GlobalSize(hMen);// 获取流长度
        // 发送数据
        Packet* packet = PackPacket(CMD_SCREEN, pdata, len);
        send(client_socket, (char*)&packet->header.magic,sizeof(PacketHeader)+len, 0);
        free(packet);
        //解锁内存
        GlobalUnlock(hMen);
    }
    pStream->Release();
    GlobalFree(hMen);
    image.ReleaseDC();
    return 0;
}
int HandleMouse(Packet* packet) {
    // 从结构体中提取出鼠标信息
    Mouse mouse;
    memcpy(&mouse.action, packet->body, packet->header.body_len);
    std::cout << "x = " << mouse.ptXY.x << ", y = " << mouse.ptXY.y << std::endl;
    // 设置鼠标位置（模拟鼠标事件）
    SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
    /*
    enum ENUM_MOUSE {
    MOUSE_MOVE = 1,  // 移动
    MOUSE_LDOWN =2,  //左键按下
    MOUSE_LUP = 3,
    MOUSE_RDOWN = 4,  //右键按下
    MOUSE_RUP = 5,
    MOUSE_MDOWN =6,
    MOUSE_MUP = 7,
    MOUSE_LDLINK = 8, // 鼠标左键双击
    MOUSE_RDLINK = 9,  // 右键双击
    MOUSE_MDLINK = 10,
    };
    */
    switch (mouse.action)
    {
    case MOUSE_MOVE:
        SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        break;
    case MOUSE_LDOWN:
        //#define MOUSEEVENTF_MOVE        0x0001 /* mouse move */
        //#define MOUSEEVENTF_LEFTDOWN    0x0002 /* left button down */
        //#define MOUSEEVENTF_LEFTUP      0x0004 /* left button up */
        //#define MOUSEEVENTF_RIGHTDOWN   0x0008 /* right button down */
        //#define MOUSEEVENTF_RIGHTUP     0x0010 /* right button up */
        //#define MOUSEEVENTF_MIDDLEDOWN  0x0020 /* middle button down */
        //#define MOUSEEVENTF_MIDDLEUP    0x0040 /* middle button up */
        //#define MOUSEEVENTF_XDOWN       0x0080 /* x button down */
        //#define MOUSEEVENTF_XUP         0x0100 /* x button down */
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_LUP:
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_RDOWN:
        mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_RUP:
        mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_MDOWN:
        mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_MUP:
        mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_LCLICK:
        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_RCLICK:
        mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_MCLICK:
        mouse_event(MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_LDCLICK:
        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_RDCLICK:
        mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    case MOUSE_MDCLICK:
        mouse_event(MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        mouse_event(MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        break;
    default:
        break;
    }
    return 0;
}
int HandleKeyBoard(Packet* packet) {

    return 0;
}
int HandleTest(Packet* packet) {

    return 0;
}
