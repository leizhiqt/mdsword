#ifndef _DEEPDOOR_H_
#define _DEEPDOOR_H_

#define DLIB_JPEG_SUPPORT
#define DLIB_PNG_SUPPORT

#define DEEPDOOR_RELEASE

extern "C"
{
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "pthread.h"
#include "signal.h"
#include "string.h"
#include "sys/time.h"
#include "time.h"

#include "log.h"
#include "dirent.h"
#include "sfline.h"
#include "thpool.h"
#include "dtype.h"
#include "conf.h"
#include "times.h"
#include "csocket.h"
}

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include "opencv2/videoio.hpp"

#include <dlib/opencv.h>
#include <dlib/dnn.h>
#include <dlib/clustering.h>
#include <dlib/string.h>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

#include <dlib/image_io.h>
#include <dlib/image_transforms.h>

#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/data_io.h>

#include <pthread.h> 

#include <iostream>
#include <fstream>

using namespace std;
using namespace dlib;

//摄像头缓存帧数
#define LOOP_FS 8
//样本最大数
#define SAMPLE_FS_MAX 5

class WVideo 
{
	private:
		string url;
		string vname;
		boolean recordVideo;
		double dbuf[1024];
		
		cv::Mat cache_mat[LOOP_FS];
		pthread_rwlock_t rwlock_cache_mat;

	public:
		WVideo(string url,string vname)
		{
			this->url = url;
			this->vname = vname;
			pthread_rwlock_init(&rwlock_cache_mat, NULL);
		}

		string getUrl();
		string getVname();
		double* getDbuf();
		
		void set_cache_mat(int k,cv::Mat cv_mat);
		void get_cache_mat(cv::Mat *cache_mat);
		void init_cache_mat(cv::Mat cv_mat);
};

string WVideo::getUrl() 
{
	return url;
}

string WVideo::getVname() 
{
	return vname;
}

double* WVideo::getDbuf() 
{
	return dbuf;
}

void WVideo::set_cache_mat(int k,cv::Mat cv_mat) 
{
	pthread_rwlock_wrlock(&rwlock_cache_mat);
	cache_mat[k]=cv_mat.clone();
	pthread_rwlock_unlock(&rwlock_cache_mat);
}

void WVideo::get_cache_mat(cv::Mat *cache_mat) 
{
	pthread_rwlock_rdlock(&rwlock_cache_mat);
	for(int i=0;i<LOOP_FS;i++)
	{
		*(cache_mat+i) = this->cache_mat[i].clone();
	}
	pthread_rwlock_unlock(&rwlock_cache_mat);
}

void WVideo::init_cache_mat(cv::Mat cv_mat) 
{
	pthread_rwlock_wrlock(&rwlock_cache_mat);
	for(int i=0;i<LOOP_FS;i++)
	{
		*(cache_mat+i) = cv_mat.clone();
	}
	pthread_rwlock_unlock(&rwlock_cache_mat);
}

//template<class T>
//class GaussFilter:public std::vector<double>
class GaussFilter
{
	private:
		std::vector<double> dbuf;
		int scale;
	public: 
		GaussFilter(int scale)
		{
		  this->scale = scale;
		}
		double gauss_av(double k);
};

double GaussFilter::gauss_av(double k) 
{
	return 0;
}

#define BUFFER_SIZE 4096

#define SERVPORT 10088 /*服务器监听端口号 */
#define BACKLOG 10 /* 最大同时连接请求数 */

static int stop = 0;
static pthread_mutex_t s_mutex;

static thpool_t thpool;

static int sf = 3;

int scale = 25;

typedef struct _c_recogn
{
	int csocket;//客户端cocket
	
	long ack;//命令序列
	long cmd;//命令ID
	char req[1024];//请求命令
	char res[1024];//返回数据
	char camera_name[128];//摄像头名称
}c_recogn_t;

//缩放比列
//0.75s
int m_width = 1280;
int m_height = 720;

//0.4s
//int m_width = 640;
//int m_height = 360;

//1.2s
//int m_width = 1920;
//int m_height = 1080;

float match_value = 0.4;

//工作模式
int job_type = 2;

char sample_path[1024] ="faces";
char save_path[1024] ="/tmp";

char conf_path[1024] ="deepdoor.cnf";

char cache_path[1024] ="cache";

char to_addr[64] ="1920849305@qq.com";

//客户端列表
std::vector<c_recogn_t> cs_pool;

//线程读写锁
pthread_rwlock_t rwlock_csocket_pool;

int recv_shake_hand(int csockfd);
int recv_protocol(const char *req);
int send_rep_txt(const int sockfd,const char *buf);

#endif
