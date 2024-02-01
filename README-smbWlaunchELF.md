[![CI](https://github.com/ps2homebrew/wLaunchELF/workflows/CI/badge.svg)](https://github.com/ps2homebrew/wLaunchELF/actions?query=workflow%3ACI)

# smbwLaunchELF
This is a special version of wLaunchELF that has support for SMB2/3 and NFS.
It can connect to Windows/Samba/Azure file shares and supports up to
version 3.1.1 of SMB.
It can connect to NFSv3 and v4 servers.
The main purpose for this version is just to copy files to/from SMB2/3 and/or NFS
file servers.

# How to build?
To build this version you need to install LIBSMB2 and LIBNFS for PS2 EE.
See the readme for [libsmb2](https://github.com/sahlberg/libsmb2/blob/master/README) and [libnfs](https://github.com/sahlberg/libnfs) for instructions on how to build and install
them in ps2dev enviroment.

IRX Support:
This version contains a support for loading smb2man.irx module, If you want to build with smb2man.irx support,
you should go to a modified version of [ps2sdk](https://github.com/Wolf3s/ps2sdk/tree/smb) which contains the smb2man´s module.
Then hit `make clean install` plus enable on [smbwLaunchELF´s Makefile](https://github.com/Wolf3s/wLaunchELF/blob/SMB2/Makefile) the following option:
```
SMB2_IRX ?= 1
```
and build it with `make all`.

SMB2
====
You can specify SMB2 servers and shares in either of the files:
  mc0:/SYS-CONF/SMB2.CNF or
  mc1:/SYS-CONF/SMB2.CNF or
  mass:/SYS-CONF/SMB2.CNF

The format of this file is :
```
NAME = Home-NAS

USERNAME = user

PASSWORD = password

URL = smb://192.168.0.1/Share/
```
For each such entry an new subdirectory will appear under the smb2: device at
the root in the file manager.
It can copy files to/from smb2 shares and the ps2.

Even use SMB3.1.1 encryption to Azure cloud shares. Wooohooo

NFS
===
You can specify NFS servers and shares in either of the files:
  mc0:/SYS-CONF/NFS.CNF or
  mc1:/SYS-CONF/NFS.CNF or
  mass:/SYS-CONF/NFS.CNF

The format of this file is :
```
NAME = Home-NFS

URL = nfs://192.168.0.1/path/export
```

For each such entry an new subdirectory will appear under the nfs: device at
the root in the file manager.
See libnfs README file for optional URL arguments, such as selecting NFSv4.

# FAQ´S
Q: Is smbwLaunchELF will be merged sometime with wLaunchELF´s Source Code?

A: For now no.

Q: Will you add SMB2 on wLaunchELF_isr(el_isra version)?

A: Yes, i will add it on a separate branch.

Q: Could you add smb2man module to the official repository of Open Source PS2SDK?

A: No, due to the libsmb2 internal source code is licensed with L-GPL which conflicts with other major licenses, i can´t merge to the official repository unfortunally. i would have to request sahlberg´s to migrate or L-GPL to BSD or mit see here: https://github.com/sahlberg/libsmb2/pull/300#discussion_r1430790631

# wLaunchELF
wLaunchELF, formerly known as uLaunchELF, also known as wLE or uLE (abbreviated), is an open-source file manager and executable launcher for the Playstation 2 console based on the original LaunchELF. It contains many different features, including a text editor, hard drive manager, FTP support, and much more.
