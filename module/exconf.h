#ifndef _CONF_H_
#define _CONF_H_

#define abs(a,b) (a+b)

#define xmalloc(x) malloc(sizeof(x))

#define BUFFER_SIZE 1024

//rpath	camera_id	task_id	ucase_id
//[data/task_id/ucase_id/camera_id/*]

//Task Camera Packs
#define CONF_TCP	"%s/%s/%s/camera%d"
//#define CONF_VP		"%s/video/camera%d"

//[data/task_id/camera*.jpg]
#define CONF_TP		"%s/%s"
#define CONF_TPP	"-C %s %s"
#define CONF_TPD	"%s/%s.tar.gz"

//files pack
//[data/task_id/ucase_id/*]
#define CONF_FP		"-C %s/%s %s"
//pack down
#define CONF_PD		"%s/%s/%s.tar.gz"

//字体
CvFont font;

//时间同步IP
char ntp_ip[16];

//数据保存路径
char rpath[128];

//视频文件路径
char vfpath[128];

//图像处理算法
int fk;

//图像计算
typedef struct _task_match task_match_t;

/*
typedef struct _fast_data{
	link_list_t tasks;//所有任务
	link_list_t tasks_msg;//任务->异常信息
	link_list_t tasks_img;//任务->图片
}fast_data_t;
*/

//客户端
typedef struct _thclient thclient_t;
typedef struct _thclient{
	int csockfd;
	task_match_t *task_match;
}thclient_t;

//存储数据目录
void set_save_fpath(char *r_spath,const int max,const int vid,const char *task_id,const char *ucase_id,const int format);
void set_save_fname(char *savepath,const int max,const int vid,const char *task_id,const char *ucase_id,const int format);

//启动配置默认值
void conf_def();
//启动配置初始化
void conf_init();

#endif
