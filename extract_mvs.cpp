/*
 * Copyright (c) 2012 Stefano Sabatini
 * Copyright (c) 2014 Clément Bœsch
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 *Modified by: Jishnu Jaykumar Padalunkal
 *Modified on: 26 Apr 2018

 *Modified by: Michael Gerstenberger
 *Modified on: 28 Dec 2018
*/

extern "C"{//This part is executed as if it was C code.
  #include "include/libavutil/motion_vector.h"
  #include "include/libavformat/avformat.h"
  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>
  #include <sys/stat.h>


  static AVFormatContext *fmt_ctx = NULL;
  static AVCodecContext *video_dec_ctx = NULL;
  static AVStream *video_stream = NULL;
  static const char *src_filename = NULL;

  static int video_stream_idx = -1;
  static AVFrame *frame = NULL;
  static int video_frame_count = 0;
}

#include "save_hdf5.h"
int output_width = 0;//!!!! for saving hdf5
int output_height = 0;
SaveHdf5 * saver;

static int decode_packet(const AVPacket *pkt)
{/*
    Decodes the vectors for each frame.
  */
    int ret = avcodec_send_packet(video_dec_ctx, pkt);
    char szFileName[255] = {0};
    FILE *file=NULL;

    int dx = 0;
    int dy = 0;
    int src_x = -1;
    int src_y = -1;

    if (ret < 0) {
        fprintf(stderr, "Error while sending a packet to the decoder");
        return ret;
    }

    while (ret >= 0)  {
        ret = avcodec_receive_frame(video_dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            fprintf(stderr, "Error while receiving a frame from the decoder");
            return ret;
        }

        if (ret >= 0) {
            int i, my_iter;
            AVFrameSideData *sd;
	          video_frame_count++;

            v3d current_frame(output_height, v2d(output_width, v1d(2, 0)));//allocate memory for current frame


            sd = av_frame_get_side_data(frame, AV_FRAME_DATA_MOTION_VECTORS);
            if (sd) {
                const AVMotionVector *mvs = (const AVMotionVector *)sd->data;
                for (i = 0; i < sd->size / sizeof(*mvs); i++) {
                    const AVMotionVector *mv = &mvs[i];

                    if(i==(sd->size / sizeof(*mvs))-1){
                      if(mv->source<0){
                        src_x = (mv->src_x)/abs(mv->source); //source relative to source
                        src_y = (mv->src_y)/abs(mv->source);
                        dx = (mv->dst_x - mv->src_x)/abs(mv->source);
                        dy = (mv->dst_y - mv->src_y)/abs(mv->source);
                      }else{
                        src_x = mv->dst_x/abs(mv->source); //source relative to destination
                        src_y = mv->dst_y/abs(mv->source);
                        dx = (mv->src_x - mv->dst_x)/abs(mv->source);
                        dy = (mv->src_y - mv->dst_y)/abs(mv->source);
                      }
                    }else{
                      if(mv->source<0){
                        src_x = mv->src_x/abs(mv->source);
                        src_y = mv->src_y/abs(mv->source);
                        dx = (mv->dst_x - mv->src_x)/abs(mv->source);
                        dy = (mv->dst_y - mv->src_y)/abs(mv->source);
                      }else{
                        src_x = mv->dst_x/abs(mv->source);
                        src_y = mv->dst_y/abs(mv->source);
                        dx = (mv->src_x - mv->dst_x)/abs(mv->source);
                        dy = (mv->src_y - mv->dst_y)/abs(mv->source);
                      }
                    }

                    int x_max = current_frame[0].size();
                    int y_max = current_frame.size();

                    int vector_x = src_x/16;
                    int vector_y = src_y/16;

                    if(vector_x >= 0 && vector_x < x_max && vector_y >=0  && vector_y < y_max){
                      current_frame[vector_y][vector_x][0] = dx;
                      current_frame[vector_y][vector_x][1] = dy;
                    };

                }
          }
	          printf("\rTotal Processed Frames:%d", video_frame_count);
	          fflush(stdout);

            av_frame_unref(frame);
            saver->append(current_frame);
        }
    }
    return 0;
}/////////////////////////////////////////////////////////////////

static int open_codec_context(AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret;
    AVStream *st;
    AVCodecContext *dec_ctx = NULL;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        return ret;
    } else {
        int stream_idx = ret;
        st = fmt_ctx->streams[stream_idx];

        dec_ctx = avcodec_alloc_context3(dec);
        if (!dec_ctx) {
            fprintf(stderr, "Failed to allocate codec\n");
            return AVERROR(EINVAL);
        }

        ret = avcodec_parameters_to_context(dec_ctx, st->codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters to codec context\n");
            return ret;
        }

        /* Init the video decoder */
        av_dict_set(&opts, "flags2", "+export_mvs", 0);
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }

        video_stream_idx = stream_idx;
        video_stream = fmt_ctx->streams[video_stream_idx];
        video_dec_ctx = dec_ctx;
    }

    return 0;
}

void extract_motion_vectors(char *videopath, char *outpath){
    int ret = 0;
    AVPacket pkt = { 0 };
    struct stat sb;

    src_filename = videopath;

    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    open_codec_context(fmt_ctx, AVMEDIA_TYPE_VIDEO);

    av_dump_format(fmt_ctx, 0, src_filename, 0);

    if (!video_stream) {
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

    frame = av_frame_alloc();

    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    printf("\n");
    printf("**************************************************************************************\n");
    printf("*       Tool : Dense Flow Decoder                                                    *\n");
    printf("*     Author : Michael Gerstenberger (see copyright for version history and authors) *\n");
    printf("*  Used Libs : FFmpeg, HDF5                                                          *\n");
    printf("*Description : A tool to extract motion vectors from H264 videos and save them as    *\n");
    printf("             : a tensor of size n_frames x width x height x 2 (for dx and dy) in     *\n");
    printf("             : HDF5 format (One vector for each 16x16 pixels macroblock. Vectors for *\n");
    printf("             : 8x8 blocks are summerized as this yields a more dense representation. *\n");
    printf("             : The unit of dx and dy are pixels of the original frames)              *\n");
    printf("*      Input : Video encoded in H264. Other formats may work as well.                *\n");
    printf("*     Output : HDF5 file named motion_vectors.h5 containing motion tensor at node    *\n");
    printf("             : motion_tensor.                                                        *\n");
    printf("*      Usage: ./decode_motion filepath_to_video.mp4                                 *\n");
    printf("**************************************************************************************\n");
    printf("\n");
    printf("--------------------------------------------------------------------------------------\n");
    //printf("framenum,source,blockw,blockh,srcx,srcy,dstx,dsty,motion_x,motion_y,motion_scale,flags\n");
    printf("\n* video width and height:  %d\t", video_dec_ctx->width);
    printf("%d\n", video_dec_ctx->height);
    printf("\n* output width and height:  %d\t", video_dec_ctx->width/16);
    printf("%d\n", video_dec_ctx->height/16);
    printf("--------------------------------------------------------------------------------------\n");

    //Initialize saver for HDF5
    saver = new SaveHdf5(outpath,"motion_tensor");//Set global variable
    output_width = video_dec_ctx->width/16;//Set global variable
    output_height = video_dec_ctx->height/16;
    saver->init(output_height,output_width);//!!!

    /* read frames from the file */
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_stream_idx)
            ret = decode_packet(&pkt);
        av_packet_unref(&pkt);
        if (ret < 0)
            break;
    }

    /* flush cached frames */
    decode_packet(NULL);
    printf("\n--------------------------------------------------------------------------------------\n");

end:
    avcodec_free_context(&video_dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);
    return ret < 0;
}

int main(int argc, char **argv)
{
  if(argc == 3){
	  extract_motion_vectors(argv[1], argv[2]);
  }else{
    printf("Provide input filename and output filename.");
  }
}
