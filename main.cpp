//跟着阳光非宅男学习存储图片,输出为jpg格式  拉流存图片
#include <iostream>
#include <stdio.h>//
#include <Windows.h>//

using namespace std;

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
 #include "libavdevice/avdevice.h"//
}
/*由于我们建立的是C++的工程
 *编译的时候使用的C++的编译器编译
 *而FFMPEG是C的库
 *因此这里需要加上extern "C"
 *否则会提示各种未定义
 */

///现在我们需要做的是让SaveFrame函数能把RGB信息定稿到一个PPM格式的文件中。
///我们将生成一个简单的PPM格式文件，请相信，它是可以工作的。
void SaveFrame(AVFrame *pFrame, int width, int height,int index)
{

    FILE *pFile;
    char szFilename[32];
    int  y;

    // Open file
    sprintf(szFilename, "frame%d.ppm", index);
    pFile=fopen(szFilename, "wb");

    if(pFile==nullptr)
        return;

    // Write header
    fprintf(pFile, "P6 %d %d 255 ", width, height);

    // Write pixel data
    for(y=0; y<height; y++)
    {
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
    }

    // Close file
    fclose(pFile);

}

/*==================================================================================
 *                  将AVFrame(YUV420格式)保存为JPEG格式的图片
 ===================================================================================*/
int SaveAsJPEG(AVFrame* pFrame, int width, int height, int index)
{
    // 输出文件路径
    char out_file[MAX_PATH] = {0};
    sprintf_s(out_file, sizeof(out_file), "%s%d.jpg", "D:/Qtlearn/202107074/bin64/", index);


    // 分配AVFormatContext对象
    AVFormatContext* pFormatCtx = avformat_alloc_context();

    // 设置输出文件格式
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);

    // 创建并初始化一个和该url相关的AVIOContext
    if( avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0)
    {
        printf("Couldn't open output file.");
        return -1;
    }

    // 构建一个新stream
    AVStream* pAVStream = avformat_new_stream(pFormatCtx, 0);
    if( pAVStream == NULL )
    {
        return -1;
    }

    // 设置该stream的信息
    AVCodecContext* pCodecCtx = pAVStream->codec;

    pCodecCtx->codec_id   = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt    = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width      = width;
    pCodecCtx->height     = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    //打印输出相关信息
    av_dump_format(pFormatCtx, 0, out_file, 1);


    //================================== 查找编码器 ==================================//
    AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if( !pCodec )
    {
        printf("Codec not found.");
        return -1;
    }

    // 设置pCodecCtx的解码器为pCodec
    if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 )
    {
        printf("Could not open codec.");
        return -1;
    }

    //================================Write Header ===============================//
    avformat_write_header(pFormatCtx, NULL);

    int y_size = pCodecCtx->width * pCodecCtx->height;

    //==================================== 编码 ==================================//
    // 给AVPacket分配足够大的空间
    AVPacket pkt;
    av_new_packet(&pkt, y_size * 3);

    //
    int got_picture = 0;
    int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
    if( ret < 0 )
    {
        printf("Encode Error.\n");
        return -1;
    }
    if( got_picture == 1 )
    {
        pkt.stream_index = pAVStream->index;
        ret = av_write_frame(pFormatCtx, &pkt);
    }

    av_free_packet(&pkt);

    //Write Trailer
    av_write_trailer(pFormatCtx);


    if( pAVStream )
    {
        avcodec_close(pAVStream->codec);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    return 0;
}

int main()
{
    av_register_all(); //初始化FFMPEG  调用了这个才能正常适用编码器和解码器
    //=========================== 创建AVFormatContext结构体 ===============================//
        //分配一个AVFormatContext，FFMPEG所有的操作都要通过这个AVFormatContext来进行
        AVFormatContext *pFormatCtx = avformat_alloc_context();
     //==================================== 打开文件 ======================================//
        const char *file_path = "D:\\in.mp4";//输入视频文件
        int ret = avformat_open_input(&pFormatCtx, file_path, NULL, NULL);
        if(ret != 0)
        {
            cout << "open error!" << endl;
            return -1;
        }




    //循环查找视频中包含的流信息，直到找到视频类型的流
        //便将其记录下来 保存到videoStream变量中
        int i;
        int videoStream;

        //=================================== 获取视频流信息 ===================================//
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
        {
            cout << "Could't find stream infomation." << endl;
            return -1;
        }

        videoStream = -1;

        for (i = 0; i < pFormatCtx->nb_streams; i++)
        {
            if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                videoStream = i;
            }
        }

        //如果videoStream为-1 说明没有找到视频流
        if (videoStream == -1)
        {
            cout << "Didn't find a video stream." << endl;
            return -1;
        }




     //=================================  查找解码器 ===================================//
        AVCodecContext* pCodecCtx = pFormatCtx->streams[videoStream]->codec;
        AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL)
        {
            cout << "Codec not found." << endl;
            return -1;
        }




     //================================  打开解码器 ===================================//
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)// 具体采用什么解码器ffmpeg经过封装  我们无须知道
        {
            cout << "Could not open codec." << endl;
            return -1;
        }




     //================================ 设置数据转换参数 ================================//
        SwsContext * img_convert_ctx;
        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, //源地址长宽以及数据格式
                                         pCodecCtx->width, pCodecCtx->height,AV_PIX_FMT_YUVJ420P,    //目的地址长宽以及数据格式
                                         SWS_BICUBIC, NULL, NULL, NULL);//算法类型  AV_PIX_FMT_YUVJ420P   AV_PIX_FMT_BGR24





    //==================================== 分配空间 ==================================//
        //一帧图像数据大小
        int numBytes = avpicture_get_size(AV_PIX_FMT_YUVJ420P, pCodecCtx->width,pCodecCtx->height);

        unsigned char *out_buffer;
        out_buffer = (unsigned char *) av_malloc(numBytes * sizeof(unsigned char));

        AVFrame * pFrame;
        pFrame = av_frame_alloc();
        AVFrame * pFrameRGB;
        pFrameRGB = av_frame_alloc();
        avpicture_fill((AVPicture *) pFrameRGB, out_buffer, AV_PIX_FMT_YUVJ420P,pCodecCtx->width, pCodecCtx->height);
        //会将pFrameRGB的数据按RGB格式自动"关联"到buffer  即pFrameRGB中的数据改变了 out_buffer中的数据也会相应的改变





    //=========================== 分配AVPacket结构体 ===============================//
        int y_size = pCodecCtx->width * pCodecCtx->height;
        AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个packet
        av_new_packet(packet, y_size); //分配packet的数据

    //视频文件的MetaData的查看, av_dump_format（）函数会自动打印MetaData信息
    av_dump_format(pFormatCtx, 0, file_path, 0);

    int index = 0;

    //读取码流中的音频若干帧或者视频一帧，填入到packet中
    while (av_read_frame(pFormatCtx, packet) >= 0)
    {
        if (packet->stream_index == videoStream) {
            //作用是解码一帧视频数据。输入一个压缩编码的结构体AVPacket，输出一个解码后的结构体AVFrame

            /*ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
            if (ret < 0) {
                printf("decode error.");
                return -1;
            }
            if (got_picture) {
                sws_scale(img_convert_ctx,
                        (uint8_t const * const *) pFrame->data,
                        pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                        pFrameRGB->linesize);
                SaveFrame(pFrameRGB, pCodecCtx->width,pCodecCtx->height,index++); //保存图片
                if (index > 50) return 0; //这里我们就保存50张图片
            }
        }过时*/



            if (avcodec_send_packet(pCodecCtx,packet) == 0)
            {
                // 一个avPacket可能包含多帧数据，所以需要使用while循环一直读取
                while (avcodec_receive_frame(pCodecCtx,pFrame) == 0)
                {
                    sws_scale(img_convert_ctx,
                              (uint8_t const * const *) pFrame->data,
                              pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                              pFrameRGB->linesize);

                    //SaveFrame(pFrameRGB, pCodecCtx->width,pCodecCtx->height,index++); //保存图片
                    SaveAsJPEG(pFrameRGB, pCodecCtx->width,pCodecCtx->height,index++); //保存图片
                    if (index > 50) return 0; //这里我们就保存50张图片

                }
            }
            else
            {
                printf("Error sending a packet for decoding");
                return -1;
            }

        }


        //av_free_packet(packet);已经过时
        av_packet_unref(packet);
    }
    av_free(out_buffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    av_packet_free(&packet);
    avformat_close_input(&pFormatCtx);

    return 0;
}
