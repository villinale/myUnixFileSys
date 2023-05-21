# myUnixFileSys
类似Unix V6++的二级文件系统

### 编写思路

1.先把所有类写出来

2.发现BufferManager是基础，实现GetBlk、Bread、Bwrite

3.尝试实现初始化文件系统的操作，根据需要的内容进行一步一步的填充