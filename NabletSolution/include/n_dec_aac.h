#ifndef __n_dec_aac_h__
#define __n_dec_aac_h__

#include "n_codec_def.h"

#ifdef N_DEC_AAC_EXPORTS
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API 
#endif

extern "C" {

typedef struct aac_decoder aac_decoder_t;

enum TDecodeHeMode{
    HEAAC_HQ_MODE=18,
    HEAAC_LP_MODE=28,
};

enum TPsSupportLevel
{
    PS_DISABLE = 0,
    PS_PARSER  = 1,
    PS_ENABLE_BL =111,
    PS_ENABLE_UR =411
};

struct aac_decoder_settings_t
{
    int32_t decode_HE_mode;                 // TDecodeHeMode, default is HEAAC_HQ_MODE
    int32_t downsample_HE;                  // 1 is enable HE profile downsample, 0 - disable
    int32_t enable_SBR;                     // 1 is enable, 0 os disable, default is 1
    int32_t flag_PS_support_lev;            // TPsSupportLevel, default is  PS_DISABLE
    int32_t layer;                          // default is -1
    uint8_t reserved[32];
};

EXPORT_API aac_decoder_t *  aac_decoder_init(void * app_ptr, void *(*get_callback)(const char * function_name));
int EXPORT_API aac_decoder_set_settings(aac_decoder_t * decoder, aac_decoder_settings_t * settings);
int EXPORT_API aac_decoder_destroy(aac_decoder_t * decoder);
int EXPORT_API aac_decoder_flush(aac_decoder_t * decoder);
//
int EXPORT_API aac_decoder_decode(aac_decoder_t * decoder, uint8_t * data, uint32_t size);

}; //extern "C"
#endif //__n_dec_aac_h__
