# myUnixFileSys
类似Unix V6++的二级文件系统

### 编写思路

1.先把所有类写出来

2.发现BufferManager是基础，实现GetBlk、Bread、Bwrite

3.然后从文件的打开开始写起，实现open

5.12：open->namei->alloc