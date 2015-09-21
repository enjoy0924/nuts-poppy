#ifndef __DEFAULT_ENCODE_WRAPPER_H__
#define __DEFAULT_ENCODE_WRAPPER_H__
#include "ace/ace_header.h"
#include "zlib.h"
#include "IoOverlappedBufPool.h"
#include "NutsWrapperMessage.pb.h"
#define PACKAGE_SIZE 12

class CDefaultEncodeWrapper
{
public:
    static bool defaultEncodeWrapper(void* inRawMessage,const int inLen, char** outBuffer, int& inMaxBufLen , int& outLen)
    {
        if (NULL == inRawMessage)
            return false;
        
        NutsWrapperMessage* nppMessage = reinterpret_cast<NutsWrapperMessage*>(inRawMessage);
        outLen = nppMessage->ByteSize();
        
        (*outBuffer) = GlobalOverlappedBufPool::instance()->allocateRawBuffer(outLen+PACKAGE_SIZE, inMaxBufLen);
        char* buffer = (*outBuffer);

        //patch the header information here, it's my birthday, yelp! : )
        buffer[0] = 0x19; buffer[1]=0x88;
        buffer[2] = 0x09; buffer[3]=0x24;

        //patch message length
        unsigned long be32 = ::htonl(outLen); char* pmsglen = reinterpret_cast<char*>(&be32);
        buffer[4] = pmsglen[0]; buffer[5] = pmsglen[1];
        buffer[6] = pmsglen[2]; buffer[7] = pmsglen[3];
        
        nppMessage->SerializeToArray(buffer + 8, outLen);
        unsigned long checksum = adler32(1, reinterpret_cast<const Bytef*>(buffer+8), outLen);
        be32 = ::htonl(checksum); pmsglen = reinterpret_cast<char*>(&be32);
        buffer[8+outLen+0] = pmsglen[0]; buffer[8+outLen+1] = pmsglen[1];
        buffer[8+outLen+2] = pmsglen[2]; buffer[8+outLen+3] = pmsglen[3];

        outLen += PACKAGE_SIZE;

        return true;
    }
};

#endif