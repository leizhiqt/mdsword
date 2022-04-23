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
extern "C"
{
#include "log.h"
}

#include "exdlib.h"

using namespace std;
using namespace dlib;

//0.人脸特征检测器模型文件
//opencv Haar LBP 人脸对象 特征检测器
string haarcascades="haarcascades/haarcascade_frontalface_alt.xml";

// Load face detection and pose estimation models.
frontal_face_detector ff_detector;

//1.人脸关键点检测器
string sp68d="shape_predictor_68_face_landmarks.dat";

//2.ResNet人脸识别模型
string dfrrmv1="dlib_face_recognition_resnet_model_v1.dat";

// We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
shape_predictor sp_landmarks_ex;

// And finally we load the DNN responsible for face recognition.
anet_type net_dnn;

std::map<string,std::vector<matrix<float,0,1>>> sample_faces_desc;

/*人脸检测&识别初始化
*dlib_init
*/
void dlib_init()
{

	// Load face detection and pose estimation models.
	ff_detector = get_frontal_face_detector();
	// We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
	deserialize(sp68d) >> sp_landmarks_ex;
	// And finally we load the DNN responsible for face recognition.
	deserialize(dfrrmv1) >> net_dnn;
}

/*销毁dlib相关资源
*dlib_destroy
*/
void dlib_destroy()
{

}

/*opencv 彩色图片转灰度图
*cv_filter_gray
*cv::Mat camerFrame
*cv::Mat &gray
*/
void cv_filter_gray(cv::Mat camerFrame,cv::Mat &gray)
{
	//普通台式机3通道BGR,移动设备为4通道
	if (camerFrame.channels() == 3)
	{
		cv::cvtColor(camerFrame, gray, CV_BGR2GRAY);
	}
	else if (camerFrame.channels() == 4)
	{
		cv::cvtColor(camerFrame, gray, CV_BGRA2GRAY);
	}
	else
		gray = camerFrame;
}

/*opencv图像预处理
*cv_filter
*cv::Mat read_mat
*cv::Mat &chip_mat
*cv::Mat &chip_gray
*cv::Mat &equalized_mat
*/
void cv_filter(cv::Mat read_mat,cv::Mat &chip_mat,cv::Mat &chip_gray,cv::Mat &equalized_mat)
{
	//>>图片预处理
	//图像大小标准化
	//cv::resize(read_mat, chip_mat, cv::Size(resize_width, resize_height), (0, 0), (0, 0), CV_INTER_CUBIC);

	chip_mat = read_mat.clone();

	//人脸检测只试用于灰度图像
	cv_filter_gray(chip_mat, chip_gray);

	//灰度图二值化滤波处理
	//cv::Mat chip_threshold;
	//cv::threshold(chip_gray,chip_threshold, 12, 1, CV_THRESH_BINARY);

	//邻域内计算阈值所采用的算法
	//ADAPTIVE_THRESH_MEAN_C 域的平均值再减去第七个参数double C的值
	//CV_ADAPTIVE_THRESH_GAUSSIAN_C 计算出领域的高斯均值再减去第七个参数double C的值

	//cv::adaptiveThreshold(chip_gray, chip_threshold, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 7, 5);
	//cv::adaptiveThreshold(chip_gray, chip_threshold, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 3, 5);
	//cv::adaptiveThreshold(chip_gray, chip_threshold, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 1, 5);
	//越大才有轮廓
	//cv::adaptiveThreshold(chip_gray, chip_threshold, 220, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 5);
	//cv::adaptiveThreshold(chip_gray, chip_threshold, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 17, 5);

	//直方图均匀化(改善图像的对比度和亮度)
	//equalizeHist(chip_threshold, equalized_mat);
	equalizeHist(chip_gray, equalized_mat);
	//<<图片预处理
}

/*人脸识别检测步骤
图像大小标准化
cv::resize(read_mat, resize_mat, cv::Size(270, 360),0,0,CV_INTER_CUBIC);

图像变换变量
cv::Mat chip_mat,chip_gray,equalized_mat;

人脸矩形向量
std::vector<rectangle> rect_faces;

人脸矩阵向量
std::vector<matrix<rgb_pixel>> id_faces;

1.通过检测器人脸检测 ff_detector:dlib检测器 cv_frontal_face_detector:opencv检测器
rect_faces = ff_detector(img);//检测人脸，获得边界框
int faces_count = cv_frontal_face_detector(rect_faces,read_mat,chip_mat,chip_gray,equalized_mat);
*/

/*opencv 人脸检测
*cv_frontal_face_detector
*std::vector<rectangle> &rect_faces	 dlib矩形数据结构集合
*cv::Mat read_mat						待检测图片
*cv::Mat &chip_mat						绘制人脸矩形图片
*cv::Mat &chip_gray						灰度图
*cv::Mat &equalized_mat					直方图均匀化的图片
*/
int cv_frontal_face_detector(std::vector<rectangle> &rect_faces,
								cv::Mat read_mat,cv::Mat &chip_mat,cv::Mat &chip_gray,cv::Mat &equalized_mat)
{
	try
	{
		//opencv人脸边界
		std::vector<cv::Rect> rects;

		//opencv Haar LBP 人脸对象 特征检测器
		cv::CascadeClassifier cv_face_detector;

		//友好错误信息提示
		try{
			cv_face_detector.load(haarcascades);
		}catch (cv::Exception e){
			printfs("%s\t%d\t[ERROR]:脸部检测器不能加载 (%s)!",__FILE__,__LINE__,haarcascades.c_str());
			exit(1);
		}
		//printfs("%s\t%d\t[DEBUG]: cv_frontal_face_detector (%s)",__FILE__,__LINE__,haarcascades.c_str());

		cv_filter(read_mat,chip_mat,chip_gray,equalized_mat);

		float searchScaleFactor = 1.2;//1.2f
		int minNeighbors = 5;//4
		int flags = cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_DO_ROUGH_SEARCH;	//只检测脸最大的人
		//int flags = cv::CASCADE_SCALE_IMAGE | cv::CASCADE_DO_ROUGH_SEARCH;			//检测多个人
		cv::Size minFeatureSize(100,100);//5,5

		//人脸检测用Cascade Classifier::detectMultiScale来进行人脸检测
		//const Mat& image: 需要被检测的图像（灰度图）
		//vector<Rect>& objects: 保存被检测出的人脸位置坐标序列
		//double scaleFactor: 每次图片缩放的比例
		//int minNeighbors: 每一个人脸至少要检测到多少次才算是真的人脸
		//int flags： 决定是缩放分类器来检测，还是缩放图像
		//Size minFeatureSize : 表示人脸的最小尺寸

		//opencv人脸检测
		cv_face_detector.detectMultiScale(equalized_mat, rects, searchScaleFactor, minNeighbors, flags, minFeatureSize);

		//opencv人脸个数
		//printfs("%s\t%d\t[DEBUG]:opencv 检测人脸数: %d",__FILE__,__LINE__,rects.size());
		if(rects.size()<1) return 0;

		//opencv->dlib 将opencv检测到的矩形转换为dlib需要的数据结构
		for(std::vector<cv::Rect>::const_iterator iter=rects.begin();iter!=rects.end();iter++)
		{
			dlib::rectangle dlib_rect;

			dlib_rect.set_left((*iter).x);
			dlib_rect.set_top((*iter).y);
			dlib_rect.set_right((*iter).x+(*iter).width);
			dlib_rect.set_bottom((*iter).y+(*iter).height);
			rect_faces.push_back(dlib_rect);
		}

		//cv_mat->cv_image
		cv_image<bgr_pixel> bgr_img(chip_mat);

		//dlib人脸检测到的数据进行人脸对齐
		std::vector<full_object_detection> shapes;
		shape_predictor sp_landmarks = sp_landmarks_ex;

		for (unsigned long i = 0; i < rect_faces.size(); ++i)
			shapes.push_back(sp_landmarks(bgr_img, rect_faces[i]));

		//绘制68特征点->chip_mat
		/*if (!shapes.empty()) {
			for(int j=0;j<shapes.size();j++){
				//68个
				for (int i = 0; i < 68; i++) {
					circle(chip_mat, cvPoint(shapes[j].part(i).x(), shapes[j].part(i).y()), 3, cv::Scalar(255, 0, 0), -1);
				}
			}
		}*/

		//绘制绿色矩形框->chip_mat 
		for(std::vector<cv::Rect>::const_iterator iter=rects.begin();iter!=rects.end();iter++)
		{
			cv::rectangle(chip_mat, *iter, cv::Scalar(0,255,0), 1, 1, 0);
		}

		//画矩形框
		/*for(std::vector<cv::Rect>::const_iterator iter=rects.begin();iter!=rects.end();iter++)
		{
			//画出脸部矩形
			cv::rectangle(equalizedImg, *iter, cv::Scalar(0, 0, 0), 1, 8, 0);
			cv::rectangle(gray, *iter, cv::Scalar(0, 0, 0), 1, 8, 0);
			cv::rectangle(temp, *iter, cv::Scalar(0, 255, 127), 1, 8, 0);
		}*/

		/*
		//设置绘制文本的相关参数  
		std::string text = "Hello World!";  
		int font_face = cv::FONT_HERSHEY_COMPLEX;   
		double font_scale = 2;  
		int thickness = 2;  
		int baseline;  
		//获取文本框的长宽  
		cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);  

		//将文本框居中绘制  
		cv::Point origin;   
		origin.x = chip_mat.cols / 2 - text_size.width / 2;
		origin.y = chip_mat.rows / 2 + text_size.height / 2;
		cv::putText(chip_mat, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);
		*/
		return rects.size();
	}
	catch (serialization_error& e)
	{
		cout << "You need dlib's default face landmarking model file to run this example." << endl;
		cout << "You can get it from the following URL: " << endl;
		cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
		cout << endl << e.what() << endl;
		return -1;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
		return -2;
	}
}

/*dlib 人脸检测
*cv_frontal_face_detector
*std::vector<rectangle> &rect_faces	 dlib矩形数据结构集合
*cv::Mat read_mat						待检测图片
*cv::Mat &chip_mat						绘制人脸矩形图片
*cv::Mat &chip_gray						灰度图
*cv::Mat &equalized_mat					直方图均匀化的图片
*/
int dlib_frontal_face_detector(std::vector<rectangle> &rect_faces,
								cv::Mat read_mat,cv::Mat &chip_mat,cv::Mat &chip_gray,cv::Mat &equalized_mat)
{
	try
	{
		//opencv人脸边界
		std::vector<cv::Rect> rects;

		//>>图片预处理
		cv_filter(read_mat,chip_mat,chip_gray,equalized_mat);

		//dlib 
		cv_image<bgr_pixel> bgr_img(chip_mat);
		
		//dlib人脸检测器 检测人脸到rect_faces集合
		rect_faces = ff_detector(bgr_img);//检测人脸，获得边界框
		//printfs("%s\t%d\t[DEBUG]:opencv 检测人脸数: %d",__FILE__,__LINE__,rect_faces.size());
		if(rect_faces.size()<1) return 0;
		
		//dlib人脸检测到的数据进行人脸对齐
		std::vector<full_object_detection> shapes;
		shape_predictor sp_landmarks = sp_landmarks_ex;

		for (unsigned long i = 0; i < rect_faces.size(); ++i)
			shapes.push_back(sp_landmarks(bgr_img, rect_faces[i]));

		//绘制68特征点->chip_mat
		/*if (!shapes.empty()) {
			for(int j=0;j<shapes.size();j++){
				//68个
				for (int i = 0; i < 68; i++) {
					circle(chip_mat, cvPoint(shapes[j].part(i).x(), shapes[j].part(i).y()), 3, cv::Scalar(255, 0, 0), -1);
				}
			}
		}*/

		//dlib->opencv dlib人脸矩形->opencv人脸边界
		for(std::vector<rectangle>::const_iterator iter=rect_faces.begin();iter!=rect_faces.end();iter++)
		{
			rectangle r=*iter;
			cv::Rect rect(cv::Point2i(r.left(),r.top()),cv::Point2i(r.right()+1,r.bottom()+1));
			rects.push_back(rect);
		}

		//画矩形框
		/*for(std::vector<cv::Rect>::const_iterator iter=rects.begin();iter!=rects.end();iter++)
		{
			//画出脸部矩形
			cv::rectangle(equalizedImg, *iter, cv::Scalar(0, 0, 0), 1, 8, 0);
			cv::rectangle(gray, *iter, cv::Scalar(0, 0, 0), 1, 8, 0);
			cv::rectangle(temp, *iter, cv::Scalar(0, 255, 127), 1, 8, 0);
		}*/
		//绘制绿色矩形框->chip_mat 
		for(std::vector<cv::Rect>::const_iterator iter=rects.begin();iter!=rects.end();iter++)
		{
			cv::rectangle(chip_mat, *iter, cv::Scalar(220,20,60), 1, 1, 0);
		}

		/*
		//设置绘制文本的相关参数  
		std::string text = "Hello World!";  
		int font_face = cv::FONT_HERSHEY_COMPLEX;   
		double font_scale = 2;  
		int thickness = 2;  
		int baseline;  
		//获取文本框的长宽  
		cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);  

		//将文本框居中绘制  
		cv::Point origin;   
		origin.x = chip_mat.cols / 2 - text_size.width / 2;
		origin.y = chip_mat.rows / 2 + text_size.height / 2;
		cv::putText(chip_mat, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);
		*/
		return rect_faces.size();
	}
	catch (serialization_error& e)
	{
		cout << "You need dlib's default face landmarking model file to run this example." << endl;
		cout << "You can get it from the following URL: " << endl;
		cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
		cout << endl << e.what() << endl;
		return -1;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
		return -2;
	}
}
/*dlib 人脸检测
*ex_frontal_face_detector
*std::vector<rectangle> &rect_faces	 dlib矩形数据结构集合
*cv::Mat read_mat						待检测图片
*cv::Mat &chip_mat						绘制人脸矩形图片
*cv::Mat &chip_gray						灰度图
*cv::Mat &equalized_mat					直方图均匀化的图片
*/
int ex_frontal_face_detector(std::vector<rectangle> &rect_faces,
								cv::Mat read_mat,cv::Mat &chip_mat,cv::Mat &chip_gray,cv::Mat &equalized_mat)
{
	return dlib_frontal_face_detector(rect_faces,read_mat,chip_mat,chip_gray,equalized_mat);
	//return cv_frontal_face_detector(rect_faces,read_mat,chip_mat,chip_gray,equalized_mat);
}
