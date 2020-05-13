#ifndef __n_codec_def_h__
#define __n_codec_def_h__

#include <stdint.h>

namespace Nablet {
extern "C" {



    // callback IDs
#define CBID_SEQ_HDR            0
#define CBID_PIC_HDR            1
#define CBID_FRAME              2
#define CBID_SAMPLE             3 //encoded access unit or chunk
#define CBID_AUD_HDR            4
#define CBID_FRAME_CUSTOM_DATA  5


    enum rate_control
    {
        rc_vbr = 0,
        rc_cbr = 1,
    };
    enum picture_structure_e
    {
        ps_progressive = 0,
        ps_tf = 1,
        ps_bf = 2,
        ps_tff = 3,
        ps_bff = 4
    };


    enum picture_type_e
    {
        Unkown_frame = 0,
        I_frame = 1,
        P_frame = 2,
        B_frame = 3
    };

    typedef struct seq_hdr_s
    {
        int32_t         profile;
        int32_t         level;
        int32_t         progressive_sequence;
        int32_t         frame_rate_extension_d;
        int32_t         frame_rate_extension_n;
        double          frame_rate;
        int32_t         sar_x;
        int32_t         sar_y;
        int32_t         width;
        int32_t         height;
        uint32_t        native_fourcc;
    } seq_hdr_t;


    typedef struct pic_hdr_s
    {
        int32_t           temporal_reference;
        int32_t           picture_coding_type;
        int32_t           picture_structure;	//one of picture_structure_e
        int32_t           top_field_first;
        uint32_t          flags;                //one of PIC_DECODED_FLAG_xxx
        uint8_t           reserved[28];
    } pic_hdr_t;
#define PIC_DECODED_FLAG_ERROR 0x00000001

    typedef struct aud_hdr_s
    {
        uint32_t        channels;           // number of audio channels
        uint32_t        sample_frequency;   // sample rate in Hz
        uint32_t        bit_per_sample;     // 0 if compressed
        uint32_t        channel_mask;       // channel mask   
    } aud_hdr_t;

    typedef struct ud_block //user data block
    {
        int32_t         data_size;
        uint8_t*        data;
    } ud_block_t;

    typedef struct user_data
    {
        int32_t         start_code;
        int32_t         n_items;
        ud_block_t*     items;
    } user_data_t;

    typedef struct custom_data //arbitrary custom data block to be transferred through decoder with reordering
    {
        uint32_t        data_size;
        uint8_t*        data;
    } custom_data_t;

    typedef struct frame_s
    {
        uint32_t        width;
        uint32_t        height;
        uint8_t *       plane[3];
        int32_t         stride[3];
        int64_t         pts;
        user_data_t     ud;
    } frame_tt;

    typedef struct sample_s
    {
        uint8_t*        ptr;
        uint32_t        len;
        int64_t         pts;
    } sample_tt;


    typedef void(*on_send_header) (int32_t id, void * app, void * header);
    typedef void(*on_frame_request) (void * app, frame_tt* frame);          //calls by video decoder to get pre_allocated frame planes
    typedef void(*on_message) (void * app_ptr, int32_t category, const char* msg);
    enum message_category_e  { mcError = 1, mcWarning, mcInfo, mcDebug };
}//namespace Nablet

#define N_FCC_YV12 'YV12'
#define N_FCC_I420 'I420'
#define N_FCC_I422 'I422'
#define N_FCC_YUY2 'YUY2'
#define N_FCC_UYVY 'UYVY'
#define N_FCC_BGRA 'BGRA'
#define N_FCC_BGR3 'BGR3'


}; //extern "C"
#endif //__n_codec_def_h__
