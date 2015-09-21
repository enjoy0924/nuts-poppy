#ifndef __DEFAULT_MESSAGE_PARSE_H__
#define __DEFAULT_MESSAGE_PARSE_H__
#include "ace/ace_header.h"
#include "Configure.h"
#include "zlib.h"
#define MIN_MSG_LEN 12

class CDefaultMessageParse
{
public:

    static bool isHead(const unsigned char* buffer){
        if (buffer[0] == 0x19 && buffer[1] == 0x88 &&
            buffer[2] == 0x09 && buffer[3] == 0x24)
            return true;
        return false;
    }

    static int getByteInterger(const char* buf)
    {
        int be32 = 0;
        memcpy(&be32, buf, sizeof(be32));
        return ntohl(be32);
    }

    static bool defalutStickerParse(const char* rawbuf, const int buflen , int& stickpos, bool& skip)
    {
        int msglen = 0;
        const char* tempdata = rawbuf;
        if (buflen >= MIN_MSG_LEN)          //[head](4)+[length](4)+{message body}+[checksum](4)
        {//this buffer probably contains at least one completable message
            if (isHead((unsigned char*)tempdata))
            {//if there is no message head found here, must be something error happened, exit here
                msglen = getByteInterger(tempdata+4); //get message length

                if (msglen+MIN_MSG_LEN <= buflen)
                {//there must be one completable message
                    unsigned long expectchecksum=adler32(1, reinterpret_cast<const Bytef*>(tempdata+8), msglen);
                    unsigned long checksum = getByteInterger(tempdata+8+msglen);
                    if (expectchecksum != checksum)
                    {//checksum is wrong, do not fire this one over to upper application, just skip it, discard it
                        skip = true;
                    }
                    stickpos = MIN_MSG_LEN+msglen;
                    return true;
                }
            }
            else
            {//dirty information discovered, tell the lower close this session
                stickpos = -1;
            }
        }
        return false;
    }
};

#endif