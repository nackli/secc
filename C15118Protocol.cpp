
#include "C15118Protocol.h"
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>
#include <string.h>
#include "LogManage.h"
#include "SeccErrorCode.h"


#include "iso1EXIDatatypes.h"
#include "appHandEXIDatatypes.h"
#include "iso1EXIDatatypesDecoder.h"
#include "iso1EXIDatatypesEncoder.h"
#include "appHandEXIDatatypesDecoder.h"
#include "appHandEXIDatatypesEncoder.h"
#include "xmldsigEXIDatatypes.h"
#include "xmldsigEXIDatatypesDecoder.h"
#include "xmldsigEXIDatatypesEncoder.h"
#include "dinEXIDatatypes.h"


#define PROTOCOL_NAME_SPACE_LEN               100     ///< 协议命名空间长度
#define V2G_HEADER_LENGTH                     0x08
#define V2G_VERSION_INV                       0xFE
#define V2G_VERSION_NO                        0x01
#define V2GTP_EXI_TYPE                        0x8001



#ifndef HiByte
#define HiByte(x)                   (((x) & 0xFF00 ) >> 8 )
#endif
#ifndef LowByte
#define LowByte(x)                  ((x) & 0xFF )
#endif

#ifndef HHByte
#define HHByte(x)  (((x) & 0xFF000000 ) >> 24 )
#endif

#ifndef HLByte
#define HLByte(x)  (((x) & 0x00FF0000 ) >> 16 )
#endif


#ifndef LHByte
#define LHByte(x)  (((x) & 0x0000FF00 ) >> 8 )
#endif


#ifndef LLByte
#define LLByte(x)  (((x) & 0x000000FF ) >> 0 )
#endif

typedef enum {
    EM_SECC_STATE_UNKOWN = 0,
    EM_SECC_STATE_SUPPORTED_APP_PROTOCOL,
    EM_SECC_STATE_SESSION_SETUP,
    EM_SECC_STATE_SERVICE_DISCOVERY,
    EM_SECC_STATE_SERVICE_DETAIL,               // optional
    EM_SECC_STATE_PAYMENT_SERVICE_SELECTION,
    EM_SECC_STATE_CERTIFICATE_INSTALLATION,     // optional
    EM_SECC_STATE_CERTIFICATE_UPDATE,           // optional
    EM_SECC_STATE_PAYMENT_DETAILS,
    EM_SECC_STATE_AUTHORIZATION,


    EM_SECC_STATE_CHARGE_PARAMETER_DISCOVERY,   // 10
    EM_SECC_STATE_CABLE_CHECK,
    EM_SECC_STATE_PRE_CHARGE,
    EM_SECC_STATE_POWER_DELIVERY,
    EM_SECC_STATE_CHARGING_STATUS,
    EM_SECC_STATE_CURRENT_DEMAND,
    EM_SECC_STATE_METERING_RECEIPT,
    EM_SECC_STATE_WELDING_DETECTION_REQ,
    EM_SECC_STATE_SESSION_STOP,
} EM_SECC_STATE;


typedef struct V2G_DATA_HEAD
{
    uint8_t   iVersion;
    uint8_t   iInverseVersion;
    uint16_t  iPayLoadType;
    uint32_t  iPlayLoadLength;
    uint8_t   *pPayLoadData;   // 指向数据域
} V2G_DATA_HEAD,*LP_V2G_DATA_HEAD;

typedef enum
{
    kProtocolUnknown = 0,
    kProtocol15118,
    kProtocol70121,
} EM_PROTOCOL_TYPE;


typedef struct {
    EM_PROTOCOL_TYPE emProtocol;
    char szNameSpace[PROTOCOL_NAME_SPACE_LEN];
    uint8_t iVersionMajor;
    uint8_t iVersionMinor;
} SUPPORT_PROTOCOL,*LP_SUPPORT_PROTOCOL;

const SUPPORT_PROTOCOL g_supportProtocol[] = {
    {kProtocolUnknown, "Unknown", 0, 0},
    {kProtocol15118, "urn:iso:15118:2:2013:MsgDef", 2, 0},
    {kProtocol70121, "urn:din:70121:2012:MsgDef",   2, 0},
};


char g_szSessionId[8] = {0};
static int OnGet15118ExtractStateCode( const unsigned char *pExiIn)
{

    struct iso1EXIDocument *pIn = (struct iso1EXIDocument *)pExiIn;
    struct iso1BodyType *pBody = &pIn->V2G_Message.Body;

    EM_SECC_STATE state = EM_SECC_STATE_UNKOWN;

    if ( pBody->SessionSetupReq_isUsed == 1) 
        state = EM_SECC_STATE_SESSION_SETUP;
    else if ( pBody->ServiceDiscoveryReq_isUsed == 1) 
        state = EM_SECC_STATE_SERVICE_DISCOVERY;
     else if ( pBody->ServiceDetailReq_isUsed == 1) 
        state = EM_SECC_STATE_SERVICE_DETAIL;
     else if ( pBody->PaymentServiceSelectionReq_isUsed == 1) 
        state = EM_SECC_STATE_PAYMENT_SERVICE_SELECTION;
     else if ( pBody->PaymentDetailsReq_isUsed == 1) 
        state = EM_SECC_STATE_PAYMENT_DETAILS;
     else  if ( pBody->AuthorizationReq_isUsed == 1) 
        state = EM_SECC_STATE_AUTHORIZATION;
     else if ( pBody->ChargeParameterDiscoveryReq_isUsed == 1) 
        state = EM_SECC_STATE_CHARGE_PARAMETER_DISCOVERY;
     else if ( pBody->PowerDeliveryReq_isUsed == 1) 
        state = EM_SECC_STATE_POWER_DELIVERY;
     else if ( pBody->MeteringReceiptReq_isUsed == 1) 
        state = EM_SECC_STATE_METERING_RECEIPT;
     else if ( pBody->SessionStopReq_isUsed == 1) 
        state = EM_SECC_STATE_SESSION_STOP;
     else if ( pBody->CertificateUpdateReq_isUsed == 1) 
        state = EM_SECC_STATE_CERTIFICATE_UPDATE;
     else if ( pBody->CertificateInstallationReq_isUsed == 1) 
        state = EM_SECC_STATE_CERTIFICATE_INSTALLATION;
     else if ( pBody->ChargingStatusReq_isUsed == 1) 
        state = EM_SECC_STATE_CHARGING_STATUS;
     else if ( pBody->CableCheckReq_isUsed == 1) 
        state = EM_SECC_STATE_CABLE_CHECK;
     else if ( pBody->PreChargeReq_isUsed == 1) 
        state = EM_SECC_STATE_PRE_CHARGE;
     else if ( pBody->CurrentDemandReq_isUsed == 1) 
        state = EM_SECC_STATE_CURRENT_DEMAND;
     else if ( pBody->WeldingDetectionReq_isUsed == 1) 
        state = EM_SECC_STATE_WELDING_DETECTION_REQ;
    
    return state;
}

static int EncodeV2GExiHead(const LP_V2G_DATA_HEAD v2gtp, unsigned char *dest, uint32_t  max_dest_len)
{
    if (max_dest_len < v2gtp->iPlayLoadLength + V2G_HEADER_LENGTH)
        return ERR_V2G_PAYLOAD_LENGTH;

    dest[0] = V2G_VERSION_NO;
    dest[1] = V2G_VERSION_INV;
    dest[2] = HiByte(V2GTP_EXI_TYPE);
    dest[3] = LowByte(V2GTP_EXI_TYPE);
    dest[4] = HHByte(v2gtp->iPlayLoadLength);
    dest[5] = HLByte(v2gtp->iPlayLoadLength);
    dest[6] = LHByte(v2gtp->iPlayLoadLength);
    dest[7] = LLByte(v2gtp->iPlayLoadLength);

    memcpy(&dest[8], v2gtp->pPayLoadData, v2gtp->iPlayLoadLength);
    return v2gtp->iPlayLoadLength + V2G_HEADER_LENGTH;
}


static bool OnSendV2gData(int iSocketHandle, void *pResData, bool bExiDocFlag = true)
{
    if(iSocketHandle < 0)
        return false;

    size_t iPos = 0;
    bitstream_t tagStream;
    //bzero(&tagStream,sizeof(tagStream));
    uint8_t szExiBuff[512] = {0};
    tagStream.size = 512;
    tagStream.data = szExiBuff;
    tagStream.pos = &iPos;
    int fRet = 0;
    if(bExiDocFlag)
        fRet = encode_iso1ExiDocument(&tagStream, (struct iso1EXIDocument *)pResData);
    else
        fRet = encode_appHandExiDocument(&tagStream, (struct appHandEXIDocument *)pResData);
    if(fRet)
    {
        LOGOUT_ERROR("Decode Error : %d",fRet);
        return false;
    }
        
    uint8_t szOutBuf[1024] = {0};
    V2G_DATA_HEAD tagV2gDataHead = {0};
    tagV2gDataHead.pPayLoadData = tagStream.data;
    if(tagStream.pos)
        tagV2gDataHead.iPlayLoadLength = *tagStream.pos;
    int iDataSize = EncodeV2GExiHead(&tagV2gDataHead, szOutBuf, sizeof(szOutBuf));
    if(send(iSocketHandle, szOutBuf, iDataSize, 0) <= 0)
        LOGOUT_ERROR("Send Res Fail : %s",strerror(errno));
    char szLogBuf[1024] = {0};
    LOGOUT_INFO("Send : %s", HEX2STRING(szLogBuf, szOutBuf, iDataSize));   
    return true;
}


static bool Decode15118V2gInfo2Exi(const LP_V2G_DATA_HEAD pV2GPacket, uint8_t *pExi)
{
  	bitstream_t bitstream;
	int fRet = 0;
    size_t iPos = 0;

	bitstream.size = pV2GPacket->iPlayLoadLength;
	bitstream.data = pV2GPacket->pPayLoadData;
	bitstream.pos = &iPos;
    //int errn = decodeNBitUnsignedInteger(&bitstream, 2, &eventCode);
   // LOGOUT_DEBUG("%d,%d",errn,eventCode);
    fRet = decode_iso1ExiDocument(&bitstream, (struct iso1EXIDocument *) pExi);
    if(fRet)
        LOGOUT_ERROR("decode Exi Error = %d", fRet); 
    return fRet == 0; 
}


static int DecodeV2GHead(uint8_t *pV2gData, uint16_t iDataLen, LP_V2G_DATA_HEAD pOutV2gHead)
{
    /**
        0x01:
        V2GTP version 1
        0x00, 0x02-0xFF:
        reserved by document
    */
    /* 版本检查，目前仅支持 版本1,第二字节是一字节取反 */
    if (pV2gData[0] != V2G_VERSION_NO || pV2gData[1] != V2G_VERSION_INV)
        return ERR_V2G_VERSION;

	pOutV2gHead->iPayLoadType = (pV2gData[2] << 8 | pV2gData[3]);


	pOutV2gHead->iPlayLoadLength = pV2gData[4] << 24 | pV2gData[5] << 16 | pV2gData[6] << 8 | pV2gData[7];
    pOutV2gHead->pPayLoadData = &pV2gData[8];

   if (pOutV2gHead->iPlayLoadLength  == 0)
        return ERR_V2G_PAYLOAD_LENGTH;

    if (pOutV2gHead->iPlayLoadLength != (uint32_t)(iDataLen - V2G_HEADER_LENGTH))
        return ERR_V2G_PAYLOAD_LENGTH;
    //LOGOUT_INFO("V2gInfo : type = 0x%x , Length = %d", pOutV2gHead->iPayLoadType, pOutV2gHead->iPlayLoadLength);
    return pOutV2gHead->iPlayLoadLength;
}

/***************************************send v2g Data***********************************************/



/****************************************************supportedAppProtocol****************************************************/
static void OnEncodeAppHandSendInfo(int iSocketHandle, int iMachPriority, int iMachSchemaID, int iMachResult)
{
    struct appHandEXIDocument tagAppHandResp;
    init_appHandEXIDocument(&tagAppHandResp);
    appHandAnonType_supportedAppProtocolRes *pRes = &tagAppHandResp.supportedAppProtocolRes;
    pRes->ResponseCode = appHandresponseCodeType_Failed_NoNegotiation;
    tagAppHandResp.supportedAppProtocolRes_isUsed = 1;
    if(iMachResult >=0 && iMachSchemaID)
    {         
        pRes->SchemaID_isUsed = 1u;   
        pRes->SchemaID = iMachSchemaID;
        if(iMachResult == 0)
            pRes->ResponseCode = appHandresponseCodeType_OK_SuccessfulNegotiation;
        else
            pRes->ResponseCode = appHandresponseCodeType_OK_SuccessfulNegotiationWithMinorDeviation;
    }

    OnSendV2gData(iSocketHandle, &tagAppHandResp, false);

    LOGOUT_INFO("ResAppInfo: ResUse = %d, ResCode = %d, SchemaID = %d ",tagAppHandResp.supportedAppProtocolRes_isUsed, 
            tagAppHandResp.supportedAppProtocolRes.ResponseCode, tagAppHandResp.supportedAppProtocolRes.SchemaID);
}

static void OnDecodeAppHandAnddResInfo(LP_V2G_DATA_HEAD pInV2gHead,int iSocketHandle)
{
    if(!pInV2gHead || iSocketHandle < 0)
        return;
    
    bitstream_t tagStreamIn;
    struct appHandEXIDocument tagAppHandIn;
    bzero(&tagStreamIn,sizeof(tagStreamIn));
    bzero(&tagAppHandIn,sizeof(tagAppHandIn));
    int iMachSchemaID = -1;
    int iMachPriority = -1;
    int iMachResult = -1;
    size_t iPos = 0;
	tagStreamIn.size = pInV2gHead->iPlayLoadLength;
	tagStreamIn.data = pInV2gHead->pPayLoadData;
	tagStreamIn.pos = &iPos;   
    int fRet = decode_appHandExiDocument(&tagStreamIn, &tagAppHandIn); 
    if(!fRet)
    {
        appHandAnonType_supportedAppProtocolReq *pReq = &tagAppHandIn.supportedAppProtocolReq;
        for(int i=0; i<pReq->AppProtocol.arrayLen; i++)
        {
            appHandAppProtocolType *pRequestProtocol = &pReq->AppProtocol.array[i];
            for(int j = 0; j < (int) (sizeof(g_supportProtocol)/sizeof(g_supportProtocol[0])); j++) 
            {
                /* 协议名称匹配 */
                if (!strcmp(pRequestProtocol->ProtocolNamespace.characters , g_supportProtocol[j].szNameSpace))
                {
                    /* 主板本不同 */
                    if (pRequestProtocol->VersionNumberMajor == g_supportProtocol[j].iVersionMajor)
                    {
                        if(iMachPriority < pRequestProtocol->Priority)
                        {
                            iMachResult = 0;
                            if(pRequestProtocol->VersionNumberMinor == g_supportProtocol[j].iVersionMinor)
                                iMachResult = 1;
                            iMachSchemaID = pRequestProtocol->SchemaID;
                            iMachPriority = pRequestProtocol->Priority;
                        }
                    }
                }
            }
        }
        OnEncodeAppHandSendInfo(iSocketHandle, iMachPriority, iMachSchemaID, iMachResult);
        LOGOUT_NOTICE("<!-----------------Finsh App Protocol--------------------->");
    }
}
/*************************************************************************************************************/
static bool OnSendSessionSetupRes(int iSocket)
{
    iso1EXIDocument tagExiDoc;
    bzero(&tagExiDoc, sizeof(tagExiDoc));

    init_iso1EXIDocument(&tagExiDoc);

    for(uint8_t i = 0 ;i < sizeof(g_szSessionId) ; i++)
        g_szSessionId[i] = i;

    tagExiDoc.V2G_Message_isUsed = 1;
    init_iso1BodyType(&tagExiDoc.V2G_Message.Body);
    
    memcpy(tagExiDoc.V2G_Message.Header.SessionID.bytes, g_szSessionId,sizeof(tagExiDoc.V2G_Message.Header.SessionID.bytes));
    tagExiDoc.V2G_Message.Header.SessionID.bytesLen = sizeof(g_szSessionId);
    tagExiDoc.V2G_Message.Header.Signature_isUsed = 0u;
    //bzero(&tagExiDoc,sizeof(tagExiDoc));
    tagExiDoc.V2G_Message.Body.SessionSetupRes_isUsed = 1u;
    
    init_iso1SessionSetupResType(&tagExiDoc.V2G_Message.Body.SessionSetupRes);
    const char *pEvseId = "CN*CC1*E45B*78C";
    memcpy(tagExiDoc.V2G_Message.Body.SessionSetupRes.EVSEID.characters, pEvseId, strlen(pEvseId));
    tagExiDoc.V2G_Message.Body.SessionSetupRes.EVSEID.charactersLen = strlen(pEvseId);
    tagExiDoc.V2G_Message.Body.SessionSetupRes.EVSETimeStamp = time(NULL);
   
    tagExiDoc.V2G_Message.Body.SessionSetupRes.EVSETimeStamp_isUsed = 1U;
    tagExiDoc.V2G_Message.Body.SessionSetupRes.ResponseCode = iso1responseCodeType_OK_NewSessionEstablished;

    return OnSendV2gData(iSocket, &tagExiDoc);
}



static bool OnSessionSetupReq(iso1EXIDocument *pExiDoc, int iSocket)
{
   //iso1SessionSetupReqType tagSessSetUp = pExiDoc->V2G_Message.Body.SessionSetupReq;
    if(pExiDoc->V2G_Message_isUsed)
        memcpy(g_szSessionId, pExiDoc->V2G_Message.Header.SessionID.bytes, pExiDoc->V2G_Message.Header.SessionID.bytesLen);
    OnSendSessionSetupRes(iSocket);
    return true;
}


static bool OnDecode15118V2GData(LP_V2G_DATA_HEAD pInV2gHead,int iSocketHandle)
{
    if(!pInV2gHead || iSocketHandle < 0 )
        return false;
    iso1EXIDocument tagExiDoc;
    bzero(&tagExiDoc, sizeof(tagExiDoc));
    if(Decode15118V2gInfo2Exi(pInV2gHead, (uint8_t *)&tagExiDoc))
    {
        int iType = OnGet15118ExtractStateCode((uint8_t *)&tagExiDoc);

        if(tagExiDoc.V2G_Message.Header.Notification_isUsed && tagExiDoc.V2G_Message.Header.Notification.FaultMsg_isUsed)
            LOGOUT_INFO("iType = %d ,%d, %s",iType,  tagExiDoc.V2G_Message.Header.Notification.FaultCode,
            tagExiDoc.V2G_Message.Header.Notification.FaultMsg.characters);

        return OnTodo15118Info(iType, &tagExiDoc, iSocketHandle);
    }

    return false;
}

static bool OnServiceDiscovery(iso1EXIDocument *pExiDoc, int iSocket)
{
    if(pExiDoc->V2G_Message_isUsed)
        memcpy(g_szSessionId, pExiDoc->V2G_Message.Header.SessionID.bytes, pExiDoc->V2G_Message.Header.SessionID.bytesLen);
    OnSendSessionSetupRes(iSocket);
    return true;
}
/*************************************************************************************************************/
C15118Protocol::C15118Protocol()
{
    m_iSocketHandle = -1;
}

C15118Protocol::~C15118Protocol()
{
    m_iSocketHandle = -1;
} 

void C15118Protocol::SetSocketHandle(int iSocketHandle)
{
    m_iSocketHandle = iSocketHandle;   
}

void C15118Protocol::ThreadProcFun()
{
    fd_set tagReadSet;  //读文件操作符
    fd_set tagExceptionfds; //异常文件操作符    
    FD_ZERO(&tagReadSet);
    FD_ZERO(&tagExceptionfds);
    FD_SET(m_iSocketHandle,&tagReadSet);
    FD_SET(m_iSocketHandle,&tagExceptionfds); 

    uint8_t buff[1024] = {0}; //数据接收缓冲区
    /*每次调用select之前都要重新在read_fds和exception_fds中设置文件描述符connfd，因为事件发生以后，文件描述符集合将被内核修改*/   
    //timeval tv = {1,0};
    uint16_t fRet = select(10 , &tagReadSet, NULL, &tagExceptionfds, NULL);
    if(fRet <= 0)
    {
        LOGOUT_ERROR("Fail to select!");
        return;
    }   
    if(FD_ISSET(m_iSocketHandle, &tagReadSet))
    {
        fRet = recv(m_iSocketHandle, buff, sizeof(buff), 0);
        if(fRet <= 0)
        {
            usleep(500000);// 降低CPU利用率,select 函数无效待查
            return; 
        }
                
        ReadEvccData(buff, fRet);             
        //LOGOUT_INFO("get %d bytes of normal data: %s",ret,buff);                
    }
    else if(FD_ISSET(m_iSocketHandle,&tagExceptionfds)) //异常事件
    {
        fRet = recv(m_iSocketHandle, buff, sizeof(buff)-1, MSG_OOB);
        if(fRet <= 0)
            return;              
        LOGOUT_INFO("get %d bytes of exception data: %s ",fRet,buff);
    }
    else
        LOGOUT_INFO("Read Data Error");         
}


bool C15118Protocol::ReadEvccData(uint8_t *szDataBuf, uint16_t iBufSize)
{
    if(m_iSocketHandle < 0 ||  !szDataBuf || !iBufSize)
        return false;
    char szLogBuf[1024] = {0};
    LOGOUT_INFO("ReadEvcc : %s", HEX2STRING(szLogBuf, szDataBuf, iBufSize)); 
    V2G_DATA_HEAD tagV2gHead = {0}; 
    DecodeV2GHead(szDataBuf, iBufSize, &tagV2gHead);
    if(m_iCurStatus == EM_SECC_STATE_UNKOWN)
    {   
         OnDecodeAppHandAnddResInfo(&tagV2gHead,m_iSocketHandle);
         m_iCurStatus = EM_SECC_STATE_SUPPORTED_APP_PROTOCOL;
    }else 
        OnDecode15118V2GData(&tagV2gHead,m_iSocketHandle);
    return true;
}

bool C15118Protocol::OnTodo15118Info(int iType, iso1EXIDocument *pExiDoc)
{
     if(!pExiDoc || m_iSocketHandle < 0 || iType < EM_SECC_STATE_SESSION_SETUP)
        return false;
    LOGOUT_DEBUG("OnTodo15118Info = %d", iType);
    switch (iType)
    {
    case EM_SECC_STATE_SESSION_SETUP:
        OnSessionSetupReq(pExiDoc, m_iSocketHandle);
        break;
    case EM_SECC_STATE_SERVICE_DISCOVERY:
        break;    
    case EM_SECC_STATE_SERVICE_DETAIL:
        break;   
    case EM_SECC_STATE_PAYMENT_SERVICE_SELECTION:
        break; 
                          
    default:
        break;
    }  
    return true;    
}