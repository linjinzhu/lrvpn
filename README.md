# lrvpn
一个linux下基于tun实现的简单转发程序, server端使用libtins库来处理IP数据包

## TODO
   1.增加端口映射

### vpnClient
    cmake .
    make
    ./vpnClient serverIP serverPort networkName
    参数为服务端IP Port 本机网卡名
### vpnServer
    cmake .
    make 
    ./vpnServer