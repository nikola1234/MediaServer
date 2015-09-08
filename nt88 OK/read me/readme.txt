
NT88 for Linux 说明

Linux版本:
	NT88 for Linux 提供静态库和动态库两种版本供用户选择， 在Linux Version 2.6.32 内核编译生成

文件说明：
	libnt88lib.a : linux 下静态库文件
	NT88Api.h: 函数接口头文件

	90-nt88.rules: 设置应用程序允许访问nt88设备， 请将此文件拷贝到 /etc/udev/rules.d 目录下