/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt_profile.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"
#include "l1profile.h"

#ifdef RTMP_UDMA_SUPPORT
#include <linux/udma_api.h>
#endif/*RTMP_UDMA_SUPPORT*/

#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
#include <linux/foe_hook.h>
#endif

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* RLM_CAL_CACHE_SUPPORT */


#ifdef CONFIG_FAST_NAT_SUPPORT
#include <net/ra_nat.h>
#endif /*CONFIG_FAST_NAT_SUPPORT*/

#define BSSID_WCID_TO_REMOVE 1

struct l1profile_info_t {
	RTMP_STRING profile_index[L1PROFILE_INDEX_LEN];
	RTMP_STRING profile_path[L2PROFILE_PATH_LEN];
	eeprom_flash_info ee_info;
	struct dev_type_name_map_t dev_name_map[MAX_INT_TYPES + 1];
	RTMP_STRING single_sku_path[L2PROFILE_PATH_LEN];
	RTMP_STRING bf_sku_path[L2PROFILE_PATH_LEN];
};

struct l1profile_attribute_t {
	RTMP_STRING name[L1PROFILE_ATTRNAME_LEN];
	UINT_32	extra;

	INT(*handler)(RTMP_ADAPTER *pAd, UINT_32 extra, RTMP_STRING *value);
};

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
UCHAR wf_apcli_active_links;
#if defined(CONFIG_FAST_NAT_SUPPORT)
int (*wf_ra_sw_nat_hook_tx_bkup)(struct sk_buff *skb, int gmac_no);
int (*wf_ra_sw_nat_hook_rx_bkup)(struct sk_buff *skb);
#endif /*CONFIG_FAST_NAT_SUPPORT*/
#endif
#if defined(RT_CFG80211_SUPPORT)
#define SECOND_INF_MAIN_DEV_NAME		"wlani"
#define SECOND_INF_MBSSID_DEV_NAME	"wlani"
#else
#define SECOND_INF_MAIN_DEV_NAME		"rai0"
#define SECOND_INF_MBSSID_DEV_NAME	"rai"
#endif
#define SECOND_INF_WDS_DEV_NAME		"wdsi"
#define SECOND_INF_APCLI_DEV_NAME	"apclii"
#define SECOND_INF_MESH_DEV_NAME		"meshi"
#define SECOND_INF_P2P_DEV_NAME		"p2pi"
#define SECOND_INF_MONITOR_DEV_NAME		"moni"
#define SECOND_INF_MSTA_DEV_NAME    "rai"

#if defined(RT_CFG80211_SUPPORT)
#define THIRD_INF_MAIN_DEV_NAME		"wlane"
#define THIRD_INF_MBSSID_DEV_NAME	"wlane"
#else
#define THIRD_INF_MAIN_DEV_NAME		"rae0"
#define THIRD_INF_MBSSID_DEV_NAME	"rae"
#endif
#define THIRD_INF_WDS_DEV_NAME		"wdse"
#define THIRD_INF_APCLI_DEV_NAME	"apclie"
#define THIRD_INF_MESH_DEV_NAME		"meshe"
#define THIRD_INF_P2P_DEV_NAME		"p2pe"
#define THIRD_INF_MONITOR_DEV_NAME		"mone"
#define THIRD_INF_MSTA_DEV_NAME    "rae"


#define xdef_to_str(s)   def_to_str(s)
#define def_to_str(s)    #s

#if defined(CONFIG_SUPPORT_OPENWRT) && defined(MT7663)
#define FIRST_EEPROM_FILE_PATH		"/etc/wireless/mt7663e/"
#define FIRST_AP_PROFILE_PATH		"/etc/wireless/mt7663e/mt7663e.1.dat"
#define FIRST_STA_PROFILE_PATH		"/etc/wireless/RT2860/RT2860.dat"

#define FIRST_CHIP_ID	xdef_to_str(CONFIG_RT_FIRST_CARD)

#define SECOND_EEPROM_FILE_PATH		"/etc/wireless/mt7663e/"
#define SECOND_AP_PROFILE_PATH		"/etc/wireless/mt7663e/mt7663e.2.dat"
#define SECOND_STA_PROFILE_PATH		"/etc/wireless/iNIC/iNIC_sta.dat"

#define SECOND_CHIP_ID	xdef_to_str(CONFIG_RT_SECOND_CARD)

#define THIRD_EEPROM_FILE_PATH		"/etc/wireless/mt7663e/"
#define THIRD_AP_PROFILE_PATH		"/etc/wireless/mt7663e/mt7663e.3.dat"
#define THIRD_STA_PROFILE_PATH		"/etc/wireless/WIFI3/RT2870AP.dat"

#define THIRD_CHIP_ID	xdef_to_str(CONFIG_RT_THIRD_CARD)

#else
#define FIRST_EEPROM_FILE_PATH	"/etc_ro/Wireless/RT2860/"
#define FIRST_AP_PROFILE_PATH		"/etc/Wireless/RT2860/RT2860.dat"
#define FIRST_STA_PROFILE_PATH      "/etc/Wireless/RT2860/RT2860.dat"
#define FIRST_CHIP_ID	xdef_to_str(CONFIG_RT_FIRST_CARD)

#define SECOND_EEPROM_FILE_PATH	"/etc_ro/Wireless/iNIC/"
#define SECOND_AP_PROFILE_PATH	"/etc/Wireless/iNIC/iNIC_ap.dat"
#define SECOND_STA_PROFILE_PATH "/etc/Wireless/iNIC/iNIC_sta.dat"

#define SECOND_CHIP_ID	xdef_to_str(CONFIG_RT_SECOND_CARD)

#define THIRD_EEPROM_FILE_PATH	"/etc_ro/Wireless/WIFI3/"
#define THIRD_AP_PROFILE_PATH	"/etc/Wireless/WIFI3/RT2870AP.dat"
#define THIRD_STA_PROFILE_PATH "/etc/Wireless/WIFI3/RT2870AP.dat"

#define THIRD_CHIP_ID	xdef_to_str(CONFIG_RT_THIRD_CARD)
#endif

#ifndef CONFIG_RT_FIRST_IF_RF_OFFSET
#define CONFIG_RT_FIRST_IF_RF_OFFSET DEFAULT_RF_OFFSET
#endif

#ifndef CONFIG_RT_SECOND_IF_RF_OFFSET
#define CONFIG_RT_SECOND_IF_RF_OFFSET DEFAULT_RF_OFFSET
#endif

#ifndef CONFIG_RT_THIRD_IF_RF_OFFSET
#define CONFIG_RT_THIRD_IF_RF_OFFSET DEFAULT_RF_OFFSET
#endif

#define MAX_L1PROFILE_INDEX	10

static struct l1profile_info_t l1profile[MAX_NUM_OF_INF] = {
	{	{0}, FIRST_AP_PROFILE_PATH, {CONFIG_RT_FIRST_IF_RF_OFFSET, EEPROM_SIZE},
		{	{INT_MAIN, INF_MAIN_DEV_NAME},
			{INT_MBSSID, INF_MBSSID_DEV_NAME},
			{INT_WDS, INF_WDS_DEV_NAME},
			{INT_APCLI,	INF_APCLI_DEV_NAME},
			{INT_MESH, INF_MESH_DEV_NAME},
			{INT_P2P, INF_P2P_DEV_NAME},
			{INT_MONITOR, INF_MONITOR_DEV_NAME},
			{INT_MSTA, INF_MSTA_DEV_NAME},
			{0}
		},
		{SINGLE_SKU_TABLE_FILE_NAME},
		{BF_SKU_TABLE_FILE_NAME}
	},
#if defined(CONFIG_RT_SECOND_CARD)
	{	{0}, SECOND_AP_PROFILE_PATH, {CONFIG_RT_SECOND_IF_RF_OFFSET, EEPROM_SIZE},
		{	{INT_MAIN,	SECOND_INF_MAIN_DEV_NAME},
			{INT_MBSSID,	SECOND_INF_MBSSID_DEV_NAME},
			{INT_WDS, SECOND_INF_WDS_DEV_NAME},
			{INT_APCLI, SECOND_INF_APCLI_DEV_NAME},
			{INT_MESH, SECOND_INF_MESH_DEV_NAME},
			{INT_P2P, SECOND_INF_P2P_DEV_NAME},
			{INT_MONITOR, SECOND_INF_MONITOR_DEV_NAME},
			{INT_MSTA, SECOND_INF_MSTA_DEV_NAME},
			{0}
		},
		{SINGLE_SKU_TABLE_FILE_NAME},
		{BF_SKU_TABLE_FILE_NAME}
	},
#endif	/* CONFIG_RT_SECOND_CARD */
#if defined(CONFIG_RT_THIRD_CARD)
	{	{0}, THIRD_AP_PROFILE_PATH, {CONFIG_RT_THIRD_IF_RF_OFFSET, EEPROM_SIZE},
		{	{INT_MAIN, THIRD_INF_MAIN_DEV_NAME},
			{INT_MBSSID, THIRD_INF_MBSSID_DEV_NAME},
			{INT_WDS, THIRD_INF_WDS_DEV_NAME},
			{INT_APCLI, THIRD_INF_APCLI_DEV_NAME},
			{INT_MESH, THIRD_INF_MESH_DEV_NAME},
			{INT_P2P, THIRD_INF_P2P_DEV_NAME},
			{INT_MONITOR, THIRD_INF_MONITOR_DEV_NAME},
			{INT_MSTA, THIRD_INF_MSTA_DEV_NAME},
			{0}
		},
		{SINGLE_SKU_TABLE_FILE_NAME},
		{BF_SKU_TABLE_FILE_NAME}
	},
#endif	/* CONFIG_RT_THIRD_CARD */
};

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

struct wifi_fwd_func_table wf_drv_tbl;

#endif


static NDIS_STATUS l1set_profile_path(RTMP_ADAPTER *pAd, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = l1profile[get_dev_config_idx(pAd)].profile_path;
	RTMP_STRING *pSemicolon = strchr(value, ';');
	UINT8 str_len;

	if (pSemicolon) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DBDC format of profile path!\n"));
		*pSemicolon = '\0';
#ifdef MULTI_PROFILE
		update_mtb_value(pAd, MTB_2G_PROFILE, extra, value);
		update_mtb_value(pAd, MTB_5G_PROFILE, extra, pSemicolon + 1);
#endif  /* MULTI_PROFILE */
	}

	str_len = strlen(value);
	if (strcmp(target, value) && (str_len < L2PROFILE_PATH_LEN)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("profile update from %s to %s\n", target, value));
		strcpy(target, value);
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("profile remain %s\n", target));

	return retVal;
}


static NDIS_STATUS l1set_eeprom_bin(RTMP_ADAPTER *pAd, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = l1profile[get_dev_config_idx(pAd)].ee_info.bin_name;
	UINT8 str_len;

	str_len = strlen(value);
	if (strcmp(target, value) && (str_len < L1PROFILE_ATTRNAME_LEN)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("eeprom binary update from %s to %s\n", target, value));
		strcpy(target, value);
		*(target+str_len) = '\0';
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("eeprom binary remain %s\n", target));

	return retVal;
}


static NDIS_STATUS l1set_eeprom_offset(RTMP_ADAPTER *pAd, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	eeprom_flash_info *target = (eeprom_flash_info *)(&(l1profile[get_dev_config_idx(pAd)].ee_info));
	UINT	int_value = 0;

	int_value = os_str_tol(value, 0, 0);

	if (target->offset != int_value) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("eeprom offset update from 0x%x to 0x%x\n", target->offset, int_value));
		target->offset = int_value;
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("eeprom offset remain 0x%x\n", target->offset));

	return retVal;
}


static NDIS_STATUS l1set_eeprom_size(RTMP_ADAPTER *pAd, UINT_32 extra, RTMP_STRING *value)
{
	UINT int_value;
	INT retVal = NDIS_STATUS_SUCCESS;
	eeprom_flash_info *target = (eeprom_flash_info *)(&(l1profile[get_dev_config_idx(pAd)].ee_info));

	int_value = os_str_tol(value, 0, 0);

	if (target->size != int_value) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("eeprom size update from 0x%x to 0x%x\n", target->size, int_value));
		target->size = int_value;
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("eeprom size remain 0x%x\n", target->size));

	return retVal;
}


static NDIS_STATUS l1set_ifname(RTMP_ADAPTER *pAd, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = NULL;
	RTMP_STRING *pSemicolon = NULL;

	target = get_dev_name_prefix(pAd, extra);
	pSemicolon = strchr(value, ';');

	if (pSemicolon) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DBDC format of ifname!\n"));
		*pSemicolon = '\0';
#ifdef MULTI_PROFILE
		update_mtb_value(pAd, MTB_DEV_PREFIX, extra, pSemicolon + 1);
#endif	/* MULTI_PROFILE */
	}

	if (strcmp(target, value)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ifname update from %s to %s\n", target, value));
		strcpy(target, value);
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ifname remain %s\n", target));

	return retVal;
}

static NDIS_STATUS l1set_single_sku_path(RTMP_ADAPTER *pAd, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = l1profile[get_dev_config_idx(pAd)].single_sku_path;

	if (strcmp(target, value)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("sku path update from %s to %s\n", target, value));
		strncpy(target, value, L2PROFILE_PATH_LEN - 1);
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("profile remain %s\n", target));

	return retVal;
}

static NDIS_STATUS l1set_bf_sku_path(RTMP_ADAPTER *pAd, UINT_32 extra, RTMP_STRING *value)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	RTMP_STRING *target = l1profile[get_dev_config_idx(pAd)].bf_sku_path;

	if (strcmp(target, value)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("BF sku path update from %s to %s\n", target, value));
		strncpy(target, value, L2PROFILE_PATH_LEN - 1);
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("profile remain %s\n", target));

	return retVal;
}

static NDIS_STATUS is_dup_key(RTMP_STRING *key)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	INT dev_idx = 0;
	RTMP_STRING *profile_index = NULL;

	for (dev_idx = 0; dev_idx < MAX_NUM_OF_INF; dev_idx++) {
		profile_index = l1profile[dev_idx].profile_index;

		if ((strlen(profile_index) > 0) && (strcmp(profile_index, key) == 0)) {
			retVal = NDIS_STATUS_FAILURE;
			dev_idx = MAX_NUM_OF_INF;	/* tend to leave loop */
		}
	}

	return retVal;
}

static NDIS_STATUS match_index_by_chipname(IN RTMP_STRING *l1profile_data,
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *chipName)
{
	INT retVal = NDIS_STATUS_FAILURE;
	INT if_idx = 0;
	RTMP_STRING	key[10] = {0};
	RTMP_STRING *tmpbuf = NULL;

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

	while (if_idx < MAX_L1PROFILE_INDEX) {
		sprintf(key, "INDEX%d", if_idx);

		if (RTMPGetKeyParameter(key, tmpbuf, MAX_PARAM_BUFFER_SIZE, l1profile_data, TRUE)) {
			if (strncmp(tmpbuf, chipName, strlen(chipName)) == 0) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 ("%s found as %s\n", chipName, key));

				if (is_dup_key(key)) {	/* There might be not only single entry for one chip */
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("%s for %s occupied, next\n", key, chipName));
				} else {
					strcpy(l1profile[get_dev_config_idx(pAd)].profile_index, key);
					retVal = NDIS_STATUS_SUCCESS;
					if_idx = MAX_L1PROFILE_INDEX;	/* found, intend to leave */
				}
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("%s mismatch with %s as %s\n", chipName, tmpbuf, key));
			}

			if_idx++;
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s not found, dismissed.\n", key));
			if_idx = MAX_L1PROFILE_INDEX;	/* hit maximum avalable index, intend to leave */
		}
	}

	os_free_mem(tmpbuf);
	return retVal;
}


static NDIS_STATUS l1get_profile_index(IN RTMP_STRING *l1profile_data, IN RTMP_ADAPTER *pAd)
{
	INT retVal = NDIS_STATUS_SUCCESS;
	INT dev_idx = get_dev_config_idx(pAd);
	RTMP_STRING chipName[10] = {0};
	RTMP_STRING *tmpbuf = NULL;

	sprintf(chipName, "MT%x", pAd->ChipID);
	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (IS_MT7615(pAd) && (pAd->RfIcType == RFIC_7615A))
		strcat(chipName, "A");

	if (match_index_by_chipname(l1profile_data, pAd, chipName) == NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%d]%s found by chip\n", dev_idx, chipName));
	} else {
		retVal = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("[%d]%s not found, keep default\n", dev_idx, chipName));
	}

	os_free_mem(tmpbuf);
	return retVal;
}


static struct l1profile_attribute_t l1profile_attributes[] = {
	{ {"profile_path"},		0,				l1set_profile_path},
	{ {"EEPROM_name"},		0,				l1set_eeprom_bin},
	{ {"EEPROM_offset"},	0,				l1set_eeprom_offset},
	{ {"EEPROM_size"},		0,				l1set_eeprom_size},
	{ {"main_ifname"},		INT_MAIN,		l1set_ifname},
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	{ {"ext_ifname"},		INT_MBSSID,		l1set_ifname},
#endif	/* MBSS_SUPPORT */
#ifdef WDS_SUPPORT
	{ {"wds_ifname"},		INT_WDS,		l1set_ifname},
#endif	/* WDS_SUPPORT */
#endif	/* CONFIG_AP_SUPPORT */
	{ {"apcli_ifname"},		INT_APCLI,		l1set_ifname},
#ifdef CONFIG_STA_SUPPORT
	{ {"msta_ifname"},		INT_MSTA,		l1set_ifname},
#endif	/* CONFIG_STA_SUPPORT */
	{ {"single_sku_path"},		0,			l1set_single_sku_path},
	{ {"bf_sku_path"},		0,			l1set_bf_sku_path},
};

#ifdef MULTI_PROFILE
INT multi_profile_check(struct _RTMP_ADAPTER *ad, CHAR *final);
#endif /*MULTI_PROFILE*/

struct dev_id_name_map {
	INT chip_id;
	RTMP_STRING *chip_name;
};

static const struct dev_id_name_map id_name_list[] = {
	{7610, "7610, 7610e 7610u"},

};

UCHAR *get_single_sku_path(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;

	src = l1profile[get_dev_config_idx(pAd)].single_sku_path;
	return src;
}

UCHAR *get_bf_sku_path(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;

	src = l1profile[get_dev_config_idx(pAd)].bf_sku_path;
	return src;
}

INT get_dev_config_idx(RTMP_ADAPTER *pAd)
{
	INT idx = -1;
#if defined(CONFIG_RT_FIRST_CARD) && defined(CONFIG_RT_SECOND_CARD)
	INT first_card = 0, second_card = 0;

	A2Hex(first_card, FIRST_CHIP_ID);
	A2Hex(second_card, SECOND_CHIP_ID);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("chip_id1=0x%x, chip_id2=0x%x, pAd->MACVersion=0x%x\n", first_card, second_card, pAd->MACVersion));
#endif /* defined(CONFIG_RT_FIRST_CARD) && defined(CONFIG_RT_SECOND_CARD) */
#if defined(CONFIG_RT_SECOND_CARD)

	if (IS_MT7637E(pAd))
		idx = 1;

#endif

	if (idx == -1)
#ifdef MULTI_INF_SUPPORT
		idx = multi_inf_get_idx((VOID *) pAd);
	else
#endif /* MULTI_INF_SUPPORT */
		idx = 0;

#if defined(CONFIG_RT_SECOND_CARD)
#if defined(CONFIG_FIRST_IF_MT7603E) || defined(CONFIG_FIRST_IF_MT7628) || defined(CONFIG_FIRST_IF_MT7620)
	/* MT7603(ra0) + MT7613 (rai0) combination */
		if (IS_MT7663(pAd))
			idx = 1;
#endif /* defined(CONFIG_FIRST_IF_MT7603E) */
#endif /* defined(MT_SECOND_CARD) */

	pAd->dev_idx = idx;
	return idx;
}


RTMP_STRING *get_dev_eeprom_binary(VOID *pvAd)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pvAd;
	return l1profile[get_dev_config_idx(pAd)].ee_info.bin_name;
}


UINT32 get_dev_eeprom_offset(VOID *pvAd)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pvAd;
	return l1profile[get_dev_config_idx(pAd)].ee_info.offset;
}


UINT32 get_dev_eeprom_size(VOID *pvAd)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pvAd;
	return l1profile[get_dev_config_idx(pAd)].ee_info.size;
}


UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
{
	struct dev_type_name_map_t *map;
	INT type_idx = 0, dev_idx = get_dev_config_idx(pAd);

	if (dev_idx < 0 || dev_idx >= MAX_NUM_OF_INF) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): invalid dev_idx(%d)!\n",
				 __func__, dev_idx));
		return NULL;
	}

	do {
		map = &(l1profile[dev_idx].dev_name_map[type_idx]);

		if (map->type == dev_type) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): dev_idx = %d, dev_name_prefix=%s\n",
					 __func__, dev_idx, map->prefix));
			return map->prefix;
		}

		type_idx++;
	} while (l1profile[dev_idx].dev_name_map[type_idx].type != 0);

	return NULL;
}


UCHAR *get_dev_l2profile(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#if defined(CONFIG_RT_FIRST_CARD) || defined(CONFIG_RT_SECOND_CARD) || defined(CONFIG_RT_THIRD_CARD)

		if (get_dev_config_idx(pAd) < 3)
			src = l1profile[get_dev_config_idx(pAd)].profile_path;
		else
#endif /* CONFIG_RT_FIRST_CARD || CONFIG_RT_SECOND_CARD || CONFIG_RT_THIRD_CARD */
		{
			src = AP_PROFILE_PATH;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef CONFIG_RT_FIRST_CARD

		if (get_dev_config_idx(pAd) == 0)
			src = FIRST_STA_PROFILE_PATH;
		else
#endif /* CONFIG_RT_FIRST_CARD */
#ifdef CONFIG_RT_SECOND_CARD
			if (get_dev_config_idx(pAd) == 1)
				src = SECOND_STA_PROFILE_PATH;
			else
#endif /* CONFIG_RT_SECOND_CARD */
#ifdef CONFIG_RT_THIRD_CARD
				if (get_dev_config_idx(pAd) == 2)
					src = THIRD_STA_PROFILE_PATH;
				else
#endif /* CONFIG_RT_THIRD_CARD */
				{
#ifdef PROFILE_PATH_DYNAMIC
					src = pAd->profilePath;
#else
					src = STA_PROFILE_PATH;
#endif /* PROFILE_PATH_DYNAMIC */
				}
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef MULTIPLE_CARD_SUPPORT
	src = (RTMP_STRING *)pAd->MC_FileName;
#endif /* MULTIPLE_CARD_SUPPORT */
	return src;
}

int ShowL1profile(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===== L1 profile settings =====\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s = %s\n", l1profile_attributes[0].name, get_dev_l2profile(pAd)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s = 0x%x\n", l1profile_attributes[2].name, get_dev_eeprom_offset(pAd)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s = 0x%x\n", l1profile_attributes[3].name, get_dev_eeprom_size(pAd)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s = %s\n", l1profile_attributes[4].name, get_dev_name_prefix(pAd, INT_MAIN)));

	return TRUE;
}

NDIS_STATUS load_dev_l1profile(IN RTMP_ADAPTER *pAd)
{
	RTMP_STRING *buffer = NULL;
	RTMP_OS_FD_EXT srcf;
	INT retval = NDIS_STATUS_SUCCESS;
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);

	if (!buffer) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("No enough memory\n"));
		retval = NDIS_STATUS_FAILURE;
		return retval;
	}

	os_zero_mem(buffer, buf_size);
	srcf = os_file_open(L1_PROFILE_PATH, O_RDONLY, 0);

	if (srcf.Status) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Open file \"%s\" failed, try embedded default!\n", L1_PROFILE_PATH));

		retval = strlen(l1profile_default);
		strcpy(buffer, l1profile_default);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Open file \"%s\" Succeed!\n", L1_PROFILE_PATH));
#ifndef OS_ABL_SUPPORT

		/* TODO: need to roll back when convert into OSABL code */
		if (srcf.fsize != 0 && buf_size < (srcf.fsize + 1))
			buf_size = srcf.fsize  + 1;

#endif /* OS_ABL_SUPPORT */
		retval = os_file_read(srcf, buffer, buf_size - 1);

		if (retval)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("Read file \"%s\"(%d) succeed!\n", L1_PROFILE_PATH, retval));
	}

	if (retval) {
		RTMP_STRING *tmpbuf = NULL;
		RTMP_STRING key[30] = {'\0'};
		UINT32	attr_index = 0;
		INT dev_idx = get_dev_config_idx(pAd);
		RTMP_STRING *profile_index = l1profile[dev_idx].profile_index;
		struct l1profile_attribute_t *l1attr = NULL;

		/* Do not fetch INDEX%d each time loading l1profile */
		if ((strlen(profile_index) == 0) && (l1get_profile_index(buffer, pAd) != NDIS_STATUS_SUCCESS))
			goto err_out;

		os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (tmpbuf == NULL)
			goto err_out;

		if (RTMPGetKeyParameter(profile_index, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Chip=%s\n", tmpbuf));

			for (attr_index = 0; attr_index < ARRAY_SIZE(l1profile_attributes); attr_index++) {
				l1attr = &l1profile_attributes[attr_index];
				sprintf(key, "%s_%s", profile_index, l1attr->name);

				if (RTMPGetKeyParameter(key, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							("%s=%s\n", l1attr->name, tmpbuf));

					if (l1attr->handler)
						l1attr->handler(pAd, l1attr->extra, tmpbuf);
					else
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("unknown handler for %s, ignored!\n", l1attr->name));
				} else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						("%s not found\n", l1attr->name));
				}
			}

			retval = NDIS_STATUS_SUCCESS;
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("%s not found!!\n", profile_index));
			retval = NDIS_STATUS_FAILURE;
		}
		os_free_mem(tmpbuf);
	} else {
		retval = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Read file \"%s\" failed(errCode=%d)!\n", L1_PROFILE_PATH, retval));
	}

err_out:

	if ((srcf.Status == 0) && (os_file_close(srcf) != 0)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Close file \"%s\" failed(errCode=%d)!\n", L1_PROFILE_PATH, retval));
		retval = NDIS_STATUS_FAILURE;
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Close file \"%s\" succeed!\n", L1_PROFILE_PATH));

	if (buffer)
		os_free_mem(buffer);

	return retval;
}


NDIS_STATUS	RTMPReadParametersHook(RTMP_ADAPTER *pAd)
{
	RTMP_STRING *src = NULL;
	RTMP_OS_FD_EXT srcf;
	INT retval = NDIS_STATUS_FAILURE;
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	RTMP_STRING *buffer = NULL;
	os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);

	if (!buffer)
		return NDIS_STATUS_FAILURE;

	os_zero_mem(buffer, buf_size);
	/*if support multi-profile merge it*/
#ifdef MULTI_PROFILE

	if (multi_profile_check(pAd, buffer) == NDIS_STATUS_SUCCESS) {
		RTMPSetProfileParameters(pAd, buffer);
		retval = NDIS_STATUS_SUCCESS;
	} else
#endif /*MULTI_PROFILE*/
	{
		src = get_dev_l2profile(pAd);

		if (src && *src) {
			srcf = os_file_open(src, O_RDONLY, 0);

			if (srcf.Status)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Open file \"%s\" failed!\n", src));
			else {
#ifndef OS_ABL_SUPPORT

				/* TODO: need to roll back when convert into OSABL code */
				if (srcf.fsize != 0 && buf_size < (srcf.fsize + 1))
					buf_size = srcf.fsize  + 1;

#endif /* OS_ABL_SUPPORT */
				retval = os_file_read(srcf, buffer, buf_size - 1);

				if (retval > 0) {
					RTMPSetProfileParameters(pAd, buffer);
					retval = NDIS_STATUS_SUCCESS;
				} else
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", src, retval));

				if (os_file_close(srcf) != 0) {
					retval = NDIS_STATUS_FAILURE;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", src, retval));
				}
			}
		}
	}


	if (buffer)
		os_free_mem(buffer);

	return retval;
}

/*Used for pre-read user configuration before system ready*/
NDIS_STATUS	RTMPPreReadProfile(RTMP_ADAPTER *pAd)
{
	RTMP_STRING *src = NULL;
	RTMP_OS_FD_EXT srcf;
	INT retval = NDIS_STATUS_FAILURE;
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	RTMP_STRING *buffer = NULL;

	src = get_dev_l2profile(pAd);

	if (src && *src) {
		srcf = os_file_open(src, O_RDONLY, 0);

		if (srcf.Status)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Open file \"%s\" failed!\n", src));
		else {
			os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);

			if (buffer) {
				os_zero_mem(buffer, buf_size);
				retval = os_file_read(srcf, buffer, buf_size - 1);

				if (retval > 0) {
					RTMPSetPreProfileParameters(pAd, buffer);
					retval = NDIS_STATUS_SUCCESS;
				} else
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", src, retval));

				os_free_mem(buffer);
			} else
				retval = NDIS_STATUS_FAILURE;

			if (os_file_close(srcf) != 0) {
				retval = NDIS_STATUS_FAILURE;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", src, retval));
			}
		}
	}

	return retval;
}

void RTMP_IndicateMediaState(
	IN	PRTMP_ADAPTER		pAd,
	IN  NDIS_MEDIA_STATE	media_state)
{
	pAd->IndicateMediaState = media_state;
#ifdef SYSTEM_LOG_SUPPORT

	if (pAd->IndicateMediaState == NdisMediaStateConnected) {
		UINT wcid = BSSID_WCID_TO_REMOVE;  /* Pat: TODO */

		RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, pAd->MacTab.Content[wcid].Addr, BSS0, 0);
	} else {
		UINT wcid = BSSID_WCID_TO_REMOVE;  /* Pat: TODO */

		RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, pAd->MacTab.Content[wcid].Addr, BSS0, 0);
	}

#endif /* SYSTEM_LOG_SUPPORT */
}

#ifdef INF_PPA_SUPPORT
static INT process_nbns_packet(
	IN PRTMP_ADAPTER	pAd,
	IN struct sk_buff		*skb)
{
	UCHAR *data;
	USHORT *eth_type;

	data = (UCHAR *)eth_hdr(skb);

	if (data == 0) {
		data = (UCHAR *)skb->data;

		if (data == 0) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Error\n", __func__));
			return 1;
		}
	}

	eth_type = (USHORT *)&data[12];

	if (*eth_type == cpu_to_be16(ETH_P_IP)) {
		INT ip_h_len;
		UCHAR *ip_h;
		UCHAR *udp_h;
		USHORT dport, host_dport;

		ip_h = data + 14;
		ip_h_len = (ip_h[0] & 0x0f) * 4;

		if (ip_h[9] == 0x11) { /* UDP */
			udp_h = ip_h + ip_h_len;
			memcpy(&dport, udp_h + 2, 2);
			host_dport = ntohs(dport);
				if ((host_dport == 67) || (host_dport == 68)) /* DHCP */
					return 0;
		}
	} else if ((data[12] == 0x88) && (data[13] == 0x8e)) /* EAPOL */
		return 0;

	return 1;
}
#endif /* INF_PPA_SUPPORT */

#ifdef RTMP_UDMA_SUPPORT
void RtmpOsPktUdmaSend(PNDIS_PACKET pNetPkt, UCHAR port)
{
	udma_send_packet(port, RTPKT_TO_OSPKT(pNetPkt));
}

#ifdef CONFIG_STA_SUPPORT
void RemoveUdmaDevMac(
	IN PRTMP_ADAPTER	pAd,
	PNDIS_PACKET pNetPkt)
{
	UINT16 TypeLen;
	PUCHAR pData;
	UINT8 LayerHdrPos = 14;

	pData = GET_OS_PKT_DATAPTR(pNetPkt);
	NdisCopyMemory(&pData[6], pAd->CurrentAddress, MAC_ADDR_LEN);
	TypeLen = (pData[12] << 8) | pData[13];

	if (TypeLen == ETH_TYPE_VLAN) {
		TypeLen = (USHORT)((pData[16] << 8) + pData[17]);
		LayerHdrPos += 4;
	}

	if (TypeLen ==  ETH_TYPE_ARP)
		NdisCopyMemory(&pData[LayerHdrPos + 8], pAd->CurrentAddress, MAC_ADDR_LEN);
	else if (TypeLen == ETH_TYPE_IPv4) {
		if (pData[LayerHdrPos + 9] == 0x11) {
			PUCHAR udpHdr;
			UINT16 srcPort, dstPort;
			BOOLEAN bHdrChanged = FALSE;

			udpHdr = &pData[LayerHdrPos + 20];
			srcPort = OS_NTOHS(get_unaligned((PUINT16)(udpHdr)));
			dstPort = OS_NTOHS(get_unaligned((PUINT16)(udpHdr + 2)));

			if (srcPort == 68 && dstPort == 67) { /*It's a DHCP packet */
				PUCHAR bootpHdr;
				UINT16 bootpFlag;
				PUCHAR dhcpHdr;

				bootpHdr = udpHdr + 8;
				bootpFlag = OS_NTOHS(get_unaligned((PUINT16)(bootpHdr + 10)));
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_TRACE, ("is bootp packet! bootpFlag=0x%x\n", bootpFlag));
				dhcpHdr = bootpHdr + 236;

				if (bootpFlag != 0x8000) { /*check if it's a broadcast request. */
					static UCHAR  DHCP_MAGIC[] = {0x63, 0x82, 0x53, 0x63};

					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_TRACE, ("the DHCP flag is a unicast, dhcp_magic=%02x:%02x:%02x:%02x\n",
							 dhcpHdr[0], dhcpHdr[1], dhcpHdr[2], dhcpHdr[3]));

					if (NdisEqualMemory(dhcpHdr, DHCP_MAGIC, 4)) {
						MTWF_LOG(DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_TRACE, ("dhcp magic macthed!\n"));
						bootpFlag = OS_HTONS(0x8000);
						NdisMoveMemory((bootpHdr + 10), &bootpFlag, 2);	/*Set the bootp flag as broadcast */
						bHdrChanged = TRUE;
					}
				}
			}

			if (bHdrChanged == TRUE)
				NdisZeroMemory((udpHdr + 6), 2); /* Modify the UDP chksum as zero */
		}
	}
}
#endif /*CONFIG_STA_SUPPORT*/
#endif /*RTMP_UDMA_SUPPORT*/

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
struct net_device *rlt_dev_get_by_name(const char *name)
{
#if (KERNEL_VERSION(2, 6, 24) <= LINUX_VERSION_CODE)
	return dev_get_by_name(&init_net, name);
#else
	return dev_get_by_name(name);
#endif
}

VOID ApCliLinkCoverRxPolicy(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	OUT BOOLEAN *DropPacket)
{
#ifdef MAC_REPEATER_SUPPORT
	void *opp_band_tbl = NULL;
	void *band_tbl = NULL;
	void *other_band_tbl = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pInvalidEntry = NULL;
	REPEATER_CLIENT_ENTRY *pOtherBandReptEntry = NULL;
	REPEATER_CLIENT_ENTRY *pAnotherBandReptEntry = NULL;
	PNDIS_PACKET pRxPkt = pPacket;
	UCHAR *pPktHdr = NULL;

	pPktHdr = GET_OS_PKT_DATAPTR(pRxPkt);

	if (wf_drv_tbl.wf_fwd_feedback_map_table)
		wf_drv_tbl.wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl, &other_band_tbl);

	if ((opp_band_tbl == NULL) && (other_band_tbl == NULL))
		return;

	if (IS_GROUP_MAC(pPktHdr)) {
		pInvalidEntry = RepeaterInvaildMacLookup(pAd, pPktHdr + 6);

		if (opp_band_tbl != NULL)
			pOtherBandReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pPktHdr + 6, FALSE);

		if (other_band_tbl != NULL)
			pAnotherBandReptEntry = RTMPLookupRepeaterCliEntry(other_band_tbl, FALSE, pPktHdr + 6, FALSE);

		if ((pInvalidEntry != NULL) || (pOtherBandReptEntry != NULL) || (pAnotherBandReptEntry != NULL)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s, recv broadcast from InvalidRept Entry, drop this packet\n", __func__));
			*DropPacket = TRUE;
		}
	}

#endif /* MAC_REPEATER_SUPPORT */
}
#endif /* CONFIG_WIFI_PKT_FWD */

void announce_802_3_packet(
	IN VOID *pAdSrc,
	IN PNDIS_PACKET pPacket,
	IN UCHAR OpMode)
{
	RTMP_ADAPTER *pAd = NULL;
	PNDIS_PACKET pRxPkt = pPacket;
#if defined(WH_EZ_SETUP) && (defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE))
	BOOLEAN bypass_rx_fwd = FALSE;
#endif

	pAd =  (RTMP_ADAPTER *)pAdSrc;
	ASSERT(pAd);
	ASSERT(pPacket);
	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=>%s(): OpMode=%d\n", __FUNCTION__, OpMode)); */

#ifdef DOT11V_WNM_SUPPORT
#ifdef CONFIG_STA_SUPPORT

	/*drop the match DMS flow data*/
	if (RxDMSHandle(pAd, pPacket) == NDIS_STATUS_FAILURE)
		return;

#endif /* CONFIG_STA_SUPPORT */
#endif /* DOT11V_WNM_SUPPORT */
	MEM_DBG_PKT_FREE_INC(pPacket);
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT

		if (RTMP_MATPktRxNeedConvert(pAd, RtmpOsPktNetDevGet(pRxPkt))) {
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

			if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE)) {
				BOOLEAN	 need_drop = FALSE;

				ApCliLinkCoverRxPolicy(pAd, pPacket, &need_drop);

				if (need_drop == TRUE) {
					/*MEM_DBG_PKT_ALLOC_INC(pPacket);*/
					RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
					return;
				}
			}

#endif /* CONFIG_WIFI_PKT_FWD */
			RTMP_MATEngineRxHandle(pAd, pRxPkt, 0);
		}

#endif /* MAT_SUPPORT */
	}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
	/* Push up the protocol stack */
#ifdef CONFIG_AP_SUPPORT
#if defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570)
	{
		extern int (*pToUpperLayerPktSent)(PNDIS_PACKET *pSkb);

		RtmpOsPktProtocolAssign(pRxPkt);
		pToUpperLayerPktSent(pRxPkt);
		return;
	}
#endif /* defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570) */
#endif /* CONFIG_AP_SUPPORT */
#ifdef IKANOS_VX_1X0
	{
		IKANOS_DataFrameRx(pAd, pRxPkt);
		return;
	}
#endif /* IKANOS_VX_1X0 */
#ifdef INF_PPA_SUPPORT
	{
		if (ppa_hook_directpath_send_fn && (pAd->PPAEnable == TRUE)) {
			INT retVal, ret = 0;
			UINT ppa_flags = 0;

			retVal = process_nbns_packet(pAd, pRxPkt);

			if (retVal > 0) {
				ret = ppa_hook_directpath_send_fn(pAd->g_if_id, pRxPkt, pRxPkt->len, ppa_flags);

				if (ret == 0) {
					pRxPkt = NULL;
					return;
				}

				RtmpOsPktRcvHandle(pRxPkt);
			} else if (retVal == 0) {
				RtmpOsPktProtocolAssign(pRxPkt);
				RtmpOsPktRcvHandle(pRxPkt);
			} else {
				dev_kfree_skb_any(pRxPkt);
				MEM_DBG_PKT_FREE_INC(pAd);
			}
		} else {
			RtmpOsPktProtocolAssign(pRxPkt);
			RtmpOsPktRcvHandle(pRxPkt);
		}

		return;
	}
#endif /* INF_PPA_SUPPORT */
	{
#ifdef CONFIG_RT2880_BRIDGING_ONLY
		PACKET_CB_ASSIGN(pRxPkt, 22) = 0xa8;
#endif
#if defined(CONFIG_RA_CLASSIFIER) || defined(CONFIG_RA_CLASSIFIER_MODULE)

		if (ra_classifier_hook_rx != NULL) {
			ra_classifier_hook_rx(pRxPkt, classifier_cur_cycle);
		}

#endif /* CONFIG_RA_CLASSIFIER */
#ifdef CONFIG_FAST_NAT_SUPPORT
		/* bruce+
		 *	ra_sw_nat_hook_rx return 1 --> continue
		 *	ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
		 */
		if (ra_sw_nat_hook_rx != NULL) {

			RtmpOsPktProtocolAssign(pRxPkt);
			RtmpOsPktNatMagicTag(pRxPkt);

			if (ra_sw_nat_hook_rx(pRxPkt))
				RtmpOsPktRcvHandle(pRxPkt);

			return;
		}
#endif /* CONFIG_FAST_NAT_SUPPORT */
	}
#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT

	if (BG_FTPH_PacketFromApHandle(pRxPkt) == 0)
		return;

#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef TRACELOG_TCP_PKT

	if (RTMPIsTcpAckPkt(pRxPkt))
		pAd->u4TcpRxAckCnt++;

#endif
#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckUpdateDataCnx(pAd, pRxPkt);
#endif
#ifdef RTMP_UDMA_SUPPORT

	if (pAd->CommonCfg.bUdmaFlag) {
		UCHAR port = pAd->CommonCfg.UdmaPortNum;
#ifdef CONFIG_STA_SUPPORT

		if (pAd->CommonCfg.bUdmaDevSet) {
			NdisCopyMemory(GET_OS_PKT_DATAPTR(pRxPkt),
						   pAd->CommonCfg.UdmaDevMac, 6);
		}

#endif /*CONFIG_STA_SUPPORT*/
		RtmpOsPktUdmaSend(pRxPkt, port);
		return;
	}

#endif/*RTMP_UDMA_SUPPORT*/
	RtmpOsPktProtocolAssign(pRxPkt);
#ifdef AUTOMATION
	automation_rx_payload_check(pAd, pRxPkt);
#endif /* AUTOMATION */

#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)

	if (ra_sw_nat_hook_set_magic)
		ra_sw_nat_hook_set_magic(pRxPkt, FOE_MAGIC_WLAN);

	if (ra_sw_nat_hook_rx != NULL) {
		if (ra_sw_nat_hook_rx(pRxPkt) == 0)
			return;
	}

#endif

#ifdef APCLI_SUPPORT
	if (RTMP_IS_PACKET_APCLI(pRxPkt)) {
		struct ethhdr *mh = eth_hdr(pRxPkt);
		if ((mh->h_dest[0] & 0x1) == 0x1) {
			UCHAR idx = 0;
			STA_ADMIN_CONFIG *apcli_entry = NULL;

#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
				(RTMPQueryLookupRepeaterCliEntryMT(pAd, mh->h_source, TRUE) == TRUE)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s------>Found SA with Repeater Fack MAC In!, Src:%02X:%02X:%02X:%02X:%02X:%02X, Dest:%02X:%02X:%02X:%02X:%02X:%02X\n",
				__func__, PRINT_MAC(mh->h_source), PRINT_MAC(mh->h_dest)));
				RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
				return;
			}
#endif
			for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
				apcli_entry = &pAd->StaCfg[idx];

				if (!apcli_entry->ApcliInfStat.ApCliInit)
				continue;

				if (MAC_ADDR_EQUAL(apcli_entry->wdev.if_addr, mh->h_source)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s------>Found SA with ApCli MAC(%d) In!, Src:%02X:%02X:%02X:%02X:%02X:%02X, Dest:%02X:%02X:%02X:%02X:%02X:%02X\n",
					__func__, idx, PRINT_MAC(mh->h_source), PRINT_MAC(mh->h_dest)));
					RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
					return;
				}
			}

		}
	}
#endif


#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE)) {
		struct sk_buff *pOsRxPkt = RTPKT_TO_OSPKT(pRxPkt);

		if (RTMP_IS_PACKET_AP_APCLI(pOsRxPkt)) {
			if (wf_drv_tbl.wf_fwd_rx_hook != NULL) {
				struct ethhdr *mh = eth_hdr(pRxPkt);
				int ret = 0;

				if ((mh->h_dest[0] & 0x1) == 0x1) {
					if (RTMP_IS_PACKET_APCLI(pOsRxPkt)) {
#ifdef MAC_REPEATER_SUPPORT
						if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
							(RTMPQueryLookupRepeaterCliEntryMT(pAd, mh->h_source, TRUE) == TRUE)) {
							/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							    ("announce_802_3_packet: drop rx pkt by RTMPQueryLookupRepeaterCliEntryMT check\n"));*/
							/*MEM_DBG_PKT_ALLOC_INC(pRxPkt);*/
							RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
							return;
						}
#endif /* MAC_REPEATER_SUPPORT */
						{
							VOID *opp_band_tbl = NULL;
							VOID *band_tbl = NULL;
							VOID *other_band_tbl = NULL;

							if (wf_drv_tbl.wf_fwd_feedback_map_table)
								wf_drv_tbl.wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl, &other_band_tbl);

							if (band_tbl != NULL) {
#ifdef MAC_REPEATER_SUPPORT
								if (MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr), mh->h_source) ||
									((((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr_DBDC != NULL) &&
									 MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)band_tbl)->Wdev_ifAddr_DBDC), mh->h_source)))
								{
									/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
									    ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table band_tbl check of source addr against Wdev_ifAddr\n"));*/
									/*MEM_DBG_PKT_ALLOC_INC(pRxPkt);*/
									RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
									return;
								}
#endif /* MAC_REPEATER_SUPPORT */
							}

							if (opp_band_tbl != NULL) {
#ifdef MAC_REPEATER_SUPPORT
								if ((MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr), mh->h_source)) ||
									((((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr_DBDC != NULL) &&
									 (MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr_DBDC), mh->h_source))))
								{
									/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
									*    ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table opp_band_tbl check of source addr against Wdev_ifAddr\n")); */
									/*MEM_DBG_PKT_ALLOC_INC(pRxPkt);*/
									RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
									return;
								}
#endif /* MAC_REPEATER_SUPPORT */
							}

							if (other_band_tbl != NULL) {
#ifdef MAC_REPEATER_SUPPORT
								if ((MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr), mh->h_source)) ||
									((((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr_DBDC != NULL) &&
									 (MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)other_band_tbl)->Wdev_ifAddr_DBDC), mh->h_source))))
								{
									/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
									 ("announce_802_3_packet: drop rx pkt by wf_fwd_feedback_map_table other_band_tbl check of source addr against Wdev_ifAddr\n"));*/
									/*MEM_DBG_PKT_ALLOC_INC(pRxPkt);*/
									RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
									return;
								}
#endif /* MAC_REPEATER_SUPPORT */
							}
						}
					}
				}

					ret = wf_drv_tbl.wf_fwd_rx_hook(pRxPkt);

					if (ret == 0) {
						/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						    ("announce_802_3_packet: wf_fwd_rx_hook returned 0\n"));*/
						return;
					} else if (ret == 2) {
						/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						    ("announce_802_3_packet: wf_fwd_rx_hook returned 2\n"));*/
						/*MEM_DBG_PKT_ALLOC_INC(pRxPkt);*/
						RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
						return;
					}

			}
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("No CB Packet RTMP_IS_PACKET_AP_APCLI(%d)\n", RTMP_IS_PACKET_AP_APCLI(pOsRxPkt)));
	}

#endif /* CONFIG_WIFI_PKT_FWD */
	RtmpOsPktRcvHandle(pRxPkt);
}




VOID RTMPFreeGlobalUtility(VOID)
{
	/*do nothing for now*/
}

VOID RTMPFreeAdapter(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE os_cookie;
	struct _PCI_HIF_T *hif;

#ifdef MULTI_INF_SUPPORT
	multi_inf_adapt_unreg((VOID *) pAd);
#endif /* MULTI_INF_SUPPORT */

	hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	os_cookie = (POS_COOKIE)pAd->OS_Cookie;
#ifdef RLM_CAL_CACHE_SUPPORT
	rlmCalCacheDeinit(&pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */
#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RTMP_FLASH_SUPPORT

	/* only if in MULTIPLE_CARD the eebuf be allocated not static */
	if (pAd->eebuf  /*&& (pAd->eebuf != pAd->chipCap.EEPROM_DEFAULT_BIN)*/) {
		os_free_mem(pAd->eebuf);
		pAd->eebuf = NULL;
	}

#endif /* RTMP_FLASH_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT

	if (pAd->ApCfg.bMACRepeaterEn == TRUE)
		AsicSetReptFuncEnable(pAd, FALSE);

	NdisFreeSpinLock(&pAd->ApCfg.CliLinkMapLock);
	NdisFreeSpinLock(&pAd->ApCfg.ReptCliEntryLock);
#endif
#endif
#endif
#ifdef CONFIG_FWOWN_SUPPORT
	NdisFreeSpinLock(&pAd->DriverOwnLock);
#endif /* CONFIG_FWOWN_SUPPORT */
	NdisFreeSpinLock(&pAd->BssInfoIdxBitMapLock);
	NdisFreeSpinLock(&pAd->WdevListLock);
	NdisFreeSpinLock(&pAd->irq_lock);

#ifdef UAPSD_SUPPORT
	NdisFreeSpinLock(&pAd->UAPSDEOSPLock); /* OS_ABL_SUPPORT */
#endif /* UAPSD_SUPPORT */
#ifdef DOT11_N_SUPPORT
	NdisFreeSpinLock(&pAd->mpdu_blk_pool.lock);
#endif /* DOT11_N_SUPPORT */
#ifdef GREENAP_SUPPORT
	NdisFreeSpinLock(&pAd->ApCfg.greenap.lock);
#endif /* GREENAP_SUPPORT */

	if (pAd->iw_stats) {
		os_free_mem(pAd->iw_stats);
		pAd->iw_stats = NULL;
	}

	if (pAd->stats) {
		os_free_mem(pAd->stats);
		pAd->stats = NULL;
	}

#ifdef MT_MAC
#ifdef CONFIG_AP_SUPPORT

	if ((IS_HIF_TYPE(pAd, HIF_MT)) && (pAd->OpMode == OPMODE_AP)) {
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
		ASSERT(pMbss);

		if (pMbss)
			bcn_buf_deinit(pAd, &pMbss->wdev.bcn_buf);
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():func_dev is NULL!\n", __func__));
			return;
		}
	}

#endif
#endif
#ifdef CONFIG_ATE
#endif /* CONFIG_ATE */
	RTMP_OS_FREE_TIMER(pAd);
	RTMP_OS_FREE_LOCK(pAd);
	RTMP_OS_FREE_TASKLET(pAd);
	RTMP_OS_FREE_TASK(pAd);
	RTMP_OS_FREE_SEM(pAd);
	RTMP_OS_FREE_ATOMIC(pAd);
	RTMPFreeHifAdapterBlock(pAd);
	/*remove chipcap*/
	hdev_ctrl_exit(pAd);
	RtmpOsVfree(pAd); /* pci_free_consistent(os_cookie->pci_dev,sizeof(RTMP_ADAPTER),pAd,os_cookie->pAd_pa); */

	if (os_cookie)
		os_free_mem(os_cookie);

	RTMPFreeGlobalUtility();
}




int RTMPSendPackets(
	IN NDIS_HANDLE dev_hnd,
	IN PPNDIS_PACKET pkt_list,
	IN UINT pkt_cnt,
	IN UINT32 pkt_total_len,
	IN RTMP_NET_ETH_CONVERT_DEV_SEARCH Func)
{
	struct wifi_dev *wdev = (struct wifi_dev *)dev_hnd;
	RTMP_ADAPTER *pAd;
	PNDIS_PACKET pPacket = pkt_list[0];

	if (!wdev->sys_handle) {
		ASSERT(wdev->sys_handle);
		return 0;
	}

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	INC_COUNTER64(pAd->WlanCounters[0].TransmitCountFrmOs);

	if (!pPacket)
		return 0;

	if (pkt_total_len < 14) {
		hex_dump("bad packet", GET_OS_PKT_DATAPTR(pPacket), pkt_total_len);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return 0;
	}

#ifdef CONFIG_ATE

	/* TODO: shiang-usw, can remove this? */
	if (ATE_ON(pAd)) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		return 0;
	}

#endif /* CONFIG_ATE */
#ifdef CONFIG_5VT_ENHANCE
	RTMP_SET_PACKET_5VT(pPacket, 0);

	if (*(int *)(GET_OS_PKT_CB(pPacket)) == BRIDGE_TAG)
		RTMP_SET_PACKET_5VT(pPacket, 1);

#endif /* CONFIG_5VT_ENHANCE */
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE)) {
		if (wf_drv_tbl.wf_fwd_tx_hook != NULL) {
			if (wf_drv_tbl.wf_fwd_tx_hook(pPacket) == 1) {
				/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, */
				/* ("RTMPSendPackets: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x : wf_fwd_tx_hook indicated Packet DROP\n", */
				/* wdev->wdev_idx,wdev->wdev_type,wdev->func_idx)); */
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				return 0;
			}
		}
	}

#endif /* CONFIG_WIFI_PKT_FWD */
	skb_orphan(pPacket);
	return wdev_tx_pkts((NDIS_HANDLE)pAd, (PPNDIS_PACKET) & pPacket, 1, wdev);
}

#ifdef CONFIG_STA_SUPPORT
/*
 * ========================================================================
 * Routine Description:
 *	Driver pre-Ioctl for STA.
 *
 * Arguments:
 *	pAdSrc			    - WLAN control block pointer
 *	pParm				- the IOCTL parameters
 *
 * Return Value:
 *	NDIS_STATUS_SUCCESS	- IOCTL OK
 *	Otherwise			- IOCTL fail
 *
 * ========================================================================
 */
INT RTMP_STA_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pParm)
{
	RT_CMD_IOCTL_INTERFACE_CONFIG *pConfig = (RT_CMD_IOCTL_INTERFACE_CONFIG *)pParm;
	POS_COOKIE pObj;
	USHORT index;
	INT	Status = NDIS_STATUS_SUCCESS;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* determine this ioctl command is comming from which interface. */
	if (pConfig->priv_flags == INT_MAIN) {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MSTA_ID;
	} else if (pConfig->priv_flags == INT_MSTA) {
		pObj->ioctl_if_type = INT_MSTA;

		/* if (!RTMPEqualMemory(net_dev->name, pAd->net_dev->name, 3))  // for multi-physical card, no MSTA */
		if (strcmp(pConfig->name, RtmpOsGetNetDevName(pAd->net_dev)) != 0) { /* sample */
			for (index = 1; index < MAX_MULTI_STA; index++) {
				if (pAd->StaCfg[index].wdev.if_dev == pConfig->net_dev) {
					pObj->ioctl_if = index;
					break;
				}
			}

			/* Interface not found! */
			if (index == MAX_MULTI_STA)
				return -ENETDOWN;
		} else  /* ioctl command from I/F(ra0) */
			pObj->ioctl_if = MAIN_MSTA_ID;

		/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: INT_MSTA interface %d\n", __func__, pObj->ioctl_if)); */
	} else if (pConfig->priv_flags == INT_APCLI) {
		pObj->ioctl_if_type = INT_APCLI;
		pObj->ioctl_if = MAIN_MSTA_ID;
	} else {
		/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("IOCTL is not supported in WDS interface\n")); */
		return -EOPNOTSUPP;
	}

	pConfig->interface_idx = pObj->ioctl_if;
	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: interface %d\n", __func__, pObj->ioctl_if)); */
	return Status;
}

#endif

#ifdef CONFIG_AP_SUPPORT
/*
 * ========================================================================
 * Routine Description:
 *	Driver pre-Ioctl for AP.
 *
 * Arguments:
 *	pAdSrc			- WLAN control block pointer
 *	pCB				- the IOCTL parameters
 *
 * Return Value:
 *	NDIS_STATUS_SUCCESS	- IOCTL OK
 *	Otherwise			- IOCTL fail
 * ========================================================================
 */
INT RTMP_AP_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pCB)
{
	RT_CMD_AP_IOCTL_CONFIG *pConfig = (RT_CMD_AP_IOCTL_CONFIG *)pCB;
	POS_COOKIE pObj;
	USHORT index;
	INT	Status = NDIS_STATUS_SUCCESS;
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if ((pConfig->priv_flags == INT_MAIN) && !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		if (pConfig->pCmdData == NULL)
			return Status;

		if (RtPrivIoctlSetVal() == pConfig->CmdId_RTPRIV_IOCTL_SET) {
			if (TRUE
#ifdef CONFIG_APSTA_MIXED_SUPPORT
				&& ((pConfig->cmd_data_len <= (strlen("OpMode") + 1))
					&& (strstr(pConfig->pCmdData, "OpMode") == NULL))
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef SINGLE_SKU
				&& (strstr(pConfig->pCmdData, "ModuleTxpower") == NULL)
#endif /* SINGLE_SKU */
			   )
				return -ENETDOWN;
		} else
			return -ENETDOWN;
	}

	pObj->pSecConfig = NULL;

	/* determine this ioctl command is comming from which interface. */
	if (pConfig->priv_flags == INT_MAIN
#ifdef RT_CFG80211_SUPPORT
		|| RTMP_CFG80211_HOSTAPD_ON(pAd)
#endif
	   ) {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
		pObj->pSecConfig = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig;
	} else if (pConfig->priv_flags == INT_MBSSID) {
		pObj->ioctl_if_type = INT_MBSSID;

		/* if (!RTMPEqualMemory(net_dev->name, pAd->net_dev->name, 3))  // for multi-physical card, no MBSSID */
		if (strcmp(pConfig->name, RtmpOsGetNetDevName(pAd->net_dev)) != 0) { /* sample */
			for (index = 1; index < pAd->ApCfg.BssidNum; index++) {
				if (pAd->ApCfg.MBSSID[index].wdev.if_dev == pConfig->net_dev) {
					pObj->ioctl_if = index;
					pObj->pSecConfig = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig;
					break;
				}
			}

			/* Interface not found! */
			if (index == pAd->ApCfg.BssidNum)
				return -ENETDOWN;
		} else  /* ioctl command from I/F(ra0) */
			pObj->ioctl_if = MAIN_MBSSID;

		MBSS_MR_APIDX_SANITY_CHECK(pAd, pObj->ioctl_if);
	}

#ifdef WDS_SUPPORT
	else if (pConfig->priv_flags == INT_WDS) {
		pObj->ioctl_if_type = INT_WDS;

		for (index = 0; index < MAX_WDS_ENTRY; index++) {
			if (pAd->WdsTab.WdsEntry[index].wdev.if_dev == pConfig->net_dev) {
				pObj->ioctl_if = index;
				pObj->pSecConfig = &pAd->WdsTab.WdsEntry[index].wdev.SecConfig;
				break;
			}

			if (index == MAX_WDS_ENTRY) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): can not find wds I/F\n", __func__));
				return -ENETDOWN;
			}
		}
	}

#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
	else if (pConfig->priv_flags == INT_APCLI) {
		pObj->ioctl_if_type = INT_APCLI;

		for (index = 0; index < MAX_APCLI_NUM; index++) {
			if (pAd->StaCfg[index].wdev.if_dev == pConfig->net_dev) {
				pObj->ioctl_if = index;
				pObj->pSecConfig = &pAd->StaCfg[index].wdev.SecConfig;
				break;
			}

			if (index == MAX_APCLI_NUM) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): can not find Apcli I/F\n", __func__));
				return -ENETDOWN;
			}
		}

		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}

#endif /* APCLI_SUPPORT */
	else {
		/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("IOCTL is not supported in WDS interface\n")); */
		return -EOPNOTSUPP;
	}

	pConfig->apidx = pObj->ioctl_if;
	return Status;
}


VOID AP_E2PROM_IOCTL_PostCtrl(
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	RTMP_STRING *msg)
{
	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: copy_to_user() fail\n", __func__));
}


VOID IAPP_L2_UpdatePostCtrl(RTMP_ADAPTER *pAd, UINT8 *mac_p, INT wdev_idx)
{
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef WDS_SUPPORT
VOID AP_WDS_KeyNameMakeUp(
	IN	RTMP_STRING *pKey,
	IN	UINT32						KeyMaxSize,
	IN	INT							KeyId)
{
	snprintf(pKey, KeyMaxSize, "Wds%dKey", KeyId);
}
#endif /* WDS_SUPPORT */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

/*
 *Invoked from WIFI forward module to register function pointers.
 */
VOID wifi_fwd_register(struct wifi_fwd_func_table *table)
{
	wf_drv_tbl.wf_fwd_tx_hook = table->wf_fwd_tx_hook;
	wf_drv_tbl.wf_fwd_rx_hook = table->wf_fwd_rx_hook;
	wf_drv_tbl.wf_fwd_entry_insert_hook = table->wf_fwd_entry_insert_hook;
	wf_drv_tbl.wf_fwd_entry_delete_hook = table->wf_fwd_entry_delete_hook;
	wf_drv_tbl.wf_fwd_set_cb_num  =  table->wf_fwd_set_cb_num;
	wf_drv_tbl.wf_fwd_set_debug_level_hook = table->wf_fwd_set_debug_level_hook;
	wf_drv_tbl.wf_fwd_check_active_hook = table->wf_fwd_check_active_hook;
	wf_drv_tbl.wf_fwd_get_rep_hook = table->wf_fwd_get_rep_hook;
	wf_drv_tbl.wf_fwd_pro_active_hook = table->wf_fwd_pro_active_hook;
	wf_drv_tbl.wf_fwd_pro_halt_hook	= table->wf_fwd_pro_halt_hook;
	wf_drv_tbl.wf_fwd_pro_enabled_hook = table->wf_fwd_pro_enabled_hook;
	wf_drv_tbl.wf_fwd_pro_disabled_hook	= table->wf_fwd_pro_disabled_hook;
	wf_drv_tbl.wf_fwd_access_schedule_active_hook = table->wf_fwd_access_schedule_active_hook;
	wf_drv_tbl.wf_fwd_access_schedule_halt_hook =	table->wf_fwd_access_schedule_halt_hook;
	wf_drv_tbl.wf_fwd_hijack_active_hook = table->wf_fwd_hijack_active_hook;
	wf_drv_tbl.wf_fwd_hijack_halt_hook = table->wf_fwd_hijack_halt_hook;
	wf_drv_tbl.wf_fwd_bpdu_active_hook = table->wf_fwd_bpdu_active_hook;
	wf_drv_tbl.wf_fwd_bpdu_halt_hook	= table->wf_fwd_bpdu_halt_hook;
	wf_drv_tbl.wf_fwd_show_entry_hook = table->wf_fwd_show_entry_hook;
	wf_drv_tbl.wf_fwd_needed_hook = table->wf_fwd_needed_hook;
	wf_drv_tbl.wf_fwd_delete_entry_hook =	table->wf_fwd_delete_entry_hook;
	wf_drv_tbl.packet_source_show_entry_hook = table->packet_source_show_entry_hook;
	wf_drv_tbl.packet_source_delete_entry_hook = table->packet_source_delete_entry_hook;
	wf_drv_tbl.wf_fwd_feedback_map_table	= table->wf_fwd_feedback_map_table;
	wf_drv_tbl.wf_fwd_probe_adapter =	table->wf_fwd_probe_adapter;
	wf_drv_tbl.wf_fwd_insert_bridge_mapping_hook = table->wf_fwd_insert_bridge_mapping_hook;
	wf_drv_tbl.wf_fwd_insert_repeater_mapping_hook = table->wf_fwd_insert_repeater_mapping_hook;
	wf_drv_tbl.wf_fwd_search_mapping_table_hook =	table->wf_fwd_search_mapping_table_hook;
	wf_drv_tbl.wf_fwd_delete_entry_inform_hook =	table->wf_fwd_delete_entry_inform_hook;
	wf_drv_tbl.wf_fwd_check_device_hook =	table->wf_fwd_check_device_hook;
	wf_drv_tbl.wf_fwd_add_entry_inform_hook =	table->wf_fwd_add_entry_inform_hook;
	wf_drv_tbl.wf_fwd_set_easy_setup_mode = table->wf_fwd_set_easy_setup_mode;
	wf_drv_tbl.wf_fwd_set_bridge_hook = table->wf_fwd_set_bridge_hook;
}

/*
 *Invoked from WIFI forward module to unregister function pointers.
 */
VOID wifi_fwd_unregister(VOID)
{

wf_drv_tbl.wf_fwd_tx_hook = NULL;
wf_drv_tbl.wf_fwd_rx_hook = NULL;
wf_drv_tbl.wf_fwd_entry_insert_hook = NULL;
wf_drv_tbl.wf_fwd_entry_delete_hook = NULL;
wf_drv_tbl.wf_fwd_set_cb_num = NULL;
wf_drv_tbl.wf_fwd_set_debug_level_hook = NULL;
wf_drv_tbl.wf_fwd_check_active_hook = NULL;
wf_drv_tbl.wf_fwd_get_rep_hook = NULL;
wf_drv_tbl.wf_fwd_pro_active_hook = NULL;
wf_drv_tbl.wf_fwd_pro_halt_hook = NULL;
wf_drv_tbl.wf_fwd_pro_enabled_hook = NULL;
wf_drv_tbl.wf_fwd_pro_disabled_hook = NULL;
wf_drv_tbl.wf_fwd_access_schedule_active_hook = NULL;
wf_drv_tbl.wf_fwd_access_schedule_halt_hook = NULL;
wf_drv_tbl.wf_fwd_hijack_active_hook = NULL;
wf_drv_tbl.wf_fwd_hijack_halt_hook = NULL;
wf_drv_tbl.wf_fwd_bpdu_active_hook = NULL;
wf_drv_tbl.wf_fwd_bpdu_halt_hook = NULL;
wf_drv_tbl.wf_fwd_show_entry_hook = NULL;
wf_drv_tbl.wf_fwd_needed_hook = NULL;
wf_drv_tbl.wf_fwd_delete_entry_hook = NULL;
wf_drv_tbl.packet_source_show_entry_hook = NULL;
wf_drv_tbl.packet_source_delete_entry_hook = NULL;
wf_drv_tbl.wf_fwd_feedback_map_table = NULL;
wf_drv_tbl.wf_fwd_probe_adapter = NULL;
wf_drv_tbl.wf_fwd_insert_bridge_mapping_hook = NULL;
wf_drv_tbl.wf_fwd_insert_repeater_mapping_hook = NULL;
wf_drv_tbl.wf_fwd_search_mapping_table_hook = NULL;
wf_drv_tbl.wf_fwd_delete_entry_inform_hook = NULL;
wf_drv_tbl.wf_fwd_check_device_hook = NULL;
wf_drv_tbl.wf_fwd_add_entry_inform_hook = NULL;
wf_drv_tbl.wf_fwd_set_easy_setup_mode = NULL;
wf_drv_tbl.wf_fwd_set_bridge_hook = NULL;


}
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifndef MT76XX_COMBO_DUAL_DRIVER_SUPPORT
EXPORT_SYMBOL(wifi_fwd_register);
EXPORT_SYMBOL(wifi_fwd_unregister);
#endif
#endif

#endif
