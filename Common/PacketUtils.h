#ifndef PACKET_UTILS_H
#define PACKET_UTILS_H

#include <cstring>  // 用于 strlen 函数
#include <cstdlib>  // 用于 malloc 和 free
#include <windef.h>

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

struct Mouse {
    int action; // 鼠标行为
    POINT ptXY; // 鼠标的坐标
};

// 获取数据包长度
int GetPacketLen(Packet* pck);

// 打包数据包
Packet* PackPacket(int cmd, char* buffer, int buffer_len);

// 解析数据包
Packet* ParsePacket(char* buffer, int len);

#endif // PACKET_UTILS_H