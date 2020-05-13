#ifndef _NB_MP4_DEMUXER_API_
#define _NB_MP4_DEMUXER_API_

// used when dll exports symbols to project with different function call type
#if defined(_WIN32)
    #define EXPORT_API  __cdecl
#else
    #define EXPORT_API
#endif

#include <stdint.h>

// demuxer instance
typedef struct nb_mp4_demuxer nb_mp4_demuxer_tt;

typedef struct sample_info_s
{
    int64_t start_time;                             /**< 27 MHz clocks */
    int64_t stop_time;                              /**< 27 MHz clocks */
    int32_t sample_size;                            /**< sample size */

    unsigned char * data;                           /**< sample data */
    int32_t ID;                                     /**< ID */
}
sample_info_t;

// stream media types
typedef enum nb_media_types
{
    nb_Unknown = 0,

    nb_MPEG1V = 1,
    nb_MPEG2V = 2,
    nb_MPEG4V = 3,
    nb_H263 = 4,
    nb_H264 = 5,
    nb_VC1 = 6,
    nb_DV = 7,
    nb_J2K = 8,
    nb_VC3 = 9,

    nb_MPEG1A = 10,
    nb_MPEG2A = 11,

    nb_AAC_RAW = 12,
    nb_AAC_LATM = 13,
    nb_AAC_ADTS = 14,

    nb_AC3 = 15,
    nb_AC3_DDPlus = 16,
    nb_AES3_337M_AC3 = 17,
    nb_AES3_337M_DDPlus = 18,

    nb_DTS = 19

} nb_media_types_t;

// describes the format of a stream
typedef struct nb_stream_format
{
    nb_media_types_t  stream_media_type;  // !< @brief the media type
    uint16_t          header_bytes;       // !< @brief number of header bytes
    uint8_t          *header_data;        // !< @brief can hold SPS for video etc.

} nb_stream_format_t;


struct nb_mp4_demux_settings
{
    uint8_t add_adts_headers;             //!< @brief If this value equal 1, demultiplexer adds ADTS header for AAC
    uint8_t annexb_output;                //!< @brief If this value equal 1, demultiplexer creates annex b output.
};


typedef struct nb_mp4_demux_track_format_s
{
    uint32_t ID;                            //!< @brief This value equal for internal track ID. Unique for every track.
    nb_stream_format_t format;              //!< @brief This field describes the stream in a standard mc_stream_format_t structure defined in mcmediatypes.h
    int32_t ES_info_length;                 //!< @brief Equal elementary stream decoder configuration size in bytes. This information is necessary for decoders.
    uint8_t *descriptors;                   //!< @brief Array of bytes which contain decoder configuration.
    uint64_t sample_count;                  //!< @brief Number of video frames for a video stream
    uint32_t samples_per_frame;             //!< @brief Number of audio samples in 1 sample_count above
    double duration;                        //!< @brief The duration of the stream in seconds.
    int32_t raw_decoder_config_len;         //!< @brief The size in bytes of the raw_decoder_config field.
    uint8_t *raw_decoder_config;            //!< @brief The decoder config (if any) as present in the container for this stream.
    uint32_t track_width;                   //!< @brief Width indicated in the track header
    uint32_t track_height;                  //!< @brief Height indicated in the track header
} nb_mp4_demux_track_format_t;


struct nb_mp4_demux_file_info
{
    int32_t stream_count;                   //!< @brief This value equal of total elementary streams count in file.
    double duration;                        //!< @brief Total file durations (in seconds). Equal duration of longest track.
    int64_t file_size;                      //!< @brief Total input file size in bytes.
};

struct nb_mp4_demux_seek_info
{
    int32_t parser_num;                        //!< @brief Parser number for the seek.
    int32_t stream_num;                        //!< @brief Stream number for the seek.
    int64_t seek_sample;                       //!< @brief Sample number to get.

    uint32_t ref_do;                           //!< @brief Demuxing will start at this point after a seek if return_exact_sample = 0.
                                               /*!< @details For video streams this will be the reference frame display order where decoding must start to decode
                                               * the target frame. For audio streams, it will be the starting sample number of the compressed audio frame or uncompressed chunk.
                                               */

    uint32_t ref_co;                           //!< @brief Demuxing will start at this point after a seek if return_exact_sample = 0.
                                               /*!< For video streams this will be the reference frame coding order where decoding must start to decode the target frame.
                                                * For audio streams, this will be the starting sample number of the compressed audio frame or uncompressed chunk.
                                                */

    int64_t ref_PTS;                            //!< @brief The reference frame timestamp (27MHz units).

    uint32_t seek_do;                           //!< @brief For video streams this will be the desired frame display order. For audio streams, this will be the desired sample number.
    uint32_t seek_co;                           //!< @brief For video streams this will be the desired frame coding order. For audio streams, this will be the desired sample number.
    int64_t seek_PTS;                           //!< @brief The desired samples timestamp (27MHz units).

    uint8_t return_exact_sample;                //!< @brief Set this to have the demuxer position the seek point to the desired sample instead of the reference frame needed to decode the desired sample.

    //!< @brief These do not change the file position, the merely return the appropriate time:
    uint8_t seek_next_key_frame;                //!< @brief return the time of the next key frame based on seek_sample
    uint8_t seek_prev_key_frame;                //!< @brief return the time of the previous key frame based on seek_sample

    double seek_start_time;                     //!< @brief Seek start time 
    double seek_stop_time;                      //!< @brief Seek stop time

};

#ifdef __cplusplus
extern "C" {
#endif


nb_mp4_demuxer_tt * EXPORT_API NB_MP4DemuxCreate(void *(EXPORT_API * callback)(void * app, const char* name), void * app);

void EXPORT_API NB_MP4DemuxDestroy(nb_mp4_demuxer_tt * nb_mp4_demuxer);

int EXPORT_API NB_MP4DemuxOpenInput(nb_mp4_demuxer_tt * nb_mp4_demuxer, char * input_file);
int EXPORT_API NB_MP4DemuxOpenInputUC(nb_mp4_demuxer_tt * nb_mp4_demuxer, wchar_t * input_file);

int EXPORT_API NB_MP4DemuxGetFileInfo(nb_mp4_demuxer_tt * nb_mp4_demuxer, struct nb_mp4_demux_file_info *file_info);

int EXPORT_API NB_MP4DemuxTrackInfo(nb_mp4_demuxer_tt * nb_mp4_demuxer, nb_mp4_demux_track_format_t * track_info, unsigned int ID);

int EXPORT_API NB_MP4OpenTrack(nb_mp4_demuxer_tt * nb_mp4_demuxer, nb_mp4_demux_settings settings, unsigned int ID);
int EXPORT_API NB_MP4DemuxTrack(nb_mp4_demuxer_tt * nb_mp4_demuxer, unsigned int ID);

int EXPORT_API NB_MP4DemuxSeek(nb_mp4_demuxer_tt * nb_mp4_demuxer,struct  nb_mp4_demux_seek_info *info);

#ifdef __cplusplus
}
#endif

#endif // _NB_MP4_DEMUXER_API_