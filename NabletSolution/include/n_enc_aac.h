#ifndef __n_enc_aac_h__
#define __n_enc_aac_h__

#include "n_codec_def.h"

#ifdef N_ENC_AAC_EXPORTS
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API 
#endif

extern "C" {

typedef struct aac_encoder aac_encoder_t;

enum aac_profile {
    AAC_MAIN = 0,
    AAC_LC,
    AAC_SSR,
    AAC_LTP
};
enum aac_stereo_mode {
    AAC_MONO = 0,
    AAC_LR_STEREO,
    AAC_MS_STEREO,
    AAC_JOINT_STEREO
};
enum aac_output_format {
    AF_ADTS = 0,
    AF_ADIF,
};

struct aac_encoder_settings_t
{
    uint32_t m_profile;             // one of aac_profile
    int32_t  m_output_format;       // aac_output_format enumeration
    uint32_t m_sampling_frequency;
    uint32_t m_channel_number;
    uint32_t m_bits_per_sample;
    uint32_t m_bitrate;
    int32_t  m_stereo_mode;         //aac_stereo_mode enum
    int32_t  m_ns_mode;             //default value is 0
    int32_t  lfe_is_absent;         //do not apply LFE filtering to last channell at 6- or 8-channels scenarios
    uint8_t  reserved[28];
};

EXPORT_API aac_encoder_t* aac_encoder_create(void * app, void *(*get_callback)(const char * function_name));
EXPORT_API int aac_encoder_init(aac_encoder_t * encoder, aac_encoder_settings_t * settings);
EXPORT_API int aac_encoder_encode(aac_encoder_t * encoder, Nablet::sample_tt* sample);
EXPORT_API int aac_encoder_destroy(aac_encoder_t * encoder);

}; //extern "C"

#endif //__n_enc_aac_h__