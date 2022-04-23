#ifndef _EXDLIB_H_
#define _EXDLIB_H_

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <dlib/opencv.h>
#include <dlib/dnn.h>
#include <dlib/image_processing/frontal_face_detector.h>

using namespace std;
using namespace dlib;

//dnn
template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET> 
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares	 = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;
using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
						alevel0<
						alevel1<
						alevel2<
						alevel3<
						alevel4<
						max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
						input_rgb_image_sized<150>
						>>>>>>>>>>>>;

#define BUFFER_SIZE 4096

//0.人脸特征检测器模型文件
//opencv Haar LBP 人脸对象 特征检测器
extern string haarcascades;

// Load face detection and pose estimation models.
extern frontal_face_detector ff_detector;

//1.人脸关键点检测器
extern string sp68d;

//2.ResNet人脸识别模型
extern string dfrrmv1;

// We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
extern shape_predictor sp_landmarks_ex;

// And finally we load the DNN responsible for face recognition.
extern anet_type net_dnn;

extern std::map<string,std::vector<matrix<float,0,1>>> sample_faces_desc;

/*人脸检测&识别初始化
*dlib_init
*/
void dlib_init();

void dlib_destroy();

/*opencv图像预处理
*cv_filter
*cv::Mat read_mat
*cv::Mat &chip_mat
*cv::Mat &chip_gray
*cv::Mat &equalized_mat
*/
void cv_filter(cv::Mat read_mat,cv::Mat &chip_mat,cv::Mat &chip_gray,cv::Mat &equalized_mat);

/*opencv 人脸检测
*cv_frontal_face_detector
*std::vector<rectangle> &rect_faces	 dlib矩形数据结构集合
*cv::Mat read_mat						待检测图片
*cv::Mat &chip_mat						绘制人脸矩形图片
*cv::Mat &chip_gray						灰度图
*cv::Mat &equalized_mat					直方图均匀化的图片
*/
int cv_frontal_face_detector(std::vector<rectangle> &rect_faces,
								cv::Mat read_mat,cv::Mat &chip_mat,cv::Mat &chip_gray,cv::Mat &equalized_mat);

/*dlib 人脸检测
*cv_frontal_face_detector
*std::vector<rectangle> &rect_faces	 dlib矩形数据结构集合
*cv::Mat read_mat						待检测图片
*cv::Mat &chip_mat						绘制人脸矩形图片
*cv::Mat &chip_gray						灰度图
*cv::Mat &equalized_mat					直方图均匀化的图片
*/
int dlib_frontal_face_detector(std::vector<rectangle> &rect_faces,
								cv::Mat read_mat,cv::Mat &chip_mat,cv::Mat &chip_gray,cv::Mat &equalized_mat);

int dlib2_frontal_face_detector(std::vector<rectangle> &rect_faces,
								cv::Mat read_mat,cv::Mat &chip_mat,cv::Mat &chip_gray,cv::Mat &equalized_mat);
/*dlib 人脸检测
*ex_frontal_face_detector
*std::vector<rectangle> &rect_faces	 dlib矩形数据结构集合
*cv::Mat read_mat						待检测图片
*cv::Mat &chip_mat						绘制人脸矩形图片
*cv::Mat &chip_gray						灰度图
*cv::Mat &equalized_mat					直方图均匀化的图片
*/
int ex_frontal_face_detector(std::vector<rectangle> &rect_faces,
								cv::Mat read_mat,cv::Mat &chip_mat,cv::Mat &chip_gray,cv::Mat &equalized_mat);
#endif
