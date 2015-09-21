#include "NppMessageFactory.h"
//#include "NetworkUtils.h"
#include "NppLog.h"
#include "WinAysncIOCP.h"

inline google::protobuf::Message* createMessage(const std::string& type_name)
{
    google::protobuf::Message* message = NULL;
    const google::protobuf::Descriptor* descriptor =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if (descriptor != NULL)
    {
        const google::protobuf::Message* prototype =
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype != NULL)
        {
            message = prototype->New();
        }
    }
    return message;
}

#define NPP_MSG_HEAD_LEN          8
#define NPP_MSG_CHECKSUM_LEN 4

bool CNppMessageFactory::FireMessageEvent(const void * serilized,const int len, CIOSession* pClientSession, bool timeout)
{
    if (pClientSession->getSessionType() != SES_TYPE_NPP)
    {
        int actualSize = 0;
        //non-npp message handle here
        char* nonnppMessage = GlobalOverlappedBufPool::instance()->allocateRawBuffer(len+8, actualSize);
        if (NULL == nonnppMessage)
            return false;

        memcpy(nonnppMessage, &len, 4);
        memcpy(nonnppMessage+4, &actualSize, 4);
        memcpy(nonnppMessage+8, serilized, len);

        pClientSession->insertMessageTask(nonnppMessage);

        return true;
    }

    //if this message is available, bAvaliable is true
    bool bAvaliable = true;

    if (NULL == pClientSession)
        return false;
    NutsWrapperMessage* pNutsWrapperMessage =reinterpret_cast<NutsWrapperMessage*>(createMessage("NutsWrapperMessage"));
    if (NULL == pNutsWrapperMessage)
        return false;
    if (timeout)
    {
        int sequence = 0;
        int bufferlen = 0;
        int npplen = 0;
        bool skip = false;
        memcpy(&sequence, (char*)serilized+4, sizeof(int));
        memcpy(&bufferlen, (char*)serilized+8, sizeof(int));

        CDefaultMessageParse::defalutStickerParse((char*)serilized+12, bufferlen, npplen, skip);
        do 
        {
            if(!pNutsWrapperMessage->ParseFromArray((char*)serilized+12+ NPP_MSG_HEAD_LEN, npplen - NPP_MSG_CHECKSUM_LEN-NPP_MSG_HEAD_LEN))
                break;
            else
            {
                pNutsWrapperMessage->set_type(MessageType::RESPONSE_TIMEOUT);
            }
        } while (false);

        GlobalOverlappedBufPool::instance()->freeRawBuffer((char*)serilized, bufferlen);
    }
    else
    {
        if(!pNutsWrapperMessage->ParseFromArray((char*)serilized + NPP_MSG_HEAD_LEN, len - NPP_MSG_CHECKSUM_LEN-NPP_MSG_HEAD_LEN))
        {//parse npp message error, release the new object
            delete pNutsWrapperMessage;
            return false;
        }

        pNutsWrapperMessage->set_source(pClientSession->getIdentity());

        if (pClientSession->isOpenTimeoutMechanism())
        {
            bAvaliable = pClientSession->ejectMessageTimeoutQueueWithSeq(pNutsWrapperMessage->sequencenum());
        }
    }

    if (bAvaliable)
    {
        pClientSession->insertMessageTask(pNutsWrapperMessage);
    }
    else
    {
        //LOG_DEBUG("<<<<<<====sequence: %d====>>>>", pNutsWrapperMessage->sequencenum());
        delete pNutsWrapperMessage;
    }
    
    return true;
}

bool CNppMessageFactory::FireIOEvent(CIOSession* pClientSession)
{
    //////////////////////////////////////////////////////////////////////////
    return true;
}

CNppMessageFactory::CNppMessageFactory()
{

}

CNppMessageFactory::~CNppMessageFactory()
{

}

