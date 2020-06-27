#include "Utils.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <opencv/cv.h>

#include <iostream>
#include <tensorflow/c/c_api.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <thread>
#include <string>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <stdbool.h>

#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

const char * ipAddress = "192.168.1.10";
int portNumber = 10000;


/*
-----------------------------TENSORFLOW--------------------------------------------------------------------------------------
*/
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

bool configure(){
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

  // Get pointer to input tensor data
  tensorData_ = TF_TensorData(inputTensor_);
  if (tensorData_ == nullptr) {
      std::cout << "Can't get tensorData_" << std::endl;
      TF_DeleteTensor(inputTensor_);
      exit(1);
  }

  // Load tensorflow model
  std::string modelFile = "models/correct/model.pb";
  graph_ = utils::LoadGraphDef(modelFile.c_str());
  if (graph_ == nullptr) {
      std::cout << "Can't load graph" << std::endl;
      return 0;
  }

  // Prepare input
  inputOp_ = {TF_GraphOperationByName(graph_, "Placeholder"), 0};
  if (inputOp_.oper == nullptr) {
      std::cout << "Can't init input_op" << std::endl;
      return 0;
  }

  // Prepare output
  outputOp_ = {TF_GraphOperationByName(graph_, "loss"), 0};
  if (outputOp_.oper == nullptr) {
      std::cout << "Can't init output_op" << std::endl;
      return 0;
  }
  return true;
}

bool predict(cv::Mat _image, bool& _result){
        if (_image.cols != imgH_ || _image.rows != imgW_) {
            cv::resize(_image, _image, cv::Size(imgW_, imgH_));
        }


        for (int i=0; i<imgS_; ++i) {
            ((float*)tensorData_)[i] = (float)_image.data[i];
        }
        TF_Tensor* outputTensor = nullptr;
        //TF_Tensor* outputTensor;
        TF_Status* status = TF_NewStatus();
        TF_SessionOptions* options = TF_NewSessionOptions();
        TF_Session* sess = TF_NewSession(graph_, options, status);
        TF_DeleteSessionOptions(options);
        if (TF_GetCode(status) != TF_OK) {
            TF_DeleteStatus(status);
            return false;
        }
            // auto start = std::chrono::high_resolution_clock::now(); 
        TF_SessionRun(sess,
                      nullptr, // Run options.
                      &inputOp_, &inputTensor_, 1, // Input tensors, input tensor values, number of inputs.
                      &outputOp_, &outputTensor, 1, // Output tensors, output tensor values, number of outputs.
                      nullptr, 0, // Target operations, number of targets.
                      nullptr, // Run metadata.
                      status // Output status.
        );
            // auto stop = std::chrono::high_resolution_clock::now(); 
            // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 
            // std::cout << "Czas miliseconds SESSION RUN: " << duration.count()/1000 << std::endl; 
        if (TF_GetCode(status) != TF_OK) {
            std::cout << "Error run session";

            std::cout<<TF_GetCode(status)<<std::endl;
            std::cout<<TF_Message(status)<<std::endl;
        
            TF_DeleteStatus(status);
            return false;
        }
        TF_CloseSession(sess, status);
        if (TF_GetCode(status) != TF_OK) {
            std::cout << "Error close session";
            TF_DeleteStatus(status);
            return false;
        }
        TF_DeleteSession(sess, status);
        if (TF_GetCode(status) != TF_OK) {
            std::cout << "Error delete session";
            TF_DeleteStatus(status);
            return false;
        }
        const auto data = static_cast<float*>(TF_TensorData(outputTensor));
        _result = true;
        if (100*data[0] >= 100*data[1]) {
            _result = false;
        }
        // std::cout << "Tensor::predict "<<" - Prediction: [[" << _result<<"]] Output vals: bad[[" << data[0] << "]], good[[" << data[1]<<"]]";
        TF_DeleteTensor(outputTensor);
        TF_DeleteStatus(status);

            // stop = std::chrono::high_resolution_clock::now(); 
            // duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 
            // std::cout << "Czas miliseconds WSZYSTKO: " << duration.count()/1000 << std::endl; 

        return true;

}

/*
-----------------------------IMG ENCODER--------------------------------------------------------------------------------------
*/

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    std::string base64_chars = 
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/";

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for(i = 0; (i <4) ; i++)
            ret += base64_chars[char_array_4[i]];
        i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
        char_array_3[j] = '\0';

        char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
        ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
        ret += '=';

    }

    return ret;

}

std::string encode_image(cv::Mat image){
    std::vector<uchar> buf;
    cv::imencode(".jpg", image, buf);
    auto *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
    std::string encoded = base64_encode(enc_msg, buf.size());
    return encoded;
}





/*
-----------------------------TCP CLIENT--------------------------------------------------------------------------------------
*/
struct sockaddr_in serwer =
{
    .sin_family = AF_INET,
    .sin_port = htons( 10000 )
};

int tcpConnect(){

    inet_pton( serwer.sin_family, ipAddress, & serwer.sin_addr );
    const int s = socket( serwer.sin_family, SOCK_STREAM, 0 );
    connect( s,( struct sockaddr * ) & serwer, sizeof( serwer ) );

    return s;
}



bool sendFrame(std::string imageString, const int &s){
    std::cout<<"Sending frame length: "<< imageString.length()<<std::endl;
    char buffer[ imageString.length()+1 ];
    strcpy(buffer,imageString.c_str());
    send( s, buffer, strlen( buffer ), 0 );

    // char recvBuffer[ 1024 ];
    // std::string confirmation = "true";
    // char confirm[confirmation.length()+1];
    // strcpy(confirm,confirmation.c_str());
    // if( recv( s, recvBuffer, sizeof( recvBuffer ), 0 ) > 0 ){
    //     std::cout<< "received  buffer length: "<< recvBuffer<< std::endl;
    //     if(std::to_string(imageString.length()) == std::string(recvBuffer)){
    //     //    
    //         std::cout<<"correctly sent";
    //         send( s, confirm, strlen( confirm ), 0 );
    //     }
    // }
    
        // puts( buffer );
        // if( entireWebsidedLoaded( buffer ) )
        // {
        //     strcpy( buffer, "^]" ); /// exit character
        //     send( s, buffer, strlen( buffer ), 0 );
        //     break;
        // }


}

bool tcpClose( const int &s){
    shutdown( s, SHUT_RDWR );
}


/*
-----------------------------MAIN--------------------------------------------------------------------------------------
*/
int main(int argc, char const* argv[]) {

    std::cout<<"Tcp connecting... "<<std::endl;
    const int s = tcpConnect();
    std::cout<<"Succesfully connected to server... "<<std::endl;
    
    std::cout<<"Tensorflow configuring... "<<std::endl;
    bool configured = configure();
    std::cout<<"Succesfully configured tensorflow... "<<std::endl;

    // cv::VideoCapture cap("rtsp://192.168.1.51:554/ch0_0.h264");
    // if(!cap.isOpened())  // check if we succeeded
    //   return -1;

    std::cout<<"Connecting to camera... "<<std::endl;
    cv::VideoCapture cap2(0);
    if(!cap2.isOpened())  // check if we succeeded
        return -1;
    std::cout<<"Succesfully connected to camera... "<<std::endl;

    cv::Mat newframe;
    std::string encoded_image;

    cv::namedWindow("edges",1);
    while(true){
        usleep(50000);

        cap2 >> newframe;
        
        cv::imshow("edges", newframe);
        cv::waitKey(200);
        cv::Mat dst;
        bool result;
        predict(newframe,result);
        std::cout<<"resutl: "<< result;
        if(result){
            std::cout<<" Sending photo to server...";
            encoded_image = encode_image(newframe);
            sendFrame(encoded_image,s);
        }else{
            std::cout<<std::endl;
            // encoded_image = encode_image(newframe);
            // sendFrame(encoded_image,s);
        }

        // cv::namedWindow("edges",1);
        // cv::imshow("edges", newframe);
        // cv::waitKey(20);
    }
    bool closed = tcpClose(s);
   
}





      


