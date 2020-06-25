
	
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv/cv.h>

#include <iostream>
#include <tensorflow/c/c_api.h>






#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "include/MvCameraControl.h"


//-----------------

bool configured_{false};
TF_Graph* graph_;
TF_Output inputOp_;
TF_Output outputOp_;
TF_Tensor* inputTensor_;
void* tensorData_;
const std::vector<std::int64_t> input_dims_{1, 224, 224, 3};
const int imgW_{224};
const int imgH_{224};
const int imgC_{3};
const int imgS_{imgH_*imgW_*imgC_};
int8_t resultLimits_[2];




//---------------

  TF_Buffer* read_file(const char* file);

  void free_buffer(void* data, size_t length) { free(data); }

  void deallocator(void* ptr, size_t len, void* arg) { free((void*)ptr); }

  typedef unsigned char byte; 

  const int num_channels = 3;
  struct Image {
    byte* data;
    int width, height;
  };





bool g_bExit = false;

// 等待用户输入enter键来结束取流或结束程序
// wait for user to input enter to stop grabbing or end the sample program
void PressEnterToExit(void)
{
    int c;
    while ( (c = getchar()) != '\n' && c != EOF );
    fprintf( stderr, "\nPress enter to exit.\n");
    while( getchar() != '\n');
    g_bExit = true;
    sleep(1);
}

bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
        printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chModelName);
        printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chModelName);
        printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
    }
    else
    {
        printf("Not support.\n");
    }

    return true;
}

static void* WorkThread(void* pUser)
{
    int nRet = MV_OK;

    // ch:获取数据包大小 | en:Get payload size
    MVCC_INTVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    nRet = MV_CC_GetIntValue(pUser, "PayloadSize", &stParam);
    if (MV_OK != nRet)
    {
        printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
        return NULL;
    }

    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
    memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
    unsigned char * pData = (unsigned char *)malloc(sizeof(unsigned char) * stParam.nCurValue);
    if (NULL == pData)
    {
        return NULL;
    }
    unsigned int nDataSize = stParam.nCurValue;

    while(1)
    {
		if(g_bExit)
		{
			break;
		}

        nRet = MV_CC_GetOneFrameTimeout(pUser, pData, nDataSize, &stImageInfo, 1000);
        if (nRet == MV_OK)
        {
            printf("GetOneFrame, Width[%d], Height[%d], nFrameNum[%d]\n", 
                stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);
        }
        else{
            printf("No data[%x]\n", nRet);
        }
    }

    free(pData);
    return 0;
}

int main(int argc, char const* argv[]) {
  int nRet = MV_OK;
  void* handle = NULL;

  MV_CC_DEVICE_INFO_LIST stDeviceList;
  memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));








  TF_Buffer* graph_def = read_file("./model/model.pb");                      
  TF_Graph* graph = TF_NewGraph();
  TF_Status* status = TF_NewStatus();                                                     
  TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();                        
  TF_GraphImportGraphDef(graph, graph_def, opts, status);
  TF_DeleteImportGraphDefOptions(opts);
  if (TF_GetCode(status) != TF_OK) {
          fprintf(stderr, "ERROR: Unable to import graph %s\n", TF_Message(status));        
          return 1;
  }       
  fprintf(stdout, "Successfully imported graph\n");                                          
  TF_SessionOptions* opt = TF_NewSessionOptions();
  TF_Session* sess = TF_NewSession(graph, opt, status);
  //TF_DeleteSessionOptions(opt);
  if (TF_GetCode(status) != TF_OK) {
    fprintf(stderr, "ERROR: Unable to create session %s\n", TF_Message(status));
    return 1;
  }
std::cout<<"session created"<< std::endl;

std::cout<<"czes after load img"<< std::endl;
    IplImage* img1;
    // img1 = cvLoadImage("./photos/26.jpg", CV_LOAD_IMAGE_COLOR);
    // if (!img1)
    // {
    //   printf("Image can NOT Load!!!\n");
    //   return 1;
    // }




    cv::Mat image = cv::imread("sample.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file

    // if(! image.data )                              // Check for invalid input
    // {
    //     std::cout <<  "Could not open or find the image" << std::endl ;
    //     return -1;
    // }
    // namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
    // cv::imshow( "Display window", image );                   // Show our image inside it.

    // cv::waitKey(0); 



  std::cout<<"check1"<< std::endl;

  TF_Operation* image_tensor = TF_GraphOperationByName(graph, "image_tensor");
  TF_Operation* detection_boxes = TF_GraphOperationByName(graph, "detection_boxes");
  TF_Operation* detection_scores = TF_GraphOperationByName(graph, "detection_scores");
  TF_Operation* detection_classes= TF_GraphOperationByName(graph, "detection_classes");
  TF_Operation* num_detections = TF_GraphOperationByName(graph, "num_detections");
    std::cout<<"check2"<< std::endl;

  if (!image_tensor && !detection_boxes && detection_scores && detection_classes && num_detections){
    printf("Image_tensor not loaded\n");
    return 0;
  }
    std::cout<<"check3"<< std::endl;

  int64_t dims[] = {1, img1->width, img1->height, num_channels };

  //---------------------------------------------------------

inputTensor_ = TF_AllocateTensor(
        TF_FLOAT,
        input_dims_.data(),
        static_cast<int>(input_dims_.size()),
        imgS_ * sizeof(float)
);
if (inputTensor_ == nullptr) {
    std::cout << "Can't allocate tensor" << std::endl;
    exit(1);
}

tensorData_ = TF_TensorData(inputTensor_);
if (tensorData_ == nullptr) {
    std::cout << "Can't get tensorData_" << std::endl;
    TF_DeleteTensor(inputTensor_);
    exit(1);
}





        cv::Mat img = cv::imread("./photos/1.bmp");
        if (img.cols != imgH_ || img.rows != imgW_) {
            cv::resize(img, img, cv::Size(imgW_, imgH_));
        }

        for (int i=0; i<imgS_; ++i) {
            ((float*)tensorData_)[i] = (float)img.data[i];
        }

        TF_Tensor* outputTensor = nullptr;

        // TF_Status* status = TF_NewStatus();
        TF_SessionOptions* options = TF_NewSessionOptions();
        TF_Session* sess2 = TF_NewSession(graph, options, status);
        TF_DeleteSessionOptions(options);

        if (TF_GetCode(status) != TF_OK) {
            TF_DeleteStatus(status);
            return false;
        }

        TF_SessionRun(sess2,
                      nullptr, // Run options.
                      &inputOp_, &inputTensor_, 1, // Input tensors, input tensor values, number of inputs.
                      &outputOp_, &outputTensor, 1, // Output tensors, output tensor values, number of outputs.
                      nullptr, 0, // Target operations, number of targets.
                      nullptr, // Run metadata.
                      status // Output status.
        );

        // if (TF_GetCode(status) != TF_OK) {
        //     std::cout << "Error run session";
        //     TF_DeleteStatus(status);
        //     return false;
        // }

        // TF_CloseSession(sess2, status);
        // if (TF_GetCode(status) != TF_OK) {
        //     std::cout << "Error close session";
        //     TF_DeleteStatus(status);
        //     return false;
        // }

        // TF_DeleteSession(sess2, status);
        // if (TF_GetCode(status) != TF_OK) {
        //     std::cout << "Error delete session";
        //     TF_DeleteStatus(status);
        //     return false;
        // }

        // const auto data = static_cast<float*>(TF_TensorData(outputTensor));

        // std::cout << "Output size: " << TF_TensorByteSize(outputTensor) << std::endl;
        // std::cout << "Output vals: " << data[0] << ", " << data[1] << std::endl;

        // TF_DeleteTensor(outputTensor);
        // TF_DeleteStatus(status);


  //----------------

  std::cout<<"check4"<< std::endl;

  TF_Tensor* imgTensor = TF_NewTensor(TF_FLOAT, dims, 4, img1, img1->imageSize * sizeof(float), &deallocator, NULL);

    std::cout<<"check5"<< std::endl;

  TF_Operation* input_image_op = TF_GraphOperationByName(graph, "image_tensor");
  if(!input_image_op) {
    printf("Failed to find Op '%s'", "image_tensor");
    return 1;
  }

    std::cout<<"check6"<< std::endl;

  TF_Output image_input;
  image_input.oper = input_image_op;
  image_input.index = 0;
  TF_Output inputs[1] = {image_input};

    std::cout<<"check7"<< std::endl;

  TF_Operation* num_detection_op = TF_GraphOperationByName(graph, "detection_boxes");
  if(!num_detection_op) {
    printf("Failed to find Op '%s'", "num_detections");
    return 1;
  }

  TF_Output num_detection_output;
  num_detection_output.oper = num_detection_op;
  num_detection_output.index = 0;

  TF_Output outputs[1] = {num_detection_output};
  TF_Tensor* output_tensors[1];

  TF_SessionRun(sess, NULL, 
                &image_input, &imgTensor, 1,
                outputs, output_tensors, 1,
                NULL, 0,
                NULL, status);

  // TF_DeviceList* TF_SessionListDevices(sess, status);   

  // printf("%p",&TF_SessionListDevices);           

  if(TF_GetCode(status) != TF_OK) {
    printf("It's coming here");
    printf("%s\n", TF_Message(status));
  }
  else {

    printf("Ran successfully\n");
    float* f = (float*)TF_TensorData(output_tensors[0]);
    printf("TF data %f\n", f[0]);

  }


  cvShowImage("Face Detection", img1);
  cvWaitKey(0);
  cvReleaseImage(&img1);

  TF_CloseSession(sess, status);
  TF_DeleteSession(sess, status);

  TF_DeleteStatus(status);
  TF_DeleteBuffer(graph_def);

  TF_DeleteGraph(graph);


  free((void*)img1);

  return 0;
}

TF_Buffer* read_file(const char* file) {                                                  
  FILE *f = fopen(file, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);                                                                  
  fseek(f, 0, SEEK_SET);  //same as rewind(f);                                            

  void* data = malloc(fsize);                                                             
  fread(data, fsize, 1, f);
  fclose(f);

  TF_Buffer* buf = TF_NewBuffer();                                                        
  buf->data = data;
  buf->length = fsize;                                                                    
  buf->data_deallocator = free_buffer;                                                    
  return buf;
} 

 
//  Test main function: 
// int main( int argc, char** argv ) {
  
//   cv::Mat image;
//   image = cv::imread("sample.jpg" , CV_LOAD_IMAGE_COLOR);

//   std::cout<<"Hello from TensorFlow C library version %s\n"<< TF_Version()<< std::endl;

  
//   if(! image.data ) {
//       std::cout <<  "Could not open or find the image" << std::endl ;
//       return -1;
//     }
  
//   cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );
//   cv::imshow( "Display window", image );
  
//   cv::waitKey(0);
//   return 0;
// }


