/* -*-  mode:c; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil;  -*- */
//---------------------------------------------------------------------------
//File name:   nfs.c
//---------------------------------------------------------------------------
#include <debug.h>
#include <netman.h>
#include <sys/time.h>
#include <ps2ip.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <launchelf.h>
#include <sbv_patches.h>
#include <nfs.h>

#include <nfsc/libnfs.h>
#include <nfsc/libnfs-raw-mount.h>

//#define DEBUG

extern int size_valid;
extern int time_valid;

#ifdef DEBUG
static int ns = -1;
static char sb[1024];
#define LOG(...) do { \
        if (ns >= 0) { \
                sprintf(sb, __VA_ARGS__); \
                lwip_write(ns, sb, strlen(sb)); \
        } \
        } while(0);
#else
#define LOG(...) do { } while(0);
#endif

enum ftype3 {
	NF3REG = 1,
	NF3DIR = 2,
	NF3BLK = 3,
	NF3CHR = 4,
	NF3LNK = 5,
	NF3SOCK = 6,
	NF3FIFO = 7,
};

struct nfs_share *nfs_shares = NULL;

static void free_nfs_share(struct nfs_share *nfs_share)
{
        if (nfs_share == NULL) {
                return;
        }
        LOG("free_nfs_share(%s)\n", nfs_share->name);
        if (nfs_share->nfs) {
                nfs_destroy_context(nfs_share->nfs);
        }
        free(nfs_share->name);
        free(nfs_share);
}

static struct nfs_share *nfs_find_share(const char *name);

static int loadNFSCNF(char *path)
{
	char *RAM_p;
        char *CNF_p, *name, *value;
        struct nfs_share *nfs_share = NULL;
        int entries = 0;

        LOG("try reading config from %s\n", path);
	if (!(RAM_p = preloadCNF(path)))
		return entries;
	CNF_p = RAM_p;
	while (get_CNF_string(&CNF_p, &name, &value)) {
                if (nfs_share == NULL) {
                        nfs_share = malloc(sizeof(struct nfs_share));
                        memset(nfs_share, 0, sizeof(struct nfs_share));
                }
                if (!strcmp(name, "NAME")) {
                        nfs_share->name = strdup(value);
                } else if (!strcmp(name, "URL")) {
                        struct nfs_context *nfs;
                        struct nfs_url *url;

                        if (nfs_find_share(nfs_share->name)) {
                                free(nfs_share);
                                nfs_share = NULL;
                                continue;
                        }
                        nfs_share->url = strdup(value);
                        nfs_share->nfs = nfs = nfs_init_context();
                        url = nfs_parse_url_dir(nfs, nfs_share->url);
                        if (nfs_mount(nfs, url->server, url->path) < 0) {
                                scr_printf("failed to connect to : %s:/%s %s\n",
                                           url->server, url->path,
                                           nfs_get_error(nfs));
                                nfs_destroy_url(url);
                                nfs_destroy_context(nfs);
                                free_nfs_share(nfs_share);
                                nfs_share = NULL;
                                continue;
                        }
                        scr_printf("Mounted share: %s:%s as %s\n",
                                   url->server, url->path, nfs_share->name);
                        LOG("Mounted %s:%s as %s\n",url->server, url->path, nfs_share->name);
                        nfs_destroy_url(url);
                        
                        nfs_share->next = nfs_shares;
                        nfs_shares = nfs_share;
                        nfs_share = NULL;
                        entries++;
                }
        }
	free(RAM_p);
	return entries;
}

void deinit_nfs(void)
{
        struct nfs_share *nfs_share;

        while (nfs_shares) {
                nfs_share = nfs_shares->next;
                free_nfs_share(nfs_shares);
                nfs_shares = nfs_share;
        }
        LOG("stopped logging to socket...\n");
}

int init_nfs(const char *ip, const char *netmask, const char *gw)
{
        int rc;

	scr_printf("Starting NFS\n");

#ifdef DEBUG
        ns = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ns >= 0) {
                struct sockaddr_in sin4;
                sin4.sin_addr.s_addr = inet_addr("10.10.10.11");
                sin4.sin_family = AF_INET;
                sin4.sin_port = htons(9999);
                if (connect(ns, (struct sockaddr *)&sin4, sizeof(sin4)) < 0) {
                        close(ns);
                        ns = -1;
                }
                LOG("started logging to socket...\n");
        }
#endif
                
        rc = loadNFSCNF("mc0:/SYS-CONF/NFS.CNF");
        if (!rc) {
                rc = loadNFSCNF("mc1:/SYS-CONF/NFS.CNF");
        }
        if (!rc) {
                rc = loadNFSCNF("mass:/SYS-CONF/NFS.CNF");
        }
	scr_printf("init_nfs.\n");

        return 0;
}

static struct nfs_share *nfs_find_share(const char *name)
{
        struct nfs_share *share;

        LOG("nfs_find_share(%s)\n", name);
        for (share = nfs_shares; share; share = share->next) {
                if (!strcmp(share->name, name)) {
                        return share;
                }
        }

        LOG("share not found\n");
        return NULL;
}

static void set_time(sceMcStDateTime *ps2time, u64 nfstime)
{
        struct tm *tm;
        time_t t = nfstime & 0xffffffff;

        tm = localtime(&t);
        if (tm == NULL) {
                return;
        }
        ps2time->Sec   = tm->tm_sec;
        ps2time->Min   = tm->tm_min;
        ps2time->Hour  = tm->tm_hour;
        ps2time->Day   = tm->tm_mday + 1;
        ps2time->Month = tm->tm_mon + 1;
        ps2time->Year  = tm->tm_year + 1900;
}

static void find_share(const char *path, char **name, char **p,
                       struct nfs_share **share)
{
        LOG("find_share(%s)\n", path);
        *name = strdup(&path[5]);
        if (*name == NULL) {
                return;
        }
        *p = strchr(*name, '/');
        if (*p == NULL) {
                return;
        }
        *((*p)++) = 0;

	*share = nfs_find_share(*name);
        if (*share == NULL) {
                return;
        }
}

int readNFS(const char *path, FILEINFO *info, int max)
{
        int n = 0;
        struct nfsdir *dir = NULL;
        struct nfs_share *share = NULL;
        char *name = NULL, *p;
        struct nfsdirent *ent;

        LOG("readNFS(%s)\n", path);
        /* Root of nfs: is a list of all the named shares */
        if (path[5] == '\0') {
                for (share = nfs_shares; share; share = share->next) {
                        if (strlen(share->name) >= MAX_NAME) {
                                continue;
                        }
                        strncpy(info[n].name, share->name, MAX_NAME-1);
                        info[n].name[MAX_NAME-1] = 0;
                        clear_mcTable(&info[n].stats);
			info[n].stats.AttrFile = MC_ATTR_norm_folder;
                        strncpy((char *)info[n].stats.EntryName, info[n].name, 32);
                        n++;
                        if (n >= max) {
                                break;
                        }
                }
                goto finished;
        }

        /* Find the share */
        LOG("not root, check %s\n", path);
        find_share(path, &name, &p, &share);
        if (share == NULL) {
                goto finished;
        }

	nfs_opendir(share->nfs, p, &dir);
	if (dir == NULL) {
                LOG("nfs_opendir_failed with %s\n", nfs_get_error(share->nfs));
                goto finished;
        }
        LOG("nfs_opendir success\n");
        while ((ent = nfs_readdir(share->nfs, dir))) {
                if (!strcmp(ent->name, ".") || !strcmp(ent->name, "..")) {
                        continue;
                }
                if (ent->type != NF3REG &&
                    ent->type != NF3DIR) {
                        continue;
                }
                if (strlen(ent->name) >= MAX_NAME) {
                        continue;
                }
		strncpy(info[n].name, ent->name, MAX_NAME);
                info[n].name[MAX_NAME-1] = 0;
		clear_mcTable(&info[n].stats);
		if (ent->type == NF3DIR) {
			info[n].stats.AttrFile = MC_ATTR_norm_folder;
		} else {
			info[n].stats.AttrFile = MC_ATTR_norm_file;
			info[n].stats.FileSizeByte = ent->size;
			info[n].stats.Reserve2 = 0;
		}

		strncpy((char *)info[n].stats.EntryName, info[n].name, 32);
		set_time(&info[n].stats._Create, ent->ctime.tv_sec);
		set_time(&info[n].stats._Modify, ent->mtime.tv_sec);
		n++;
		if (n >= max)
			break;
        }
	size_valid = 1;
	time_valid = 1;

 finished:
        free(name);
        if (dir) {
                nfs_closedir(share->nfs, dir);
        }
        LOG("returned %d entries\n", n);
        return n;
}

int NFSmkdir(const char *path, int fileMode)
{
        struct nfs_share *share = NULL;
        char *name = NULL, *p = NULL;
        int rc = 0;

        LOG("NFSmkdir [%s]\n", path);
        if (path[5] == '\0') {
                return -EINVAL;
        }

        find_share(path, &name, &p, &share);
        if (share == NULL) {
                free(name);
                return -EINVAL;
        }

        rc = nfs_mkdir(share->nfs, p);
        if (rc) {
                goto finished;
        }

finished:
        free(name);
        return rc;
}

int NFSrmdir(const char *path)
{
        struct nfs_share *share = NULL;
        char *name = NULL, *p = NULL;
        int rc = 0;

        LOG("NFSrmdir\n");
        if (path[5] == '\0') {
                return -EINVAL;
        }

        find_share(path, &name, &p, &share);
        if (share == NULL) {
                free(name);
                return -EINVAL;
        }

        if (p[strlen(p) - 1] == '/') {
                p[strlen(p) - 1] = 0;
        }
        rc = nfs_rmdir(share->nfs, p);
        if (rc) {
                goto finished;
        }

finished:
        free(name);
        return rc;
}

int NFSunlink(const char *path)
{
        struct nfs_share *share = NULL;
        char *name = NULL, *p = NULL;
        int rc = 0;

        LOG("NFSunlink\n");
        if (path[5] == '\0') {
                return -EINVAL;
        }

        find_share(path, &name, &p, &share);
        if (share == NULL) {
                free(name);
                return -EINVAL;
        }

        rc = nfs_unlink(share->nfs, p);
        if (rc) {
                goto finished;
        }

finished:
        free(name);
        return rc;
}

struct NFSFH *NFSopen(const char *path, int mode)
{
        struct nfs_share *share = NULL;
        char *name = NULL, *p = NULL;
        struct NFSFH *fh;

        LOG("NFSopen %s\n", path);
        if (path[5] == '\0') {
                return NULL;
        }

        find_share(path, &name, &p, &share);
        if (share == NULL) {
                free(name);
                return NULL;
        }
        fh = malloc(sizeof(struct NFSFH));
        if (fh == NULL) {
                free(name);
                return NULL;
        }

        fh->nfs = share->nfs;
        LOG("nfs_open [%s]\n", p);
        nfs_open(share->nfs, p, mode, &fh->fh);
        free(name);
        if(fh->fh == NULL) {
                free(fh);
                return NULL;
        }
        return fh;
}

int NFSclose(struct NFSFH *fh)
{
        LOG("NFSclose\n");
        return nfs_close(fh->nfs, fh->fh);
}

int NFSread(struct NFSFH *fh, char *buf, int size)
{
        LOG("NFSread\n");
        return nfs_read(fh->nfs, fh->fh, size, buf);
}

int NFSwrite(struct NFSFH *fh, char *buf, int size)
{
        LOG("NFSwrite\n");
        return nfs_write(fh->nfs, fh->fh, size, buf);
}

int NFSlseek(struct NFSFH *fh, int where, int how)
{
        int rc;
        uint64_t co = 0;
        
        LOG("NFSlseek %d %d\n", where, how);
        rc = nfs_lseek(fh->nfs, fh->fh, where, how, &co);
        if (rc < 0) {
                return -EIO;
        }
        LOG("rc:%d  %s\n", rc, nfs_get_error(fh->nfs));
        return co;
}

int NFSrename(const char *path, const char *newpath)
{
        struct nfs_share *share = NULL;
        char *name = NULL, *p = NULL;
        int ret;

        LOG("NFSrename\n");
        if (path[5] == '\0') {
                return -EINVAL;
        }

        find_share(path, &name, &p, &share);
        if (share == NULL) {
                free(name);
                return -ENOENT;
        }

        ret = nfs_rename(share->nfs, p, newpath);
        free(name);

        return ret;
}

//---------------------------------------------------------------------------
//End of file: nfs.c
//---------------------------------------------------------------------------
