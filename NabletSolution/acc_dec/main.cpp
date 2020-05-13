#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <windows.h>

#include "../include/n_dec_aac.h"
#pragma comment(lib, "../lib/bin/nb_dec_aac.lib")


using namespace Nablet;

#ifndef WAVE_FORMAT_PCM
#pragma pack(push, 1)
typedef struct waveformat_tag {
    uint16_t    wFormatTag;        /* format type */
    uint16_t    nChannels;         /* number of channels (i.e. mono, stereo, etc.) */
    uint32_t    nSamplesPerSec;    /* sample rate */
    uint32_t    nAvgBytesPerSec;   /* for buffer estimation */
    uint16_t    nBlockAlign;       /* block size of data */
} WAVEFORMAT;

/* flags for wFormatTag field of WAVEFORMAT */
#define WAVE_FORMAT_PCM     1


/* specific waveform format structure for PCM data */
typedef struct pcmwaveformat_tag {
    WAVEFORMAT  wf;
    uint16_t    wBitsPerSample;
} PCMWAVEFORMAT;
#pragma pack(pop)
#endif /* WAVE_FORMAT_PCM */


class CAacDecode
{
protected:
    FILE*                   m_fSrc;
    FILE*                   m_fDest;
    aac_decoder_t*          m_pDecoder;
    aac_decoder_settings_t  m_settings;
    int32_t                 m_iWaveDataPos;
    int32_t                 m_nFramesDecoded;

    static  void            OnMessage(void * app_ptr, int32_t category, const char* msg );
    static  void            OnSendHeader(int id, void * app, void * header);
    static  void *          get_callback(const char * function_name);
            void            FrameDecoded(sample_tt* chunk);
            int             WriteWavHeader(int BitsPerSample, int Channels, int SamplesPerSec);
            int             UpdateWavHeader(void);            
public:
    CAacDecode();
    virtual ~CAacDecode();
    int Init(const char* cSrc, const char* cDst );
    int Run();
    int Close();
};
CAacDecode::CAacDecode()
:m_fSrc(NULL)
,m_fDest(NULL)
,m_pDecoder(NULL)
,m_iWaveDataPos(0)
,m_nFramesDecoded(0)
{
    memset(&m_settings, 0, sizeof(m_settings) );
}
CAacDecode::~CAacDecode()
{
    Close();
}
int CAacDecode::Init(const char* cSrc, const char* cDst )
{

    m_pDecoder = aac_decoder_init(this, get_callback);
    if(!m_pDecoder  )
    {
        printf("can not create decoder\n");
        return 1;
    }

    m_fSrc = fopen(cSrc, "rb");
    if(!m_fSrc)
    {
        printf("can not open source file\n");
        return 2;
    }

    m_fDest = fopen(cDst, "wb");
    if(!m_fDest)
    {
        printf("can not open destination file\n");
        return 3;
    }
    m_nFramesDecoded = 0;
    int r = aac_decoder_set_settings(m_pDecoder, &m_settings);
    return r;
}
int CAacDecode::Close()
{
    if(m_pDecoder)
    {
        aac_decoder_flush(m_pDecoder);
        aac_decoder_destroy(m_pDecoder);
        m_pDecoder = NULL;
        printf("Decoding done %d frames delivered\n", m_nFramesDecoded);        
    }    
    if(m_fSrc)
    {
        fclose(m_fSrc);
        m_fSrc = NULL;
    }
    if(m_fDest)
    {
        UpdateWavHeader();
        fclose(m_fDest);
        m_fDest = NULL;
    }

    return 0;
}
int CAacDecode::Run()
{
static uint8_t buf[16*1024];
size_t n;
int r;
    do
    {
        n = fread(buf, 1, sizeof(buf), m_fSrc);
        if(n > 0)
        {
            r = aac_decoder_decode(m_pDecoder, buf, (uint32_t)  n);
            
        }
    } while(sizeof(buf)==n && 0==r);
    return 0;
}

void CAacDecode::OnMessage(void * app_ptr, int32_t category, const char* msg )
{
static const char* cCategories[] = {"", "ERROR", "WARNING", "INFO", "DEBUG"};

    printf("%s: %s\n", cCategories[category], msg);   
}
void CAacDecode::OnSendHeader(int id, void * app_ptr, void * header)
{
    CAacDecode* pInstance = (CAacDecode*) app_ptr;
    switch(id)
    {
        case CBID_AUD_HDR:
            {
                static bool bFormatPrinted(false);
                if(!bFormatPrinted)
                {
                    aud_hdr_t* ah = (aud_hdr_t*) header;
                    printf("Audio: %d channel(s), %d Hz, %d bits, channel mask: %x \n", ah->channels, ah->sample_frequency, ah->bit_per_sample, ah->channel_mask);
                    pInstance->WriteWavHeader(ah->bit_per_sample, ah->channels, ah->sample_frequency);
                    bFormatPrinted = true;
                }
            }
            break;
        case CBID_SAMPLE:
            pInstance->FrameDecoded( (sample_tt*) header);
            break;
    }
    
}
void CAacDecode::FrameDecoded(sample_tt* chunk)
{
    printf("Audio chunk decoded %d bytes, pts=%.3f\n", chunk->len, chunk->pts/27000000.0);
    if(m_fDest)
    {
        fwrite(chunk->ptr, 1, chunk->len, m_fDest);
    }
    m_nFramesDecoded++;
}
int  CAacDecode::WriteWavHeader(int BitsPerSample, int Channels, int SamplesPerSec)
{
    if(!m_fDest) return -1;
    PCMWAVEFORMAT pwf = { };

    pwf.wBitsPerSample      = BitsPerSample;
    pwf.wf.wFormatTag       = WAVE_FORMAT_PCM;
    pwf.wf.nChannels        = (uint16_t) Channels;
    pwf.wf.nSamplesPerSec   = SamplesPerSec;            
    pwf.wf.nBlockAlign      = (uint16_t) (pwf.wBitsPerSample / 8 * pwf.wf.nChannels);
    pwf.wf.nAvgBytesPerSec  = pwf.wf.nSamplesPerSec * pwf.wf.nBlockAlign;

        uint32_t dwSize(0);
        fwrite("RIFF", 1, 4, m_fDest);
        fseek(m_fDest, 4, SEEK_CUR);//reserve file size
        //fwrite(&dwSize, 1, 4, m_fDest);
        fwrite("WAVE", 1, 4, m_fDest);
        fwrite("fmt\x20", 1, 4, m_fDest);
        dwSize = sizeof(pwf);
        fwrite(&dwSize, 1, 4, m_fDest);
        fwrite(&pwf, sizeof(pwf), 1, m_fDest);
        fwrite("data", 1, 4, m_fDest);
        
        m_iWaveDataPos = ftell(m_fDest);
        fseek(m_fDest, 4, SEEK_CUR);//reserve data size
        return 0;        
}
int  CAacDecode::UpdateWavHeader(void)
{
    if(!m_fDest) return -1;
    
	long size = ftell(m_fDest) - 4;

    fseek(m_fDest, 4, SEEK_SET);
    fwrite(&size, 1, 4, m_fDest);//file size

    fseek(m_fDest, m_iWaveDataPos, SEEK_SET);
    //size = (long) (i64SamplePos * pInfo->bytesPerSample);
	size-=m_iWaveDataPos;//???
    fwrite(&size, 1, 4, m_fDest);//data size
    return 0;
}

void * CAacDecode::get_callback(const char * function_name)
{
    if(0==strcmp(function_name, "message" ) )
        return (void*) OnMessage;
    if(0==strcmp(function_name, "send_header" ) )
        return (void*) OnSendHeader;
    else
        printf("unknown function name '%s'requested", function_name);
    return NULL;
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        printf("*************************\n");
        printf(" AAC decoder\n");
        printf(" Copyright(c) 2020 Tecom\n");
        printf(" www.tecomgroup.ru\n");
        printf("*************************\n");
        printf("usage:\n");
        printf("%s <source.aac> <decoded.wav>\n", strrchr(argv[0], '\\') + 1);
        return 1;
    }
    CAacDecode Decoder;

    int r = Decoder.Init(argv[1], argv[2]);
    if(r)
    {
        return r;
    }

    Decoder.Run();
    Decoder.Close();

	return 0;
}

