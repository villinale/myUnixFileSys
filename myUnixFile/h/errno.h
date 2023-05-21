#pragma once

// 其实这些错误码在C++中本身就已经被定义了
#define EPERM 1         // Operation not permitted 操作不允许
#define ENOENT 2        // No such file or directory 没有这样的文件或目录
#define EBADF 9         // Bad file number 坏的文件描述符
#define EFAULT 14       // Bad address 错误的地址
#define EEXIST 17       // File exists 文件存在
#define ENOTDIR 20      // Not a directory 不是一个目录
#define EINVAL 22       // Invalid argument 无效的参数
#define ENFILE 23       // File table overflow 打开太多的文件系统
#define ENOSPC 28       // No space left on device 设备上没有空间
#define ENAMETOOLONG 36 // Filename too long 文件名太长
#define EOVERFLOW 75    // Value too large for defined data type 值太大,对于定义数据类型
#define EUSERS 87       // Too many users 用户太多

/*
#define	ESRCH	3	//No such process 没有这样的过程
#define	EINTR	4	//Interrupted system call 系统调用被中断
#define	EIO		5	//I/O error I/O错误
#define	ENXIO	6	//No such device or address 没有这样的设备或地址
#define	E2BIG	7	//Arg list too long 参数列表太长
#define	ENOEXEC	8	//Exec format error 执行格式错误
#define	ECHILD	10	//No child processes 没有子进程
#define	EAGAIN	11	//Try again 资源暂时不可用
#define	ENOMEM	12	//Out of memory 内存溢出
#define	EACCES	13	//Permission denied 拒绝许可
#define	ENOTBLK	15	//Block device required 块设备请求
#define	EBUSY	16	//Device or resource busy 设备或资源忙
#define	EXDEV	18	//Cross-device link 无效的交叉链接
#define	ENODEV	19	//No such device 设备不存在
#define	EISDIR	21	//Is a directory 是一个目录
#define	EMFILE	24	//Too many open files 打开的文件过多
#define	ENOTTY	25	//Not a tty device 不是tty设备
#define	ETXTBSY	26	//Text file busy 文本文件忙
#define	EFBIG	27	//File too large 文件太大
#define	ESPIPE	29	//Illegal seek 非法移位
#define	EROFS	30	//Read-only file system 只读文件系统
#define	EMLINK	31	//Too many links 太多的链接
#define	EPIPE	32	//Broken pipe 管道破裂
#define	EDOM	33	//Math argument out of domain 数值结果超出范围
#define	ERANGE	34	//Math result not representable 数值结果不具代表性
#define	EDEADLK	35	//Resource deadlock would occur 资源死锁错误
#define	ENOLCK	37	//No record locks available 没有可用锁
#define	ENOSYS	38	//Function not implemented 功能没有实现
#define	ENOTEMPTY	39	//Directory not empty 目录不空
#define	ELOOP	40	//Too many symbolic links encountered 符号链接层次太多
#define	EWOULDBLOCK	41	//Same as EAGAIN 和EAGAIN一样
#define	ENOMSG	42	//No message of desired type 没有期望类型的消息
#define	EIDRM	43	//Identifier removed 标识符删除
#define	ECHRNG	44	//Channel number out of range 频道数目超出范围
#define	EL2NSYNC	45	//Level 2 not synchronized 2级不同步
#define	EL3HLT	46	//Level 3 halted 3级中断
#define	EL3RST	47	//Level 3 reset 3级复位
#define	ELNRNG	48	//Link number out of range 链接数超出范围
#define	EUNATCH	49	//Protocol driver not attached 协议驱动程序没有连接
#define	ENOCSI	50	//No CSI structure available 没有可用CSI结构
#define	EL2HLT	51	//Level 2 halted 2级中断
#define	EBADE	52	//Invalid exchange 无效的交换
#define	EBADR	53	//Invalid request descriptor 请求描述符无效
#define	EXFULL	54	//Exchange full 交换全
#define	ENOANO	55	//No anode 没有阳极
#define	EBADRQC	56	//Invalid request code 无效的请求代码
#define	EBADSLT	57	//Invalid slot 无效的槽
#define	EDEADLOCK	58	//Same as EDEADLK 和EDEADLK一样
#define	EBFONT	59	//Bad font file format 错误的字体文件格式
#define	ENOSTR	60	//Device not a stream 设备不是字符流
#define	ENODATA	61	//No data available 无可用数据
#define	ETIME	62	//Timer expired 计时器过期
#define	ENOSR	63	//Out of streams resources 流资源溢出
#define	ENONET	64	//Machine is not on the network 机器不上网
#define	ENOPKG	65	//Package not installed 没有安装软件包
#define	EREMOTE	66	//Object is remote 对象是远程的
#define	ENOLINK	67	//Link has been severed 联系被切断
#define	EADV	68	//Advertise error 广告的错误
#define	ESRMNT	69	//Srmount error srmount错误
#define	ECOMM	70	//Communication error on send 发送时的通讯错误
#define	EPROTO	71	//Protocol error 协议错误
#define	EMULTIHOP	72	//Multihop attempted 多跳尝试
#define	EDOTDOT	73	//RFS specific error RFS特定的错误
#define	EBADMSG	74	//Not a data message 非数据消息
#define	ENOTUNIQ	76	//Name not unique on network 名不是唯一的网络
#define	EBADFD	77	//File descriptor in bad state 文件描述符在坏状态
#define	EREMCHG	78	//Remote address changed 远程地址改变了
#define	ELIBACC	79	//Cannot access a needed shared library 无法访问必要的共享库
#define	ELIBBAD	80	//Accessing a corrupted shared library 访问损坏的共享库
#define	ELIBSCN	81	//A .lib section in an .out is corrupted 库段. out损坏
#define	ELIBMAX	82	//Linking in too many shared libraries 试图链接太多的共享库
#define	ELIBEXEC	83	//Cannot exec a shared library directly 不能直接执行一个共享库
#define	EILSEQ	84	//Illegal byte sequence 无效的或不完整的多字节或宽字符
#define	ERESTART	85	//Interrupted system call should be restarted 应该重新启动中断的系统调用
#define	ESTRPIPE	86	//Streams pipe error 流管错误
#define	ENOTSOCK	88	//Socket operation on non-socket 套接字操作在非套接字上
#define	EDESTADDRREQ	89	//Destination address required 需要目标地址
#define	EMSGSIZE	90	//Message too long 消息太长
#define	EPROTOTYPE	91	//Protocol wrong type for socket socket协议类型错误
#define	ENOPROTOOPT	92	//Protocol not available 协议不可用
#define	EPROTONOSUPPORT	93	//Protocol not supported 不支持的协议
#define	ESOCKTNOSUPPORT	94	//Socket type not supported 套接字类型不受支持
#define	EOPNOTSUPP	95	//Operation not supported on transport 不支持的操作
#define	EPFNOSUPPORT	96	//Protocol family not supported 不支持的协议族
#define	EAFNOSUPPORT	97	//Address family not supported by protocol 协议不支持的地址
#define	EADDRINUSE	98	//Address already in use 地址已在使用
#define	EADDRNOTAVAIL	99	//Cannot assign requested address 无法分配请求的地址
#define	ENETDOWN	100	//Network is down 网络瘫痪
#define	ENETUNREACH	101	//Network is unreachable 网络不可达
#define	ENETRESET	102	//Network dropped 网络连接丢失
#define	ECONNABORTED	103	//Software caused connection 软件导致连接中断
#define	ECONNRESET	104	//Connection reset by 连接被重置
#define	ENOBUFS	105	//No buffer space available 没有可用的缓冲空间
#define	EISCONN	106	//Transport endpoint 传输端点已经连接
#define	ENOTCONN	107	//Transport endpoint 传输终点没有连接
#define	ESHUTDOWN	108	//Cannot send after transport 传输后无法发送
#define	ETOOMANYREFS	109	//Too many references 太多的参考
#define	ETIMEDOUT	110	//Connection timed 连接超时
#define	ECONNREFUSED	111	//Connection refused 拒绝连接
#define	EHOSTDOWN	112	//Host is down 主机已关闭
#define	EHOSTUNREACH	113	//No route to host 没有主机的路由
#define	EALREADY	114	//Operation already 已运行
#define	EINPROGRESS	115	//Operation now in 正在运行
#define	ESTALE	116	//Stale NFS file handle 陈旧的NFS文件句柄
#define	EUCLEAN	117	//Structure needs cleaning 结构需要清洗
#define	ENOTNAM	118	//Not a XENIX-named 不是XENIX命名的
#define	ENAVAIL	119	//No XENIX semaphores 没有XENIX信号量
#define	EISNAM	120	//Is a named type file 是一个命名的文件类型
#define	EREMOTEIO	121	//Remote I/O error 远程输入/输出错误
#define	EDQUOT	122	//Quota exceeded 超出磁盘配额
#define	ENOMEDIUM	123	//No medium found 没有磁盘被发现
#define	EMEDIUMTYPE	124	//Wrong medium type 错误的媒体类型
#define	ECANCELED	125	//Operation Canceled 取消操作
#define	ENOKEY	126	//Required key not available 所需键不可用
#define	EKEYEXPIRED	127	//Key has expired 关键已过期
#define	EKEYREVOKED	128	//Key has been revoked 关键被撤销
#define	EKEYREJECTED	129	//Key was rejected by service 关键被拒绝服务
#define	EOWNERDEAD	130	//Owner died 所有者死亡
#define	ENOTRECOVERABLE	131	//State not recoverable 状态不可恢复
#define	ERFKILL	132	//Operation not possible due to RF-kill 由于RF-kill而无法操作
*/