#include <string>
#include "libraw/libraw.h"
#include <Core/BlockObject.h>
#include <Core/Bitmap.h>
#include <Core/ATData.h>

#include "opencv2/opencv.hpp"
inline cv::Scalar BGR2YCrCb(const cv::Scalar& bgr)
{
    const auto B = bgr[0];
    const auto G = bgr[1];
    const auto R = bgr[2];
    const auto delta = 128.0;
    const auto Y = 0.299 * R + 0.587 * G + 0.114 * B;
    const auto Cb = (B - Y) * 0.564 + delta;
    const auto Cr = (R - Y) * 0.713 + delta;
    return cv::Scalar(Y,Cr,Cb);
}

inline cv::Scalar YCrCb2BGR(const cv::Scalar& YCB)
{
    const auto Y = YCB[0];
    const auto Cr = YCB[1];
    const auto Cb = YCB[2];
    const auto delta = 128.0;
    
    const auto B = (Cb - 0.5) * 1. / 0.564 + Y;
    const auto R = (Cr - 0.5) * 1. / 0.713 + Y;
    const auto  G = 1. / 0.587 * (Y - 0.299 * R - 0.114 * B);
    return cv::Scalar(B, G, R);
}

int main(int argc, char* argv[]){

    LibRaw libraw;
    int ret = libraw.open_file("E:/TestData/renti/1/N001.CR2");

    if (ret != int(LIBRAW_SUCCESS))
    {
        return -1;
    }
    libraw.imgdata.params.use_camera_wb = 1;
    ret = libraw.unpack();
    ret = libraw.dcraw_process();
    libraw_processed_image_t* image_temp = libraw.dcraw_make_mem_image(&ret);
    cv::Mat image = cv::Mat(libraw.imgdata.sizes.height, libraw.imgdata.sizes.width, CV_8UC3, image_temp->data);
    cv::imwrite("E:/TestData/renti/cv1_jpg/N001.jpg", image);

    cv::Mat img_data = cv::imread("E:/TestData/renti/1/N001.CR2",cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
    cv::imwrite("E:/TestData/renti/cv1_jpg/N001.jpg", img_data);
        AI3D::CORE::BlockObject block;
        auto atdata = std::make_shared<AI3D::CORE::ATData>();
        std::string file = argv[1];// "D:/TestData/wutestxml/block_AT_7200.xml";
        block.LoadATXML(file, atdata);

        //time.PrintSeconds();
        block.SetATData(atdata);

        for (auto iter : atdata->GetImages())
        {
            AI3D::CORE::Image image =iter.second;
            AI3D::CORE::Bitmap bitmap;
            bitmap.Read(image.GetPath() + "/" + image.GetName());
            auto p0 = iter.second.GetColorParam()[0];
            auto p1 = iter.second.GetColorParam()[1];
            auto p2 = iter.second.GetColorParam()[2];
            for (int y = 0; y < bitmap.GetHeight(); ++y)
            {
                for (int x = 0; x < bitmap.GetWidth(); ++x)
                {
                    AI3D::CORE::BitmapColor<uint8_t> color;

                    //InterpolateNearestNeighbor
                    bitmap.GetPixel(x, y, &color);
                    color.b *= p0 / 4.0;
                    color.g *= p1;
                    color.r *= p2;
                   /* uint8_t BGR[3];
                    BGR[0] = color.b;
                    BGR[1] = color.g;
                    BGR[2] = color.r;

                    cv::Scalar colorbgr(BGR[0], BGR[1], BGR[2]);
                    cv::Scalar BGR1 = BGR2YCrCb(colorbgr);
                    BGR1[0] *= p0;
                    BGR1[1] *= p1;
                    BGR1[2] *= p2;
                    cv::Scalar newcolor = YCrCb2BGR(BGR1);
                    color.b = newcolor[0];
                    color.g = newcolor[1];
                    color.r = newcolor[2];*/
                    bitmap.SetPixel(x, y, color);
                }

            }

            
            bitmap.Write(image.GetPath() + "/" + image.GetName() + "BGR4.jpg", FREE_IMAGE_FORMAT::FIF_JPEG);
        }
        return 0;
    }