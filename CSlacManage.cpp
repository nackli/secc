#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "CMsgManage.h"
#include "LogManage.h"
#include "CSlacManage.h"
extern "C" {
#include "./ether/channel.h"
#include "./tools/getoptv.h"
#include "./tools/putoptv.h"
#include "./tools/memory.h"
#include "./tools/number.h"
#include "./tools/config.h"
#include "./tools/types.h"
#include "./tools/timer.h"
#include "./tools/flags.h"
#include "./tools/error.h"
#include "./tools/permissions.h"
#include "./ether/channel.h"
#include "./slac/slac.h"
#include "./mme/mme.h"
#include "key/HPAVKey.h"
}

#define AMP_LEN 58
#define PARAM_2_CHAR(x) #x
typedef enum EVSE_STATE
{
    EVSE_STATE_UNAVAILABLE = 0,
    EVSE_STATE_UNOCCUPIED,
    EVSE_STATE_UNMATCHED,
    EVSE_STATE_MATCHED,
    EVSE_STATE_MAX
}EVSE_STATE;


typedef struct __packed CM_AMP_CNF
{
    struct ethernet_hdr tagEtherNet;
    struct homeplug_fmi tagHomeplug;
    unsigned char iResType;
} CM_AMP_CNF,*LP_CM_AMP_CNF;

typedef struct __packed CM_AMP_REQ
{
    struct ethernet_hdr tagEtherNet;
    struct homeplug_fmi tagHomeplug;
    unsigned short iAmpLen;
    unsigned char arrAmpData[AMP_LEN];
} CM_AMP_REQ,*LP_CM_AMP_REQ;


#define EVSE_SID "CN*CC1*E45B*78C" // Station Identifier
static unsigned char g_NIM_ID[SLAC_NID_LEN] = {0x72, 0x4C, 0xF8, 0x5F, 0x4E, 0xD7, 0x09};
static unsigned char g_NMK_ID[SLAC_NMK_LEN] = {0x8F, 0x3A, 0x64, 0x26, 0xFF, 0x06, 0x1A, 0xD5, 0xBC, 0x43, 0x0C, 0xCC, 0x88, 0xF7, 0x39,0x90};
static const unsigned char g_PlcMacQca[6] = {0x00, 0xb0, 0x52, 0x00, 0x00, 0x01};
static const unsigned char g_PlcMacMse[6] = {0x00, 0x13, 0xd7, 0x11, 0x00, 0x01};

#define FREE_MEM(x) if((x)) {delete (x);(x) = NULL;}
#define FREE_MEMS(x) if((x)) {delete [](x);(x) = NULL;}
/******************************************************************************************************************************/
void SlacSession_LogInfo (struct session * pSession)

{
	if (_anyset (pSession->flags, SLAC_SESSION))
	{
		char string [256];
		LOGOUT_DEBUG ("session.RunID %s", HEXSTRING (string, pSession->RunID));
		LOGOUT_DEBUG ("session.APPLICATION_TYPE %d", pSession->APPLICATION_TYPE);
		LOGOUT_DEBUG ("session.SECURITY_TYPE %d", pSession->SECURITY_TYPE);
		LOGOUT_DEBUG ("session.RESP_TYPE %d", pSession->RESP_TYPE);
		LOGOUT_DEBUG ("session.NUM_SOUNDS %d", pSession->NUM_SOUNDS);
		LOGOUT_DEBUG ("session.TIME_OUT %d", pSession->TIME_OUT);
		LOGOUT_DEBUG ("session.NumGroups %d", pSession->NumGroups);
		LOGOUT_DEBUG ("session.AAG %s", hexstring (string, sizeof (string), pSession->AAG, sizeof (pSession->AAG)));
		LOGOUT_DEBUG ("session.MSOUND_TARGET %s", HEXSTRING (string, pSession->MSOUND_TARGET));
		LOGOUT_DEBUG ("session.FORWARDING_STA %s", HEXSTRING (string, pSession->FORWARDING_STA));
		LOGOUT_DEBUG ("session.PEV_ID %s", HEXSTRING (string, pSession->PEV_ID));
		LOGOUT_DEBUG ("session.PEV_MAC %s", HEXSTRING (string, pSession->PEV_MAC));
		LOGOUT_DEBUG ("session.EVSE_ID %s", HEXSTRING (string, pSession->EVSE_ID));
		LOGOUT_DEBUG ("session.EVSE_MAC %s", HEXSTRING (string, pSession->EVSE_MAC));
		LOGOUT_DEBUG ("session.RND %s", HEXSTRING (string, pSession->RND));
		LOGOUT_DEBUG ("session.NMK %s", HEXSTRING (string, pSession->NMK));
		LOGOUT_DEBUG ("session.NID %s", HEXSTRING (string, pSession->NID));
		LOGOUT_DEBUG ("session.original_nmk %s", HEXSTRING (string, pSession->original_nmk));
		LOGOUT_DEBUG ("session.original_nid %s", HEXSTRING (string, pSession->original_nid));
		LOGOUT_DEBUG ("session.state %d", pSession->state);
		LOGOUT_DEBUG ("session.sounds %d", pSession->sounds);
		LOGOUT_DEBUG ("session.limit %d", pSession->limit);
		LOGOUT_DEBUG ("session.pause %d", pSession->pause);
		LOGOUT_DEBUG ("session.chargetime %d", pSession->chargetime);
		LOGOUT_DEBUG ("session.settletime %d", pSession->settletime);
		LOGOUT_DEBUG ("session.counter %d", pSession->counter);
		LOGOUT_DEBUG ("session.flags 0x%04X", pSession->flags);
	}
	return;
}
static void ChangeNMK(unsigned char *pNMK)
{
    const char *fmt_chg_nmk = "cp /ubi/conf/secc/qca7000-1.2.5-new.pib /tmp/15118.evse;"
                              "/ubi/local/bin/open-plc/modpib  -N %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x /tmp/15118.evse";
    char cmd_chg_nmk[512];
    sprintf(cmd_chg_nmk, fmt_chg_nmk,
            pNMK[0], pNMK[1], pNMK[2], pNMK[3], pNMK[4], pNMK[5], pNMK[6], pNMK[7],
            pNMK[8], pNMK[9], pNMK[10], pNMK[11], pNMK[12], pNMK[13], pNMK[14], pNMK[15]);
    LOGOUT_DEBUG("plctool cmd = %s\n", cmd_chg_nmk);
    system(cmd_chg_nmk);
    system("/ubi/local/bin/open-plc/plctool -R;  /ubi/local/bin/open-plc/plcboot -N /ubi/conf/secc/MAC-7000-v1.2.5-00-CS.nvm -P /tmp/15118.evse");
    LOGOUT_DEBUG("PLC Set Key Ok.\n");    
}

static void OnGenerateNetInfo(struct session * pSession)
{
    char uSeed[64];
    sprintf(uSeed, "%02x%02x%02x%d", pSession->EVSE_MAC[3], pSession->EVSE_MAC[5], pSession->EVSE_MAC[5], rand());

    HPAVKeyNMK(g_NMK_ID, uSeed);
    HPAVKeyNID(g_NIM_ID, g_NMK_ID, 0);
    ChangeNMK(g_NMK_ID);
}



static void OnInitSlacSession(struct session * pSession)
{
    pSession->next = pSession->prev = pSession;
	hexencode (pSession->EVSE_ID, sizeof (pSession->EVSE_ID),   EVSE_SID);
    
    memcpy(pSession->NID, g_NIM_ID,sizeof(g_NIM_ID));
    memcpy(pSession->NMK, g_NMK_ID,sizeof(g_NMK_ID));
	// hexencode (pSession->NMK, sizeof (pSession->NMK),  EVSE_NMK);
	// hexencode (pSession->NID, sizeof (pSession->NID),  EVSE_NID);
	pSession->NUM_SOUNDS =  SLAC_MSOUNDS;
	pSession->TIME_OUT = SLAC_TIMETOSOUND;
	pSession->RESP_TYPE = SLAC_RESPONSE_TYPE;
	pSession->chargetime =  SLAC_CHARGETIME;
	pSession->settletime =  SLAC_SETTLETIME;
	memcpy (pSession->original_nmk, pSession->NMK, sizeof (pSession->original_nmk));
	memcpy (pSession->original_nid, pSession->NID, sizeof (pSession->original_nid));
	pSession->state = EVSE_STATE_UNOCCUPIED;
	//slac_session(pSession);
}

static signed OnSalcIdentifier(struct session * pSession, struct channel * channel)
{
	memcpy (pSession->EVSE_MAC, channel->host, sizeof (pSession->EVSE_MAC));
	return (0);
}
   
static void OnSendeCmSetKey(struct session * pSession ,struct channel *pChannel)
{
    struct __packed cm_set_key_request
	{
		struct ethernet_hdr ethernet;
		struct homeplug_fmi homeplug;
		uint8_t KEYTYPE;
		uint32_t MYNOUNCE;
		uint32_t YOURNOUNCE;
		uint8_t PID;
		uint16_t PRN;
		uint8_t PMN;
		uint8_t CCOCAP;
		uint8_t NID [SLAC_NID_LEN];
		uint8_t NEWEKS;
		uint8_t NEWKEY [SLAC_NMK_LEN];
		uint8_t RSVD [3];
	};
    struct cm_set_key_request  *request = new cm_set_key_request;
	memset (request, 0, sizeof (cm_set_key_request));
	
	EthernetHeader (& request->ethernet, pChannel->peer, pChannel->host, pChannel->type);
	HomePlugHeader1 (& request->homeplug, HOMEPLUG_MMV, (CM_SET_KEY | MMTYPE_REQ));
	request->KEYTYPE = SLAC_CM_SETKEY_KEYTYPE;
	memset (& request->MYNOUNCE, 0xAA, sizeof (request->MYNOUNCE));
	memset (& request->YOURNOUNCE, 0x00, sizeof (request->YOURNOUNCE));
	request->PID = SLAC_CM_SETKEY_PID;
	request->PRN = HTOLE16 (SLAC_CM_SETKEY_PRN);
	request->PMN = SLAC_CM_SETKEY_PMN;
	request->CCOCAP = SLAC_CM_SETKEY_CCO;
	memcpy (request->NID, pSession->NID, sizeof (request->NID));
	request->NEWEKS = SLAC_CM_SETKEY_EKS;
	memcpy (request->NEWKEY, pSession->NMK, sizeof (request->NEWKEY));
 	if (sendpacket (pChannel, request, sizeof (struct cm_set_key_request)) <= 0)
		LOGOUT_ERROR("PLC Send packet Fail %s" ,strerror(errno));
    else
        LOGOUT_INFO ("--> CM_SET_KEY.REQ");
    FREE_MEM(request);   
    return;     
}

static void OnSlacParamReq(struct session * pSession, struct channel * pChannel, struct message * pMsg,int iLength)
{
    if(!pSession || !pChannel || !pMsg || !iLength)
    {
        LOGOUT_DEBUG ("<-- Param error");
        return;
    } 
	extern byte const broadcast [ETHER_ADDR_LEN];
	struct cm_slac_param_request * request = (struct cm_slac_param_request *) (pMsg);
	struct cm_slac_param_confirm * confirm = (struct cm_slac_param_confirm *) (pMsg);

    //LOGOUT_DEBUG ("<-- CM_SLAC_PARAM.REQ");
    pSession->APPLICATION_TYPE = request->APPLICATION_TYPE;
    pSession->SECURITY_TYPE = request->SECURITY_TYPE;
    memcpy (pSession->PEV_MAC, request->ethernet.OSA, sizeof (pSession->PEV_MAC));
    memcpy (pSession->FORWARDING_STA, request->ethernet.OSA, sizeof (pSession->FORWARDING_STA));
    memcpy (pSession->RunID, request->RunID, sizeof (pSession->RunID));   

    memset (pMsg, 0, sizeof (* pMsg));
    EthernetHeader (& confirm->ethernet, pSession->PEV_MAC, pChannel->host, pChannel->type);
    HomePlugHeader1 (& confirm->homeplug, HOMEPLUG_MMV, (CM_SLAC_PARAM | MMTYPE_CNF));
    memcpy (confirm->MSOUND_TARGET, broadcast, sizeof (confirm->MSOUND_TARGET));
    confirm->NUM_SOUNDS = pSession->NUM_SOUNDS;
    confirm->TIME_OUT = pSession->TIME_OUT;
    confirm->RESP_TYPE = pSession->RESP_TYPE;
    memcpy (confirm->FORWARDING_STA, pSession->FORWARDING_STA, sizeof (confirm->FORWARDING_STA));
    confirm->APPLICATION_TYPE = pSession->APPLICATION_TYPE;
    confirm->SECURITY_TYPE = pSession->SECURITY_TYPE;
    memcpy (confirm->RunID, pSession->RunID, sizeof (confirm->RunID));
    confirm->CipherSuite = HTOLE16 ((uint16_t) (pSession->counter));  
    if (sendmessage (pChannel, pMsg, (ETHER_MIN_LEN - ETHER_CRC_LEN)) <= 0)
        LOGOUT_ERROR ("Plc Send CNF timeout or network error");
    else
        LOGOUT_DEBUG("--> CM_SLAC_PARAM.CNF");
}

static bool OnStartAttenCharInd(struct session * pSession, struct channel * pChannel, struct message * pMsg,int iLength,byte *byAAG)
{
    if(!pSession || !pChannel || !pMsg || !iLength)
    {
        LOGOUT_DEBUG ("<-- Param error");
        return false;
    } 
    //LOGOUT_DEBUG ("<-- CM_START_ATTEN_CHAR.IND "); 
    struct cm_start_atten_char_indicate * pIndicate = (struct cm_start_atten_char_indicate *) (pMsg);
    if (!memcmp (pSession->RunID, pIndicate->ACVarField.RunID, sizeof (pSession->RunID)))
    {
        if (pIndicate->APPLICATION_TYPE != pSession->APPLICATION_TYPE)
            LOGOUT_DEBUG ( "APPLICATION_TYPE");
        if (pIndicate->SECURITY_TYPE != pSession->SECURITY_TYPE)
            LOGOUT_DEBUG ( "SECURITY_TYPE");
        pSession->NUM_SOUNDS = pIndicate->ACVarField.NUM_SOUNDS;
        pSession->TIME_OUT = pIndicate->ACVarField.TIME_OUT;
        if (pIndicate->ACVarField.RESP_TYPE != pSession->RESP_TYPE)
            LOGOUT_DEBUG ("RESP_TYPE");
        memcpy (pSession->FORWARDING_STA, pIndicate->ACVarField.FORWARDING_STA, sizeof (pSession->FORWARDING_STA)); 
        pSession->sounds = 0;
        memset(pSession->AAG, 0, sizeof(pSession->AAG));
        memset(byAAG, 0, sizeof(*byAAG));
        pSession->state = EVSE_STATE_UNOCCUPIED; 
        return true;              
    }
    return false;
}

static bool OnSendAttenCharInd2Ev(struct session * pSession, struct channel * pChannel, struct message * pMsg)
{
    struct cm_atten_char_indicate * pIndicate = (struct cm_atten_char_indicate *) (pMsg);
    SlacSession_LogInfo(pSession);
	memset (pMsg, 0, sizeof (* pMsg));
	EthernetHeader (& pIndicate->ethernet, pSession->PEV_MAC, pChannel->host, pChannel->type);
	HomePlugHeader1 (& pIndicate->homeplug, HOMEPLUG_MMV, (CM_ATTEN_CHAR | MMTYPE_IND));
	pIndicate->APPLICATION_TYPE = pSession->APPLICATION_TYPE;
	pIndicate->SECURITY_TYPE = pSession->SECURITY_TYPE;
	memcpy (pIndicate->ACVarField.SOURCE_ADDRESS, pSession->PEV_MAC, sizeof (pIndicate->ACVarField.SOURCE_ADDRESS));
	memcpy (pIndicate->ACVarField.RunID, pSession->RunID, sizeof (pIndicate->ACVarField.RunID));
	memset (pIndicate->ACVarField.SOURCE_ID, 0, sizeof (pIndicate->ACVarField.SOURCE_ID));
	memset (pIndicate->ACVarField.RESP_ID, 0, sizeof (pIndicate->ACVarField.RESP_ID));
	pIndicate->ACVarField.NUM_SOUNDS = pSession->sounds;
	pIndicate->ACVarField.ATTEN_PROFILE.NumGroups = pSession->NumGroups;
	memcpy (pIndicate->ACVarField.ATTEN_PROFILE.AAG, pSession->AAG, pSession->NumGroups);
	if (sendmessage (pChannel, pMsg, sizeof (*pIndicate)) <= 0)
	{
        LOGOUT_ERROR("Send timeout or network error");
		return false;
	}
    else
        LOGOUT_DEBUG ("--> CM_ATTEN_CHAR.IND");
    return true;
}

static bool OnMnbcSoundInd(struct session * pSession, struct channel * pChannel, struct message * pMsg,int iLength)
{
    static int iIndex = 0;
    struct cm_mnbc_sound_indicate * indicate = (struct cm_mnbc_sound_indicate *) (pMsg);
	if (! memcmp (pSession->RunID, indicate->MSVarField.RunID, sizeof (pSession->RunID)))
    {
        if (_anyset (pSession->flags, SLAC_SESSION))
        {
            char string [256] = {0};
            LOGOUT_DEBUG ("CM_MNBC_SOUND.IND.APPLICATION_TYPE %d", indicate->APPLICATION_TYPE);
            LOGOUT_DEBUG ("CM_MNBC_SOUND.IND.SECURITY_TYPE %d", indicate->SECURITY_TYPE);
            LOGOUT_DEBUG ("CM_MNBC_SOUND.IND.MSVarField.SenderID %s", HEXSTRING (string, indicate->MSVarField.SenderID));
            LOGOUT_DEBUG ("CM_MNBC_SOUND.IND.MSVarField.Count %d", indicate->MSVarField.CNT);
            LOGOUT_DEBUG ("CM_MNBC_SOUND.IND.MSVarField.RunID %s", HEXSTRING (string, indicate->MSVarField.RunID));
            LOGOUT_DEBUG ("CM_MNBC_SOUND.IND.MSVarField.RND %s", HEXSTRING (string, indicate->MSVarField.RND));
        }  
        if (memcmp (pSession->PEV_MAC, indicate->ethernet.OSA, sizeof (pSession->PEV_MAC)))
            LOGOUT_ERROR ("Unexpected OSA");  
    }
    if(iIndex > 15)
    {
        iIndex = 0;
        OnSendAttenCharInd2Ev(pSession, pChannel, pMsg);
    }
        
    iIndex ++;
    return true;
}

static bool OnAttenCharRep(struct session * pSession, struct channel * pChannel, struct message * pMsg,int iLength)
{
    if(!pSession || !pChannel || !pMsg || !iLength)
    {
        LOGOUT_DEBUG ("<-- Param error");
        return false;
    } 
    return true;
}

static bool OnValiDateReq(struct session * pSession, struct channel * pChannel, struct message * pMsg,int iLength)
{
    if(!pSession || !pChannel || !pMsg || !iLength)
    {
        LOGOUT_DEBUG ("<-- Param error");
        return false;
    } 
    return true;
}

static bool OnAmpMapReq(struct session * pSession, struct channel * pChannel, struct message * pMsg,int iLength)
{
    if(!pSession || !pChannel || !pMsg || !iLength)
    {
        LOGOUT_DEBUG ("<-- Param error");
        return false;
    }     

    LP_CM_AMP_REQ lpCmAmpReq = (LP_CM_AMP_REQ )pMsg;
    LOGOUT_INFO("--> CM_AMP_MAP.REQ");

    memset(lpCmAmpReq, 0, sizeof(CM_AMP_REQ));
    EthernetHeader(&lpCmAmpReq->tagEtherNet, pSession->PEV_MAC, pChannel->host, pChannel->type);
    HomePlugHeader1(&lpCmAmpReq->tagHomeplug, HOMEPLUG_MMV, (CM_AMP_MAP | MMTYPE_REQ));

    lpCmAmpReq->iAmpLen = AMP_LEN;
    for (int i = 0; i < 58 / 2; i++) {
        if (pSession->AAG[i * 2] < 25)
            lpCmAmpReq->arrAmpData[i] = 0xD0;

        if (pSession->AAG[i * 2 + 1] < 25)
            lpCmAmpReq->arrAmpData[i] |= 0x0D;
    }

    if (sendmessage(pChannel, pMsg, sizeof(CM_AMP_REQ)) <= 0) {
        LOGOUT_ERROR("Send timeout or network error");
        return -1;
    }

    return true;
}


static bool OnCMSlacMath(struct session * pSession, struct channel * pChannel, struct message * pMsg,int iLength)
{
    if(!pSession || !pChannel || !pMsg || !iLength)
    {
        LOGOUT_DEBUG ("<-- Param error");
        return false;
    }   
	struct cm_slac_match_request * pRequest = (struct cm_slac_match_request *) (pMsg);
	struct cm_slac_match_confirm * pConfirm = (struct cm_slac_match_confirm *) (pMsg);
    if (! memcmp (pSession->RunID, pRequest->MatchVarField.RunID, sizeof (pSession->RunID)))
    {
        //LOGOUT_DEBUG ("<-- CM_SLAC_MATCH.REQ");
        memcpy (pSession->PEV_ID, pRequest->MatchVarField.PEV_ID, sizeof (pSession->PEV_ID));
        memcpy (pSession->PEV_MAC, pRequest->MatchVarField.PEV_MAC, sizeof (pSession->PEV_MAC));
        memcpy (pSession->RunID, pRequest->MatchVarField.RunID, sizeof (pSession->RunID)); 

        if (_anyset (pSession->flags, SLAC_VERBOSE))
        {
            char string [256];
            LOGOUT_DEBUG ("CM_SLAC_MATCH.REQ.APPLICATION_TYPE %d", pRequest->APPLICATION_TYPE);
            LOGOUT_DEBUG ("CM_SLAC_MATCH.REQ.SECURITY_TYPE %d", pRequest->SECURITY_TYPE);
            LOGOUT_DEBUG ("CM_SLAC_MATCH.REQ.MVFLength %d", LE16TOH (pRequest->MVFLength));
            LOGOUT_DEBUG ("CM_SLAC_MATCH.REQ.PEV_ID %s", HEXSTRING (string, pRequest->MatchVarField.PEV_ID));
            LOGOUT_DEBUG ("CM_SLAC_MATCH.REQ.PEV_MAC %s", HEXSTRING (string, pRequest->MatchVarField.PEV_MAC));
            LOGOUT_DEBUG ("CM_SLAC_MATCH.REQ.EVSE_ID %s", HEXSTRING (string, pRequest->MatchVarField.EVSE_ID));
            LOGOUT_DEBUG ("CM_SLAC_MATCH.REQ.EVSE_MAC %s", HEXSTRING (string, pRequest->MatchVarField.EVSE_MAC));
            LOGOUT_DEBUG ("CM_SLAC_MATCH.REQ.RunID %s", HEXSTRING (string, pRequest->MatchVarField.RunID));
        }  
        
        memset (pMsg, 0, sizeof (* pMsg));
        EthernetHeader (& pConfirm->ethernet, pSession->PEV_MAC, pChannel->host, pChannel->type);
        HomePlugHeader1 (& pConfirm->homeplug, HOMEPLUG_MMV, (CM_SLAC_MATCH | MMTYPE_CNF));
        pConfirm->APPLICATION_TYPE = pSession->APPLICATION_TYPE;
        pConfirm->SECURITY_TYPE = pSession->SECURITY_TYPE;
        pConfirm->MVFLength = HTOLE16 (sizeof (pConfirm->MatchVarField));
        memcpy (pConfirm->MatchVarField.PEV_ID, pSession->PEV_ID, sizeof (pConfirm->MatchVarField.PEV_ID));
        memcpy (pConfirm->MatchVarField.PEV_MAC, pSession->PEV_MAC, sizeof (pConfirm->MatchVarField.PEV_MAC));
        memcpy (pConfirm->MatchVarField.EVSE_ID, pSession->EVSE_ID, sizeof (pConfirm->MatchVarField.EVSE_ID));
        memcpy (pConfirm->MatchVarField.EVSE_MAC, pSession->EVSE_MAC, sizeof (pConfirm->MatchVarField.EVSE_MAC));
        memcpy (pConfirm->MatchVarField.RunID, pSession->RunID, sizeof (pConfirm->MatchVarField.RunID));
        memcpy (pConfirm->MatchVarField.NID, pSession->NID, sizeof (pConfirm->MatchVarField.NID));
        memcpy (pConfirm->MatchVarField.NMK, pSession->NMK, sizeof (pConfirm->MatchVarField.NMK));
        LOGOUT_DEBUG ("--> CM_SLAC_MATCH.CNF \r\n");
        if (sendmessage (pChannel, pMsg, sizeof (* pConfirm)) <= 0)
           LOGOUT_ERROR("Send timeout or network error");
        else
        {  
            uint8_t iMsgData = EM_MSG_SLAC_MATCH;
            SendMsgUint8(iMsgData);           
            OnAmpMapReq(pSession, pChannel, pMsg, iLength);
        }
            
    }              
    return true;
}

static bool OnAmpMapCnf(struct session * pSession, struct channel * pChannel, struct message * pMsg,int iLength)
{
    if(!pSession || !pChannel || !pMsg || !iLength)
    {
        LOGOUT_DEBUG ("<-- Param error");
        return false;
    }   
    LP_CM_AMP_CNF lpCmAmpCnf = (LP_CM_AMP_CNF)pMsg;

    memset(lpCmAmpCnf, 0, sizeof(CM_AMP_CNF));
    EthernetHeader(&lpCmAmpCnf->tagEtherNet, pSession->PEV_MAC, pChannel->host, pChannel->type);
    HomePlugHeader1(&lpCmAmpCnf->tagHomeplug, HOMEPLUG_MMV, (CM_AMP_MAP | MMTYPE_CNF));

    // 0x00 success
    // 0x01 failure
    lpCmAmpCnf->iResType = 0x00;

    if (sendmessage(pChannel, pMsg, sizeof(CM_AMP_CNF)) <= 0) {
        LOGOUT_ERROR("Send timeout or network error");
        return -1;
    }

    LOGOUT_INFO("--> CM_AMP_MAP.CNF");
    return true;
}


static bool OnCmAttenProfileInd(struct session * pSession, struct channel * pChannel, 
                                struct message * pMsg,int iLength, byte *byAAG)
{
    struct cm_atten_profile_indicate * pIndicate = (struct cm_atten_profile_indicate *) (pMsg);
    if (! memcmp(pSession->PEV_MAC, pIndicate->PEV_MAC, sizeof (pSession->PEV_MAC)))
    {
        if (_anyset (pSession->flags, SLAC_SESSION))
        {
            char string [256];
            LOGOUT_DEBUG("CM_ATTEN_PROFILE.PEV_MAC %s", HEXSTRING (string, pIndicate->PEV_MAC));
            LOGOUT_DEBUG("CM_ATTEN_PROFILE.NumGroups %d", pIndicate->NumGroups);
            LOGOUT_DEBUG("CM_ATTEN_PROFILE.AAG %s", hexstring (string, sizeof (string), pIndicate->AAG, pIndicate->NumGroups));
        }
        for (pSession->NumGroups = 0; pSession->NumGroups < pIndicate->NumGroups; pSession->NumGroups++)
            byAAG [pSession->NumGroups] += pIndicate->AAG [pSession->NumGroups];

        pSession->NumGroups = pIndicate->NumGroups;
        if(SLAC_GROUPS > pSession->sounds)
            pSession->sounds++;
    }  
    if (pSession->sounds > 0)
    {
        for (pSession->NumGroups = 0; pSession->NumGroups < SLAC_GROUPS; ++ pSession->NumGroups)
            pSession->AAG [pSession->NumGroups] = byAAG [pSession->NumGroups] / pSession->sounds;
    }
    return true;
}

static void OnAnalysisInfo(struct message *pMsg,struct session *pSession,struct channel *pChannel, int iReadSize,byte *byAAG)
{
    if(!pMsg || !pSession)
        return;
    struct homeplug * homeplug = (struct homeplug *)(pMsg);
   
    if(!UnwantedMessage(pMsg, iReadSize, HOMEPLUG_MMV,(CM_SLAC_PARAM | MMTYPE_REQ)))
    {
        LOGOUT_DEBUG ("<-- :"PARAM_2_CHAR(CM_SLAC_PARAM | MMTYPE_REQ));
        OnSlacParamReq(pSession, pChannel, pMsg, iReadSize);
    } 
    else if(!UnwantedMessage(pMsg, iReadSize, HOMEPLUG_MMV,(CM_START_ATTEN_CHAR | MMTYPE_IND)))
    {
        LOGOUT_DEBUG ("<-- :"PARAM_2_CHAR(CM_START_ATTEN_CHAR | MMTYPE_IND));
        OnStartAttenCharInd(pSession, pChannel, pMsg, iReadSize,byAAG);
    }    
    else if(!UnwantedMessage(pMsg, iReadSize, HOMEPLUG_MMV,(CM_MNBC_SOUND | MMTYPE_IND)))
    {
        LOGOUT_DEBUG ("<-- :"PARAM_2_CHAR(CM_MNBC_SOUND | MMTYPE_IND));  
        OnMnbcSoundInd(pSession, pChannel, pMsg, iReadSize);
    }
    else if(!UnwantedMessage(pMsg, iReadSize, HOMEPLUG_MMV,(CM_ATTEN_CHAR | MMTYPE_RSP)))
    {
        LOGOUT_DEBUG ("<-- :"PARAM_2_CHAR(CM_ATTEN_CHAR | MMTYPE_RSP));  
        OnAttenCharRep(pSession, pChannel, pMsg, iReadSize);
    }   
    else if(!UnwantedMessage(pMsg, iReadSize, HOMEPLUG_MMV,(CM_VALIDATE | MMTYPE_REQ)))
    {
        LOGOUT_DEBUG ("<-- :"PARAM_2_CHAR(CM_VALIDATE | MMTYPE_REQ));
        OnValiDateReq(pSession, pChannel, pMsg, iReadSize);
    }     
    else if(!UnwantedMessage(pMsg, iReadSize, HOMEPLUG_MMV,(CM_SLAC_MATCH | MMTYPE_REQ)))
    {
        LOGOUT_DEBUG ("<-- :"PARAM_2_CHAR(CM_SLAC_MATCH | MMTYPE_REQ)); 
        OnCMSlacMath(pSession, pChannel, pMsg, iReadSize);
    }         
    else if(!UnwantedMessage(pMsg, iReadSize, HOMEPLUG_MMV,(CM_AMP_MAP | MMTYPE_CNF)))
    {
        LOGOUT_DEBUG ("<-- :"PARAM_2_CHAR(CM_AMP_MAP | MMTYPE_CNF)); 
        OnAmpMapCnf(pSession, pChannel, pMsg, iReadSize);
    }     
    else if(!UnwantedMessage(pMsg, iReadSize, HOMEPLUG_MMV,(CM_AMP_MAP | MMTYPE_REQ)))
    {
        LOGOUT_DEBUG ("<-- :"PARAM_2_CHAR(CM_AMP_MAP | MMTYPE_REQ));   
        OnAmpMapReq(pSession, pChannel, pMsg, iReadSize);
    }
    else if(!UnwantedMessage(pMsg, iReadSize, HOMEPLUG_MMV,(CM_ATTEN_PROFILE | MMTYPE_IND)))
    {
        LOGOUT_DEBUG ("<-- :"PARAM_2_CHAR(CM_ATTEN_PROFILE | MMTYPE_IND));  
        OnCmAttenProfileInd(pSession, pChannel, pMsg, iReadSize,byAAG);
    }
    else
        LOGOUT_DEBUG("Ev type : 0x%x,0x%x",homeplug->homeplug.MMTYPE,homeplug->homeplug.MMV);
                             
}
/******************************************************************************************************************************/
CSlacManage::CSlacManage()
{
    m_pPlcChannel = new struct channel;
    m_pPlcSession = new struct session;
    m_pAAgData = new byte[SLAC_GROUPS];
    memset(m_pAAgData, 0, SLAC_GROUPS);
}

CSlacManage::~CSlacManage()
{
    FREE_MEM(m_pPlcChannel);
    FREE_MEM(m_pPlcSession); 
    FREE_MEMS(m_pAAgData);  
}
//struct channel g_channel;

void CSlacManage::ThreadProcFun()
{
    struct message message = {0};
    int iLength = 0;
    if(!m_pPlcChannel)
    {
        sleep(1);
        return;
    }
    if((iLength = readpacket (m_pPlcChannel, &message, sizeof (message))) > 0)
    {
        OnAnalysisInfo(&message, m_pPlcSession, m_pPlcChannel, iLength,m_pAAgData);
        //LOGOUT_INFO("ThreadProcFun = %s,%s,%s,%d", message.ethernet.ODA, message.ethernet.OSA, message.content, message.ethernet.MTYPE);
    }
}

void CSlacManage::InitSlacInfo()
{
    extern struct channel channel;
    
    memcpy(m_pPlcChannel, &channel, sizeof(channel));

    initchannel (m_pPlcChannel);
    desuid ();

    memcpy(m_pPlcChannel->peer, g_PlcMacMse, sizeof(g_PlcMacMse)); 
    m_pPlcChannel->timeout = SLAC_TIMEOUT;
    m_pPlcChannel->ifname = "eth1";
    //_setbits (m_pPlcChannel->flags, CHANNEL_VERBOSE);
    //_setbits (m_pPlcSession->flags, SLAC_SESSION);
    
    openchannel(m_pPlcChannel);
    OnInitSlacSession (m_pPlcSession);
	OnSalcIdentifier (m_pPlcSession, m_pPlcChannel);
    LOGOUT_INFO("InitSlacInfo =  %d,%s", m_pPlcChannel->fd, m_pPlcChannel->ifname);
}

 void CSlacManage::OnSendSetKeyReq()
 {
    OnSendeCmSetKey(m_pPlcSession,m_pPlcChannel); 
 }

 void CSlacManage::UnInitSlacInfo()
 {
    closechannel(m_pPlcChannel);
    FREE_MEM(m_pPlcChannel);
    FREE_MEM(m_pPlcSession); 
    FREE_MEMS(m_pAAgData); 
 }