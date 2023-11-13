/* -*-  mode:c; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil;  -*- */
//---------------------------------------------------------------------------
//File name:   nfs.h
//---------------------------------------------------------------------------
struct nfs_share {
        struct nfs_share *next;
        struct nfs_context *nfs;
        char *name;
        char *url;
};

extern struct nfs_share *nfs_shares;

int init_nfs(const char *ip, const char *netmask, const char *gw);
void deinit_nfs(void);

struct NFSFH {
        struct nfs_context *nfs;
        struct nfsfh *fh;
};

struct NFSFH *NFSopen(const char *path, int mode);
int NFSlseek(struct NFSFH *fh, int where, int how);
int NFSread(struct NFSFH *fh, char *buf, int size);
int NFSwrite(struct NFSFH *fh, char *buf, int size);
int NFSclose(struct NFSFH *fh);

int readNFS(const char *path, FILEINFO *info, int max);
int NFSmkdir(const char *dir, int fileMode);
int NFSrmdir(const char *dir);
int NFSunlink(const char *dir);
int NFSrename(const char *path, const char *newpath);

//---------------------------------------------------------------------------
//End of file: nfs.h
//---------------------------------------------------------------------------
