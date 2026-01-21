#ifndef PACKET_UTILS_H
#define PACKET_UTILS_H

#include <cstring>  // 用于 strlen 函数
#include <cstdlib>  // 用于 malloc 和 free

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

// 获取数据包长度
int GetPacketLen(Packet* pck);

// 打包数据包
Packet* PackPacket(int cmd, char* buffer, int buffer_len);

// 解析数据包
Packet* ParsePacket(char* buffer, int len);

#endif // PACKET_UTILS_H