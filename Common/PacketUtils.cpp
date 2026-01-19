#include "PacketUtils.h"
#include <cstring>  // 用于 memcpy

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
    return nullptr;
}