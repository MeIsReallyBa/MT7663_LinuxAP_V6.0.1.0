/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    fsm_sync.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
				2016-08-25		Unify the SCAN API into one
*/


#include "rt_config.h"

#ifdef CONFIG_STA_SUPPORT
VOID StaSiteSurvey(
	IN  PRTMP_ADAPTER pAd,
	IN  PNDIS_802_11_SSID pSsid,
	IN  UCHAR ScanType,
	IN  struct wifi_dev *wdev)
{
	MLME_SCAN_REQ_STRUCT    ScanReq;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
		/*
		 * Still scanning, ignore this scanning.
		 */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("StaSiteSurvey:: Scanning now\n"));
		return;
	}

	if (INFRA_ON(pStaCfg))
		ScanType = SCAN_IMPROVED;

#ifdef WSC_STA_SUPPORT

	if (ScanType == SCAN_WSC_ACTIVE)
		ScanType = SCAN_ACTIVE;

#endif /* WSC_STA_SUPPORT */
	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;

	if ((pSsid) && (pSsid->SsidLength > 0) && (pSsid->SsidLength <= (NDIS_802_11_LENGTH_SSID))) {
		ScanReq.SsidLen = pSsid->SsidLength;
		NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}

	ScanReq.BssType = BSS_ANY;
	ScanReq.ScanType = ScanType;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\033[1;32m %s go\033[0m\n", __func__));
	cntl_scan_request(wdev, &ScanReq);
}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID ApSiteSurvey_by_wdev(
	PRTMP_ADAPTER	pAd,
	PNDIS_802_11_SSID	pSsid,
	UCHAR				ScanType,
	BOOLEAN				ChannelSel,
	struct wifi_dev		*wdev)
{
	MLME_SCAN_REQ_STRUCT    ScanReq;
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	UINT8 band_idx = BAND0;
#ifdef APCLI_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
#ifdef CON_WPS
	UCHAR ifIdx;
#endif /*ifIdx*/
#ifdef CON_WPS
	ifIdx = wdev->func_idx;

	if ((ifIdx < MAX_APCLI_NUM)
		&& (pAd->ApCfg.ConWpsApCliDisabled == FALSE)
		&& (pAd->StaCfg[ifIdx].wdev.WscControl.conWscStatus & CON_WPS_STATUS_APCLI_RUNNING))
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\033[1;32m ApSiteSurvey_by_wdev don't need Init BSS table\033[0m\n"));
	else
#endif /*CON_WPS*/
		if ((ScanCtrl->PartialScan.bScanning == TRUE && ScanCtrl->Channel == 0) ||
			ScanCtrl->PartialScan.bScanning == FALSE)
			BssTableInit(ScanTab);

	pAd->ApCfg.bAutoChannelAtBootup[band_idx] = ChannelSel;
	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;

	if ((pSsid) && (pSsid->SsidLength > 0) && (pSsid->SsidLength <= (NDIS_802_11_LENGTH_SSID))) {
		ScanReq.SsidLen = pSsid->SsidLength;
		NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}

	ScanReq.BssType = BSS_ANY;
	ScanReq.ScanType = ScanType;
	cntl_scan_request(wdev, &ScanReq);
}
#endif /* CONFIG_AP_SUPPORT */

/* 3: Customer CH List Scan */

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID  TriEventInit(RTMP_ADAPTER *pAd)
{
	UCHAR i;

	for (i = 0; i < MAX_TRIGGER_EVENT; i++)
		pAd->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;

	pAd->CommonCfg.TriggerEventTab.EventANo = 0;
	pAd->CommonCfg.TriggerEventTab.EventBCountDown = 0;
}

INT TriEventTableSetEntry(
	IN RTMP_ADAPTER *pAd,
	OUT TRIGGER_EVENT_TAB * Tab,
	IN UCHAR *pBssid,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen,
	IN UCHAR RegClass,
	IN UCHAR ChannelNo)
{
	/* Event A, legacy AP exist.*/
	if (HtCapabilityLen == 0) {
		UCHAR index;

		/*
			Check if we already set this entry in the Event Table.
		*/
		for (index = 0; index < MAX_TRIGGER_EVENT; index++) {
			if ((Tab->EventA[index].bValid == TRUE) &&
				(Tab->EventA[index].Channel == ChannelNo) &&
				(Tab->EventA[index].RegClass == RegClass)
			   ) {
				return 0;
			}
		}

		/*
			If not set, add it to the Event table
		*/
		if (Tab->EventANo < MAX_TRIGGER_EVENT) {
			RTMPMoveMemory(Tab->EventA[Tab->EventANo].BSSID, pBssid, 6);
			Tab->EventA[Tab->EventANo].bValid = TRUE;
			Tab->EventA[Tab->EventANo].Channel = ChannelNo;

			if (RegClass != 0) {
				/* Beacon has Regulatory class IE. So use beacon's*/
				Tab->EventA[Tab->EventANo].RegClass = RegClass;
			} else {
				/* Use Station's Regulatory class instead.*/
				/* If no Reg Class in Beacon, set to "unknown"*/
				/* TODO:  Need to check if this's valid*/
				Tab->EventA[Tab->EventANo].RegClass = 0; /* ????????????????? need to check*/
			}

			Tab->EventANo++;
		}
	}

#ifdef DOT11V_WNM_SUPPORT
	/* Not complete yet. Ignore for compliing successfully.*/
#else
	else if (pHtCapability->HtCapInfo.Forty_Mhz_Intolerant) {
		/* Event B.   My BSS beacon has Intolerant40 bit set*/
		Tab->EventBCountDown = pAd->CommonCfg.Dot11BssWidthChanTranDelay;
	}

#endif /* DOT11V_WNM_SUPPORT */
	return 0;
}

BOOLEAN build_trigger_event_table(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem, BCN_IE_LIST *ie_list)
{
	/* Check if this scan channel is the effeced channel */
	struct wifi_dev *wdev = (struct wifi_dev *)Elem->wdev;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
		((ie_list->Channel > 0) && (ie_list->Channel <= 14))) {
		int chListIdx = 0;

		/*
			First we find the channel list idx by the channel number
		*/
		for (chListIdx = 0; chListIdx < pChCtrl->ChListNum; chListIdx++) {
			if (ie_list->Channel == pChCtrl->ChList[chListIdx].Channel)
				break;
		}

		if (chListIdx < pChCtrl->ChListNum) {

			/*
				If this channel is effected channel for the 20/40 coex operation. Check the related IEs.
			*/
			if (pChCtrl->ChList[chListIdx].bEffectedChannel == TRUE) {
				UCHAR RegClass;
				OVERLAP_BSS_SCAN_IE BssScan;
				/* Read Beacon's Reg Class IE if any. */
				PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, &RegClass);
				/* printk("\x1b[31m TriEventTableSetEntry \x1b[m\n"); */
				TriEventTableSetEntry(pAd, &pAd->CommonCfg.TriggerEventTab, ie_list->Bssid,
									  &ie_list->HtCapability, ie_list->HtCapabilityLen, RegClass, ie_list->Channel);
			}
		}
	}

	return TRUE;
}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */

