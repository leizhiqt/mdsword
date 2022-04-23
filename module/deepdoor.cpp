// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

	This example program shows how to find frontal human faces in an image and
	estimate their pose.  The pose takes the form of 68 landmarks.  These are
	points on the face such as the corners of the mouth, along the eyebrows, on
	the eyes, and so forth.  
	

	This example is essentially just a version of the face_landmark_detection_ex.cpp
	example modified to use OpenCV's VideoCapture object to read from a camera instead 
	of files.


	Finally, note that the face detector is fastest when compiled with at least
	SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD
	chip then you should enable at least SSE2 instructions.  If you are using
	cmake to compile this program you can enable them by using one of the
	following commands when you create the build project:
		cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON
		cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON
		cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON
	This will set the appropriate compiler options for GCC, clang, Visual
	Studio, or the Intel compiler.  If you are using another compiler then you
	need to consult your compiler's manual to determine how to enable these
	instructions.  Note that AVX is the fastest but requires a CPU from at least
	2011.  SSE4 is the next fastest and is supported by most current machines.  
*/
#include <exdlib.h>
#include "deepdoor.h"

//强制开启调试模式
//#define DEEPDOOR_DEBUG

//强制关闭调试模式
//#undef DEEPDOOR_DEBUG

//
#define MAX_Filter(x,y) ((x) > (y) ? (x) : (y))
#define MIN_Filter(x,y) ((x) < (y) ? (x) : (y))

double Threshold_Value = 0.6;

double Max_Filter_Double(double *pBuf, int num)
{
	int i = 0;
	double max = 0;
	
	max = *pBuf++;
	for (i = 1; i < num; i++)
	{
		max = MAX_Filter(*pBuf, max);
	}
	return max;
}

double Ma_Filter_Double(double *pBuf, int num)
{
	int i = 0;
	double max, min, temp;
	
	max = min = temp = *pBuf++;
	for (i = 1; i < num; i++)
	{
		max = MAX_Filter(*pBuf, max);
		min = MIN_Filter(*pBuf, min);
		temp += *pBuf++;
	}
	return ((temp - max - min) / (num - 2));
}

//临时配置
//01 05 00 10 ff 00 8d ff	开
//01 05 00 10 00 00 cc 0f	关
char r11[] = "\x01\x05\x00\x10\xff\x00\x8d\xff";
char r10[] = "\x01\x05\x00\x10\x00\x00\xcc\x0f";

char r21[] = "\x01\x05\x00\x11\xff\x00\xdc\x3f";
char r20[] = "\x01\x05\x00\x11\x00\x00\x9d\xcf";

char r31[] = "\x01\x05\x00\x12\xff\x00\x6d\xcf";
char r30[] = "\x01\x05\x00\x12\x00\x00\x2c\x3f";

char r41[] = "\x01\x05\x00\x13\xff\x00\x7d\xff";
char r40[] = "\x01\x05\x00\x13\x00\x00\x3c\x0f";

//查询DI
char qdi[] = "\x01\x01\x00\x00\x00\x04\x3d\xc9";

//第4字节
//01:01:01:00:51:88
//00000000 00
//00000001 01
//00000010 02
//00000100 04
//00001000 08

static std::map<string,string> plc;

void plc_init()
{
	plc.insert(std::map<string,string>::value_type("camera1","24.47.20.201"));
	plc.insert(std::map<string,string>::value_type("camera2","24.47.20.201"));
	plc.insert(std::map<string,string>::value_type("camera3","24.47.20.206"));
}

int plc_find(const char* camera_name,string &str1)
{
	//查找并对比识别
	printfs("%s\t%d\t[DEBUG]: plc_find camera_name:%s",__FILE__,__LINE__,camera_name);
	std::map<string,string>::iterator iter;

	iter=plc.find(camera_name);
	if(iter != plc.end())
	{
		str1 = iter->second;
		printfs("%s\t%d\t[DEBUG]: plc_find:%s",__FILE__,__LINE__,str1.c_str());
		return 0;
	}
	else
	{
		printfs("%s\t%d\t[DEBUG]: plc_find:Do not Find",__FILE__,__LINE__);
		return 1;
	}
}

int open_plc(client_t* client, const char* camera_name)
{
	client_connect(client);

	//毫秒 微秒
	double second = 0;
	struct timeval tv;
	struct timeval tv_prv;
	gettimeofday(&tv,NULL);
	tv_prv = tv;
	
//	while(1)
//	{
//		//查询
//		memcpy(client->buf,qdi,8);
//		client_send(client);
//
//		client_recv(client);
//		client->flags=client->buf[3];
//
//		if(client->flags!=0x00) break;
//
//		gettimeofday(&tv,NULL);
//		//秒 微秒
//		second = (tv.tv_sec-tv_prv.tv_sec)+(double)(tv.tv_usec-tv_prv.tv_usec)/1000000;//秒
//
//		printfs("%s\t%d\t[DEBUG]:open_plc timeout:%lf",__FILE__,__LINE__,second);
//		if(second>60){
//			printfs("%s\t%d\t[DEBUG]:goto end_process",__FILE__,__LINE__);
//			return -1;
//		}
//		usleep(1000*1000);
//	}

	printfs("%s\t%d\t[DEBUG]: rq:%x",__FILE__,__LINE__,client->flags);
	usleep(1000);

	if(0 != strcmp(camera_name, "camera2"))
	{
		memcpy(client->buf,r10,8);
		client_send(client);
		usleep(200*1000);
	//client_recv(client);
	}
	if(0 != strcmp(camera_name, "camera1"))
	{
		memcpy(client->buf,r20,8);
		client_send(client);
		usleep(200*1000);
	//client_recv(client);
	}
//	if(client->flags==0x04)
//	{
		memcpy(client->buf,r30,8);
		client_send(client);
	usleep(200*1000);
	//client_recv(client);
//	}
//	if(client->flags==0x08)
//	{
		memcpy(client->buf,r40,8);
		client_send(client);
	usleep(200*1000);
	//client_recv(client);
//	}
	
	return 0;
}

void close_plc(client_t* client)
{
	printfs("%s\t%d\t[DEBUG]: rq:%x",__FILE__,__LINE__,client->flags);

//	if(client->flags==0x01)
//	{
		memcpy(client->buf,r11,8);
		client_send(client);
	usleep(200*1000);
	//client_recv(client);
//	}
//	if(client->flags==0x02)
//	{
		memcpy(client->buf,r21,8);
		client_send(client);
	usleep(200*1000);
	//client_recv(client);
//	}
//	if(client->flags==0x04)
//	{
		memcpy(client->buf,r31,8);
		client_send(client);
	usleep(200*1000);
	//client_recv(client);
//	}
//	if(client->flags==0x08)
//	{
		memcpy(client->buf,r41,8);
		client_send(client);
	usleep(200*1000);
	//client_recv(client);
//	}
	client_close(client);
}

/*主模式验证
*dlib_master_auth
*const char * camera_name	摄像头名称
*char * save_path_ch		异常或者验证成功后保存图片的路径
*/
int dlib_master_auth(const char* camera_name,char* diff_save_path,int* face_count,char* str_id,double* compare);

int cv_load_sample(string sample_path,std::map<string,std::vector<matrix<float,0,1>>> &sample_faces_desc);

/*人脸检测&识别初始化
*dlib_init
*/
void conf_init()
{
	dlib_init();

	cv_load_sample(sample_path,sample_faces_desc);

	//pthread_mutex_init(&lock_broadcast,NULL);
	//pthread_mutex_lock(&lock_broadcast);
	//pthread_mutex_unlock(&lock_broadcast);
	
	//pthread_rwlock_init(&rwlock_sample, NULL);
	
	pthread_rwlock_init(&rwlock_csocket_pool, NULL);
	//pthread_rwlock_init(&rwlock_broadcast, NULL);
}

/*销毁dlib相关资源
*dlib_destroy
*/
void destroy()
{
	//pthread_mutex_destroy(&lock_broadcast);
	
	//pthread_rwlock_destroy(&rwlock_sample);
	pthread_rwlock_destroy(&rwlock_csocket_pool);
	//pthread_rwlock_destroy(&rwlock_broadcast);
}

/*统计客户端数
*count_broadcast
*/
int count_broadcast()
{
	int k=0;
	pthread_rwlock_rdlock(&rwlock_csocket_pool);
	k=cs_pool.size();
	pthread_rwlock_unlock(&rwlock_csocket_pool);
	return k;
}

/*广播消息
*send_broadcast
*char *msg 发送数据
*/
void send_broadcast(const char *msg)
{
	char buf[1024];

	pthread_rwlock_rdlock(&rwlock_csocket_pool);
	for (auto c_recogn : cs_pool)
	{
		printfs("%s\t%d\t[DEBUG]: send_broadcast:%d %d",__FILE__,__LINE__,c_recogn.csocket,c_recogn.cmd);

		if(c_recogn.cmd==130)
		{
			send_rep_txt(c_recogn.csocket,msg);
			c_recogn.cmd=134;
		}
	}
	pthread_rwlock_unlock(&rwlock_csocket_pool);
}

/*检测到人脸信息后给所有客户端推送json信息
*ch_send_msg
*const char *camera_name 摄像头名称
*/
void ch_send_msg(const char *camera_name,int forever)
{
	if(cs_pool.size()<1) return;

	char buf[1024];
	//更新客户端状态 等待接收人脸检查的客户端
	pthread_rwlock_rdlock(&rwlock_csocket_pool);
	//printfs("%s\t%d\t[DEBUG]: cs_pool.size:%d",__FILE__,__LINE__,cs_pool.size());
	for (int i = 0; i < cs_pool.size();i++)
	{
		//printfs("%s\t%d\t[DEBUG]:cs_pool[%d].cmd:%d strcmd:%d %s %s",__FILE__,__LINE__,i,cs_pool[i].cmd,strcmp(camera_name,cs_pool[i].camera_name),camera_name,cs_pool[i].camera_name);
		//已经开启会话
		if (cs_pool[i].cmd == 130 && strcmp(camera_name,cs_pool[i].camera_name)==0)
		{
			snprintf(buf,sizeof(buf),"{'cmd':'%d','ack':'%ld','captureName':'%s'}\n",cs_pool[i].cmd,cs_pool[i].ack,camera_name);
			send_rep_txt(cs_pool[i].csocket,buf);
			if(!forever)cs_pool[i].cmd = 134;
			
			printfs("%s\t%d\t[DEBUG]: cs_pool find camera_name:%s cs_pool[%d].camera_name:%s cmd:%ld csocket:%d",__FILE__,__LINE__,camera_name,i,cs_pool[i].camera_name,cs_pool[i].cmd,cs_pool[i].csocket);
			//break;
		}
	}
	pthread_rwlock_unlock(&rwlock_csocket_pool);
}

/*所有客户端推送json信息
*ca_send_msg
*const char *camera_name 摄像头名称
*const char *msg 发送副信息
*/
void ca_send_msg(const char *camera_name,const char *msg)
{
	if(cs_pool.size()<1) return;

	char buf[1024];
	//更新客户端状态 等待接收人脸检查的客户端
	pthread_rwlock_rdlock(&rwlock_csocket_pool);
	//printfs("%s\t%d\t[DEBUG]: cs_pool.size:%d",__FILE__,__LINE__,cs_pool.size());
	for (int i = 0; i < cs_pool.size();i++)
	{
		//printfs("%s\t%d\t[DEBUG]:cs_pool[%d].cmd:%d strcmd:%d %s %s",__FILE__,__LINE__,i,cs_pool[i].cmd,strcmp(camera_name,cs_pool[i].camera_name),camera_name,cs_pool[i].camera_name);
		//已经开启会话
		if (strcmp(camera_name,cs_pool[i].camera_name)==0)
		{
			snprintf(buf,sizeof(buf),"{\"cmd\":%d,\"ack\":%ld,\"captureName\":\"%s\",%s}\n",cs_pool[i].cmd,cs_pool[i].ack,camera_name,msg);
			send_rep_txt(cs_pool[i].csocket,buf);

			printfs("%s\t%d\t[DEBUG]: cs_pool find camera_name:%s cs_pool[%d].camera_name:%s cmd:%ld csocket:%d",__FILE__,__LINE__,camera_name,i,cs_pool[i].camera_name,cs_pool[i].cmd,cs_pool[i].csocket);
			//break;
		}
	}
	pthread_rwlock_unlock(&rwlock_csocket_pool);
}

/*检测到人脸信息后处理进程
*check_faces_process
*const char *camera_name 摄像头名称
*int p_type 处理类型
*/
int check_faces_process(const char *camera_name,int p_type)
{
	int k=0;
	char save_path_ch[512];
	char str_id[128];
	char buf[1024];
	int face_count=0;
	double compare=0.0;
	char *p=NULL;

	snprintf(str_id,sizeof(str_id),"%s","0");

	string ip_addr;
	int wait_close=0;
	client_t client;
	//client->host=gethostbyname(ipaddr);
//void close_plc(const char* ipaddr)

	//printfs("%s\t%d\t[DEBUG]: check_faces_process camera_name:%s p_type:%d",__FILE__,__LINE__,camera_name,p_type);
	switch(p_type)
		{
			case 0:
				//检查是否有客户端
				if(count_broadcast()>0) 
				{
					//printfs("%s\t%d\t[DEBUG]: send_check 130 response",__FILE__,__LINE__);
					//{'cmd':'%d','ack':'%ld','captureName':'%s'}\n
					//广播人脸检测
					ch_send_msg(camera_name,1);

//					//>>yes
//					//设置保存路径
//					sprintf(buf, "%s/yes_%s_%d.jpg",save_path,video->getVname().c_str(),k);
//					//保存检测图片
//					cv::imwrite(buf, resize_mat);

//					printfs("%s\t%d\t[DEBUG]:send 130 yes:%s",__FILE__,__LINE__,buf);
//					//<<
				}
				break;
			case 1:
				k=dlib_master_auth(camera_name,save_path_ch,&face_count,str_id,&compare);
				//k:0 正常进入 k:1 异常进入
				//发送 email信息
				if(k==0)
				{
					send_email(camera_name,"正常出入",save_path_ch,to_addr);
				}
				else if(k==1)
				{
					send_email(camera_name,"异常告警",save_path_ch,to_addr);
				}
				break;
			case 2:
				//处理人脸识别
				//k:0 人脸识别正确 k:1 人脸未识别
				k=dlib_master_auth(camera_name,save_path_ch,&face_count,str_id,&compare);

				//打开PLC
				if(k==0)
				{
					printfs("%s\t%d\t[DEBUG]: camera_name:%s",__FILE__,__LINE__,camera_name);
					if(plc_find(camera_name,ip_addr)==0)
					{
						client.host=gethostbyname(ip_addr.c_str());
						client.port=4196;
						memset(client.buf, '\0',sizeof(client.buf));
						client.buf_size=8;
						client.flags=0;

						open_plc(&client, camera_name);
						wait_close=1;
					}
				}

				//{'cmd':'134','ack':'%ld','captureName':'%s','type':'1','filePath':'%s','faceNumber':%d,data:[ {'id':%d,'compare':%lf}]}\n
				//{'cmd':'134','ack':'%ld','type':'0','captureName':'%s','filePath':'%s','faceNumber':%d,data:[{'id':%d,'compare':%lf}]}\n
				p = strrchr(str_id,'/');
				if(p!=str_id+strlen(str_id)-1 && p!=NULL){
					snprintf(buf,sizeof(buf),"\"type\":\"%d\",\"filePath\":\"%s\",\"faceNumber\":%d,\"data\":[{\"id\":\"%s\",\"compare\":%-5.4lf}]",k,save_path_ch,face_count,p+1,compare);
				}else{
					snprintf(buf,sizeof(buf),"\"type\":\"%d\",\"filePath\":\"%s\",\"faceNumber\":%d,\"data\":[{\"id\":\"%s\",\"compare\":%-5.4lf}]",k,save_path_ch,face_count,str_id,compare);
				}

				printfs("%s\t%d\t[DEBUG]:ca_send_msg buf:%s",__FILE__,__LINE__,buf);
				//发送人脸识别信息
				ca_send_msg(camera_name,buf);

				if(wait_close)
				{
					usleep(1000*1000*15);//15s
					close_plc(&client);
				}
				break;
			default:
				break;
		}
	return k;
}

/*缓存图片
*filter_cache
*const char* mk_path 缓存目录
*/
int filter_cache(const char* mk_path,cv::Mat save_mat)
{
	mkdir_p(mk_path);

	printfs("%s\t%d\t[DEBUG]: mk_path:%s",__FILE__,__LINE__,mk_path);

	static int k=0;

	char save_path[512];
	memset(save_path,0,sizeof(save_path));

	sprintf(save_path,"%s/%d.jpg",mk_path,k++%LOOP_FS);

	cv::imwrite(save_path, save_mat);

	return 0;
}

/*加载图片->人脸矩阵
*cv::Mat read_mat opencv mat 图片数据
*std::vector<rectangle> rect_faces 人脸矩形向量
*std::vector<matrix<rgb_pixel>> &id_faces 人脸矩阵向量
*/
int cv_load_face_vector(cv::Mat read_mat,std::vector<rectangle> rect_faces,std::vector<matrix<rgb_pixel>> &id_faces)
{
	 shape_predictor sp_landmarks = sp_landmarks_ex;
	 //dlib 
	 cv_image<bgr_pixel> bgr_img(read_mat);
	 for (auto face : rect_faces)
	 {
		//2.关键点检测 68关键点
		auto shape = sp_landmarks(bgr_img, face);
		matrix<rgb_pixel> face_chip;
		//图片标准化为150*150像素大小 1:N
		extract_image_chip(bgr_img, get_face_chip_details(shape,150,0.25), face_chip);
		//extract_image_chip(bgr_img, get_face_chip_details(shape),face_chip);
		//加入标准人脸数据
		id_faces.push_back(move(face_chip));
	 }
	 return 0;
}

/*把识别率最高的图片加入样本库
*string id 图片目录
*string sample_file 样本图片全路径
*/
int cv_add_sample_id(char* id,char* sample_file)
{
	int RET=0;

	//样本数据
	char sample_sub_path[512]={0};
	sprintf(sample_sub_path,"%s/%s",sample_path,id);

	printfs("%s\t%d\t[DEBUG]: sample_sub_path:%s",__FILE__,__LINE__,sample_sub_path);

	DIR *pDir; 
	if((pDir=opendir(sample_sub_path))==NULL) 
		return -1;

	struct dirent	*ent; 
	int		i=0; 
	while((ent=readdir(pDir))!=NULL)
	{
		//统计文件数
		if(ent->d_type & DT_REG)
		{
			i++;
		}
	}

	if(i>SAMPLE_FS_MAX) i=SAMPLE_FS_MAX;

	memset(sample_sub_path,sizeof(sample_sub_path),'\0');
	sprintf(sample_sub_path,"%s/%s/ext_%d.jpg",sample_path,id,i);

	RET=cp_file(sample_sub_path,sample_file);

	//printfs("%s\t%d\t[DEBUG]: cv_add_sample_id:",__FILE__,__LINE__);
	closedir(pDir);
	
	//刷新样本库
	cv_load_sample(sample_path,sample_faces_desc);

	return RET;
}

/*目录列表
*string file_path 目录
*std::vector<string> &childs 查询后的向量
*int type 0所有目录 1所有子目录 2所有子文件
*/
int file_dirs(string file_path,std::vector<string> &childs,int type)
{
	DIR *pDir; 
	if((pDir=opendir(file_path.c_str()))==NULL) 
		return -1;

	struct dirent	*ent;
	int		i=0; 
	char	childpath[512]; 
	memset(childpath,0,sizeof(childpath)); 

	printfs("%s\t%d\t[DEBUG]: file_path:%s",__FILE__,__LINE__,file_path.c_str());

	const char *fp_path = file_path.c_str();
	while((ent=readdir(pDir))!=NULL)
	{ 
		if(strcmp(".",ent->d_name)==0 || strcmp("..",ent->d_name)==0) continue;

		if(ent->d_type & DT_DIR && (type==1 || type==0))
		{ 
			sprintf(childpath,"%s/%s",fp_path,ent->d_name);
			childs.push_back(childpath);
			printfs("%s\t%d\t[DEBUG]: child_path:%s",__FILE__,__LINE__,childpath);
		}
		
		if(ent->d_type & DT_REG && (type==2||type==0))
		{ 
			sprintf(childpath,"%s/%s",fp_path,ent->d_name);
			childs.push_back(childpath);
			printfs("%s\t%d\t[DEBUG]: child_path:%s",__FILE__,__LINE__,childpath);
		}
	}
	closedir(pDir);

	printfs("%s\t%d\t[DEBUG]: file_dirs finsh",__FILE__,__LINE__);
	return 0;
}

/*启动的时候直接加载到内存所有样本库
  加载目录下所有图片->128D向量
*string sample_path 图片目录
*std::map<string,std::vector<matrix<float,0,1>>> &sample_faces_desc 传地址128D向量所有样本集合
*/
int cv_load_sample(string sample_path,std::map<string,std::vector<matrix<float,0,1>>> &sample_faces_desc)
{
	sample_faces_desc.clear();
	anet_type net = net_dnn;

	std::vector<string> childs;
	file_dirs(sample_path,childs,1);
	
	for(std::vector<string>::const_iterator iter=childs.begin();iter!=childs.end();++iter)
	{
		string id_path = (*iter);
		printfs("%s\t%d\t[DEBUG]:sample_path_id:%s",__FILE__,__LINE__,id_path.c_str());
		
		std::vector<string> id_samples;
		file_dirs(id_path,id_samples,2);

		//对文件夹下的每一个人脸进行:
		//1.人脸检测
		//2.关键点检测
		//3.描述子提取

		//单张图片的cvMat
		cv::Mat read_mat;
		//图像变换变量
		cv::Mat chip_mat,chip_gray,equalized_mat;
		//人脸矩阵
		std::vector<matrix<rgb_pixel>> id_faces;

		for(std::vector<string>::const_iterator is_iter=id_samples.begin();is_iter!=id_samples.end();++is_iter)
		{
			string id_sample = (*is_iter);
			printfs("%s\t%d\t[DEBUG]:sample_path_id_file:%s",__FILE__,__LINE__,id_sample.c_str());

			read_mat = cv::imread(id_sample,3);
			//人脸矩形向量
			std::vector<rectangle> rect_faces;

			//人脸检测器
			int faces_count = ex_frontal_face_detector(rect_faces,read_mat,chip_mat,chip_gray,equalized_mat);

			//获得边界框
			//rect_faces = ff_detector(img);
			printfs("%s\t%d\t[DEBUG]: faces_count:%d",__FILE__,__LINE__,faces_count);

			//加载图片->人脸矩阵
			cv_load_face_vector(read_mat,rect_faces,id_faces);
		}

		// This call asks the DNN to convert each face image in faces into a 128D vector.
		// In this 128D vector space, images from the same person will be close to each other
		// but vectors from different people will be far apart.  So we can use these vectors to
		// identify if a pair of images are from the same person or from different people.  
		//# 3.描述子提取，128D向量
		// And finally we load the DNN responsible for face recognition.
		printfs("%s\t%d\t[DEBUG]: id_faces:%d",__FILE__,__LINE__,id_faces.size());

		//id_faces 128D
		std::vector<matrix<float,0,1>> id_faces_desc;
		//人脸矩阵->128D
		id_faces_desc = net(id_faces);

		printfs("%s\t%d\t[DEBUG]: cv_load_face_vector",__FILE__,__LINE__);
		sample_faces_desc.insert(std::map<string, std::vector<matrix<float,0,1>>>::value_type(id_path,id_faces_desc));

		printfs("%s\t%d\t[DEBUG]: cv_load_sample_id finsh",__FILE__,__LINE__);
	}
	printfs("%s\t%d\t[DEBUG]: cv_load_sample finsh",__FILE__,__LINE__);
	return 0;
}

/*查找ID对应的128D向量
*cv_get_sample
*string sample_path 图片目录
*std::vector<matrix<float,0,1>> &id_faces_desc 传地址128D向量
*/
int cv_get_sample(string sample_path,std::vector<matrix<float,0,1>> &id_faces_desc)
{
	id_faces_desc.clear();

	std::map<string,std::vector<matrix<float,0,1>>>::iterator iter;
	for(iter=sample_faces_desc.begin();iter!=sample_faces_desc.end();++iter){
		printfs("%s\t%d\t[DEBUG]: cv_load_sample_id id:%s",__FILE__,__LINE__,iter->first.c_str());
		//std::vector<matrix<float,0,1>> id_faces_desc = iter->second;
		if(strstr(iter->first.c_str(),sample_path.c_str()) != NULL){
			printfs("%s\t%d\t[DEBUG]: cv_get_sample find:%s",__FILE__,__LINE__,sample_path.c_str());
			id_faces_desc = iter->second;
			return 0;
		}
	}
	return 0;
}

/*dlib_diff_128matrix
*计算集合矩阵差
*std::vector<matrix<float,0,1>> test_faces_matrix   样本矩阵
*std::vector<matrix<float,0,1>> sample_faces_matrix 测试矩阵
*double* diff_matrix_min 返回计算最小差值
*/
int dlib_diff_128matrix(const std::vector<matrix<float,0,1>> test_faces_matrix,const std::vector<matrix<float,0,1>> sample_faces_matrix,double* diff_matrix_min)
{
	char buf[BUFFER_SIZE];//缓冲区
	memset(buf,'\0',BUFFER_SIZE);

	printfs("%s\t%d\t[DEBUG]: dlib_diff_128matrix test_faces:%d sample_faces:%d",__FILE__,__LINE__,test_faces_matrix.size(),sample_faces_matrix.size());

	//矩阵差
	double diff_matrix=0;

	for(size_t j=0;j<test_faces_matrix.size();++j)
	{
		//矩阵最小差
		*diff_matrix_min = 1;

		for(size_t i=0;i<sample_faces_matrix.size();++i)
		{
			diff_matrix = length(test_faces_matrix[j] - sample_faces_matrix[i]);

			snprintf(buf,sizeof(buf),"test_faces_matrix[%d]-sample_faces_matrix[%d] = %lf",j,i,diff_matrix);

			#ifdef DEEPDOOR_DEBUG
			save_txt("diff_128matrix.txt",buf);
			#endif
			printfs("%s\t%d\t[DEBUG]: 人脸识别详细数据:%s",__FILE__,__LINE__,buf);

			if(diff_matrix<*diff_matrix_min) *diff_matrix_min = diff_matrix;

			if(*diff_matrix_min<match_value)
			{
				return 0;
			}
		}

		snprintf(buf,sizeof(buf),"\ttest_faces_matrix[%d] min diff_matrix = %lf match_value=%lf",j,*diff_matrix_min,match_value);
		#ifdef DEEPDOOR_DEBUG
		save_txt("diff_128matrix.txt",buf);
		#endif

		//没有任何样本匹配
		if(*diff_matrix_min==1) return 2;
	}

	//没有任何样本匹配
	printfs("%s\t%d\t[DEBUG]: dlib_diff_128matrix RET 1",__FILE__,__LINE__);
	return 1;
}

/*dlib_find_mat
*样本库矩阵中查找相似度最大的矩阵
*const std::vector<matrix<float,0,1>> test_faces_matrix						测试矩阵集合
*const std::map<string,std::vector<matrix<float,0,1>>> sample_faces_desc	按照sample_id对应的矩阵map
*char* str_id 路径
*/
int dlib_find_mat(const std::vector<matrix<float,0,1>> test_faces_matrix,std::map<string,std::vector<matrix<float,0,1>>> sample_faces_desc,char* str_id,double* compare)
{
	double diff_matrix_min=1;
	double diff_matrix=1;

	std::map<string,std::vector<matrix<float,0,1>>>::iterator iter;
	for(iter=sample_faces_desc.begin();iter!=sample_faces_desc.end();iter++)
	{
		//printfs("%s\t%d\t[DEBUG]: dlib_find_mat id:%s",__FILE__,__LINE__,iter->first.c_str());
		std::vector<matrix<float,0,1>> id_faces_desc = iter->second;

		snprintf(str_id,127,"%s",iter->first.c_str());

		if(dlib_diff_128matrix(test_faces_matrix,id_faces_desc,&diff_matrix)==0)
		{
			*compare=1-diff_matrix;
			printfs("%s\t%d\t[DEBUG]: dlib_find_mat SUCCESS id:%s %lf",__FILE__,__LINE__,iter->first.c_str(),1-diff_matrix);
			return 0;
		}
		
		if(diff_matrix<diff_matrix_min){
			diff_matrix_min=diff_matrix;
			*compare=1-diff_matrix_min;
		}

		/*std::vector<matrix<float,0,1>>::iterator iter_id;
		for(iter_id=id_faces_desc.begin();iter_id!=id_faces_desc.end();iter_id++)
		{
			
		}*/
	}
	printfs("%s\t%d\t[DEBUG]: dlib_find_mat FAILED",__FILE__,__LINE__);
	return 1;
}

/*单张图片与对应的人脸样本库对比
*dlib_diff_mat
*const char *sample_sub_path	样本子库 NULL全库对比
*const cv::Mat read_mat
*const std::vector<rectangle> rect_faces
*char* str_id 路径
*/
int dlib_diff_mat(const char *sample_sub_path,const cv::Mat read_mat,const std::vector<rectangle> rect_faces,char* str_id,double* compare)
{
	int k=0;

	std::vector<matrix<float,0,1>> test_faces_matrix;
	std::vector<matrix<rgb_pixel>> test_faces;

	cv_load_face_vector(read_mat,rect_faces,test_faces);

	// This call asks the DNN to convert each face image in faces into a 128D vector.
	// In this 128D vector space, images from the same person will be close to each other
	// but vectors from different people will be far apart.  So we can use these vectors to
	// identify if a pair of images are from the same person or from different people.  
	//# 3.描述子提取，128D向量
	anet_type net = net_dnn;
	test_faces_matrix = net(test_faces);

	printfs("%s\t%d\t[DEBUG]: dlib_diff_mat test_faces:%d",__FILE__,__LINE__,test_faces_matrix.size());
	if(test_faces_matrix.size()<1) return 2;

	if(sample_sub_path==NULL)
	{
		//查找并对比识别
		k=dlib_find_mat(test_faces_matrix,sample_faces_desc,str_id,compare);
		return k;
	}

	//查找并对比识别
	std::vector<matrix<float,0,1>> id_faces_desc;
	std::map<string,std::vector<matrix<float,0,1>>>::iterator iter;
	iter=sample_faces_desc.find(sample_sub_path);
	if(iter != sample_faces_desc.end())
	{
		id_faces_desc = iter->second;

		double diff_matrix_min=1;
		k=dlib_diff_128matrix(test_faces_matrix,id_faces_desc,&diff_matrix_min);
		*compare=diff_matrix_min;
		return k;
	}
	else
	{
		return 1;
	}	//cout<<"Do not Find"<<endl;

	return k;
}

//声明获取单路视频的资源数据
WVideo* get_video(string vname);

//#########################################
/*检测授权人相似度
*dlib_diff_auth
*const char *sample_sub_path	样本数据
*const char *camera_name		摄像头名称
*int *face_count				人脸数
*char **buf						返回json数据
*/
int dlib_diff_auth(const char *sample_sub_path,const char* camera_name,char* diff_save_path,int* face_count)
{
	//毫秒 微秒
	double second = 0;
	struct timeval tv;
	struct timeval tv_prv;
	gettimeofday(&tv,NULL);
	tv_prv = tv;

	int RET=0;

	char ms[31]={0};

	//测试数据
	WVideo* video = get_video(camera_name);

	printfs("%s\t%d\t[DEBUG]: dlib_diff_auth camera_name:%s",__FILE__,__LINE__,camera_name);

	if(video==NULL) return -1;

	//读取的图像
	cv::Mat read_mat;
	//图像大小标准化
	cv::Mat resize_mat,chip_mat,chip_gray,equalized_mat;
	//测试库
	cv::Mat cache_mat[LOOP_FS];
	//获取缓存帧
	video->get_cache_mat((cv::Mat *)&cache_mat);

	for(int i=0;i<LOOP_FS;i++)
	{
		read_mat = cache_mat[i];

		//图像大小标准化
		//cv::resize(read_mat, resize_mat, cv::Size(270, 360),0,0,CV_INTER_CUBIC);

		//对比
		//# 1.人脸检测 多个人脸
		std::vector<rectangle> rect_faces;

		//rect_faces = ff_detector(img);//检测人脸，获得边界框

		//int faces_count = ex_frontal_face_detector(rect_faces,read_mat,chip_mat,chip_gray,equalized_mat);
		*face_count = ex_frontal_face_detector(rect_faces,read_mat,chip_mat,chip_gray,equalized_mat);

		//printfs("%s\t%d\t[DEBUG]: auth:%d",__FILE__,__LINE__,k);
		/*
		//>>检查缓存帧是否更新
		//设置保存路径
		sprintf(diff_save_path, "%s/diff_auth_%s_%d.jpg",save_path,camera_name,i);

		//保存检测图片
		cv::imwrite(diff_save_path, chip_mat);
		printfs("%s\t%d\t[DEBUG]: save to:%s",__FILE__,__LINE__,diff_save_path);
			 //<<检查缓存帧是否更新
			 */
		//RET==0 验证合法人员记录
		char str_id[128]={0};
		double compare;
		if((RET=dlib_diff_mat(sample_sub_path,read_mat,rect_faces,str_id,&compare))==0)
		{
			//if(k++ > 0)
			//{
				//>>
				sys_ms((char *)&ms);
				//设置保存路径
				sprintf(diff_save_path, "%s/ok_%s_%s.jpg",save_path,camera_name,ms);

				//保存检测图片
				cv::imwrite(diff_save_path, chip_mat);
				printfs("%s\t%d\t[DEBUG]: ok save:%s",__FILE__,__LINE__,diff_save_path);
				//<<

				//>>
				gettimeofday(&tv,NULL);
				//秒 微秒
				second = (tv.tv_sec-tv_prv.tv_sec)*1000+(double)(tv.tv_usec-tv_prv.tv_usec)/1000;//毫秒
				printfs("%s\t%d\t[DEBUG]:单张识别时间 (Seconds:%ld Microseconds:%ld) - (prv Seconds:%ld prv Microseconds:%ld) 耗时毫秒:%lf",__FILE__,__LINE__,tv.tv_sec,tv.tv_usec,tv_prv.tv_sec,tv_prv.tv_usec,second);
				tv_prv = tv;
				//<<
				return 0;
			//}			
		}

		//RET==1 记录异常信息并返回
		if(RET==1)
		{
			//if(k++ > 0)
			//{
				//>>
				sys_ms((char *)&ms);
				//设置保存路径
				sprintf(diff_save_path, "%s/warn_%s_%s.jpg",save_path,camera_name,ms);

				//保存检测图片
				cv::imwrite(diff_save_path, chip_mat);
				printfs("%s\t%d\t[DEBUG]: warn save:%s",__FILE__,__LINE__,diff_save_path);
				//<<

				//>>
				gettimeofday(&tv,NULL);
				//秒 微秒
				second = (tv.tv_sec-tv_prv.tv_sec)*1000+(double)(tv.tv_usec-tv_prv.tv_usec)/1000;//毫秒
				printfs("%s\t%d\t[DEBUG]:警告识别结束时间 (Seconds:%ld Microseconds:%ld) - (prv Seconds:%ld prv Microseconds:%ld) 耗时毫秒:%lf",__FILE__,__LINE__,tv.tv_sec,tv.tv_usec,tv_prv.tv_sec,tv_prv.tv_usec,second);
				tv_prv = tv;
				//<<
				return 1;
			//}
		}
	}

	//>>
	gettimeofday(&tv,NULL);
	//秒 微秒
	second = (tv.tv_sec-tv_prv.tv_sec)*1000+(double)(tv.tv_usec-tv_prv.tv_usec)/1000;//毫秒
	printfs("%s\t%d\t[DEBUG]:人脸识别验证总时间 (Seconds:%ld Microseconds:%ld) - (prv Seconds:%ld prv Microseconds:%ld) 耗时毫秒:%lf",__FILE__,__LINE__,tv.tv_sec,tv.tv_usec,tv_prv.tv_sec,tv_prv.tv_usec,second);
	tv_prv = tv;
	//<<
	printfs("%s\t%d\t[DEBUG]: auth must 2:%d",__FILE__,__LINE__,RET);
	return 2;
}

/*主模式验证
*dlib_master_auth
*const char *camera_name	摄像头名称
*char* diff_save_path		检测后保存文件路径
*int*  face_count			人脸数
*char* str_id				检测到的id
*double* compare			相似度
*/
int dlib_master_auth(const char* camera_name,char* diff_save_path,int* face_count,char* str_id,double* compare)
{
	//毫秒 微秒
	double second = 0;
	struct timeval tv;
	struct timeval tv_prv;
	gettimeofday(&tv,NULL);
	tv_prv = tv;

	int RET=0;

	char ms[31]={0};

	//测试数据
	WVideo* video = get_video(camera_name);

	printfs("%s\t%d\t[DEBUG]: dlib_diff_auth camera_name:%s",__FILE__,__LINE__,camera_name);

	if(video==NULL) return -1;

	//读取的图像
	cv::Mat read_mat;
	//图像大小标准化
	cv::Mat resize_mat,chip_mat,chip_gray,equalized_mat;
	//测试库
	cv::Mat cache_mat[LOOP_FS];
	//获取缓存帧
	video->get_cache_mat((cv::Mat *)&cache_mat);

	for(int i=0;i<LOOP_FS;i++)
	{
		read_mat = cache_mat[i];

		//图像大小标准化
		//cv::resize(read_mat, resize_mat, cv::Size(270, 360),0,0,CV_INTER_CUBIC);

		//对比
		//# 1.人脸检测 多个人脸
		std::vector<rectangle> rect_faces;

		//rect_faces = ff_detector(img);//检测人脸，获得边界框

		//int faces_count = ex_frontal_face_detector(rect_faces,read_mat,chip_mat,chip_gray,equalized_mat);
		*face_count = ex_frontal_face_detector(rect_faces,read_mat,chip_mat,chip_gray,equalized_mat);

		//printfs("%s\t%d\t[DEBUG]: auth:%d",__FILE__,__LINE__,k);
		/*
		//>>检查缓存帧是否更新
		//设置保存路径
		sprintf(diff_save_path, "%s/diff_auth_%s_%d.jpg",save_path,camera_name,i);

		//保存检测图片
		cv::imwrite(diff_save_path, chip_mat);
		printfs("%s\t%d\t[DEBUG]: save to:%s",__FILE__,__LINE__,diff_save_path);
			 //<<检查缓存帧是否更新
			 */
		//RET==0 验证合法人员记录
		if((RET=dlib_diff_mat(NULL,read_mat,rect_faces,str_id,compare))==0)
		{
			//if(k++ > 0)
			//{
				//>>
				sys_ms((char *)&ms);
				//设置保存路径
				sprintf(diff_save_path, "%s/ok_%s_%s.jpg",save_path,camera_name,ms);

				//保存检测图片
				cv::imwrite(diff_save_path, chip_mat);
				printfs("%s\t%d\t[DEBUG]: ok save:%s",__FILE__,__LINE__,diff_save_path);
				//<<

				//>>
				gettimeofday(&tv,NULL);
				//秒 微秒
				second = (tv.tv_sec-tv_prv.tv_sec)+(double)(tv.tv_usec-tv_prv.tv_usec)/1000000;//秒
				printfs("%s\t%d\t[DEBUG]:单张识别时间 (Seconds:%ld Microseconds:%ld) - (prv Seconds:%ld prv Microseconds:%ld) 耗时秒:%lf",__FILE__,__LINE__,tv.tv_sec,tv.tv_usec,tv_prv.tv_sec,tv_prv.tv_usec,second);
				tv_prv = tv;
				//<<
				return 0;
			//}
		}

		//RET==1 记录异常信息并返回
		if(RET==1)
		{
			//if(k++ > 0)
			//{
				//>>
				sys_ms((char *)&ms);
				//设置保存路径
				sprintf(diff_save_path, "%s/warn_%s_%s.jpg",save_path,camera_name,ms);

				//保存检测图片
				cv::imwrite(diff_save_path, chip_mat);
				printfs("%s\t%d\t[DEBUG]: warn save:%s",__FILE__,__LINE__,diff_save_path);
				//<<

				//>>
				gettimeofday(&tv,NULL);
				//秒 微秒
				second = (tv.tv_sec-tv_prv.tv_sec)+(double)(tv.tv_usec-tv_prv.tv_usec)/1000000;//秒
				printfs("%s\t%d\t[DEBUG]:警告识别结束时间 (Seconds:%ld Microseconds:%ld) - (prv Seconds:%ld prv Microseconds:%ld) 耗时毫秒:%lf",__FILE__,__LINE__,tv.tv_sec,tv.tv_usec,tv_prv.tv_sec,tv_prv.tv_usec,second);
				tv_prv = tv;
				//<<
				return 1;
			//}
		}
	}

	//>>
	gettimeofday(&tv,NULL);
	//秒 微秒
	second = (tv.tv_sec-tv_prv.tv_sec)+(double)(tv.tv_usec-tv_prv.tv_usec)/1000000;//秒
	printfs("%s\t%d\t[DEBUG]:人脸识别验证总时间 (Seconds:%ld Microseconds:%ld) - (prv Seconds:%ld prv Microseconds:%ld) 耗时毫秒:%lf",__FILE__,__LINE__,tv.tv_sec,tv.tv_usec,tv_prv.tv_sec,tv_prv.tv_usec,second);
	tv_prv = tv;
	//<<

	printfs("%s\t%d\t[DEBUG]: auth must 2:%d",__FILE__,__LINE__,RET);
	return 2;
}

//############
void process_video(cv::VideoCapture cap,char *video_name,char *video_url,WVideo *video)
{
	int loop=0;
	//尝试一直连接
	/*while (!cap.isOpened())
	{
		printfs("%s\t%d\t[DEBUG]: try Video URL:(%s) %d",__FILE__,__LINE__,video_url,loop++);
		usleep(30*1000);
		if(loop==1000) exit(0);
	}*/
	char video_save_name[128];
	snprintf(video_save_name,sizeof(video_save_name),"%s.avi",video_name);
	string video_save_names = video_save_name;

	//cv::namedWindow(video->getVname().c_str());
	//毫秒 微秒
	double second = 0;
	struct timeval tv;
	struct timeval tv_prv;
	gettimeofday(&tv,NULL);
	tv_prv = tv;
	int ck=0;//检查状态

	#ifdef DEEPDOOR_DEBUG
	cv::VideoWriter writer(video_save_names, CV_FOURCC('M', 'J', 'P', 'G'), 25.0, cv::Size(cap.get(CV_CAP_PROP_FRAME_WIDTH),cap.get(CV_CAP_PROP_FRAME_HEIGHT)));
	//cv::VideoWriter writer(video_save_names, CV_FOURCC('M', 'J', 'P', 'G'), 25.0, cv::Size(1280,720));
	#endif
	char buf[BUFFER_SIZE] = { 0 };

	int dk = 0;
	// Grab and process frames until the main window is closed by the user.
	cv::Mat read_mat;
	//图像大小标准化
	cv::Mat resize_mat,chip_mat,chip_gray,equalized_mat;

	int head_fr = 0;
	while(1)
	{
		// read frame
		if (!cap.read(read_mat))
		{
			break;
		}

		//图像大小标准化
		//src：输入图像
		//dst：输出图像
		//interpolation：差值方法
		//	CV_INTER_NN - 最近邻差值
		//	CV_INTER_LINEAR -  双线性差值 (缺省使用)
		//	CV_INTER_AREA -  使用象素关系重采样。当图像缩小时候，该方法可以避免波纹出现。当图像放大时，类似于  CV_INTER_NN  方法
		//	CV_INTER_CUBIC -  立方差值
		cv::resize(read_mat, resize_mat,cv::Size(m_width,m_height),0,0,CV_INTER_CUBIC);
		//初始化缓存帧
		if(head_fr==0)
		{
			head_fr=1;
			video->init_cache_mat(resize_mat);
		}

		//人脸检测
		std::vector<rectangle> rect_faces;
		dk=ex_frontal_face_detector(rect_faces,resize_mat,chip_mat,chip_gray,equalized_mat);

		printfs("%s\t%d\t[DEBUG]: video_name:%s dx:%d",__FILE__,__LINE__,video_name,dk);
		//检测到人脸
		if(dk>0)
		{
			//缓存图像帧
			video->set_cache_mat(0,resize_mat);

		 //video->set_cache_mat(k,resize_mat);
			//if(k++==LOOP_FS-1) k=0;

			/*//>>yes
			//设置保存路径
			sprintf(buf, "%s/read_yes_%s_%d.jpg",save_path,video_name,k);
			//保存检测图片
			cv::imwrite(buf, resize_mat);

			printfs("%s\t%d\t[DEBUG]:read cache save:%s",__FILE__,__LINE__,buf);
			//<<*/

			printfs("%s\t%d\t[DEBUG]:ck:%d",__FILE__,__LINE__,ck);
			if(ck==0)
			{
				gettimeofday(&tv,NULL);
				//秒 微秒
				second = (tv.tv_sec-tv_prv.tv_sec)+(double)(tv.tv_usec-tv_prv.tv_usec)/1000000;//秒
				
				printfs("%s\t%d\t[DEBUG]:check second:%lf",__FILE__,__LINE__,second);
				if(second<10){
					printfs("%s\t%d\t[DEBUG]:goto end_process",__FILE__,__LINE__);
					goto end_process;
				}
				//else
				//	k=100;
			}

			printfs("%s\t%d\t[DEBUG]:check_faces_process ck:%d",__FILE__,__LINE__,ck);
			//检测人脸后的处理0|1
			ck=check_faces_process((const char *)video_name,job_type);//0 1 2
			printfs("%s\t%d\t[DEBUG]:check_faces_process ck:%d",__FILE__,__LINE__,ck);
			//重置计时器
			if(ck==0)
			{
				gettimeofday(&tv,NULL);
				tv_prv = tv;
			}
		}
/*			else //未检测到人脸
		{
			//>>no
			//设置保存路径
			sprintf(buf, "%s/no_%s_%d.jpg",save_path,video->getVname().c_str(),k);
			//保存检测图片
			cv::imwrite(buf, resize_mat);
			printfs("%s\t%d\t[DEBUG]:no save:%s",__FILE__,__LINE__,buf);
			//<<
		}

		k++;
*/
		end_process:
		#ifdef DEEPDOOR_DEBUG
		//save read_mat
		writer << read_mat;

		//show chip_mat

		//Display it all on the screen
		//cv::imshow("直方图均匀化", equalized_mat);
		//cv::imshow("灰度化", chip_gray);

		cv::imshow(video_name, chip_mat);

		if (cv::waitKey(30) == 'q')
		{
			break;
		}
		#endif

		//释放CPU 20微秒
		usleep(20*1000);
	}
	printfs("%s\t%d\t[DEBUG]: %s play end",__FILE__,__LINE__,video_url);

	#ifdef DEEPDOOR_DEBUG
	cv::destroyWindow(video_name);
	//cv::destroyAllWindows();
	#endif
}

//############
/*single_video
*处理单路视频
*void *v 子线程资源数据
*/
void* single_video(void *v)
{
	WVideo *video = (WVideo *)v;

	char video_name[512];
	char video_url[512];

	snprintf(video_name,sizeof(video_name),"%s",video->getVname().c_str());
	snprintf(video_url,sizeof(video_url),"%s",video->getUrl().c_str());

	printfs("%s\t%d\t[DEBUG]:video_name(%s) video_url:(%s)",__FILE__,__LINE__,video_name,video_url);


	if(strstr(video_url,"/dev/video")!=NULL)
	{
		//printf("video[%c]\n",video_url[10]);
		
		int i = (int)(video_url[10]-48);
		cv::VideoCapture cap(i);//打开设备
		if(cap.isOpened())  
		{
			//snprintf(video_name,sizeof(video_name),"video%d",i);
			snprintf(video_url,sizeof(video_url),"video%d",i);

			process_video(cap,video_name,video_url,video);
		}
	}
	else
	{
		cv::VideoCapture cap(video_url); //打开文件
		
		if(cap.isOpened())  
		{
			process_video(cap,video_name,video_url,video);
		}
	}

	printfs("%s\t%d\t[DEBUG]: %s single_video finsh",__FILE__,__LINE__,video_url);
	//delete video;
}

//##########多路摄像头处理###############
//多路摄像头结构管理
WVideo **multiple_video = NULL;

//多路视频采集入口
void multiple_video_service()
{
	//printf("kc=%d\n",kc);
	multiple_video = (WVideo **) malloc(sizeof(WVideo *) * kc);
	
	for(int i=0;i<kc;i++)
	{
		printfs("%s\t%d\t[DEBUG]:%s=%s",__FILE__,__LINE__,kv[i][0],kv[i][1]);

		string vname(kv[i][0]);
		string url(kv[i][1]);

		//赋值对象
		*(multiple_video+i) = new WVideo(url,vname);

		//single_video 加入任务队列
		//single_video_gui
		//single_video_service
		//single_video
		if(thpool_add_work(&thpool,single_video, (void*) *(multiple_video+i))<0)
		{
			printfs("%s\t%d\t[DEBUG]:thpool full",__FILE__,__LINE__);
			//send_full(csockfd);
		}
		usleep(500);
	}
}

//多路视频采集回收
void multiple_video_destroy()
{
	for(int i=0;i<kc;i++)
	{
		printfs("%s\t%d\t[DEBUG]:destroy %s=%s",__FILE__,__LINE__,kv[i][0],kv[i][1]);
		delete *(multiple_video+i);
	}

	free(multiple_video);
}

//多路视频采集回收
WVideo* get_video(string vname)
{
	WVideo *video = NULL;
	string val;
	printfs("%s\t%d\t[DEBUG]: get_video",__FILE__,__LINE__);

	char v_name0[512];
	char v_name1[512];
	memset(v_name0,'\0',sizeof(v_name0));
	memset(v_name1,'\0',sizeof(v_name1));
	
	snprintf(v_name0,sizeof(v_name0),"%s",vname.c_str());
	for(int i=0;i<kc;i++)
	{
		//赋值对象
		video = *(multiple_video+i);
		val = video->getVname();

		snprintf(v_name1,sizeof(v_name1),"%s",val.c_str());
		
		printfs("%s\t%d\t[DEBUG]: get_video vname:%s vname:%s check:%d",__FILE__,__LINE__,v_name1,v_name0,strcmp(v_name0,v_name1));

		if(!strcmp(v_name0,v_name1))
		return video;

		//printfs("%s\t%d\t[DEBUG]:%s=%s",__FILE__,__LINE__,kv[i][0],kv[i][1]);
	}

	return NULL;
}

//socket客户端服务
void* thservice(void *v)
{
	int csockfd = *(int *)v;

	pthread_t pid = getpid();
	pthread_t tid = pthread_self();

	printfs("%s\t%d\t[DEBUG]:thservice 开始:csockfd:%d pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,csockfd,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	//执行 命令处理
	recv_shake_hand(csockfd);

	//关闭socket
	close(csockfd);
	printfs("%s\t%d\t[DEBUG]:thservice 结束:csockfd:%d pid:%u pthread_id:%u (0x%x)",__FILE__,__LINE__,csockfd,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
}

//服务端已经满载
void send_full(const int csockfd){
	uchar buf[5];
	snprintf((char *)buf,5,"%s\n","END");
	send_rep_txt(csockfd,(char *)buf);
	//关闭socket
	close(csockfd);
}

//协议命令处理
int recv_cmd(const int csockfd,const char *req)
{
	if(req==NULL || strlen(req)<1)
		return -1;


	int RET = -1;

	int bsize = 2048;

	char buf[bsize+1];
	
	int protocol = 0;
	int cmd = 100;
	char cmd2[bsize+1];
	char rep_cmd2[bsize+1];
	char reps[bsize+1];
	
	char *s=NULL;
	
	uchar *us=NULL;
	
	int k = 0;
	memset(buf,'\0',bsize+1);
	memcpy(buf,req,bsize);

	memset(reps,'\0',bsize+1);
	memset(rep_cmd2,'\0',bsize+1);

	//printfs("%s\t%d\t[DEBUG]: recv_cmd cmd:%s",__FILE__,__LINE__,buf);
	
	char *token=strtok(buf,":");
	while(token!=NULL){
		//printfs("%s\t%d\t[DEBUG]: recv_cmd ERROR k:%d token:%s",__FILE__,__LINE__,k,token);
		switch(k)
		{
			case 0:
				if(!strcmp(token,"01"))
					protocol=1;
				else
					protocol=0;

				strcat(reps,token);
				break;
			case 1:
				cmd = atoi(token);

				strcat(reps,":");
				strcat(reps,token);
				break;
			case 2:
				bsize = strlen(token);
				memset(cmd2,'\0',bsize+1);
				memcpy(cmd2,token,bsize);

				strcat(reps,":");
				strcat(reps,"%s");
				break;
			case 3:
				if(!strcmp(token,"02"))
					protocol=protocol&1;
				else
					protocol=protocol&0;

				strcat(reps,":");
				strcat(reps,token);
				break;
			default:
				break;
		}

		token=strtok(NULL,":");
		k++;
	}

	printfs("%s\t%d\t[DEBUG]: recv_cmd k:%d protocol:%d",__FILE__,__LINE__,k,protocol);

	if(!protocol || k!=4)
		return -1;

	int id=0;

	int faces_count=0;
	long ack=0;
	char str_id[128];
	char camera_name[128];
	char save_path_ch[512];
	WVideo* video = NULL;
	c_recogn_t c_recogn;
	//检查连接
	std::list<c_recogn_t>::iterator it;

	printfs("%s\t%d\t[DEBUG]: recv_cmd cmd:%d",__FILE__,__LINE__,cmd);
	switch(cmd)
		{
			case 130://01:130:123456,/tmp,camera1:02 开启会话
				//130 开启会话 图片保存路径
				k = 0;
				printfs("%s\t%d\t[DEBUG]: recv_cmd k:%d protocol:%d",__FILE__,__LINE__,k,protocol);

				printfs("%s\t%d\t[DEBUG]: recv_cmd cmd2:%s",__FILE__,__LINE__,cmd2);
				token=strtok(cmd2,",");
				while(token!=NULL){
					//printfs("%s\t%d\t[DEBUG]: recv_cmd cmd2 k:%d token:%s",__FILE__,__LINE__,k,token);
					switch(k)
					{
						case 0:
							ack = atol(token);
							printfs("%s\t%d\t[DEBUG]:recv_cmd cmd2 ack:%ld",__FILE__,__LINE__,ack);
							break;
						case 1:
							snprintf(save_path,sizeof(save_path),"%s",token);
							mkdir_p(save_path);
							printfs("%s\t%d\t[DEBUG]:recv_cmd cmd2 save_path:%s",__FILE__,__LINE__,save_path);
							break;
						case 2:
							snprintf(camera_name,sizeof(camera_name),"%s",token);
							printfs("%s\t%d\t[DEBUG]:recv_cmd cmd2 camera_name:%s",__FILE__,__LINE__,camera_name);
							break;
						default:
							//printfs("%s\t%d\t[DEBUG]:cmd2 file_name:%s",__FILE__,__LINE__,token);
							//send_nop(task_match->csockfd);
							break;
					}

					token=strtok(NULL,",");
					k++;
				}

				pthread_rwlock_rdlock(&rwlock_csocket_pool);
				//printfs("%s\t%d\t[DEBUG]:recv_cmd csockfd:%d cs_pool:%d",__FILE__,__LINE__,csockfd,cs_pool.size());
				for (int i = 0; i < cs_pool.size();i++)
				{
					//已经开启会话
					//if (strcmp(cs_pool[i].camera_name,camera_name)==0)
					if (cs_pool[i].csocket==csockfd)
					{
						cs_pool[i].cmd=130;
						cs_pool[i].ack=ack;
						snprintf(cs_pool[i].camera_name,sizeof(cs_pool[i].camera_name),"%s",camera_name);
						snprintf(cs_pool[i].req,sizeof(cs_pool[i].req),"%s",req);
						snprintf(cs_pool[i].res,sizeof(cs_pool[i].res),"%s\n",req);
						//std::memcpy(&c_recogn,&cs_pool[i],sizeof(c_recogn_t));
						k=-100;
						break;
					}
				}
				pthread_rwlock_unlock(&rwlock_csocket_pool);

				if(k==-100) goto send;

				//加入队列
				c_recogn.csocket=csockfd;
				c_recogn.ack=ack;
				c_recogn.cmd=cmd;

				snprintf(c_recogn.camera_name,sizeof(c_recogn.camera_name),"%s",camera_name);

				snprintf(c_recogn.req,sizeof(c_recogn.req),"%s",req);
				snprintf(c_recogn.res,sizeof(c_recogn.res),"%s\n",req);

				pthread_rwlock_wrlock(&rwlock_csocket_pool);
				cs_pool.push_back(c_recogn);
				pthread_rwlock_unlock(&rwlock_csocket_pool);

				send:
				k=0;
				//k=send_rep_txt(csockfd,c_recogn.res);
				printfs("%s\t%d\t[DEBUG]:recv_cmd csockfd:%d cs_pool:%d",__FILE__,__LINE__,csockfd,cs_pool.size());

				break;
			case 131://01:131:0:02 心跳帧
				printfs("%s\t%d\t[DEBUG]: recv_cmd cmd2 k:%d cmd2:%s",__FILE__,__LINE__,k,cmd2);
				break;
			case 132://01:131:02 关闭会话
				//snprintf(buf,10,"%s\n",kid);
				k=-2;
				//退出后统一移除队列
				snprintf(buf,10,"success\n");
				send_rep_txt(csockfd,buf);
				break;
			case 133://01:133:02 刷新样本
				//133 过时命令
				k = 0;
				snprintf(buf,10,"success\n");
				 send_rep_txt(csockfd,buf);
				break;
			case 134://01:134:ack,4,camera2:02 人脸识别
				//134 刷新样本 图片保存路径
				k = 0;
				printfs("%s\t%d\t[DEBUG]: recv_cmd cmd2:%s",__FILE__,__LINE__,cmd2);
				
				token=strtok(cmd2,",");
				while(token!=NULL){
					//printfs("%s\t%d\t[DEBUG]: recv_cmd cmd2 k:%d token:%s",__FILE__,__LINE__,k,token);
					switch(k)
					{
						case 0:
							 ack = atol(token);
							//printfs("%s\t%d\t[DEBUG]:recv_cmd cmd2 task_id:%s",__FILE__,__LINE__,task_match->task_id);
							break;
						case 1:
							 snprintf(str_id,sizeof(str_id),"%s",token);
							//printfs("%s\t%d\t[DEBUG]:recv_cmd cmd2 task_id:%s",__FILE__,__LINE__,task_match->task_id);
							break;
						case 2:
							 snprintf(camera_name,sizeof(camera_name),"%s",token);
							//printfs("%s\t%d\t[DEBUG]:recv_cmd cmd2 task_id:%s",__FILE__,__LINE__,task_match->task_id);
							break;
						default:
							//printfs("%s\t%d\t[DEBUG]:cmd2 file_name:%s",__FILE__,__LINE__,token);
							//send_nop(task_match->csockfd);
							break;
					}

					token=strtok(NULL,",");
					k++;
				}
				printfs("%s\t%d\t[DEBUG]: k:%d id:%s camera_name:%s",__FILE__,__LINE__,k,str_id,camera_name);
				id = atoi(str_id);

				printfs("%s\t%d\t[DEBUG]: k:%d id:%d camera_name:%s",__FILE__,__LINE__,k,id,camera_name);
				if(k>0)
				{
					snprintf(buf,sizeof(buf),"%s/%s",sample_path,str_id);//sample_sub_path 保留根据id减小搜索样本数 逐步废弃
					if(id<0 || dlib_diff_auth(buf,camera_name,save_path_ch,&faces_count)!=0)
					{
						//printfs("%s\t%d\t[DEBUG]:识别失败",__FILE__,__LINE__);
						snprintf(buf,sizeof(buf),"{'cmd':'134','ack':'%ld','type':'1','captureName':'%s','filePath':'%s','faceNumber':%d,data:[]}\n",ack,camera_name,save_path_ch,faces_count);
					}
					else
					{
						//printfs("%s\t%d\t[DEBUG]:识别成功",__FILE__,__LINE__);
						snprintf(buf,sizeof(buf),"{'cmd':'134','ack':'%ld','type':'0','captureName':'%s','filePath':'%s','faceNumber':%d,data:[{'id':%d,'compare':%lf}]}\n",ack,camera_name,save_path_ch,faces_count,id,0.7);
					}
					//printfs("%s\t%d\t[DEBUG]:send_rep_txt buf:%s",__FILE__,__LINE__,buf);
					send_rep_txt(csockfd,buf);
				}
				break;
			case 135://01:135:id,file_path:02 加入样本
				//135 加入样本
				k = 0;
				printfs("%s\t%d\t[DEBUG]: recv_cmd cmd2:%s",__FILE__,__LINE__,cmd2);

				token=strtok(cmd2,",");
				while(token!=NULL){
					//printfs("%s\t%d\t[DEBUG]: recv_cmd cmd2 k:%d token:%s",__FILE__,__LINE__,k,token);
					switch(k)
					{
						case 0:
							snprintf(str_id,sizeof(str_id),"%s",token);
							//printfs("%s\t%d\t[DEBUG]:recv_cmd cmd2 task_id:%s",__FILE__,__LINE__,task_match->task_id);
							break;
						case 1:
							snprintf(save_path_ch,sizeof(save_path_ch),"%s/%s",save_path,token);
							//printfs("%s\t%d\t[DEBUG]:recv_cmd cmd2 task_id:%s",__FILE__,__LINE__,task_match->task_id);
							break;
						default:
							break;
					}

					token=strtok(NULL,",");
					k++;
				}
				printfs("%s\t%d\t[DEBUG]: recv_cmd cmd2:%s %d",__FILE__,__LINE__,cmd2,k);
				if(k>0)
				{
					//add_sample_id
					cv_add_sample_id(str_id,save_path_ch);

					snprintf(buf,10,"success\n");
					send_rep_txt(csockfd,buf);
				}
				k=0;
				break;
			default:
				break;
		}
	//printfs("%s\t%d\t[DEBUG]: recv_cmd ERROR k:%d token:%s",__FILE__,__LINE__,k,token);
	return k;
}

//MSG_DONTWAIT 阻塞
//MSG_PEEK 非阻塞 不清空TCP缓存
//0
//协议握手
int recv_shake_hand(int csockfd)
{
	char buf[BUFFER_SIZE+1];
	int recv_len = 0;
	int *web_end = (int *)buf;
	int wsh = 1;
	while(1)
	{
		//阻塞接收
		memset(buf,'\0',BUFFER_SIZE+1);
		recv_len=recv(csockfd, buf, BUFFER_SIZE, 0);//阻塞的模式
		//printfs("%s\t%d\t[DEBUG]: received data:%s k=%d",__FILE__,__LINE__,buf,k);

		//客户端正常关闭
		if(recv_len==0)
		{
			printfs("%s\t%d\t[DEBUG]:客户端关闭 received error recv_len:%d csockfd:%d",__FILE__,__LINE__,recv_len,csockfd);
			break;
		}else if(recv_len<0)
		{
			if(recv_len == EAGAIN)
				goto next;

			printfs("%s\t%d\t[DEBUG]:客户端关闭 received error recv_len:%d csockfd:%d",__FILE__,__LINE__,recv_len,csockfd);
			break;
		}

		//printfs("%s\t%d\t[DEBUG]: received data:%s from %s",__FILE__,__LINE__,buf,inet_ntoa(client[i].addr.sin_addr));

		//接收的数据处理

		//去掉换行符
		brline(buf,recv_len);

		//自定义协议通信
		printfs("%s\t%d\t[DEBUG]: received recv_cmd:>%s< %d",__FILE__,__LINE__,buf,strlen(buf));

		//协议正确处理后等待处理下一次的接收数据
		//协议不正确退出
		if(recv_cmd(csockfd,buf) < 0)
			break;

		next:
		//释放CPU
		usleep(200*1000);
	}

	//csockfd 移出连接池
	pthread_rwlock_wrlock(&rwlock_csocket_pool);
	std::vector<c_recogn_t>::iterator it;
	for(it=cs_pool.begin();it!=cs_pool.end();)  
	{  
		if( (*it).csocket == csockfd)  
			it = cs_pool.erase(it);  
		else  
			it++;
	}  
	pthread_rwlock_unlock(&rwlock_csocket_pool);

	printfs("%s\t%d\t[DEBUG]: recv_shake_hand close recv_len:%d",__FILE__,__LINE__,recv_len);
	return recv_len;
}

int send_rep_txt(const int sockfd,const char *buf)
{
	int k=0;
	if((k=send(sockfd,buf,strlen(buf),0))<1)
	{
		printfs("%s\t%d\t[DEBUG]:send_rep_txt response ERROR:%d buf:%s",__FILE__,__LINE__,k,buf);
		return -1;
	}
	printfs("%s\t%d\t[DEBUG]:send_rep_txt response Success:%d buf:%s",__FILE__,__LINE__,k,buf);

	//send_eof(sockfd);
	//printfs("%s\t%d\t[DEBUG]:send_nop k:%d",__FILE__,__LINE__,k);
	return 0;
}

void start()
{
	int ssockfd=0;
	int csockfd=0;

	struct sockaddr_in my_addr; /* 本机地址信息 */
	struct sockaddr_in remote_addr; /* 客户端地址信息 */
	struct timeval tv;

	static int timeout = 31536000;//15秒

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	if ((ssockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error");
		exit(1);
	}

	//close now
	struct linger so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	setsockopt(ssockfd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof so_linger);

	//设置发送超时
	setsockopt(ssockfd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(struct timeval));
	//设置接收超时
	setsockopt(ssockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(struct timeval));

	//非阻塞模式
	//int flags = fcntl(ssockfd, F_GETFL, 0);
	//fcntl(ssockfd,F_GETFL,flags|O_NONBLOCK );

	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(SERVPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);
	if (bind(ssockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind error");
		exit(1);
	}
	if (listen(ssockfd, BACKLOG) == -1) {
		perror("listen error");
		exit(1);
	}

	unsigned int sin_size = sizeof(struct sockaddr_in);
	int c = 0;
	
	fd_set fset;
	while(1)
	{
		pthread_mutex_lock(&s_mutex);
		if(stop) break;//exit
		pthread_mutex_unlock(&s_mutex);

		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		FD_ZERO(&fset);
		FD_SET(ssockfd,&fset);
		c=select(ssockfd+1,&fset,NULL,NULL,&tv);

		if(c==0)
		{
			usleep(100);//每秒10000次处理
			continue;
		}
		else if(c<0)
		{
			break;
		}

		if(FD_ISSET(ssockfd,&fset)) // new connection
		{
			//传值到客户端
			if ((csockfd = accept(ssockfd, (struct sockaddr *)&remote_addr,&sin_size)) == -1)
			{
				perror("accept error");
				if(errno == EAGAIN)
					continue;
			}

			printfs("%s\t%d\t[DEBUG]:received a connection from %s",__FILE__,__LINE__,inet_ntoa(remote_addr.sin_addr));
			//thservice 加入任务队列
			if(thpool_add_work(&thpool,thservice, (void*)&csockfd)<0)
			{
				printfs("%s\t%d\t[DEBUG]:thpool full",__FILE__,__LINE__);
				send_full(csockfd);
			}
		}
		//sleep(1);
	}

	close(ssockfd);
	printf("start exit\n");
}

void usage()
{
	fprintf(stdout, "\t -logf deepdoor.log -c deepdoor.cnf -haar haarcascades/haarcascade_frontalface_alt2.xml -sp faces -sv /tmp -68 shape_predictor_68_face_landmarks.dat -v1 dlib_face_recognition_resnet_model_v1.dat -sv /tmp -mhv 0.4 -email mingde_face@126.com\n");

	fprintf(stdout, "\t -logf deepdoor.log -c deepdoor.cnf -haar haarcascades/haarcascade_frontalface_alt.xml -68 shape_predictor_68_face_landmarks.dat -v1 dlib_face_recognition_resnet_model_v1.dat -sp faces -sv /tmp -mhv 0.4\n");

	fprintf(stdout, "\t -logf /usr/local/share/applications/deepdoor-1.1/deepdoor.log -c /usr/local/share/applications/deepdoor-1.1/deepdoor.cnf -haar /usr/local/share/applications/deepdoor-1.1/haarcascades/haarcascade_frontalface_alt2.xml -sp /usr/local/share/applications/deepdoor-1.1/faces -68 /usr/local/share/applications/deepdoor-1.1/shape_predictor_68_face_landmarks.dat -v1 /usr/local/share/applications/deepdoor-1.1/dlib_face_recognition_resnet_model_v1.dat -sv /tmp -mhv 0.4 -email mingde_face@126.com\n");

	fprintf(stdout, "\t -logf /usr/local/share/applications/deepdoor-1.1/deepdoor.log -c /usr/local/share/applications/deepdoor-1.1/deepdoor.cnf -haar /usr/local/share/applications/deepdoor-1.1/haarcascades/haarcascade_frontalface_alt2.xml -sp /usr/local/share/applications/deepdoor-1.1/faces -68 /usr/local/share/applications/deepdoor-1.1/shape_predictor_68_face_landmarks.dat -v1 /usr/local/share/applications/deepdoor-1.1/dlib_face_recognition_resnet_model_v1.dat -sv /tmp -mhv 0.4\n");

	fprintf(stdout, "\t [-sf 8] [-rd]\n");
	fprintf(stdout, "\t -sp [sample faces default:/usr/local/share/personnel/photo]\n");
	fprintf(stdout, "\t -sv [savepath ]\n");
	//fprintf(stdout, "\t1.avi shape_predictor_68_face_landmarks.dat \n");
	//fprintf(stdout, "\t[1.avi|rtsp://192.168.1.10/main] 视频\n");
	//fprintf(stdout, "\tshape_predictor_68_face_landmarks.dat 68点模型文件\n");
}

int test_cv_sample(string sample_path);
int test_dlib_diff_path(char *test_path,std::vector<rectangle> rect_faces,char *sample_path);

int main(int argc, char** argv)
try
{
	//环境变量
	/*for( int i = 0; envp[i] != NULL; ++i )
	{
		printf( "%d : %s \n", i, envp[i] );
	}*/

	//default log
	logInit("deepdoor.log",0);

	//printfs("%s\t%d\t[DEBUG]: argc=%d",__FILE__,__LINE__,argc);
	if(argc<2)
	{
		usage();
		return -1;
	}

	//default on console
	int d = 1;
	//tmp buffer
	char buf[64] = {0};
	int k = 0;
	while (argc > k) {
		//printf("%s\n",argv[k]);

		//必选项
		//-logf
		if (strcmp(argv[k],"-logf") == 0){
			k++;
			#ifdef	DEEPDOOR_DEBUG
				logInit(argv[k],1);
			#else
				logInit(argv[k],0);
			#endif
			printfs("%s\t%d\t[DEBUG]:logf:%s",__FILE__,__LINE__,argv[k]);
		}
		//-configure for load
		if (strcmp(argv[k],"-c") == 0){
			k++;
			snprintf(conf_path,sizeof(conf_path),"%s",argv[k]);
			conf_load(conf_path);
			printfs("%s\t%d\t[DEBUG]:conf_path:%s",__FILE__,__LINE__,conf_path);
		}
		//-haar
		if (strcmp(argv[k],"-haar") == 0){
			k++;
			haarcascades=argv[k];
			printfs("%s\t%d\t[DEBUG]:haarcascades:%s",__FILE__,__LINE__,haarcascades.c_str());
		}
		//-sample
		if (strcmp(argv[k],"-68") == 0){
			k++;
			sp68d=argv[k];
			printfs("%s\t%d\t[DEBUG]:sp68d:%s",__FILE__,__LINE__,sp68d.c_str());
		}
		//-v1
		if (strcmp(argv[k],"-v1") == 0){
			k++;
			dfrrmv1=argv[k];
			printfs("%s\t%d\t[DEBUG]:dfrrmv1:%s",__FILE__,__LINE__,dfrrmv1.c_str());
		}

		//-sample
		if (strcmp(argv[k],"-sp") == 0){
			k++;
			snprintf(sample_path,sizeof(sample_path),"%s",argv[k]);
			printfs("%s\t%d\t[DEBUG]:sample_path:%s",__FILE__,__LINE__,sample_path);
		}

		//可选项
		/*if ((strcmp(argv[k],"-a") == 0)){
			k++;
			dlib_init();
			//load_sample();
			if(dlib_diff_auth("4","camera2")==0)
				 printf("识别成功\n");
			else
				 printf("识别失败\n");
			return 1;
		}*/

		//-help
		if ((strcmp(argv[k],"?") == 0) || (strcmp(argv[k],"-h") == 0)){
			k++;
			usage();
			return 1;
			//c = atoi(argv[k]);
		}

		//-ck
		if ((strcmp(argv[k],"-ck") == 0)){
			k++;
			conf_init();

			string s1;
			s1=argv[k];
			test_cv_sample(s1);
			return 0;
		}

		//-pictures
		if (strcmp(argv[k],"-svm") == 0){
			//k++;
			//char *p = argv[++k];
			//detector_svm(p);
			return 2;
		}
		
		if (strcmp(argv[k],"-mhv") == 0){
			 k++;
			 match_value = atof(argv[k]);
		}
		
		if (strcmp(argv[k],"-sf") == 0){
			k++;
			sf=atoi(argv[k]);
		}

		//-savepath
		if (strcmp(argv[k],"-sv") == 0){
			k++;
			snprintf(save_path,sizeof(save_path),"%s",argv[k]);
			printfs("%s\t%d\t[DEBUG]:save_path:%s",__FILE__,__LINE__,save_path);
		}

		//record
//		if (strcmp(argv[k],"-rd") == 0){
//			rd=1;
//		}

		//-savepath
		if (strcmp(argv[k],"-email") == 0){
			k++;
			job_type=1;
			snprintf(to_addr,sizeof(to_addr),"%s",argv[k]);
			printfs("%s\t%d\t[DEBUG]:to_addr:%s",__FILE__,__LINE__,save_path);
		}
		
		k++;
	}
	printf("\n");

	//线程池
	thpool_init(&thpool,15,5);
	usleep(50);

	//机器学习框架加载
	conf_init();

	//PLC配置初始化
	plc_init();

	printfs("%s\t%d\t[DEBUG]:multiple_video_service",__FILE__,__LINE__);
	//multiple_video_service 启动多路视频识别
	multiple_video_service();
	printfs("%s\t%d\t[DEBUG]:server start",__FILE__,__LINE__);
	//启动服务
	start();

	printfs("%s\t%d\t[DEBUG]:thpool_destroy",__FILE__,__LINE__);
	thpool_destroy(&thpool);

	//多路视频采集回收
	multiple_video_destroy();

	destroy();

	//等待其他线程执行完
	usleep(1000);

	printfs("%s\t%d\t[DEBUG]:Main App exit",__FILE__,__LINE__);
	//dlib::sleep(3000);
}
catch(exception& e)
{
   cout << e.what() << endl;
}

/*test_cv_sample
*单张图片与对应的人脸样本库对比
*char *test_path
*char *sample_path
*/
int test_cv_sample(string sample_path)
{
	std::vector<matrix<float,0,1>> sample_faces_matrix;

	//cv_load_sample_id(sample_path,sample_faces_matrix);
	cv_get_sample(sample_path,sample_faces_matrix);
	
	printfs("%s\t%d\t[DEBUG]: dlib_diff_path sample_faces:%d",__FILE__,__LINE__,sample_faces_matrix.size());
}

/*test_dlib_diff_path
*单张图片与对应的人脸样本库对比
*char *test_path
*char *sample_path
*/
int test_dlib_diff_path(char *test_path,std::vector<rectangle> rect_faces,char *sample_path)
{
	int k=0;

	std::vector<matrix<float,0,1>> sample_faces_matrix;

	//cv_load_sample_id(sample_path,sample_faces_matrix);
	cv_get_sample(sample_path,sample_faces_matrix);
	
	printfs("%s\t%d\t[DEBUG]: dlib_diff_path sample_faces:%d",__FILE__,__LINE__,sample_faces_matrix.size());

	cv::Mat read_mat = cv::imread(test_path,3);
	char str_id[128]={0};
	double compare;
	k=dlib_diff_mat(sample_path,read_mat,rect_faces,str_id,&compare);

	//printfs("%s\t%d\t[DEBUG]: dlib_diff_mat :%lf",__FILE__,__LINE__,1-diff_matrix_min);
	
	return k;
}

//=============================
/*opencv_gpu
*opencv_gpu 加速
*/

/*
int opencv_gpu()
{
	int num_devices = cv::gpu::getCudaEnabledDeviceCount();  
	if (num_devices <= 0)  
	{  
		std::cerr << "There is no devoce" << std::endl;  
		return -1;  
	}  
	int enable_device_id = -1;  
	for (int i = 0; i < num_devices; i++)  
	{  
		cv::gpu::DeviceInfo dev_info(i);  
		if (dev_info.isCompatible())  
		{  
			enable_device_id = i;  
		}  
	}  
	if (enable_device_id < 0)  
	{  
		std::cerr << "GPU module isn't built for GPU" << std::endl;  
		return -1;  
	}  

	cv::gpu::setDevice(enable_device_id);  

	cv::Mat src_image = cv::imread("test.jpg");  
	cv::Mat dst_image;  
	cv::gpu::GpuMat d_src_img(src_image);//upload src image to gpu  
	cv::gpu::GpuMat d_dst_img;  

	cv::gpu::cvtColor(d_src_img, d_dst_img, CV_BGR2GRAY);//////////////////canny  
	d_dst_img.download(dst_image);//download dst image to cpu  
	cv::imshow("test", dst_image);  
	cv::waitKey(50000);
}

*/
