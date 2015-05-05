# Introduction #

如何从源码编译安装iptux？
Build and Install iptux

## SVN ##

**从SVN版安装：**

**Build from SVN codes:**

```
$ svn checkout http://iptux.googlecode.com/svn/trunk/ iptux-read-only

$ mkdir iptux-make

$ cd iptux-make

$ aclocal

$ autoconf

$ automake

$ ../iptux-read-only/configure

$ make

$ sudo make install
```

## Stable ##

**稳定版安装：**

**Build from stable codes:**

注意版本号：
Note for the version numbers:

```
$ wget http://iptux.googlecode.com/files/iptux-0.x.x.tar.gz

$ tar xzvf iptux-0.x.x.tar.gz

$ cd iptux-0.x.x


$ aclocal

$ autoconf

$ automake
/*如果没有这一句可能会提示没有makefile.in文件*/

$ ./configure

$ make

$ sudo make install
```