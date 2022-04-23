#ifndef _MATCH_H_
#define _MATCH_H_

#define MATCH_HISTOGRAM 1
#define MATCH_DIFFFRAME 2
#define MATCH_FREQUENCY 3
#define MATCH_COLOR 4

typedef struct _simple_match
{
	//匹配最大值
	float max;

	//视图界面
	int egui;

	//播放原视频窗口
	int eov;

	char vName[64];
	char vName1[64];

	int width;
	int height;
}simple_match_t;

//帧对比数据结构
typedef struct _match_video
{
	simple_match_t simple_match;

	char ss_video[64];

	int vid;

	//3帧差法
	IplImage *hsvFrame1,*hsvFrame2,*hsvFrame3;
	//灰度帧
	IplImage *bkFrame1,*bkFrame2,*bkFrame3;
	//3帧灰度矩阵
	CvMat *bg1,*bg2,*bg3;
	
	CvMat *diffMat1;//1,2帧差矩阵
	CvMat *diffMat2;//2,3帧差矩阵

	CvMat *zeroMat32;//dd 前后帧差分矩阵
	CvMat *zeroMat8;//dd 前后帧差分灰度矩阵

	//输入帧
	IplImage *hsvFrame;
	IplImage *hsvBkFrame;
	CvMat *hsvBkMat;

	struct CvRect mh_rect;

	//输出:
	char vName[64];
	char vName1[64];
	char msg[1024];

	//输出帧
	IplImage *matchFrame;
	IplImage *matchBkFrame;

	//3帧差法:帧状态0,1,2
	int kf;

	//总帧数累加
	int v;

	//频率算法
	int level;	//修正等级0~10

	double matching;//匹配度0~1
	float max;//匹配最大值

	int (*run)(struct _match_video *);

}match_video_t;

typedef struct _task_match
{
	char task_id[128];//任务ID
	char ucase_id[128];//用例ID

	int s_task;//开始执行任务

	int stop;

	int ccamras;

	//默认配置
	simple_match_t simple_match;

	//多摄像头
	CvCapture **captures;
	//摄像头匹配池
	match_video_t **match_pool;
	//保存视频
	CvVideoWriter **cvs;
}task_match_t;

//
task_match_t _task_match;

//加载图像
//image=cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);

//privte:
//视频处理加入时间文字
int cvDisplayTxt(IplImage* frImg);

//剪切图像
void mhRectangle(IplImage *img,const struct CvRect *mh_rect);

//public:
//视频初始化
void match_task_init(task_match_t *task_match);
void match_video_init(match_video_t *match_video);

//视频播放 Match入口函数
int matchVideo(task_match_t *task_match);

int matchFile(task_match_t *task_match);

//获取一帧后处理数据结构
int matchProcess(match_video_t *match_video);

//释放 Match资源
void matchDestroy(match_video_t *match_video);

//算法选择
void match_video_run(match_video_t *match_video,int f);

//回调函数
//图像处理算法
//灰度直方图对比法
int matchHistogram(match_video_t *match_video);

//帧差法
int matchDiffFrame(match_video_t *match_video);

//专门抓取频率
int matchFrequency(match_video_t *match_video);

//指示灯红色判断
int matchColor(match_video_t *match_video);

//空算法
int matchNop(match_video_t *match_video);

//========Load too Test===============
/*Test Load Run
*加载图片测试方法入口
*loadBlackWhite		black-and-white
*loadHSV 		hue-saturation-value
*图片对比
*s1 对比图片
*s2 模板图片
*/
int loadBlackWhite(const char* s1,const char * s2,int (*callFun)(match_video_t *match_video));
int loadHSV(const char* s1,const char * s2,int (*callFun)(match_video_t *match_video));

//========Test API===============
//图像形态处理--直方图统计
int testHistogram(match_video_t *match_video);

//图像形态处理--直方图对比
int testCompareHist(match_video_t *match_video);

//图像形态处理--模板匹配
int testCompareTemplate(match_video_t *match_video);

//轮廓提取
int testContour(match_video_t *match_video);

//颜色HSV模式范围调节
int adjustHSV(const char* s1);
int adjustSplitBGR(const char *s1);
int adjustBGR(const char *s1);
int adjustGRAY(const char *s1);
void testBGR(const char* s1);
int light(const char* s1);

int hsvCompareHist(const char *s1);

int testBGR2HSV(const char* s1);
int testSplit(const char *s1);

int red_filter(const char* s1);

//角点检测
int testHarris(match_video_t *match_video);

//轮廓匹配
int testShapes(match_video_t *match_video);

//测试图像形态处理
int testImage(match_video_t *match_video);

//======Demo==================

//背景减除法--移动侦测
int background(int camera,const char* fileName);
#endif
