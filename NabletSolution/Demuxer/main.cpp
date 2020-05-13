#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string>
#include <iostream>


#include <math.h>
#include <sys/types.h>

#include <stdint.h>

#include "../include/nb_mp4_demuxer.h"
#pragma comment(lib, "../lib/bin/nb_mp4demux.lib")

FILE * output1;
FILE * output2;

static void data_send(void *app,  void * sample_info)
{
    if (sample_info)
    {
        sample_info_t *info = (sample_info_t*) sample_info;

        printf ("%I64d\n", info->start_time / 2700000);

        if (info->ID == 1)
        {
            if (output1 == NULL)
                output1 = fopen("C:\\NabletSolution\\Debug\\dump.tmp","wb");

            fwrite(info->data, info->sample_size, 1, output1);
        }
        else if (info->ID == 2)
        {
            if (output2 == NULL)
                output1 = fopen("C:\\NabletSolution\\Debug\\dump0.tmp", "wb");

            fwrite(info->data, info->sample_size, 1, output2);
        }
    }
}

static void * EXPORT_API callback(void * app, const char* name)
{
    if(!strcmp(name,"data_send_ex"))
        return (void*)data_send;
    return NULL;
}


int main(int argc, char * argv[])
{
     nb_mp4_demux_file_info file_info;
     nb_mp4_demux_track_format_t track_info;
     nb_mp4_demux_settings settings;
     nb_mp4_demux_seek_info seek_info;

     void * app = (void *)0x12345; // TEST
   
    // create demuxer
    nb_mp4_demuxer_tt * demuxer = NB_MP4DemuxCreate(callback, app);

    if (demuxer == NULL)
        return -1;

    if (argc != 3)
    {
        printf("*************************\n");
        printf(" demuxer\n");
        printf(" Copyright(c) 2020 Tecom\n");
        printf(" www.tecomgroup.ru\n");
        printf("*************************\n");
        printf("usage:\n");
        printf("%s <source.mp4> trackID\n", strrchr(argv[0], '\\') + 1);
        printf("    trackID - track number (0,1,2..31)\n");
        return 1;
    }

    NB_MP4DemuxOpenInput(demuxer, argv[1]); /*Need full name*/
    NB_MP4DemuxGetFileInfo(demuxer, &file_info); 
    if (file_info.stream_count == 0 || file_info.stream_count > 31)
    {
        printf("Source file has no media");
        return 2;
    }

    /* file_info.stream_count - number of tracks */

    int trackID = atoi(argv[2]);
    NB_MP4DemuxTrackInfo(demuxer, &track_info, trackID); //uses 1 track  !!! 1-0

    // open both tracks for demuxing
    settings.add_adts_headers = 1;          //kostiv was 1  !!! 1-0
    settings.annexb_output = 1;
    NB_MP4OpenTrack(demuxer, settings, trackID);  //kostiv was 0  !!! 1-0 TRACK

        // seek to the beginning of the file
    memset(&seek_info, 0, sizeof(nb_mp4_demux_seek_info));
    seek_info.parser_num = 1;
    seek_info.stream_num = trackID; //kostiv was 0  !!! 1-0
    seek_info.seek_sample = 100;
    if (NB_MP4DemuxSeek(demuxer, &seek_info))
        return -1;

    int track1 = 0;
    while (true)
    {
        if (track1 == 0)
            track1 = NB_MP4DemuxTrack(demuxer, 0);  //kostiv must be 0

        if (track1 == 1)
            break; // error
    }

    NB_MP4DemuxDestroy(demuxer);
    system("Pause");

    return 0;
}