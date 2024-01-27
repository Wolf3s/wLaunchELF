[![CI](https://github.com/ps2homebrew/wLaunchELF/workflows/CI/badge.svg)](https://github.com/ps2homebrew/wLaunchELF/actions?query=workflow%3ACI)

# smbLaunchELF
This is a special version of LaunchELF that has support for SMB2/3 and NFS.
It can connect to Windows/Samba/Azure file shares and supports up to
version 3.1.1 of SMB.
It can connect to NFSv3 and v4 servers.
The main purpose for this version is just to copy files to/from SMB2 and/or NFS
file servers to avoid having to use clunky FTP.

To build this version you need to install LIBSMB2 and LIBNFS for PS2 EE.
See hompage for libsmb2, libnfs for instructions on how to build and install
them in ps2dev.

SMB2
====
You can specify SMB2 servers and shares in either of the files:
  mc0:/SYS-CONF/SMB2.CNF or
  mc1:/SYS-CONF/SMB2.CNF or
  mass:/SYS-CONF/SMB2.CNF

The format of this file is :

NAME = Home-NAS

USERNAME = user

PASSWORD = password

URL = smb://192.168.0.1/Share/

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

NAME = Home-NFS

URL = nfs://192.168.0.1/path/export

For each such entry an new subdirectory will appear under the nfs: device at
the root in the file manager.
See libnfs README file for optional URL arguments, such as selecting NFSv4.

# wLaunchELF

wLaunchELF, formerly known as uLaunchELF, also known as wLE or uLE (abbreviated), is an open-source file manager and executable launcher for the Playstation 2 console based on the original LaunchELF. It contains many different features, including a text editor, hard drive manager, FTP support, and much more.
