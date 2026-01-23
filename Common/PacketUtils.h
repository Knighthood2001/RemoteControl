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
    MOUSE_MOVE = 1,      // 鼠标移动
    MOUSE_LDOWN = 2,     // 左键按下
    MOUSE_LUP = 3,       // 左键释放
    MOUSE_RDOWN = 4,     // 右键按下
    MOUSE_RUP = 5,       // 右键释放
    MOUSE_MDOWN = 6,     // 中键按下
    MOUSE_MUP = 7,       // 中键释放
    MOUSE_LCLICK = 8,    // 左键单击
    MOUSE_RCLICK = 9,    // 右键单击
    MOUSE_MCLICK  = 10,  // 中键单击
    MOUSE_LDCLICK = 11,  // 左键双击
    MOUSE_RDCLICK = 12,  // 右键双击
    MOUSE_MDCLICK = 13,  // 中键双击
};

struct Mouse {
    int action; // 鼠标行为
    POINT ptXY; // 鼠标的坐标
};

struct KeyBoard {
    int virtual_code; // 虚拟码0x41等
    int key_status;   // 按下/释放（0/1）
};

// 获取数据包长度
int GetPacketLen(Packet* pck);

// 打包数据包
Packet* PackPacket(int cmd, char* buffer, int buffer_len);

// 解析数据包
Packet* ParsePacket(char* buffer, int len);

#endif // PACKET_UTILS_H