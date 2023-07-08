#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "medialib.h"
#include "thr_list.h"
#include "server_conf.h"
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <unistd.h>

static pthread_t tid;                           //线程id
static struct mlib_listentry_st *list_entry;    //频道列表
static int list_count;                          //频道列表长度

//发送频道列表线程
void *thr_list_send(void *p)
{
    int totalsize;                        //频道列表数据总长度
    int i = 0;                            //循环变量
    ssize_t len;                          //单个频道数据长度
    int ret;                              //返回值
    struct msg_list_st *entlistp;         //频道列表结构体指针
    struct msg_listentry_st *entryp;      //频道列表项结构体指针

    //计算频道列表数据总长度
    totalsize = sizeof(chnid_t);
    for(i = 0; i < list_count; i++)
    {
        syslog(LOG_DEBUG, "[thr_list.c][thr_list_send] chnid: %d, desc %s", list_entry[i].chnid, list_entry[i].desc);
        totalsize += sizeof(struct msg_listentry_st) + strlen(list_entry[i].desc);
    }

    //开辟频道列表数据空间
    entlistp = malloc(totalsize);
    if(entlistp == NULL)
    {
        syslog(LOG_ERR, "[thr_list.c][thr_list_send] malloc() %s", strerror(errno));
        exit(1);
    }

    //设置频道号
    entlistp->chnid = LISTCHNID;

    //存储数据
    entryp = entlistp->entry;
    for(i = 0; i < list_count; i++)
    {
        len = sizeof(struct msg_listentry_st) + strlen(list_entry[i].desc);
        entryp->chnid = list_entry[i].chnid;
        entryp->len = htons(len);
        strcpy((char *)entryp->desc, list_entry[i].desc);
        entryp = (struct msg_listentry_st *)((char *)entryp + len);
    }
    syslog(LOG_DEBUG, "[thr_list.c][thr_list_send] totalsize :%d", totalsize);

    //发送频道列表数据
    while(1)
    {
        ret = sendto(serversd, entlistp, totalsize, 0, (void *)&sndaddr, sizeof(sndaddr));
        if(ret  < 0)
        {
            syslog(LOG_WARNING, "[thr_list.c][thr_list_send]sendto(): %s", strerror(errno));
        }
        else
        {
            syslog(LOG_DEBUG, "[thr_list.c][thr_list_send]sendto() success");
        }
        sleep(1);
    }
}

//创建发送频道列表线程
int thr_list_create(struct mlib_listentry_st *listptr, ssize_t size)
{
    int err;
    list_count = size;
    list_entry = listptr;
    err = pthread_create(&tid, NULL, thr_list_send, NULL);
    if(err)
    {
        syslog(LOG_ERR, "[thr_list.c][thr_list_create] pthread_create(): %s", strerror(err));
        return -1;
    }
    return 0;
}

//销毁发送频道列表线程
void thr_list_destroy(void)
{
    //取消线程
    pthread_cancel(tid);
    //等待线程结束,回收资源
    pthread_join(tid, NULL);
}


