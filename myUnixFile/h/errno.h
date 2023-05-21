#pragma once

// ��ʵ��Щ��������C++�б�����Ѿ���������
#define EPERM 1         // Operation not permitted ����������
#define ENOENT 2        // No such file or directory û���������ļ���Ŀ¼
#define EBADF 9         // Bad file number �����ļ�������
#define EFAULT 14       // Bad address ����ĵ�ַ
#define EEXIST 17       // File exists �ļ�����
#define ENOTDIR 20      // Not a directory ����һ��Ŀ¼
#define EINVAL 22       // Invalid argument ��Ч�Ĳ���
#define ENFILE 23       // File table overflow ��̫����ļ�ϵͳ
#define ENOSPC 28       // No space left on device �豸��û�пռ�
#define ENAMETOOLONG 36 // Filename too long �ļ���̫��
#define EOVERFLOW 75    // Value too large for defined data type ֵ̫��,���ڶ�����������
#define EUSERS 87       // Too many users �û�̫��

/*
#define	ESRCH	3	//No such process û�������Ĺ���
#define	EINTR	4	//Interrupted system call ϵͳ���ñ��ж�
#define	EIO		5	//I/O error I/O����
#define	ENXIO	6	//No such device or address û���������豸���ַ
#define	E2BIG	7	//Arg list too long �����б�̫��
#define	ENOEXEC	8	//Exec format error ִ�и�ʽ����
#define	ECHILD	10	//No child processes û���ӽ���
#define	EAGAIN	11	//Try again ��Դ��ʱ������
#define	ENOMEM	12	//Out of memory �ڴ����
#define	EACCES	13	//Permission denied �ܾ����
#define	ENOTBLK	15	//Block device required ���豸����
#define	EBUSY	16	//Device or resource busy �豸����Դæ
#define	EXDEV	18	//Cross-device link ��Ч�Ľ�������
#define	ENODEV	19	//No such device �豸������
#define	EISDIR	21	//Is a directory ��һ��Ŀ¼
#define	EMFILE	24	//Too many open files �򿪵��ļ�����
#define	ENOTTY	25	//Not a tty device ����tty�豸
#define	ETXTBSY	26	//Text file busy �ı��ļ�æ
#define	EFBIG	27	//File too large �ļ�̫��
#define	ESPIPE	29	//Illegal seek �Ƿ���λ
#define	EROFS	30	//Read-only file system ֻ���ļ�ϵͳ
#define	EMLINK	31	//Too many links ̫�������
#define	EPIPE	32	//Broken pipe �ܵ�����
#define	EDOM	33	//Math argument out of domain ��ֵ���������Χ
#define	ERANGE	34	//Math result not representable ��ֵ������ߴ�����
#define	EDEADLK	35	//Resource deadlock would occur ��Դ��������
#define	ENOLCK	37	//No record locks available û�п�����
#define	ENOSYS	38	//Function not implemented ����û��ʵ��
#define	ENOTEMPTY	39	//Directory not empty Ŀ¼����
#define	ELOOP	40	//Too many symbolic links encountered �������Ӳ��̫��
#define	EWOULDBLOCK	41	//Same as EAGAIN ��EAGAINһ��
#define	ENOMSG	42	//No message of desired type û���������͵���Ϣ
#define	EIDRM	43	//Identifier removed ��ʶ��ɾ��
#define	ECHRNG	44	//Channel number out of range Ƶ����Ŀ������Χ
#define	EL2NSYNC	45	//Level 2 not synchronized 2����ͬ��
#define	EL3HLT	46	//Level 3 halted 3���ж�
#define	EL3RST	47	//Level 3 reset 3����λ
#define	ELNRNG	48	//Link number out of range ������������Χ
#define	EUNATCH	49	//Protocol driver not attached Э����������û������
#define	ENOCSI	50	//No CSI structure available û�п���CSI�ṹ
#define	EL2HLT	51	//Level 2 halted 2���ж�
#define	EBADE	52	//Invalid exchange ��Ч�Ľ���
#define	EBADR	53	//Invalid request descriptor ������������Ч
#define	EXFULL	54	//Exchange full ����ȫ
#define	ENOANO	55	//No anode û������
#define	EBADRQC	56	//Invalid request code ��Ч���������
#define	EBADSLT	57	//Invalid slot ��Ч�Ĳ�
#define	EDEADLOCK	58	//Same as EDEADLK ��EDEADLKһ��
#define	EBFONT	59	//Bad font file format ����������ļ���ʽ
#define	ENOSTR	60	//Device not a stream �豸�����ַ���
#define	ENODATA	61	//No data available �޿�������
#define	ETIME	62	//Timer expired ��ʱ������
#define	ENOSR	63	//Out of streams resources ����Դ���
#define	ENONET	64	//Machine is not on the network ����������
#define	ENOPKG	65	//Package not installed û�а�װ�����
#define	EREMOTE	66	//Object is remote ������Զ�̵�
#define	ENOLINK	67	//Link has been severed ��ϵ���ж�
#define	EADV	68	//Advertise error ���Ĵ���
#define	ESRMNT	69	//Srmount error srmount����
#define	ECOMM	70	//Communication error on send ����ʱ��ͨѶ����
#define	EPROTO	71	//Protocol error Э�����
#define	EMULTIHOP	72	//Multihop attempted ��������
#define	EDOTDOT	73	//RFS specific error RFS�ض��Ĵ���
#define	EBADMSG	74	//Not a data message ��������Ϣ
#define	ENOTUNIQ	76	//Name not unique on network ������Ψһ������
#define	EBADFD	77	//File descriptor in bad state �ļ��������ڻ�״̬
#define	EREMCHG	78	//Remote address changed Զ�̵�ַ�ı���
#define	ELIBACC	79	//Cannot access a needed shared library �޷����ʱ�Ҫ�Ĺ����
#define	ELIBBAD	80	//Accessing a corrupted shared library �����𻵵Ĺ����
#define	ELIBSCN	81	//A .lib section in an .out is corrupted ���. out��
#define	ELIBMAX	82	//Linking in too many shared libraries ��ͼ����̫��Ĺ����
#define	ELIBEXEC	83	//Cannot exec a shared library directly ����ֱ��ִ��һ�������
#define	EILSEQ	84	//Illegal byte sequence ��Ч�Ļ������Ķ��ֽڻ���ַ�
#define	ERESTART	85	//Interrupted system call should be restarted Ӧ�����������жϵ�ϵͳ����
#define	ESTRPIPE	86	//Streams pipe error ���ܴ���
#define	ENOTSOCK	88	//Socket operation on non-socket �׽��ֲ����ڷ��׽�����
#define	EDESTADDRREQ	89	//Destination address required ��ҪĿ���ַ
#define	EMSGSIZE	90	//Message too long ��Ϣ̫��
#define	EPROTOTYPE	91	//Protocol wrong type for socket socketЭ�����ʹ���
#define	ENOPROTOOPT	92	//Protocol not available Э�鲻����
#define	EPROTONOSUPPORT	93	//Protocol not supported ��֧�ֵ�Э��
#define	ESOCKTNOSUPPORT	94	//Socket type not supported �׽������Ͳ���֧��
#define	EOPNOTSUPP	95	//Operation not supported on transport ��֧�ֵĲ���
#define	EPFNOSUPPORT	96	//Protocol family not supported ��֧�ֵ�Э����
#define	EAFNOSUPPORT	97	//Address family not supported by protocol Э�鲻֧�ֵĵ�ַ
#define	EADDRINUSE	98	//Address already in use ��ַ����ʹ��
#define	EADDRNOTAVAIL	99	//Cannot assign requested address �޷���������ĵ�ַ
#define	ENETDOWN	100	//Network is down ����̱��
#define	ENETUNREACH	101	//Network is unreachable ���粻�ɴ�
#define	ENETRESET	102	//Network dropped �������Ӷ�ʧ
#define	ECONNABORTED	103	//Software caused connection ������������ж�
#define	ECONNRESET	104	//Connection reset by ���ӱ�����
#define	ENOBUFS	105	//No buffer space available û�п��õĻ���ռ�
#define	EISCONN	106	//Transport endpoint ����˵��Ѿ�����
#define	ENOTCONN	107	//Transport endpoint �����յ�û������
#define	ESHUTDOWN	108	//Cannot send after transport ������޷�����
#define	ETOOMANYREFS	109	//Too many references ̫��Ĳο�
#define	ETIMEDOUT	110	//Connection timed ���ӳ�ʱ
#define	ECONNREFUSED	111	//Connection refused �ܾ�����
#define	EHOSTDOWN	112	//Host is down �����ѹر�
#define	EHOSTUNREACH	113	//No route to host û��������·��
#define	EALREADY	114	//Operation already ������
#define	EINPROGRESS	115	//Operation now in ��������
#define	ESTALE	116	//Stale NFS file handle �¾ɵ�NFS�ļ����
#define	EUCLEAN	117	//Structure needs cleaning �ṹ��Ҫ��ϴ
#define	ENOTNAM	118	//Not a XENIX-named ����XENIX������
#define	ENAVAIL	119	//No XENIX semaphores û��XENIX�ź���
#define	EISNAM	120	//Is a named type file ��һ���������ļ�����
#define	EREMOTEIO	121	//Remote I/O error Զ������/�������
#define	EDQUOT	122	//Quota exceeded �����������
#define	ENOMEDIUM	123	//No medium found û�д��̱�����
#define	EMEDIUMTYPE	124	//Wrong medium type �����ý������
#define	ECANCELED	125	//Operation Canceled ȡ������
#define	ENOKEY	126	//Required key not available �����������
#define	EKEYEXPIRED	127	//Key has expired �ؼ��ѹ���
#define	EKEYREVOKED	128	//Key has been revoked �ؼ�������
#define	EKEYREJECTED	129	//Key was rejected by service �ؼ����ܾ�����
#define	EOWNERDEAD	130	//Owner died ����������
#define	ENOTRECOVERABLE	131	//State not recoverable ״̬���ɻָ�
#define	ERFKILL	132	//Operation not possible due to RF-kill ����RF-kill���޷�����
*/