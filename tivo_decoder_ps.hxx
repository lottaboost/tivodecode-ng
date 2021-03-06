#ifndef TIVO_DECODER_PS_HXX_
#define TIVO_DECODER_PS_HXX_

#ifdef HAVE_CONFIG_H
#include "tdconfig.h"
#endif

#include <cstdio>

#include "tivo_decoder_base.hxx"

//============================
// PS Specific data structures
//============================

typedef enum
{
    PACK_NONE,
    PACK_SPECIAL,
    PACK_PES_SIMPLE,            // packet length == data length
    PACK_PES_COMPLEX,           // crazy headers need skipping
}
packet_type;

typedef struct
{
    // the byte value match for the packet tags
    uint8_t code_match_lo;      // low end of the range of matches
    uint8_t code_match_hi;      // high end of the range of matches

    // what kind of PES is it?
    packet_type packet;
}
packet_tag_info;

/* All elements are in big-endian format and are packed */

class TiVoDecoderPS : public TiVoDecoder
{
    private:
        uint32_t marker;
        
    public:
        virtual bool process();
        int process_frame(uint8_t code, hoff_t packet_start);
    
        TiVoDecoderPS(TuringState *pTuringState, HappyFile *pInfile,
                      HappyFile *pOutfile);
        ~TiVoDecoderPS();
        
} __attribute__((packed));

#endif /* TIVO_DECODER_PS_HXX_ */
