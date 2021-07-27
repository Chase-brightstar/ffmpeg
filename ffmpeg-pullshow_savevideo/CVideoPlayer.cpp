//CVideoPlayer.cpp
#include "CVideoPlayer.h"
#include <QDebug>

CVideoPlayer::CVideoPlayer()
{

}

void CVideoPlayer::run()
{
    videoDecode();
}

void CVideoPlayer::videoDecode()
{

    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;
    AVPacket *packet;
    uint8_t *outBuffer;

    static struct SwsContext *img_convert_ctx;
    unsigned int i;
    int videoStream, numBytes;
    //int ret, got_picture;
    //av_register_all();//初始化ffmpeg 调用了这个才能正常适用编码器和解码器
    avformat_network_init(); // 初始化网络模块


    //Allocate an AVFormatContext.初始化上下文
    pFormatCtx = avformat_alloc_context();


   //const char *filePath = "D:\\in.mp4";
   char filePath[1024]="rtsp://admin:fzs123456@10.103.34.101:554/Streaming/Channels/101";
  // char filePath[1024]="rtmp://192.168.1.107:1935/live/home";

   //meng
    AVDictionary *options=NULL;
    av_dict_set(&options,"bufsize","6553600",0);
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    if(0 != avformat_open_input(&pFormatCtx, filePath, nullptr, &options))
    {
        emit signalDecodeError(-1);
        return;
    }



    if(avformat_find_stream_info(pFormatCtx, nullptr))
    {
        emit signalDecodeError(-2);
        return;
    }
    videoStream = -1;
    //循环查找视频中包含的流信息，直到找到视频类型的流
    //便将其记录下来 保存到videoStream变量中
    //这里我们现在只处理视频流  音频流先不管他
    for(i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        qDebug() << "pFormatCtx->streams[" << i << "]->codec->codec_type = " << pFormatCtx->streams[i]->codec->codec_type << endl;
        //0:视频类型 1:音频类型
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            videoStream = i;
    }
    qDebug() << "videoStream===========" << videoStream << "  pFormatCtx->nb_streams==" << pFormatCtx->nb_streams << endl;
    //如果videoStream为-1 说明没有找到视频流
    if(videoStream == -1)
    {
        emit signalDecodeError(-3);
        return;
    }
    //查找解码器
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    qDebug() << "pCodecCtx->codec_id===========" << pCodecCtx->codec_id << endl;
    //测试时这个值为27，查到枚举值对应的是AV_CODEC_ID_H264 ，即是H264压缩格式的文件。
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(nullptr == pCodec)
    {
        emit signalDecodeError(-4);
        return;
    }

    //打开解码器
    if(avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
    {
        emit signalDecodeError(-5);
        return;
    }




    //分配pFrame结构体
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    //保存文件
    FILE *fpSaveVideo = nullptr;
    fpSaveVideo = fopen("123.h264", "wb");

    //这里我们改成了 将解码后的YUV数据转换成RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, \
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, \
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, nullptr, nullptr, nullptr);

    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height);
    outBuffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *)pFrameRGB, outBuffer, AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height);
    int y_size = pCodecCtx->width * pCodecCtx->height;
    packet = (AVPacket *)malloc(sizeof(AVPacket));//分配一个packet
    av_new_packet(packet, y_size);//分配packet的数据
    av_dump_format(pFormatCtx, 0,  filePath, 0);//输出视频信息

    int index = 0;
    while (1)
    {
        if(av_read_frame(pFormatCtx, packet) < 0)
        {
            qDebug() << "index===============" << index;
            break;//这里认为视频读取完了
        }
        if(packet->stream_index == videoStream)
        {


            /*过时
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if(ret < 0)
            {
                emit signalDecodeError(-6);
                //return;
            }
            if(got_picture)
            {
                sws_scale(img_convert_ctx, (uint8_t const * const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                ++index;
                //把这个RGB数据 用QImage加载
                QImage tempImage((uchar*)outBuffer, pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
               // QImage image = tempImage.copy();//把图像复制一份 传递给界面显示
                qDebug() << "image.width==" << tempImage.width() << "image.height==" << tempImage.height();
                emit signalGetOneFrame(tempImage);
                //if (index > 10)
                //    return; //这里我们就保存10张图片
            }
            */



            if (avcodec_send_packet(pCodecCtx,packet) == 0)
            {
                // 一个avPacket可能包含多帧数据，所以需要使用while循环一直读取
                while (avcodec_receive_frame(pCodecCtx,pFrame) == 0)
                {
                    sws_scale(img_convert_ctx,
                              (uint8_t const * const *) pFrame->data,
                              pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                              pFrameRGB->linesize);
                    //把这个RGB数据 用QImage加载
                    QImage tempImage((uchar*)outBuffer, pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);
                   // QImage image = tempImage.copy();//把图像复制一份 传递给界面显示

                    qDebug() << "image.width==" << tempImage.width() << "image.height==" << tempImage.height();
                    emit signalGetOneFrame(tempImage);
                }
            }
            else
            {
                printf("Error sending a packet for decoding");
                return ;
            }


            //meng 此包数据是否为视频码流后写入本地文件
            fwrite(packet->data, 1, packet->size, fpSaveVideo);
                        fflush(fpSaveVideo);
        }
        av_packet_unref(packet);
    }

    // meng关闭文件
    if(fpSaveVideo)
    {
        fclose(fpSaveVideo);
    }
    fpSaveVideo = nullptr;


    av_free(outBuffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

}
