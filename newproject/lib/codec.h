#pragma once
#include "net.h"
#include "slice.h"

namespace rasp
{
    struct CodecBase
    {
        // > 0 .parse all msg, ==0 pase some msg, <0 parse error!
        virtual int tryDecode(Slice data, Slice& msg) = 0;
        virtual void encode(Slice msg, Buffer& buff) = 0;
        virtual CodecBase* clone() = 0;
        virtual ~CodecBase(){}
    };

    //msg end with \r\n
    struct LinCodec : public CodecBase
    {
        virtual ~LinCodec(){}
        virtual int tryDecode(Slice data, Slice& msg) override;
        virtual void encode(Slice msg, Buffer& buff) override;
        virtual CodecBase* clone() override;
    };

    //msg given Len
    struct LengthCodec : public CodecBase
    {
        virtual ~LengthCodec(){}
        virtual int tryDecode(Slice data, Slice& msg) override;
        virtual void encode(Slice msg, Buffer& buff) override;
        virtual CodecBase* clone() override;
    };
}