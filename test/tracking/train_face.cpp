#define DLIB_JPEG_SUPPORT
#define DLIB_PNG_SUPPORT

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

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

using namespace std;
//using namespace cv;
using namespace dlib;

int resize_width = 960;//1920 960
int resize_height = 540;//1080 540

double interocular_distance(const full_object_detection& det)
{
    dlib::vector<double, 2> l, r;
    double cnt = 0;
    // Find the center of the left eye by averaging the points around 
    // the eye.
    for (unsigned long i = 36; i <= 41; ++i)
    {
        l += det.part(i);
        ++cnt;
    }
    l /= cnt;

    // Find the center of the right eye by averaging the points around 
    // the eye.
    cnt = 0;
    for (unsigned long i = 42; i <= 47; ++i)
    {
        r += det.part(i);
        ++cnt;
    }
    r /= cnt;

    // Now return the distance between the centers of the eyes
    return length(l - r);
}

std::vector<std::vector<double> > get_interocular_distances(
    const std::vector<std::vector<full_object_detection> >& objects
    )
{
    std::vector<std::vector<double> > temp(objects.size());
    for (unsigned long i = 0; i < objects.size(); ++i)
    {
        for (unsigned long j = 0; j < objects[i].size(); ++j)
        {
            temp[i].push_back(interocular_distance(objects[i][j]));
        }
    }
    return temp;
}

//训练函数
void train()
{
    try
    {
        //一、preprocessing
        //1. 载入训练集，测试集
        const std::string faces_directory = "/home/zlei/Pictures/faces";
        
        dlib::array<array2d<unsigned char> > images_train, images_test;
        std::vector<std::vector<full_object_detection> > faces_train, faces_test;

        load_image_dataset(images_train, faces_train, faces_directory + "/training_with_face_landmarks.xml");
        load_image_dataset(images_test, faces_test, faces_directory + "/testing_with_face_landmarks.xml");

        // 二、training
        //1. 定义trainer类型
        shape_predictor_trainer trainer;
        //设置训练参数
        trainer.set_oversampling_amount(300); 
        trainer.set_nu(0.05);
        trainer.set_tree_depth(2);
        trainer.be_verbose();

        // 2. 训练，生成人脸关键点检测器
        shape_predictor sp = trainer.train(images_train, faces_train);

        // 三、测试
        cout << "mean training error: " <<
            test_shape_predictor(sp, images_train, faces_train, get_interocular_distances(faces_train)) << endl;
        cout << "mean testing error:  " <<
            test_shape_predictor(sp, images_test, faces_test, get_interocular_distances(faces_test)) << endl;

        // 四、存储
        serialize("sp.dat") << sp;
    }
    catch (exception& e)
    {
        cout << "\nexception thrown!" << endl;
        cout << e.what() << endl;
    }
}


int fhog_object_detector_ex()
{
    try
    {
        //./imglab -c /home/zlei/Pictures/faces/training.xml  /home/zlei/Pictures/faces
        //./imglab /home/zlei/Pictures/faces/testing.xml
        //./imglab /home/zlei/Pictures/faces/training.xml
        
        //一、preprocessing
        //1. 载入训练集，测试集
        const std::string faces_directory = "faces";
        dlib::array<array2d<unsigned char> > images_train, images_test;
        std::vector<std::vector<rectangle> > face_boxes_train, face_boxes_test;

        load_image_dataset(images_train, face_boxes_train, faces_directory + "/training.xml");
        load_image_dataset(images_test, face_boxes_test, faces_directory + "/testing.xml");

        //2.图片上采样
        upsample_image_dataset<pyramid_down<2> >(images_train, face_boxes_train);
        upsample_image_dataset<pyramid_down<2> >(images_test, face_boxes_test);

        //3.训练图片做镜像处理，扩充训练集
        add_image_left_right_flips(images_train, face_boxes_train);

        //二、training
        //1.定义scanner类型，用于扫描图片并提取特征（HOG）
        typedef scan_fhog_pyramid<pyramid_down<6> > image_scanner_type;
        image_scanner_type scanner;

        //2. 设置scanner扫描窗口大小
        scanner.set_detection_window_size(80, 80);

        //3.定义trainer类型（SVM），用于训练人脸检测器                
        structural_object_detection_trainer<image_scanner_type> trainer(scanner);
        // Set this to the number of processing cores on your machine.
        trainer.set_num_threads(4);
        // 设置SVM的参数C，C越大表示更好地去拟合训练集，当然也有可能造成过拟合。通过尝试不同C在测试集上的效果得到最佳值
        trainer.set_c(1); 
        trainer.be_verbose();
        //设置训练结束条件，"risk gap"<0.01时训练结束，值越小表示SVM优化问题越精确，训练时间也会越久。
        //通常取0.1-0.01.在verbose模式下每一轮的risk gap都会打印出来。
        trainer.set_epsilon(0.01);

        //4.训练，生成object_detector
        object_detector<image_scanner_type> detector = trainer.train(images_train, face_boxes_train);

        //三、测试
        // 输出precision, recall, average precision.
        cout << "training results: " << test_object_detection_function(detector, images_train, face_boxes_train) << endl;
        cout << "testing results:  " << test_object_detection_function(detector, images_test, face_boxes_test) << endl;

        //显示hog
        image_window hogwin(draw_fhog(detector), "Learned fHOG detector");

        // 显示测试集的人脸检测结果
        image_window win;
        for (unsigned long i = 0; i < images_test.size(); ++i)
        {
            // Run the detector and get the face detections.
            std::vector<rectangle> dets = detector(images_test[i]);
            win.clear_overlay();
            win.set_image(images_test[i]);
            win.add_overlay(dets, rgb_pixel(255, 0, 0));
            cout << "Hit enter to process the next image..." << endl;
            cin.get();
        }

        //四、模型存储
        serialize("face_detector.svm") << detector;
        // you can recall it using the deserialize() function.
        object_detector<image_scanner_type> detector2;
        deserialize("face_detector.svm") >> detector2;
    }
    catch (exception& e)
    {
        cout << "\nexception thrown!" << endl;
        cout << e.what() << endl;
    }
}

void detector_svm(char *path_img)
{
        // Finally we get to the training code.  dlib contains a number of
        // object detectors.  This typedef tells it that you want to use the one
        // based on Felzenszwalb's version of the Histogram of Oriented
        // Gradients (commonly called HOG) detector.  The 6 means that you want
        // it to use an image pyramid that downsamples the image at a ratio of
        // 5/6.  Recall that HOG detectors work by creating an image pyramid and
        // then running the detector over each pyramid level in a sliding window
        // fashion.   
        typedef scan_fhog_pyramid<pyramid_down<6> > image_scanner_type; 
        image_scanner_type scanner;
        // Then you can recall it using the deserialize() function.
        object_detector<image_scanner_type> detector;
        deserialize("face_detector.svm") >> detector;

        array2d<rgb_pixel> img;
        cout << "detector_svm3" << endl;
        load_image(img, path_img);
       cout << "detector_svm3" << endl;
        array2d<unsigned char> img_gray;  
        gaussian_blur(img, img_gray); 
       cout << "detector_svm4" << endl;
        //image_window hogwin(draw_fhog(detector), "Learned fHOG detector");
        image_window win;
        // Run the detector and get the face detections.
       std::vector<rectangle> dets = detector(img_gray);
              cout << "detector_svm5" << endl;
        cout << "find faces:"<< dets.size()<< endl;
       //cout << dets[0].left()<<"\t"<<dets[0].top()<<"\t"<< dets[0].right() <<"\t"<< dets[0].bottom()<< endl;
       
       cout << "detector_svm6" << endl;
       win.clear_overlay();
       win.set_image(img_gray);
       win.add_overlay(dets, rgb_pixel(255,0,0));
       cout << "Hit enter to process the next image..." << endl;
       
       cin.get();
}

//关键点检测函数
int cpoint(int argc, char** argv)
{
    try
    {
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor sp;
        //将上一步训练好的sp.dat 载入
        deserialize("sp.dat") >> sp;
        image_window win, win_faces;
        const std::string image = "faces/2007_007763.jpg";
        cout << "processing image " << image << endl;
        array2d<rgb_pixel> img;
        load_image(img, image);
            // Make the image larger so we can detect small faces.
        pyramid_up(img);

            // Now tell the face detector to give us a list of bounding boxes
            // around all the faces in the image.
            std::vector<rectangle> dets = detector(img);
            cout << "Number of faces detected: " << dets.size() << endl;

            // Now we will go ask the shape_predictor to tell us the pose of
            // each face we detected.
            std::vector<full_object_detection> shapes;
            for (unsigned long j = 0; j < dets.size(); ++j)
            {
                full_object_detection shape = sp(img, dets[j]);
                cout << "number of parts: "<< shape.num_parts() << endl;
                cout << "pixel position of first part:  " << shape.part(0) << endl;
                cout << "pixel position of second part: " << shape.part(1) << endl;
                // You get the idea, you can get all the face part locations if
                // you want them.  Here we just store them in shapes so we can
                // put them on the screen.
                shapes.push_back(shape);
            }

            // Now let's view our face poses on the screen.
            win.clear_overlay();
            win.set_image(img);
            win.add_overlay(render_face_detections(shapes));

            // We can also extract copies of each face that are cropped, rotated upright,
            // and scaled to a standard size as shown here:
            dlib::array<array2d<rgb_pixel> > face_chips;
            extract_image_chips(img, get_face_chip_details(shapes), face_chips);
            win_faces.set_image(tile_images(face_chips));

            cout << "Hit enter to process the next image..." << endl;
            cin.get();
        
    }
    catch (exception& e)
    {
        cout << "\nexception thrown!" << endl;
        cout << e.what() << endl;
    }
}

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

int cv_frontal_face_detector(cv::CascadeClassifier &faceDetector,shape_predictor &sp_landmarks,
                  cv::Mat &read_mat,cv::Mat &read_gray,cv::Mat &equalized_mat)
{
	try
	{
      //图像大小标准化
      //cv::resize(read_mat, chip_mat, cv::Size(resize_width, resize_height), (0, 0), (0, 0), cv::INTER_LINEAR);
      
      //人脸检测只试用于灰度图像
      cv_filter_gray(read_mat, read_gray);
      
      //灰度图二值化滤波处理
      //cv::Mat read_threshold;
      //cv::threshold(read_gray,read_threshold, 12, 1, CV_THRESH_BINARY);
      
      //邻域内计算阈值所采用的算法
      //ADAPTIVE_THRESH_MEAN_C 域的平均值再减去第七个参数double C的值
      //CV_ADAPTIVE_THRESH_GAUSSIAN_C 计算出领域的高斯均值再减去第七个参数double C的值
      
      //cv::adaptiveThreshold(chip_gray, chip_threshold, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 7, 5);
      //cv::adaptiveThreshold(chip_gray, chip_threshold, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 3, 5);
      //cv::adaptiveThreshold(read_gray, read_threshold, 220, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 5);
      
      //直方图均匀化(改善图像的对比度和亮度)
      //equalizeHist(read_threshold, equalized_mat);
	   equalizeHist(read_gray, equalized_mat);

      float searchScaleFactor = 1.3;//1.2f
      int minNeighbors = 3;//4
      //int flags = cv::CASCADE_FIND_BIGGEST_OBJECT|cv::CASCADE_DO_ROUGH_SEARCH;	//只检测脸最大的人
      int flags = cv::CASCADE_SCALE_IMAGE|cv::CASCADE_DO_ROUGH_SEARCH;//检测多个人
      cv::Size minFeatureSize(9,9);//2,2
      
      /*	   
      //int flags = cv::CASCADE_FIND_BIGGEST_OBJECT|cv::CASCADE_DO_ROUGH_SEARCH;	//只检测脸最大的人
      int flags = cv::CASCADE_SCALE_IMAGE|cv::CASCADE_DO_ROUGH_SEARCH;//检测多个人
      
      //|CASCADE_FIND_BIGGEST_OBJECT
            //|CASCADE_DO_ROUGH_SEARCH
            //|CASCADE_DO_CANNY_PRUNING
            //| CASCADE_SCALE_IMAGE,
            
      cv::Size minFeatureSize(9,9);//2,2
      float searchScaleFactor = 1.15f;//1.1f
      int minNeighbors = 3;//4
      */

      //人脸检测用Cascade Classifier::detectMultiScale来进行人脸检测
      //const Mat& image: 需要被检测的图像（灰度图）
      //vector<Rect>& objects: 保存被检测出的人脸位置坐标序列
      //double scaleFactor: 每次图片缩放的比例
      //int minNeighbors: 每一个人脸至少要检测到多少次才算是真的人脸
      //int flags： 决定是缩放分类器来检测，还是缩放图像
      //Size minFeatureSize : 表示人脸的最小尺寸

      //人脸矩形框集合
      std::vector<cv::Rect> rects;
      //opencv人脸检测
      //pthread_mutex_lock(&lock_detector);
      faceDetector.detectMultiScale(equalized_mat, rects, searchScaleFactor, minNeighbors, flags, minFeatureSize);
      //pthread_mutex_unlock(&lock_detector);
      //opencv人脸个数
      //printfs("%s\t%d\t[DEBUG]:opencv 检测人脸数: %d",__FILE__,__LINE__,rects.size());

      //画矩形框
      /*for(std::vector<cv::Rect>::const_iterator iter=rects.begin();iter!=rects.end();iter++)
      {
         //画出脸部矩形
         cv::rectangle(equalizedImg, *iter, cv::Scalar(0, 0, 0), 1, 8, 0);
		   cv::rectangle(gray, *iter, cv::Scalar(0, 0, 0), 1, 8, 0);
		   cv::rectangle(temp, *iter, cv::Scalar(0, 255, 127), 1, 8, 0);
      }*/

		//dlib 
		cv_image<bgr_pixel> bgr_img(read_mat);
		
		// Detect faces 
		std::vector<rectangle> faces;

		//opencv->dlib
      for(std::vector<cv::Rect>::const_iterator iter=rects.begin();iter!=rects.end();iter++)
      {
         cv::rectangle(read_mat, *iter, cv::Scalar(0,255,0), 1, 1, 0);//在img上绘制出检测到的面部矩形框，绿色框 
         //将opencv检测到的矩形转换为dlib需要的数据结构，这里没有判断检测不到人脸的情况
         dlib::rectangle dlib_rect;
         dlib_rect.set_left((*iter).x);
         dlib_rect.set_top((*iter).y);
         dlib_rect.set_right((*iter).x+(*iter).width);
         dlib_rect.set_bottom((*iter).y+(*iter).height);
         faces.push_back(dlib_rect);
      }
      
		// Detect faces for dlib
		//dlib人脸检测器检测人脸到faces集合
		//faces = detector(cimg);
		
		// Find the pose of each face.
		std::vector<full_object_detection> shapes;
		for (unsigned long i = 0; i < faces.size(); ++i)
			shapes.push_back(sp_landmarks(bgr_img, faces[i]));

		if (!shapes.empty()) {
		   for(int j=0;j<shapes.size();j++){
		      //68个
   			for (int i = 0; i < 68; i++) {
				   circle(read_mat, cvPoint(shapes[j].part(i).x(), shapes[j].part(i).y()), 3, cv::Scalar(255, 0, 0), -1);
			   }
		   }
		}
	}
	catch (serialization_error& e)
	{
		cout << "You need dlib's default face landmarking model file to run this example." << endl;
		cout << "You can get it from the following URL: " << endl;
		cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
		cout << endl << e.what() << endl;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
}

int cv_sp_img(string img_url,string haarcascades)
{
	try
	{
      //opencv Haar LBP 人脸对象 特征检测器
      cv::CascadeClassifier faceDetector;

      //友好错误信息提示
      try{
         faceDetector.load(haarcascades);
      }catch (cv::Exception e){
         std::cerr << "脸部检测器不能加载 (";
         std::cerr << haarcascades << ")!" << std::endl;
         exit(1);
      }
      
		// Load face detection and pose estimation models.
		//dlib 人脸对象 特征检测器
		//frontal_face_detector detector = get_frontal_face_detector();

		//人脸对齐
		shape_predictor sp_landmarks;
		deserialize("shape_predictor_68_face_landmarks.dat") >> sp_landmarks;
		
      cv::Mat read_mat,read_gray,equalized_mat;
      
      read_mat = cv::imread(img_url,3);
		// Grab and process frames until the main window is closed by the user.
      cv_frontal_face_detector(faceDetector,sp_landmarks,read_mat,read_gray,equalized_mat);
      
		//Display it all on the screen
		imshow("直方图均匀化", equalized_mat);
	   imshow("灰度化", read_gray);
	   imshow("标准化原图", read_mat);
	   
	   cv::waitKey(0);
	}
	catch (serialization_error& e)
	{
		cout << "You need dlib's default face landmarking model file to run this example." << endl;
		cout << "You can get it from the following URL: " << endl;
		cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
		cout << endl << e.what() << endl;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
}

int opencv_sp(string url,string haarcascades)
{
	try
	{
		cv::VideoCapture cap(url);
		if (!cap.isOpened())
		{
			cerr << "Unable to connect to camera" << endl;
			return 1;
		}

      //opencv Haar LBP 人脸对象 特征检测器
      cv::CascadeClassifier faceDetector;

      //友好错误信息提示
      try{
         faceDetector.load(haarcascades);
      }catch (cv::Exception e){
         std::cerr << "脸部检测器不能加载 (";
         std::cerr << haarcascades << ")!" << std::endl;
         exit(1);
      }
      
		// Load face detection and pose estimation models.
		//dlib 人脸对象 特征检测器
		//frontal_face_detector detector = get_frontal_face_detector();

		//人脸对齐
		shape_predictor sp_landmarks;
		deserialize("shape_predictor_68_face_landmarks.dat") >> sp_landmarks;
		
		//image_window win;
      cv::Mat read_mat,chip_mat,chip_gray,equalized_mat;
      
		// Grab and process frames until the main window is closed by the user.
		while (1)
		{
			// read a frame
         if (!cap.read(read_mat) || cv::waitKey(0) == 'q')
         {
             break;
         }
			
         cv_frontal_face_detector(faceDetector,sp_landmarks,read_mat,chip_gray,equalized_mat);
         
			//Display it all on the screen
			imshow("直方图均匀化", equalized_mat);
		   imshow("灰度化", chip_gray);
		   imshow("标准化原图", chip_mat);
		}
	}
	catch (serialization_error& e)
	{
		cout << "You need dlib's default face landmarking model file to run this example." << endl;
		cout << "You can get it from the following URL: " << endl;
		cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
		cout << endl << e.what() << endl;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
}

int main(int argc,char ** argv)
{
   //train();
   //out1.mp4
   //haarcascade_frontalface_alt.xml
   //haarcascade_frontalface_alt2.xml
   //haarcascade_frontalface_default.xml
   // /home/zlei/project-c/mdsword/data/lbpcascades/lbpcascade_frontalface.xml
   // /home/zlei/project-c/mdsword/data/haarcascades/haarcascade_frontalface_alt2.xml
   string url(argv[1]);
   //opencv_sp(url,"/home/zlei/project-c/mdsword/data/haarcascades/haarcascade_frontalface_alt2.xml");
   cv_sp_img(url,"/home/zlei/project-c/mdsword/data/haarcascades/haarcascade_frontalface_alt2.xml");
   return 0;
}
