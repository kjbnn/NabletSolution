#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../include/n_enc_aac.h"
#pragma comment(lib, "../lib/bin/nb_enc_aac.lib")

#pragma pack(push,1)
// AIFF Type Definitions for a wave file
typedef char ID[4];

struct wav_chunk
{
    ID ckID;
    unsigned long ckSize;
    ID formType;
};

struct wav_common_chunk
{
    ID ckID;
    unsigned long ckSize;
    short audioFormat;
    short numChannels;
    long sampleRate;
    long byteRate;
    short blockAlign;
    short bitsPerSample;
};

struct wav_data_chunk
{
    ID ckID;
    unsigned long ckSize;
};

#pragma pack(pop)


using namespace Nablet;

class CAacEncode
{
protected:
    FILE*                   m_fSrc;
    FILE*                   m_fDest;
    wav_chunk               m_chunk;
    wav_common_chunk        m_common_chunk;
    wav_data_chunk          m_data_chunk;

    aac_encoder_t*          m_pEncoder;
    aac_encoder_settings_t  m_settings;
    uint32_t                m_nFramesEncoded;

    static void OnMessage(void * app_ptr, int32_t category, const char* msg );
    static void OnSendHeader(int id, void * app, void * header);
    static void * get_callback(const char * function_name);
    
    int32_t ReadWaveHeader();
    void FrameEncoded(sample_tt* sample);

public:
    CAacEncode();
    virtual ~CAacEncode();

    int Init(const char* cSource, const char* cDest);
    int Run(void);
    void Close(void);
};
CAacEncode::CAacEncode()
:m_fSrc(NULL)
,m_fDest(NULL)
,m_pEncoder(NULL)
,m_nFramesEncoded(0)
{
    memset(&m_settings, 0, sizeof(m_settings) );
}
CAacEncode::~CAacEncode()
{
    Close();
}
int32_t CAacEncode::ReadWaveHeader()
{
    unsigned int total_bytes_read = 0;
    unsigned int bytes_read;
    unsigned char bfr[4];
    unsigned char common_found = 0;
    unsigned char data_found = 0;

    bytes_read = fread(&m_chunk, 1, sizeof(wav_chunk), m_fSrc);
    if (bytes_read != sizeof(wav_chunk))
    {
        printf("Unable to read wave chunk from input file.\n");
        return -1;
    }

    total_bytes_read += bytes_read;

    if ((m_chunk.ckID[0] != 'R') ||
        (m_chunk.ckID[1] != 'I') ||
        (m_chunk.ckID[2] != 'F') ||
        (m_chunk.ckID[3] != 'F') ||
        (m_chunk.formType[0] != 'W') ||
        (m_chunk.formType[1] != 'A') ||
        (m_chunk.formType[2] != 'V') ||
        (m_chunk.formType[3] != 'E'))
    {
        printf("Unsupported wave file.\n");
        return -1;
    }

    while (!common_found && !data_found)
    {
        bytes_read = fread(bfr, 1, 4, m_fSrc);
        if (bytes_read != 4)
        {
            printf("Unsupported wave file.\n");
            return -1;
        }
        total_bytes_read += bytes_read;

        if ((bfr[0] == 'f') &&
            (bfr[1] == 'm') &&
            (bfr[2] == 't') &&
            (bfr[3] == ' '))
        {
            bytes_read = fread(&m_common_chunk.ckSize, 1, sizeof(wav_common_chunk) - 4, m_fSrc);
            if (bytes_read != sizeof(wav_common_chunk) - 4)
            {
                printf("Unable to read wave common chunk from input file.\n");
                return -1;
            }
            total_bytes_read += bytes_read;

            if (m_common_chunk.ckSize > 16)
            {
                int i = m_common_chunk.ckSize - 16;
                while (i >= 0)
                {
                    bytes_read = fread(bfr, 1, 4, m_fSrc);
                    i -= bytes_read;
                    total_bytes_read += bytes_read;
                }
            }
            common_found = 1;
        }

        if ((bfr[0] == 'd') &&
            (bfr[1] == 'a') &&
            (bfr[2] == 't') &&
            (bfr[3] == 'a'))
        {
            bytes_read = fread(&m_data_chunk.ckSize, 1, sizeof(wav_data_chunk) - 4, m_fSrc);
            if (bytes_read != sizeof(wav_data_chunk) - 4)
            {
                printf("Unable to read wave data chunk from input file.\n");
                return -1;
            }
            data_found = 1;
            total_bytes_read += bytes_read;
        }
    }

    if (m_common_chunk.audioFormat != 1 && m_common_chunk.audioFormat != -2) //WAVE_FORMAT_PCM
    {
        printf("Unsupported wave format %u.\n", m_common_chunk.audioFormat);
        return -1;
    }

    return total_bytes_read;
}

#define _CRT_SECURE_NO_WARNINGS

int CAacEncode::Init(const char* cSource, const char* cDest)
{
    m_nFramesEncoded = 0;
    m_pEncoder = aac_encoder_create(this, get_callback);
    if(!m_pEncoder  )
    {
        printf("can not create decoder\n");
        return 1;
    }

    m_fSrc = fopen(cSource, "rb");
    if(!m_fSrc)
    {
        printf("can not open source file\n");
        return 2;
    }
    int r = ReadWaveHeader();
    if (r < 0)
        return 2;

    m_fDest = fopen(cDest, "wb");
    if(!m_fDest)
    {
        printf("can not open destination file\n");
        return 3;
    }
    m_settings.m_profile = AAC_LC;
    m_settings.m_bitrate = 192000;
    m_settings.m_channel_number     = m_common_chunk.numChannels;
    m_settings.m_bits_per_sample = m_common_chunk.bitsPerSample;
    m_settings.m_sampling_frequency = m_common_chunk.sampleRate;
    if (m_settings.m_channel_number == 2)
        m_settings.m_stereo_mode = AAC_LR_STEREO;
    else
        m_settings.m_stereo_mode = AAC_MONO;
    
    m_settings.lfe_is_absent = 1;

    r = aac_encoder_init(m_pEncoder, &m_settings);
    return r;
}
int CAacEncode::Run(void)
{
    int iBufSize = (m_common_chunk.bitsPerSample * m_common_chunk.sampleRate * m_common_chunk.numChannels) / 8;
    //int iBufSize = (m_common_chunk.bitsPerSample * m_common_chunk.numChannels * 1024) / 8;
    uint8_t* buf = new uint8_t[iBufSize ];
    
    sample_tt sample = {};
    sample.ptr = buf;
    sample.len = iBufSize;
    sample.pts = -1;

    size_t n;
    int r;
    do
    {
        n = fread(buf, 1, iBufSize, m_fSrc);
        if(n == iBufSize)
        {
            r = aac_encoder_encode(m_pEncoder, &sample);
        }
        else
        {
            if (n > 0)
            {
                sample.len = n;
                r = aac_encoder_encode(m_pEncoder, &sample);
            }
            break;
        }
    } while(0==r);
    delete [] buf;
    return 0;
}
void CAacEncode::Close(void)
{
    printf("Closing, %u frames encoded\n", m_nFramesEncoded);
    if(m_pEncoder)
    {
        aac_encoder_destroy(m_pEncoder);
        m_pEncoder = NULL;
    }    
    if(m_fSrc)
    {
        fclose(m_fSrc);
        m_fSrc = NULL;
    }
    if(m_fDest)
    {
        fclose(m_fDest);
        m_fDest = NULL;
    }
}


void CAacEncode::OnMessage(void * app_ptr, int32_t category, const char* msg )
{
static const char* cCategories[] = {"", "ERROR", "WARNING", "INFO", "DEBUG"};

    printf("%s: %s\n", cCategories[category], msg);   
}
void CAacEncode::OnSendHeader(int id, void * app_ptr, void * header)
{
    CAacEncode* pInstance = (CAacEncode*) app_ptr;
    pInstance->FrameEncoded( (sample_tt*) header);    
}
void CAacEncode::FrameEncoded(sample_tt* sample)
{    
    if(m_fDest)
    {
        printf("AAC frame encoded, pts=%.6f\n", sample->pts/27000000.0);
        size_t n = fwrite(sample->ptr, 1, sample->len, m_fDest);
        if(n!=sample->len)
        {
            printf("Destination writing error\n");
        }
        m_nFramesEncoded++;
    }
}

void * CAacEncode::get_callback(const char * function_name)
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
        printf(" AAC encoder\n");
        printf(" Copyright(c) 2020 Tecom\n");
        printf(" www.tecomgroup.ru\n");
        printf("*************************\n");
        printf("usage:\n");
        printf("%s <source_pcm.wav> <encoded.aac> \n", strrchr(argv[0], '\\') + 1);
        return 1;
    }
    CAacEncode Encoder;

    int r = Encoder.Init(argv[1], argv[2]);
    if(r)
    {
        return r;
    }

    Encoder.Run();
    Encoder.Close();

	return 0;
}

