/**
 * @file 
 * @author 
 * @date 2020/12/14
 * @brief
 *
 */



#ifndef __CC_ERROR_CODE_H__
#define __CC_ERROR_CODE_H__

/**
 *  @defgroup ERR_MACRO  错误编号
 *  @{
 */
#define V2G_OPTIONAL_MSG            -1001   // 可选协议
#define V2G_STOP_MSG                -1002   // 停止协议
#define V2G_PAUSE_MSG               -1003   // 暂停协议
#define V2G_STATE_DISORDER          -1004   // 没有按照顺序来处理消息            FAILED_SequenceError
#define V2G_OLDSESSION_JOINED       -1005   // 旧的Session加入

/*ADD BY ZHAOZHEN 20220608 FOR 增加未插入TF卡错误码*/
#define ERR_NO_TF_CARD       -1006   // 未插入TF卡

#define V2G_MSG_UNKOWN_TYPE         -1010

#define ERR_V2G_INVALID_SERVICE_ID  -1014   // 错误的服务号
#define ERR_V2G_INVALID_PAYMENT     -1015   // 错误的支付方式
#define ERR_V2G_NO_CHARGE_SERVICE   -1016   // 支付方付未选择充电服务
#define ERR_V2G_INVALID_CERT        -1017   // 证书错误
#define ERR_V2G_LIST_OF_ROOTCERT_EXCEED     -1018   // ListOfRootCertificateIDs超出规定数量
#define ERR_V2G_REQUEST_NEW_CERT    -1019   // 请求新证书失败
#define ERR_V2G_NOT_A_SECURE_LINK   -1020   // 连接方式错误, 非TLS连接



#define ERR_V2G_HANDSHAKE           -1030   // 握手失败
#define ERR_V2G_REJECT_BYEVSE       -1031   // 由EVSE主动拒绝
#define ERR_V2G_EXCHANGE_PARAM      -1032   // 交换充电参数出错
#define ERR_V2G_ALGORITHM           -1033   // 算法错误
#define ERR_V2G_HASH                -1034   // Hash值错误
#define ERR_V2G_VERIFY              -1035   // 签名校验错误



#define ERR_V2G_DISCONNECT          -1040   ///< 数据连接丢失
#define ERR_V2G_WRONG_PROTOCOL      -1041   ///< 错误的协议

#define ERR_SESS_INIT               -1100   ///< Session初始化错误
#define ERR_SESS_REGISTER           -1101   ///< Session注册错误
#define ERR_SESS_MATCH_EVSE         -1102   ///< 没有匹配的EVSE与连接匹配



#define ERR_SYS_MALLOC              -1200   ///< 分配内存错误




#define ERR_V2G_VERSION             -10001
#define ERR_V2G_PAYLOAD_TYPE        -10002  // 类型错误
#define ERR_V2G_PAYLOAD_LENGTH      -10003  //


#define ERR_V2G_EVCCID              -10004  // EVCCID错误
#define ERR_V2G_DECODE_EXI          -10005  // EXI解码错误
#define ERR_V2G_UNKNOWN_SESSION     -10006  // session和当前连接不匹配
#define ERR_V2G_NEED_HANDSHAKE      -10007  // 需要先握手
#define ERR_V2G_MSG_HEADER          -10008  ///< 消息的头错误，同时会指明错误类型



#define ERR_V2G_TRANS_PROTOCOL      -10009  // 协议错误
#define ERR_V2G_POWERDELIVERY_HAVE_NOT_STARTED       -10010   // TC_SECC_DC_VTB_PowerDelivery_013 , 必须之前发送过start命令
#define ERR_V2G_INVALID_CP_STATE    -10011   // 错误的CP电压
#define ERR_V2G_EV_NOT_READY        -10012   // 车辆未准备好
#define ERR_V2G_EV_ERROR            -10013   // 车辆故障码
#define ERR_V2G_CHARGE_PROFILE      -10014   // 请求功率超过限制
#define ERR_V2G_SASCHEDULE_ID       -10015   // 非法的schedule
#define ERR_V2G_ENERGY_TRANS_TYPE   -10016   // 错误的能源方式
#define ERR_V2G_INVALID_TRANS_TYPE  -10016   // 非法的schedule
#define ERR_V2G_ONGOING_TIMEOUT     -10017   // 超时
#define ERR_V2G_SQUENCE_TIMER_OUT   -10018  // 等待消息超时
#define ERR_V2G_WEDDING_DETECT_TIMOUT   -10019  ///< 绝缘超时



#define ERR_V2G_MSG_HASH            -10100  ///< hash值错误
#define ERR_PNC_SIGNATURE           -10101  ///< 签名错误
#define ERR_PNC_ENCODE_SIGNIFO      -10102  ///< signinfo编码错误
#define ERR_PNC_ENCODE_FRAG         -10103  ///< frag编码错误
#define ERR_PNC_DIGST_NOT_EXIST     -10104  ///< hash值不存在
#define ERR_PNC_DIGST_NOT_MATCH     -10105  ///< hash值不存在
#define ERR_PNC_INVALID_REQUEST     -10106  ///<
#define ERR_PNC_SIGNATUE_NOT_EXISTS -10107  ///< 未提供签名值



#define ERR_PNC_CERT_FORMAT         -10110  ///< 证书格式
#define ERR_PNC_CERT_OEM_EXPIRE     -10111  ///< 证书格式
#define ERR_PNC_CERT_CHAIN_OERDER   -10112  ///<
#define ERR_PNC_CONTRACT_COUNTRY    -10113  ///<
#define ERR_PNC_CONTRACT_EMAID      -10114  ///<

#define ERR_PNC_OEM_LEAF_DOMAIN     -10115  ///<
#define ERR_PNC_CERT_CHAIN_OUT_NUMBER     -10116  ///< 证书数量超标



#define ERR_PNC_BIO                 -10120  ///<



#define ERR_DRIVER_ADC_L             -14000  ///< ADC 低值范围错误
#define ERR_DRIVER_ADC_H             -14001  ///< ADC 高值范围错误



#define ERR_RKN_MSG_TIMEOUT         -15000  // 瑞凯诺的消息超时
#define ERR_RKN_SESSION_TIMEOUT     -15001  //
#define ERR_RKN_SERVICE_DISCOVERY_TIMEOUT   -15002  //
#define ERR_RKN_PAYMENT_SELECT_TIMEOUT      -15003  //
#define ERR_RKN_EXCHANGE_PARAM_TIMEOUT      -15004  //
#define ERR_RKN_DELIVERY_TIMEOUT            -15005  //
#define ERR_RKN_AUTH_TIMEOUT                -15006  //
#define ERR_RKN_CABLE_CHECK_TIMEOUT         -15007
#define ERR_RKN_PRECHARGE_TIMEOUT           -15008
#define ERR_RKN_CURRENT_DEMAND_TIMEOUT      -15009
#define ERR_RKN_WELDING_TIMEOUT             -15010
#define ERR_RKN_SESSION_STOP_TIMEOUT        -15011


/**
 * SLAC 错误信息
 */
#define ERR_SLAC_INIT_CP_VOLTAGE            -15050  ///< 刚刚启动时，CP电压错误
#define ERR_SLAC_INIT_PARAM_TIMOUT          -15051  ///< SLAC_PARAM 超时
#define ERR_SLAC_LINK_DETECT_TIMOUT         -15052  ///< 连接检测超时
#define ERR_SLAC_SDP_REQ_TIMOUT             -15053  ///< 等待SDP超时
#define ERR_SLAC_DEV_COMM_TIMOUT            -15054  ///< SECC通讯超时








/**
 * 星充消息错误类型
 */
#define ERR_STAR_MSG_TIMEOUT                -15100  // 消息超时
#define ERR_STAR_MSG_HEART                  -15101  // 心跳错误
#define ERR_STAR_SESSION_TIMEOUT            -15103
#define ERR_STAR_PAYMENT_SERVICE_SELECTION_TIMEOUT  -15104
#define ERR_STAR_AUTH_TIMEOUT               -15105
#define ERR_STAR_CABLE_CHECK_TIMEOUT        -15106
#define ERR_EXCHANG_PARAMETER_TIMEOUT       -15107
#define ERR_STAR_PRECHARGE_TIMEOUT          -15108
#define ERR_STAR_POWER_DELIVERY_TIMEOUT     -15109
#define ERR_STAR_CURRENT_DEMAND_TIMEOUT     -15110
#define ERR_STAR_WELDING_DETECTION_TIMEOUT  -15111
#define ERR_STAR_SCHEDULES_DECODE           -15113
#define ERR_STAR_CERT_INSTALL_TIMEOUT       -15114
#define ERR_STAR_CERT_UPDATE_TIMEOUT        -15115







#define ERR_EVSE_PLUGIN_WHEN_CHARGING       -16001  //充电时，检测到PLUGIN事件，不符合逻辑,              cp 6 => 9
#define ERR_EVSE_PLUGOUT_WHEN_CHARGING      -16002  //充电时，检测到PLUGOUT事件，不符合逻辑
#define ERR_EVSE_BC_STOP_OUTPUT_WHEN_CHARGING      -16003  // BC充电时， 检测到 6 =》 9， 停止输出
#define ERR_EVSE_UNMATCHED_MAC              -16004  ///< 未匹配的MAC地址
#define ERR_EVSE_SW_OFF_BC                  -16005  ///< 充电过程中，SW断开
#define ERR_EVSE_SW_OFF_HLC                 -16006  ///< 充电过程中，SW断开
#define ERR_EVSE_NOT_READY                  -16007  ///< 充电过程中，Ready = 0







#define ERR_TBL_IS_DISABLED                  -16100      ///< SECC未使能

#define ERR_TBL_FW_PACKAGE_LENGTH            -16200      ///< 升级包长度
#define ERR_TBL_FW_PACKAGE_SEQUENCE          -16201      ///< 升级包顺序
#define ERR_TBL_FW_HASH                      -16205      ///< 包校检错误

#endif

