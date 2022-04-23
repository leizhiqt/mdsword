#include <iostream>
#include <freeglut.h>            // 包含OpenGL实用库
#include <gdal_priv.h>

#include <assert.h>

using namespace std;

unsigned int  texture;    // 纹理对象
unsigned char* imgBuf = nullptr;
int imgWidth;
int imgHeight;

void ReadImage(const char *fpath)
{
    GDALAllRegister();

    GDALDataset* img = (GDALDataset *)GDALOpen(fpath, GA_ReadOnly);
    //GDALDataset* img = (GDALDataset *)GDALOpen("dst.tif", GA_ReadOnly);
    if (img == nullptr)
    {
        return;
    } 

    imgWidth = img->GetRasterXSize();  //图像宽度
    imgHeight = img->GetRasterYSize();  //图像高度
    int bandNum = img->GetRasterCount();    //波段数 
    int depth = GDALGetDataTypeSize(img->GetRasterBand(1)->GetRasterDataType()) / 8;    //图像深度

    //申请buf
    size_t imgBufNum = (size_t)imgWidth * imgHeight * bandNum * depth;
    size_t imgBufOffset = (size_t)imgWidth * (imgHeight - 1) * bandNum * depth;
    imgBuf = new GByte[imgBufNum];
    //读取
    img->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, imgBuf + imgBufOffset, imgWidth, imgHeight,
        GDT_Byte, bandNum, nullptr, bandNum*depth, -imgWidth*bandNum*depth, depth);

    GDALClose(img);
}

void InitGL(const char *fpath)
{
     glClearColor(0.0, 0.0, 0.0, 0.0);

     glShadeModel(GL_SMOOTH);      //平滑着色
     glEnable(GL_DEPTH_TEST);      //深度测试
     glEnable(GL_CULL_FACE);    //只渲染某一面
     glFrontFace(GL_CCW);    //逆时针正面
      
     glEnable(GL_TEXTURE_2D);    //启用2D纹理映射 

     //载入纹理图像：
     ReadImage(fpath);

     //生成纹理对象：
     glGenTextures(1, &texture);       
}

void DrawGLScene()
{
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

     glBindTexture(GL_TEXTURE_2D, texture);    //绑定纹理：

     glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //支持4字节对齐

     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);      //S方向上贴图
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);      //T方向上贴图
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      //放大纹理过滤方式
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);      //缩小纹理过滤方式

     glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imgBuf);//载入纹理：

     glMatrixMode(GL_MODELVIEW);                        // 选择模型观察矩阵
     glLoadIdentity();                                  // 重置模型观察矩阵   
     glMatrixMode(GL_PROJECTION);                        // 选择投影矩阵     
     glLoadIdentity();

     glEnable(GL_TEXTURE_2D);    //启用2D纹理映射
     glBegin(GL_QUADS);
     glTexCoord2f(0.0f, 0.0f); 
     glVertex3f(-0.5f, -0.5f, 0.0f);
     glTexCoord2f(1.0f, 0.0f); 
     glVertex3f(0.5f, -0.5f, 0.0f);
     glTexCoord2f(1.0f, 1.0f);
     glVertex3f(0.5f, 0.5f, 0.0f);
     glTexCoord2f(0.0f, 1.0f);
     glVertex3f(-0.5f, 0.5f, 0.0f);
     glEnd();
     glDisable(GL_TEXTURE_2D);

     glutSwapBuffers();
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)    // 重置OpenGL窗口大小
{
    glViewport(0, 0, width, height);
}


/***linux下一个中文占用三个字节,windows占两个字节***/  
void chinese_or_english(char *str)  
{  
  char chinese[4] = {0};  
  for (int i = 0; i < strlen(str); i++)
  {  
    //if (str[i] >= 0 && str[i] <= 127) {      //ascII  
    if ((str[i] & 0x80) == 0)
    {
      printf("%c", str[i]); 
    }  
    else 
    {  
      chinese[0] = str[i];  
      chinese[1] = str[i + 1];  
      chinese[2] = str[i + 2];  
      i++;    //skip one more  
      i++;  
      printf("%s", chinese); 
    }  
  }
  printf("\n");
}

int abcd()  
{  
  char str[] = "tai太阳yang";  
  //printf("chinese:%d\n", strlen(str)); 
  //chinese_or_english(str);
  printf("chinese:%s\n", str); 
  return 0;  
}

int test_gl(int argc, char** argv, char** envp)  
{  
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("opengl");
   
    //InitGL((const char *)(argv[1]));
    InitGL((const char *)(argv[1]));
    
    glutDisplayFunc(DrawGLScene);
    glutReshapeFunc(ReSizeGLScene);
    //glutKeyboardFunc(keyboard);
    //glutMouseWheelFunc(mouse_wheel);
    //glutIdleFunc(idle);

    glutMainLoop();
  return 0;  
} 

int main(int argc, char** argv, char** envp)
{
     //test_gl
     test_gl(argc,argv,envp);
     //abcd();
    return 0;
}


