# lrvpn
一个linux下基于tun实现的简单转发程序, 使用epoll实现io多路复用  
server端使用libtins库来处理IP数据包  
支持多客户端连接

### vpnClient
    cmake .
    make
    ./vpnClient serverIP serverPort networkName
    参数为服务端IP Port 本机网卡名
### vpnServer
    cmake .
    make 
    ./vpnServer port
    参数为端口