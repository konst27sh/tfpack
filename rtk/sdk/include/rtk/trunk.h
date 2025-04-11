/*
 * Copyright (C) 2009-2016 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : Definition of TRUNK API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) User Configuration Trunk
 *
 */

#ifndef __RTK_TRUNK_H__
#define __RTK_TRUNK_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>

/*
 * Symbol Definition
 */
 #define RTK_MAX_NUM_OF_TRUNK_MBR_PORT              8

#if defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
#define TRUNK_DISTRIBUTION_ALGO_SPA_BIT         0x01
#define TRUNK_DISTRIBUTION_ALGO_SMAC_BIT        0x02
#define TRUNK_DISTRIBUTION_ALGO_DMAC_BIT        0x04
#define TRUNK_DISTRIBUTION_ALGO_SIP_BIT         0x08
#define TRUNK_DISTRIBUTION_ALGO_DIP_BIT         0x10
#define TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT  0x20
#define TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT  0x40
#define TRUNK_DISTRIBUTION_ALGO_MASKALL         0x7F
#endif

#if defined(CONFIG_SDK_RTL9300) || defined(CONFIG_SDK_RTL9310)
#define TRUNK_DISTRIBUTION_ALGO_L2_SPA_BIT         0x01
#define TRUNK_DISTRIBUTION_ALGO_L2_SMAC_BIT        0x02
#define TRUNK_DISTRIBUTION_ALGO_L2_DMAC_BIT        0x04
#define TRUNK_DISTRIBUTION_ALGO_L2_VLAN_BIT         0x08
#define TRUNK_DISTRIBUTION_ALGO_L2_MASKALL         0xF

#define TRUNK_DISTRIBUTION_ALGO_L3_SPA_BIT         0x01
#define TRUNK_DISTRIBUTION_ALGO_L3_SMAC_BIT        0x02
#define TRUNK_DISTRIBUTION_ALGO_L3_DMAC_BIT        0x04
#define TRUNK_DISTRIBUTION_ALGO_L3_VLAN_BIT         0x08
#define TRUNK_DISTRIBUTION_ALGO_L3_SIP_BIT         0x10
#define TRUNK_DISTRIBUTION_ALGO_L3_DIP_BIT         0x20
#define TRUNK_DISTRIBUTION_ALGO_L3_SRC_L4PORT_BIT  0x40
#define TRUNK_DISTRIBUTION_ALGO_L3_DST_L4PORT_BIT  0x80
#define TRUNK_DISTRIBUTION_ALGO_L3_PROTO_BIT  0x100
#define TRUNK_DISTRIBUTION_ALGO_L3_FLOW_LABEL_BIT  0x200
#define TRUNK_DISTRIBUTION_ALGO_L3_MASKALL         0x3FF
#endif

#if defined(CONFIG_SDK_RTL9310)
#define INVALID_TRUNK_MEMBER_PORT (0x3F)
#else
#define INVALID_TRUNK_MEMBER_PORT (0x3F)
#endif

typedef enum rtk_trunk_bindType_e
{
    BIND_TYPE_L2 = 0,
    BIND_TYPE_IPV4,
    BIND_TYPE_IPV6,
    BIND_TYPE_END
} rtk_trunk_bindType_t;

typedef enum rtk_trunk_hashParamType_e
{
    PARAM_TYPE_L2 = 0,
    PARAM_TYPE_L3,
    PARAM_TYPE_END,
} rtk_trunk_hashParamType_t;

typedef enum rtk_trunk_mode_e
{
    TRUNK_MODE_STACKING = 0,
    TRUNK_MODE_STANDALONE,
    TRUNK_MODE_END
} rtk_trunk_mode_t;

/* Flood mode of trunk */
typedef enum rtk_trunk_floodMode_e
{
    FLOOD_MODE_BY_HASH = 0,
    FLOOD_MODE_BY_CONFIG,
    FLOOD_MODE_END
} rtk_trunk_floodMode_t;

typedef struct  rtk_trunk_l2distAlgoParam_s
{
    uint8 spa;
    uint8 smac;
    uint8 dmac;
    uint8 vlan;
} rtk_trunk_l2distAlgoParam_t;

typedef struct  rtk_trunk_l3distAlgoParam_s
{
    uint8 spa;
    uint8 smac;
    uint8 dmac;
    uint8 vlan;
    uint8 sip;
    uint8 dip;
    uint8 sport;
    uint8 dport;
    uint8 proto;
    uint8 flowLabel;
} rtk_trunk_l3distAlgoParam_t;

typedef struct  rtk_trunk_distAlgoShift_s
{
    uint32 spa_shift;
    uint32 smac_shift;
    uint32 dmac_shift;
    uint32 vlan_shift;
    uint32 sip_shift;
    uint32 dip_shift;
    uint32 sport_shift;
    uint32 dport_shift;
    uint32 proto_shift;
    uint32 flowLabel_shift;
} rtk_trunk_distAlgoShift_t;

typedef struct rtk_trk_egrPort_s {
    uint32     num_ports;  /* Number of ports in the trunk group */
    rtk_dev_port_t egr_port[RTK_MAX_NUM_OF_TRUNK_MBR_PORT];
} rtk_trk_egrPort_t;

typedef enum rtk_trunk_separateType_e
{
    SEPARATE_NONE = 0,
    SEPARATE_KNOWN_MULTI, /*known L2/IP multicast packets*/
    SEPARATE_FLOOD, /*DLF and broadcast packets*/
    SEPARATE_KNOWN_MULTI_AND_FLOOD,
    SEPARATE_END
} rtk_trunk_separateType_t;

typedef enum rtk_trunk_tunnelHashSrc_e
{
    TUNNEL_HASH_SRC_OUTER_HEADER = 0,
    TUNNEL_HASH_SRC_INNER_HEADER,
    TUNNEL_HASH_SRC_END
} rtk_trunk_tunnelHashSrc_t;

typedef enum rtk_trunk_stkTrkHash_e
{
    STACK_TRK_HASH_KEEP = 0,
    STACK_TRK_HASH_RECALCULATE,
    STACK_TRK_HASH_END
} rtk_trunk_stkTrkHash_t;

typedef enum rtk_trunk_portType_e
{
    TRUNK_PORT_TYPE_NORMAL = 0,
    TRUNK_PORT_TYPE_TRUNK,
    TRUNK_PORT_TYPE_END
} rtk_trunk_portType_t;
/*
 * Data Declaration
 */

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Module Name : TRUNK */

/* Function Name:
 *      rtk_trunk_init
 * Description:
 *      Initialize trunk module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8380, 8390, 9300, 9310
 * Note:
 *      Must initialize trunk module before calling any trunk APIs.
 * Changes:
 *      None
 */
extern int32
rtk_trunk_init(uint32 unit);

/* Module Name    : Trunk                    */
/* Sub-module Name: User configuration trunk */

/* Function Name:
 *      rtk_trunk_mode_get
 * Description:
 *      Get the trunk mode from the specified device.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - pointer buffer of trunk mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      The enum of the trunk mode as following
 *      - TRUNK_MODE_STACKING
 *      - TRUNK_MODE_STANDALONE
 * Changes:
 *      [SDK_3.0.0]
 *          (1) The purpose of this API is changed. Current version is to get trunk stacking/stand-alone mode state.
 */
extern int32
rtk_trunk_mode_get (uint32 unit, rtk_trunk_mode_t *pMode);

/* Function Name:
 *      rtk_trunk_mode_set
 * Description:
 *      Set the trunk mode to the specified device.
 * Input:
 *      unit - unit id
 *      mode - trunk mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      9300, 9310
 * Note:
 *      The enum of the trunk mode as following
 *      - TRUNK_MODE_STACKING
 *      - TRUNK_MODE_STANDALONE
 * Changes:
 *      [SDK_3.0.0]
 *          (1) The purpose of this API is changed. Current version is to get trunk stacking/stand-alone mode state.
 */
extern int32
rtk_trunk_mode_set(uint32 unit, rtk_trunk_mode_t mode);

/* Function Name:
 *      rtk_trunk_port_get
 * Description:
 *      Get the members of the trunk id from the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid                - trunk group id
 * Output:
 *      pTrunk_member_portmask - pointer buffer of trunk member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380, 8390, 9300, 9310
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8380, 0~15 in 8390,
 *          0~29 in 9300 stand-alone mode, 0~63 in 9300 stacking mode.
 *          0~25 in 9310 stand-alone mode, 0~127 in 9310 stacking mode.
 * Changes:
 *      [SDK_3.0.0]
 *          (1) For 9300 or 9310, this API can only be used in trunk stand alone mode.
 */
extern int32
rtk_trunk_port_get(uint32 unit, rtk_trk_t trk_gid, rtk_portmask_t *pTrunk_member_portmask);

/* Function Name:
 *      rtk_trunk_port_set
 * Description:
 *      Set the members of the trunk id to the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid                - trunk group id
 *      pTrunk_member_portmask - trunk member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_LA_TRUNK_ID       - invalid trunk ID
 *      RT_ERR_LA_MEMBER_OVERLAP - the specified port mask is overlapped with other group
 * Applicable:
 *      8380, 8390, 9300, 9310
 * Note:
 *      (1) The valid range of trk_gid is 0~7 in 8380, 0~15 in 8390,
 *          0~29 in 9300 stand-alone mode, 0~63 in 9300 stacking mode.
 *          0~25 in 9310 stand-alone mode, 0~127 in 9310 stacking mode.
 *      (2) In 9300 & 9310, this is a backward compatible API that would do rtk_trunk_localPort_set() and rtk_trunk_egrPort_set() internally.
 *      (3) In 9300 & 9310 stacking mode, this API is not supposed to be used since trunk egress port could be on different units that this API is unable to specify.
 * Changes:
 *      [SDK_3.0.0]
 *          (1) For 9300 or 9310, this API can only be used in trunk stand alone mode.
 */
extern int32
rtk_trunk_port_set(uint32 unit, rtk_trk_t trk_gid, rtk_portmask_t *pTrunk_member_portmask);

/* Function Name:
 *      rtk_trunk_localPort_get
 * Description:
 *      Get the local member ports of a trunk id from the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid               - trunk group id that contains local trunk member ports
 * Output:
 *      pTrk_local_ports        - pointer buffer of local trunk member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of trk_gid is 0~29 in 9300 stand-alone mode, 0~63 in 9310 stacking mode.
 *           The valid range of trk_gid is 0~25 in 9310 stand-alone mode, 0~127 in 9310 stacking mode.
 *      (2) The local trunk member portmask would be used for source port filtering, and non-unicast packet forwarding.
 *      (3) The local portmask are trunk members of the unit. For example, if trunk 10 contains member ports
 *         (unit 0, port 1), (unit 0, port 2) and (unit 3, port1). Local portmask of trunk 10 of unit 0 is port 1 and port 2.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 *      [SDK_3.1.0]
 *          API name changed.
 */
extern int32
rtk_trunk_localPort_get (uint32 unit, rtk_trk_t trk_gid, rtk_portmask_t *pTrk_local_ports);


/* Function Name:
 *      rtk_trunk_localPort_set
 * Description:
 *      Set the local member ports of a trunk id from the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid               - trunk group id that contains local trunk member ports
 *      pTrk_local_ports         - local trunk member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of trk_gid is 0~29 in 9300 stand-alone mode, 0~63 in 9310 stacking mode.
 *           The valid range of trk_gid is 0~25 in 9310 stand-alone mode, 0~127 in 9310 stacking mode.
 *      (2) The local trunk member portmask would be used for source port filtering, and non-unicast packet forwarding.
 *      (3) The local portmask are trunk members of the unit. For example, if trunk 10 contains member ports
 *         (unit 0, port 1), (unit 0, port 2) and (unit 3, port1). Local portmask of trunk 10 of unit 0 is port 1 and port 2.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 *      [SDK_3.1.0]
 *          API name changed.
 */
extern int32
rtk_trunk_localPort_set (uint32 unit, rtk_trk_t trk_gid, rtk_portmask_t *pTrk_local_ports);


/* Function Name:
 *      rtk_trunk_egrPort_get
 * Description:
 *      Get the trunk egress candidate ports from the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid                - trunk group id
 * Output:
 *      pTrk_egr_ports       - pointer buffer of trunk egress candidate ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of trk_gid is 0~29 in 9300 stand-alone mode, 0~63 in 9310 stacking mode.
 *           The valid range of trk_gid is 0~25 in 9310 stand-alone mode, 0~127 in 9310 stacking mode.
 *      (2) The traffic sent to trunk group would be hashed and an outgoing port would be picked from egress candidate ports.
 *      (3) Separated traffic or traffic ignoring trunk hash would not follow egress candidate ports to forward.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_egrPort_get(uint32 unit, rtk_trk_t trk_gid, rtk_trk_egrPort_t *pTrk_egr_ports);

/* Function Name:
 *      rtk_trunk_egrPort_set
 * Description:
 *      Set the trunk egress candidate ports to the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid                - trunk group id
 *      pTrk_egr_ports         - trunk egress candidate ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of trk_gid is 0~29 in 9300 stand-alone mode, 0~63 in 9310 stacking mode.
 *           The valid range of trk_gid is 0~25 in 9310 stand-alone mode, 0~127 in 9310 stacking mode.
 *      (2) The traffic sent to trunk group would be hashed and an outgoing port would be picked from egress candidate ports.
 *      (3) Separated traffic or traffic ignoring trunk hash would not follow egress candidate ports to forward.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_egrPort_set(uint32 unit, rtk_trk_t trk_gid, rtk_trk_egrPort_t *pTrk_egr_ports);

/* Function Name:
 *      rtk_trunk_tunnelHashSrc_get
 * Description:
 *      Get the trunk hash option for tunnel packets to decide hashing by outer or inner header from the specified device.
 * Input:
 *      unit                   - unit id
 * Output:
 *      pTunnelHashSrc      - pointer buffer of option that trunk hash based on outer or inner layer for tunnel packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9310
 * Note:
 *      (1) The option would not be applied on tunnel start and tunnel terminate packets (both of them would hash by inner header).
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_tunnelHashSrc_get(uint32 unit, rtk_trunk_tunnelHashSrc_t *pTunnelHashSrc);

/* Function Name:
 *      rtk_trunk_tunnelHashSrc_set
 * Description:
 *      Get the trunk hash option for tunnel packets to decide hashing by outer or inner header from the specified device.
 * Input:
 *      unit                   - unit id
 *      trk_gid               - trunk group id
 *      tunnelHashSrc         - option that trunk hash based on outer or inner layer for tunnel packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9310
 * Note:
 *      (1) The option would not be applied on tunnel start and tunnel terminate packets (both of them would hash by inner header).
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_tunnelHashSrc_set(uint32 unit, rtk_trunk_tunnelHashSrc_t tunnelHashSrc);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmBind_get
 * Description:
 *      Get the distribution algorithm ID binded for a trunk group from the specified device.
 * Input:
 *      unit      - unit id
 *      trk_gid   - trunk group id
 * Output:
 *      pAlgo_idx - pointer buffer of the distribution algorithm ID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380, 8390
 * Note:
 *      9300, 9310 use API rtk_trunk_distributionAlgorithmTypeBind_get to implement the similar featrue.
 *      (1) The valid range of trk_gid is 0~7 in 8380, 0~15 in 8390.
 *      (2) The valid range of algo_idx is 0~1 in 8380, 0~3 in 8390.
 * Changes:
 *      None
 */
extern int32
rtk_trunk_distributionAlgorithmBind_get(uint32 unit, rtk_trk_t trk_gid, uint32 *pAlgo_idx);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmBind_set
 * Description:
 *      Set the distribution algorithm ID binded for a trunk group from the specified device.
 * Input:
 *      unit     - unit id
 *      trk_gid  - trunk group id
 *      algo_idx - index the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_LA_ALGO_ID  - invalid trunk algorithm ID
 * Applicable:
 *      8380, 8390
 * Note:
 *      9300, 9310 use API rtk_trunk_distributionAlgorithmTypeBind_set to implement the similar featrue.
 *      (1) The valid range of trk_gid is 0~7 in 8380, 0~15 in 8390.
 *      (2) The valid range of algo_idx is 0~1 in 8380, 0~3 in 8390.
 * Changes:
 *      None
 */
extern int32
rtk_trunk_distributionAlgorithmBind_set(uint32 unit, rtk_trk_t trk_gid, uint32 algo_idx);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmTypeBind_get
 * Description:
 *      Get the distribution algorithm ID binded for packet types for a trunk group from the specified device.
 * Input:
 *      unit      - unit id
 *      trk_gid   - trunk group id
 *      type - bind packet type
 * Output:
 *      pAlgo_idx - pointer buffer of the distribution algorithm ID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of trk_gid is 0~63 in 9300, 0~127 in 9310.
 *      (2) The valid range of algo_idx is 0~1 in 9300, 0~1 in 9310.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_distributionAlgorithmTypeBind_get(uint32 unit, rtk_trk_t trk_gid, rtk_trunk_bindType_t type, uint32 *pAlgo_idx);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmTypeBind_set
 * Description:
 *      Set the distribution algorithm ID binded for packet types for a trunk group from the specified device.
 * Input:
 *      unit     - unit id
 *      trk_gid  - trunk group id
 *      type - bind packet type
 *      algo_idx - index the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_LA_ALGO_ID  - invalid trunk algorithm ID
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of trk_gid is 0~63 in 9300, 0~127 in 9310.
 *      (2) The valid range of algo_idx is 0~1 in 9300, 0~1 in 9310.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_distributionAlgorithmTypeBind_set(uint32 unit, rtk_trk_t trk_gid, rtk_trunk_bindType_t type, uint32 algo_idx);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmParam_get
 * Description:
 *      Get the distribution algorithm by algorithm ID from the specified device.
 * Input:
 *      unit          - unit id
 *      algo_idx      - algorithm index
 * Output:
 *      pAlgo_bitmask - pointer buffer of bitmask of the distribution algorithm
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_ALGO_ID   - invalid trunk algorithm ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380, 8390
 * Note:
 *      9300, 9310 use API rtk_trunk_distributionAlgorithmTypeParam_get to implement the similar featrue.
 *      (1) The valid range of algo_idx is 0~1 in 8380, 0~3 in 8390.
 * Changes:
 *      None
 */
extern int32
rtk_trunk_distributionAlgorithmParam_get(uint32 unit, uint32 algo_idx, uint32 *pAlgo_bitmask);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmParam_set
 * Description:
 *      Set the distribution algorithm by algorithm ID from the specified device.
 * Input:
 *      unit         - unit id
 *      algo_idx     - algorithm index
 *      algo_bitmask - bitmask of the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_ALGO_ID  - invalid trunk algorithm ID
 *      RT_ERR_LA_HASHMASK - invalid hash mask
 * Applicable:
 *      8380, 8390
 * Note:
 *      9300, 9310 use API rtk_trunk_distributionAlgorithmTypeParam_set to implement the similar featrue.
 *      (1) The valid range of algo_idx is 0~1 in 8380, 0~3 in 8390.
 *      (2) Following factors can be used in any combination to customize the distribution algorithm.
 *          - TRUNK_DISTRIBUTION_ALGO_SPA_BIT        (source port)
 *          - TRUNK_DISTRIBUTION_ALGO_SMAC_BIT       (source mac)
 *          - TRUNK_DISTRIBUTION_ALGO_DMAC_BIT       (destination mac)
 *          - TRUNK_DISTRIBUTION_ALGO_SIP_BIT        (source ip)
 *          - TRUNK_DISTRIBUTION_ALGO_DIP_BIT        (destination ip)
 *          - TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT (source layer4 port)
 *          - TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT (destination layer4 port)
 * Changes:
 *      None
 */
extern int32
rtk_trunk_distributionAlgorithmParam_set(uint32 unit, uint32 algo_idx, uint32 algo_bitmask);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmTypeParam_get
 * Description:
 *      Get the distribution algorithm by packet type and algorithm ID from the specified device.
 * Input:
 *      unit          - unit id
 *      type         - packet type
 *      algo_idx    - algorithm index
 * Output:
 *      pAlgo_bitmask - pointer buffer of bitmask of the distribution algorithm
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_ALGO_ID   - invalid trunk algorithm ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of algo_idx is 0~1 in 9300, 0~1 in 9310.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_distributionAlgorithmTypeParam_get(uint32 unit, rtk_trunk_hashParamType_t type, uint32 algo_idx, uint32 *pAlgo_bitmask);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmTypeParam_set
 * Description:
 *      Set the distribution algorithm by algorithm ID from the specified device.
 * Input:
 *      unit         - unit id
 *      algo_idx     - algorithm index
 *      algo_bitmask - bitmask of the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_ALGO_ID  - invalid trunk algorithm ID
 *      RT_ERR_LA_HASHMASK - invalid hash mask
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of algo_idx is 0~1 in 9300 and 9310.
 *      (2) Following factors can be used in any combination to customize the L2 distribution algorithm.
 *          - TRUNK_DISTRIBUTION_ALGO_L2_SPA_BIT        (source port)
 *          - TRUNK_DISTRIBUTION_ALGO_L2_SMAC_BIT       (source mac)
 *          - TRUNK_DISTRIBUTION_ALGO_L2_DMAC_BIT       (destination mac)
 *          - TRUNK_DISTRIBUTION_ALGO_L2_VLAN_BIT        (vlan id)
 *      (3) Following factors can be used in any combination to customize the L3 distribution algorithm.
 *          - TRUNK_DISTRIBUTION_ALGO_L3_SPA_BIT        (source port)
 *          - TRUNK_DISTRIBUTION_ALGO_L3_SMAC_BIT       (source mac)
 *          - TRUNK_DISTRIBUTION_ALGO_L3_DMAC_BIT       (destination mac)
 *          - TRUNK_DISTRIBUTION_ALGO_L3_VLAN_BIT        (vlan id)
 *          - TRUNK_DISTRIBUTION_ALGO_L3_SIP_BIT        (source ip)
 *          - TRUNK_DISTRIBUTION_ALGO_L3_DIP_BIT        (destination ip)
 *          - TRUNK_DISTRIBUTION_ALGO_L3_SRC_L4PORT_BIT (source layer4 port)
 *          - TRUNK_DISTRIBUTION_ALGO_L3_DST_L4PORT_BIT (destination layer4 port)
 *          - TRUNK_DISTRIBUTION_ALGO_L3_PROTO_BIT (protocol)
 *          - TRUNK_DISTRIBUTION_ALGO_L3_FLOW_LABEL_BIT (ipv6 flow label)
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_distributionAlgorithmTypeParam_set(uint32 unit, rtk_trunk_hashParamType_t type, uint32 algo_idx, uint32 algo_bitmask);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmShift_get
 * Description:
 *      Get the shift bits of distribution algorithm parameters from the specified device.
 * Input:
 *      unit     - unit id
 *      algo_idx - algorithm index
 * Output:
 *      pShift   - pointer buffer of shift bits of the distribution algorithm
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_ALGO_ID   - invalid trunk algorithm ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380, 8390
 * Note:
 *      9300, 9310 use API rtk_trunk_distributionAlgorithmShiftGbl_get to implement the similar featrue.
 *      Instead use the fixed hash algorithm provided by the device, the API can shift each hash algorithm
 *      factor to have different distribution path.
 * Changes:
 *      None
 */
extern int32
rtk_trunk_distributionAlgorithmShift_get(uint32 unit, uint32 algo_idx, rtk_trunk_distAlgoShift_t *pShift);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmShift_set
 * Description:
 *      Set the shift bits of distribution algorithm parameters from the specified device.
 * Input:
 *      unit     - unit id
 *      algo_idx - algorithm index
 *      pShift   - shift bits of the distribution algorithm parameters
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_LA_ALGO_ID    - invalid trunk algorithm ID
 *      RT_ERR_LA_ALGO_SHIFT - invalid trunk algorithm shift
 * Applicable:
 *      8380, 8390
 * Note:
 *      9300, 9310 use API rtk_trunk_distributionAlgorithmShiftGbl_set to implement the similar featrue.
 *      Instead use the fixed hash algorithm provided by the device, the API can shift each hash algorithm
 *      factor to have different distribution path.
 * Changes:
 *      None
 */
extern int32
rtk_trunk_distributionAlgorithmShift_set(uint32 unit, uint32 algo_idx, rtk_trunk_distAlgoShift_t *pShift);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmShiftGbl_get
 * Description:
 *      Get the global shift bits of distribution algorithm parameters from the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pShift   - pointer buffer of shift bits of the distribution algorithm
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) Instead use the fixed hash algorithm provided by the device, the API can shift each hash algorithm
 *      factor to have different distribution path.
 *      (2) Valid shift range is from 0 to 5 bits.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_distributionAlgorithmShiftGbl_get(uint32 unit, rtk_trunk_distAlgoShift_t *pShift);

/* Function Name:
 *      rtk_trunk_distributionAlgorithmShiftGbl_set
 * Description:
 *      Set the global shift bits of distribution algorithm parameters from the specified device.
 * Input:
 *      unit     - unit id
 *      pShift   - shift bits of the distribution algorithm parameters
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID       - invalid unit id
 *      RT_ERR_NOT_INIT      - The module is not initial
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 *      RT_ERR_LA_ALGO_SHIFT - invalid trunk algorithm shift
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) Instead use the fixed hash algorithm provided by the device, the API can shift each hash algorithm
 *      factor to have different distribution path.
 *      (2) Valid shift range is from 0 to 5 bits.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_distributionAlgorithmShiftGbl_set(uint32 unit, rtk_trunk_distAlgoShift_t *pShift);


/* Function Name:
 *      rtk_trunk_trafficSeparate_get
 * Description:
 *      Get the traffic separation setting of a trunk group from the specified device.
 * Input:
 *      unit          - unit id
 *      trk_gid       - trunk group id
 * Output:
 *      pSeparateType - pointer buffer of separated traffic type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8380, 8390
 * Note:
 *      9300, 9310 use API rtk_trunk_trafficSeparateEnable_get to implement the similar featrue.
 *      SEPARATE_NONE: disable traffic separation
 *      SEPARATE_KNOWN_MULTI: trunk MSB link up port is dedicated to TX known multicast traffic
 *      SEPARATE_FLOOD: trunk MSB link up port is dedicated to TX flooding (L2 lookup miss) traffic
 *      SEPARATE_KNOWN_MULTI_AND_FLOOD: trunk MSB link up port is dedicated to TX known multicast and flooding (L2 lookup miss) traffic
 * Changes:
 *      None
 */
extern int32
rtk_trunk_trafficSeparate_get(uint32 unit, rtk_trk_t trk_gid, rtk_trunk_separateType_t *pSeparateType);

/* Function Name:
 *      rtk_trunk_trafficSeparate_set
 * Description:
 *      Set the traffic separation setting of a trunk group from the specified device.
 * Input:
 *      unit         - unit id
 *      trk_gid      - trunk group id
 *      separateType - traffic separation setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_INPUT       - Invalid input parameter
 * Applicable:
 *      8380, 8390
 * Note:
 *      9300, 9310 use API rtk_trunk_trafficSeparateEnable_set to implement the similar featrue.
 *      SEPARATE_NONE: disable traffic separation
 *      SEPARATE_KNOWN_MULTI: trunk MSB link up port is dedicated to TX known multicast traffic
 *      SEPARATE_FLOOD: trunk MSB link up port is dedicated to TX flooding (L2 lookup miss) traffic
 *      SEPARATE_KNOWN_MULTI_AND_FLOOD: trunk MSB link up port is dedicated to TX known multicast and flooding (L2 lookup miss) traffic
 * Changes:
 *      None
 */
extern int32
rtk_trunk_trafficSeparate_set(uint32 unit, rtk_trk_t trk_gid, rtk_trunk_separateType_t separateType);

/* Function Name:
 *      rtk_trunk_trafficSeparateEnable_get
 * Description:
 *      Get the enable status of traffic separation type of a trunk group from the specified device.
 * Input:
 *      unit          - unit id
 *      trk_gid       - trunk group id
 *      separateType - separated traffic type
 * Output:
 *      pEnable - pointer buffer of enable state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The configurable separation type includes SEPARATE_KNOWN_MULTI and SEPARATE_FLOOD.
 *      (2) The separated port is the link up egress member port with largest index.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_trafficSeparateEnable_get(uint32 unit, rtk_trk_t trk_gid, rtk_trunk_separateType_t separateType, rtk_enable_t *pEnable);


/* Function Name:
 *      rtk_trunk_trafficSeparateEnable_set
 * Description:
 *      Set the enable status of traffic separation type of a trunk group from the specified device.
 * Input:
 *      unit          - unit id
 *      trk_gid       - trunk group id
 *      separateType - separated traffic type
 *      enable - enable state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The configurable separation type includes SEPARATE_KNOWN_MULTI and SEPARATE_FLOOD.
 *      (2) The separated port is the link up egress member port with largest index.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_trafficSeparateEnable_set(uint32 unit, rtk_trk_t trk_gid, rtk_trunk_separateType_t separateType, rtk_enable_t enable);

/* Function Name:
 *      rtk_trunk_trafficSeparateDivision_get
 * Description:
 *      Get the division control of known multicast to other separated packets from the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pEnable    - pointer buffer of known multicast packets separation divide control enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) This function only works when SEPARATE_KNOWN_MULTI and SEPARATE_FLOOD are both enabled.
 *           In that case, if the division function is enabled, separated DLF or broadcast packet would choose trunk member ports with
 *           largest index to forward; separated known multicast packets would choose second largest index to forward.
 *      (2) This function only works when at least 3 trunk member ports link up.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_trafficSeparateDivision_get(uint32 unit, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_trunk_trafficSeparateDivision_set
 * Description:
 *      Set the division control of known multicast to other separated packets to the specified device.
 * Input:
 *      unit        - unit id
 *      enable    - known multicast packets separation divide control enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT          - invalid input parameter
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) This function only works when SEPARATE_KNOWN_MULTI and SEPARATE_FLOOD are both enabled.
 *           In that case, if the division function is enabled, separated DLF or broadcast packet would choose trunk member ports with
 *           largest index to forward; separated known multicast packets would choose second largest index to forward.
 *      (2) This function only works when at least 3 trunk member ports link up.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_trafficSeparateDivision_set(uint32 unit, rtk_enable_t enable);

/* Function Name:
 *      rtk_trunk_stkTrkPort_get
 * Description:
 *      Get the stacking portmask of designated trunk group from the specified device.
 * Input:
 *      unit          - unit id
 *      stk_trk_gid     - stacking trunk group id
 * Output:
 *      pStkPorts - pointer buffer of stacking portmask of trunk group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_STACK_TRUNK_ID  - invalid stacking trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of stk_trk_gid is from 0 ~ 1.
 *      (2) The stacking portmask indicates if stacking ports from port ID 0 to 55
 *           belong to the trunk group for 9310
 *           OR indicates if stacking ports from port ID 24 to 27
 *           belong to the trunk group for 9300.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_stkTrkPort_get(uint32 unit, rtk_stk_trk_t stk_trk_gid, rtk_portmask_t *pStkPorts);

/* Function Name:
 *      rtk_trunk_stkTrkPort_set
 * Description:
 *      Get the stacking portmask of designated trunk group from the specified device.
 * Input:
 *      unit          - unit id
 *      stk_trk_gid     - stacking trunk group id
 *      pStkPorts  - pointer to  stacking portmask of trunk group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_STACK_TRUNK_ID  - invalid stacking trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range of stk_trk_gid is from 0 ~ 1.
 *      (2) The stacking portmask indicates if stacking ports from port ID 0 to 55
 *           belong to the trunk group for 9310
 *           OR indicates if stacking ports from port ID 24 to 27
 *           belong to the trunk group for 9300.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_stkTrkPort_set(uint32 unit, rtk_stk_trk_t stk_trk_gid, rtk_portmask_t *pStkPorts);

/* Function Name:
 *      rtk_trunk_stkTrkHash_get
 * Description:
 *      Get the option that if stacking trunk would follow the hash result in stacking header or re-calculate hash by its own distribution algorithm ID
 * Input:
 *      unit      - unit id
 * Output:
 *      pStkTrkHash - pointer buffer of the option that if stacking trunk would follow the hash result in stacking header or re-calculate hash by its own distribution algorithm ID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) If STACK_TRK_HASH_KEEP is selected, the hash result in stacking header would be applied to select stacking trunk outgoing port.
 *      (2) If STACK_TRK_HASH_RECALCULATE is selected, use rtk_trunk_stkDistributionAlgorithmBind_set() to set the distribution algorithm ID.
 *           The recalculated hash result only decides the stacking trunk outgoing port and would not alter the hash result in stacking header.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_stkTrkHash_get(uint32 unit, rtk_trunk_stkTrkHash_t *pStkTrkHash);


/* Function Name:
 *      rtk_trunk_stkTrkHash_set
 * Description:
 *      Set the option that if stacking trunk would follow the hash result in stacking header or re-calculate hash by its own hash mask index
 * Input:
 *      unit     - unit id
 *      stkTrkHash - the option that if stacking trunk would follow the hash result in stacking header or re-calculate hash by its own hash mask index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_INPUT          - invalid input parameter
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) If STACK_TRK_HASH_KEEP is selected, the hash result in stacking header would be applied to select stacking trunk outgoing port.
 *      (2) If STACK_TRK_HASH_RECALCULATE is selected, use rtk_trunk_stkDistributionAlgorithmBind_set() to set the distribution algorithm ID.
 *           The recalculated hash result only decides the stacking trunk outgoing port and would not alter the hash result in stacking header.
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_stkTrkHash_set(uint32 unit, rtk_trunk_stkTrkHash_t stkTrkHash);


/* Function Name:
 *      rtk_trunk_stkDistributionAlgorithmTypeBind_get
 * Description:
 *      Get the distribution algorithm ID binding for a packe type of a stacking trunk group from the specified device.
 * Input:
 *      unit      - unit id
 *      stk_trk_gid     - trunk group id of stacking port
 *      type     - bind packet type
 * Output:
 *      pAlgo_idx - pointer buffer of the distribution algorithm ID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_LA_STACK_TRUNK_ID  - invalid stacking port trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range fo stk_trk_gid is from 0 ~ 1.
 *      (2) The valid range of algo_idx is 0~1
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_stkDistributionAlgorithmTypeBind_get(uint32 unit, rtk_stk_trk_t stk_trk_gid, rtk_trunk_bindType_t type, uint32 *pAlgo_idx);

/* Function Name:
 *      rtk_trunk_stkDistributionAlgorithmTypeBind_set
 * Description:
 *      Set the distribution algorithm ID binding for a packet type of a stacking trunk group to the specified device.
 * Input:
 *      unit     - unit id
 *      stk_trk_gid   - trunk group id of stacking port
 *      type - bind packet type
 *      algo_idx - index the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_LA_STACK_TRUNK_ID  - invalid stacking port trunk ID
 *      RT_ERR_LA_ALGO_ID  - invalid trunk algorithm ID
 * Applicable:
 *      9300, 9310
 * Note:
 *      (1) The valid range fo stk_trk_gid is from 0 ~ 1.
 *      (2) The valid range of algo_idx is 0~1
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_stkDistributionAlgorithmTypeBind_set(uint32 unit, rtk_stk_trk_t stk_trk_gid, rtk_trunk_bindType_t type, uint32 algo_idx);

/* Function Name:
 *      rtk_trunk_localFirst_get
 * Description:
 *      Get the local-first load-balacing enable status from the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pLocalFirst - pointer to local-first load-balancing enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      None
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_localFirst_get(uint32 unit, rtk_enable_t *pLocalFirst);

/* Function Name:
 *      rtk_trunk_localFirst_set
 * Description:
 *      Set the local-first load-balacing enable status to the specified device.
 * Input:
 *      unit        - unit id
 *      localFirst - local-first load-balancing enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_NOT_INIT    - The module is not initial
 *      RT_ERR_INPUT       - Invalid input parameter
 * Applicable:
 *      9300, 9310
 * Note:
 *      Local first load balancing only works in stacking mode
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_localFirst_set(uint32 unit, rtk_enable_t localFirst);

/* Function Name:
 *      rtk_trunk_localFirstFailOver_get
 * Description:
 *      Get the local-first load balacing congest and link-fail avoidance enable status from the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pCongstAvoid - pointer to congest avoidance enable status
 *      pLinkFailAvoid - pointer to link fail avoidance enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      the failover funtion only works when local-first load balancing is enabled
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_localFirstFailOver_get(uint32 unit, rtk_enable_t *pCongstAvoid, rtk_enable_t *pLinkFailAvoid);

/* Function Name:
 *      rtk_trunk_localFirstFailOver_set
 * Description:
 *      Set the local-first load balacing congest and link-fail avoidance enable status to the specified device.
 * Input:
 *      unit          - unit id
 *      congstAvoid - congest avoidance enable status
 *      linkFailAvoid - link fail avoidance enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *      the failover funtion only works when local-first load balancing is enabled
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 */
extern int32
rtk_trunk_localFirstFailOver_set(uint32 unit, rtk_enable_t congstAvoid, rtk_enable_t linkFailAvoid);

/* Function Name:
 *      rtk_trunk_srcPortMap_get
 * Description:
 *      Get the info about whether DEV+Port belongs to some trunk and if yes, get its trunk ID.
 * Input:
 *      unit                   - unit id
 *      devPort             - device port
 * Output:
 *      pIsTrkMbr - pointer to get trunk or not
 *      pTrk_gid   - pointer to get trunk id.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 *      [3.2.0]
 *          Change paremeter 'rtk_unit_port_t unitPort' to 'rtk_dev_port_t devPort'.
 */
extern int32
rtk_trunk_srcPortMap_get(uint32 unit,  rtk_dev_port_t devPort, uint32 *pIsTrkMbr, rtk_trk_t *pTrk_gid);

/* Function Name:
 *      rtk_trunk_srcPortMap_set
 * Description:
 *      Set the info about whether DEV+Port belongs to some trunk and if yes, set its trunk ID.
 * Input:
 *      unit                   - unit id
 *      devPort             - device port
 *      isTrkMbr            - trunk or not
 *      trk_gid              -  trunk id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      9300, 9310
 * Note:
 *
 * Changes:
 *      [SDK_3.0.0]
 *          New added function.
 *      [3.2.0]
 *          Change paremeter 'rtk_unit_port_t unitPort' to 'rtk_dev_port_t devPort'.
 */
extern int32
rtk_trunk_srcPortMap_set(uint32 unit, rtk_dev_port_t devPort, uint32 isTrkMbr, rtk_trk_t trk_gid);


#endif /* __RTK_TRUNK_H__ */
