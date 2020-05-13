#ifndef MUX_MP4_API_
#define MUX_MP4_API_

#if defined(_WIN32) || defined(_WIN64)
#define EXPORT_API   __cdecl
#else
#define EXPORT_API
#endif

#if !defined(__GNUC__)
typedef unsigned long long u_int64_t;
#endif

typedef struct mp4_muxer mp4_muxer_tt;

#define MAX_STREAMS         32

#define MP4_FORMAT          0 //mp4 file ISO 14496-12/14
#define QUICKTIME_FORMAT    1
#define F_MP4               2

#ifdef __GNUC__
#pragma pack(push,1)
#else
#pragma pack(push)
#pragma pack(1)
#endif

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mp4_muxer_settings_s
{
    int stream_compatibility;
    unsigned int video_sequence_length;
    int AAC_backward_compatible_signaling;
    int remove_filler_nalus;
    int time_mode;
    int atom_order;
    char *output_path;

    u_int64_t mdat_size;
    int fragmented;
    int loging;
    int write_edts;
    int disable_sidx;
}
mp4_muxer_settings_t;

typedef struct sample_struct_s
{
    unsigned char * data;

    u_int64_t start_time;   // 10 MHz clocks
    u_int64_t stop_time;    // 10 MHz clocks
    u_int64_t sample_size;

    unsigned int id;

    enum stream_types
    {
        UNKOWN_TYPE = 0, 
        VIDEO_TYPE,
        AUDIO_TYPE
    };

    stream_types stream_type;
}
sample_struct_t;

int EXPORT_API mp4_muxer_defaults(mp4_muxer_settings_t *set, int format_type);
mp4_muxer_tt * EXPORT_API mp4_muxer_create(void *(EXPORT_API *get_callback)(char* name), const mp4_muxer_settings_t *set, void * app);
int EXPORT_API mp4_muxer_init(mp4_muxer_tt *muxer, void *reserved, char *output);
int EXPORT_API mp4_muxer_add_input_file(mp4_muxer_tt *muxer, char *input);
unsigned long EXPORT_API mp4_muxer_process(mp4_muxer_tt *muxer);
int EXPORT_API mp4_muxer_done(mp4_muxer_tt *muxer);
void EXPORT_API mp4_muxer_destroy(mp4_muxer_tt *muxer);
int EXPORT_API mp4_muxer_check_settings(void *(EXPORT_API *get_callback)(char* name), const mp4_muxer_settings_t *set, unsigned int options, void *app);
int EXPORT_API mp4_muxer_add_sample(mp4_muxer_tt *muxer, const sample_struct_t * sample);
int EXPORT_API mp4_muxer_add_stream(mp4_muxer_tt *muxer);
int EXPORT_API mp4_muxer_set_language(mp4_muxer_tt *muxer, short lang, int track_id);
int EXPORT_API mp4_muxer_set_track_id(mp4_muxer_tt *muxer, int track_id);
int EXPORT_API mp4_muxer_add_sample_ex(mp4_muxer_tt *muxer, const unsigned char *data, const u_int64_t sample_size, const unsigned int id, const u_int64_t start_time);
int EXPORT_API mp4_muxer_set_app_tag(mp4_muxer_tt *muxer, const char * tag);
// unicode functions
int EXPORT_API mp4_muxer_initW(mp4_muxer_tt *muxer, void *reserved, wchar_t *output);
int EXPORT_API mp4_muxer_add_input_fileW(mp4_muxer_tt *muxer, wchar_t *input);

#ifdef __cplusplus
}
#endif

#endif // MUX_MP4_API_

