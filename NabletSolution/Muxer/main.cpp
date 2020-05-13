#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "../include/mp4_muxer_API.h"
#include "..//include/callback.h"
#pragma comment(lib, "../lib/bin/nb_mp4_muxer.lib")


typedef struct cmd_settings_s
{
    int input_streams;
    int output_format;
    char *output_filename;
    char *input_filenames[MAX_STREAMS];

} cmd_settings_t;

void sample_status_information(int status, void * app, const char * fmt, ...)
{
    char lst[256];
    va_list marker;

    va_start(marker,fmt);
    vsprintf(lst,fmt,marker);
    va_end(marker);

    printf("%s\n", lst);
}

static void parse_cmd(cmd_settings_t *cmd_settings, char *input)
{
    switch (*input)
    {
        case 'i':
        case 'I':
            if (cmd_settings->input_streams + 1 < MAX_STREAMS)
            {
                cmd_settings->input_filenames[cmd_settings->input_streams] = input + 1;
                cmd_settings->input_streams++;
            }
            else
            {
                printf("too many input streams, max is %d\n", MAX_STREAMS);
                exit(1);
            }
            break;

        case 'o':
        case 'O':
            cmd_settings->output_filename = input + 1;
            break;

        default:
            printf ("Unknown input param %s!\n", input);
            exit(1);
    }
}






int main(int argc, char * argv[])
{
    mp4_muxer_settings_t set;
    mp4_muxer_tt * mp4_muxer = nullptr;

    time_t start_time;
    time_t end_time;

    cmd_settings_t cmd_settings;

    memset(&cmd_settings, 0, sizeof(cmd_settings_t));

    if  (argc < 3) 
    { 
        printf("*************************\n");
        printf(" Muxer\n");
        printf(" Copyright(c) 2020 Tecom\n");
        printf(" www.tecomgroup.ru\n");
        printf("*************************\n");
        printf("usage:\n");
        printf("%s -i<input file> [+ -i<input file>] -o<output file>\n", strrchr(argv[0], '\\') + 1);
        printf("    input file - any of h264, acc files\n");
        printf("    output file - mp4 file\n");
        return 0;
    }

    // read cmd params
    for (int i = 1; i < argc; i++)
    {
        if (*(argv[i]) == '-')
        {
            parse_cmd(&cmd_settings, argv[i] + 1);
        }
        else
        {
            return 0;
        }
    }

    //zero set
    if (!mp4_muxer_defaults(&set, MP4_FORMAT))
    {
        printf("Failed to set defaults\n");
        return 1;
    }

    set.atom_order = 0;
    set.time_mode = 1;
    set.remove_filler_nalus = 0;
    set.mdat_size = 0;
    set.atom_order = 0;
    set.loging = 0;
    set.write_edts = 0;

    // dummy for testing, passed back due callback
    void * app = (void*)0x12345;

    // create muxer
    mp4_muxer = mp4_muxer_create(get_callback, &set, app);
    if (mp4_muxer == NULL)
    {
        printf("failed to create a muxer\n");
        return 1;
    }

    if (mp4_muxer_init(mp4_muxer, 0, cmd_settings.output_filename))
    {
        mp4_muxer_destroy(mp4_muxer);
        return 1;
    }
    

    int file_mode = 0; 
    // kostiv stream-file 0-1
    if (cmd_settings.input_streams > 0)
        file_mode = 1;

    int stream_id = 0;

    if (file_mode == 0)
    {
        return 0;
        mp4_muxer_add_stream(mp4_muxer); // add a stream, call this again for another stream in streaming mode
        mp4_muxer_add_stream(mp4_muxer);
    }
    else
    {
        // add streams 
        for (int i  = 0; i < cmd_settings.input_streams; i++)
        {
            if (mp4_muxer_add_input_file(mp4_muxer, cmd_settings.input_filenames[i]))
            {
                mp4_muxer_destroy(mp4_muxer);
                return 1;
            }
        }
    }
  
    
    time (&start_time);
    //mp4_muxer_set_language(mp4_muxer, 0x15C7, 0);
    //mp4_muxer_set_track_id(mp4_muxer, 1);
    printf ("start muxing\n");

    if (file_mode == 0)
    {
        sample_struct_t sample;
        memset (&sample, 0, sizeof (sample_struct_t));

        // add 10 samples
        for (int i = 0; i < 10; i++)
        {
            /* dummy code
            sample.sample_size = 10000 set size of sample
            sample.start_time = x; // 10 MHz clocks
            mp4_muxer_add_sample(mp4_muxer, &sample);
            NOTE : DO NOT CHANGE THE SAMPLE ID, SET BY MUXER AND INDICATES THE STREAM ID
            */
        }
    }
    else // file mode
    {
        // muxing
        mp4_muxer_process(mp4_muxer);
    }


    time (&end_time);
    // cleanup
    if (mp4_muxer)
    {
        mp4_muxer_done(mp4_muxer);
        mp4_muxer_destroy(mp4_muxer);
        mp4_muxer = NULL;
    }

    printf ("Muxing time <sec> : %4i\n", (int) (end_time - start_time) );
    system("Pause");
    return 0;
}

