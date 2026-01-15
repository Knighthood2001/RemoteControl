# 远程控制开发

服务端流程符合 TCP 标准：创建监听 Socket → bind() 绑定地址端口 → listen() 开启监听 → accept() 阻塞等待客户端连接 → recv() 接收数据 → send() 回发数据。

客户端流程符合 TCP 标准：创建 Socket → connect() 连接服务端 → send() 发送数据 → recv() 接收服务端回显数据。