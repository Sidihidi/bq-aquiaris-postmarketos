/* SPDX-License-Identifier: GPL-2.0 */
/*
 * os/linux/glue/gl_kal_fw.c
 *
 *   KAL firmware-image shim for the linux-7.0.12 (postmarketOS / BQ krillin)
 *   port of the MediaTek MT6582 'mt_wifi' full-MAC driver.
 *
 *   Replaces the Android-3.10-era filp_open()/set_fs()/f_op->read() path of the
 *   stock gl_kal.c with the mainline request_firmware() API. The firmware blob
 *   is looked up as:
 *
 *       /lib/firmware/mediatek/mt6582/WIFI_RAM_CODE
 *
 *   The stock core (wlanImageSectionDownload() in common/wlan_lib.c, driven from
 *   nic/nic_pwr_mgt.c:wlanHarvardFormatDownload) only ever calls
 *   kalFirmwareImageMapping() / kalFirmwareImageUnmapping(); the Open/Close/Load
 *   helpers are re-implemented here (file-static, request_firmware backed) purely
 *   so the mapping helper keeps the exact same open->size->load->close shape the
 *   original had, which makes the diff against the stock trivial to audit.
 *
 *   API deltas honoured (per the port contract):
 *     - NO set_fs()/get_fs()/get_ds()  (removed on mainline).
 *     - request_firmware()/release_firmware() instead of filp_open()/f_op->read.
 *     - the firmware 'struct device' comes from prGlueInfo->rHifInfo.Dev
 *       (populated by glSetHifInfo during probe: &pdev->dev).
 *
 *   kalGetFwStartAddress()/kalGetFwLoadAddress() are kept here too (they belong
 *   to the fw shim conceptually and read straight out of rRegInfo).
 */

#include "precomp.h"
#include "gl_kal_prototypes.h"

#include <linux/firmware.h>
#include <linux/vmalloc.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/err.h>

#if CFG_ENABLE_FW_DOWNLOAD

/*
 * Mainline firmware path. request_firmware() prepends /lib/firmware (and the
 * other configured search paths), so this is the leaf under mediatek/mt6582/.
 * The stock CFG_FW_FILENAME ("WIFI_RAM_CODE") is kept as the basename so the
 * physical blob shipped by the stock ROM can be reused verbatim.
 */
#ifndef WIFI_FW_IMAGE_PATH
#define WIFI_FW_IMAGE_PATH  "mediatek/mt6582/" CFG_FW_FILENAME
#endif

#ifndef ALIGN_4
#define ALIGN_4(_value)     (((_value) + 3) & ~3UL)
#endif

/*
 * request_firmware() hands back a single contiguous, const, kernel-resident
 * buffer for the WHOLE image; there is no "open then seek+read section by
 * section" like the old filp path. We stash the handle in prGlueInfo->prFw so
 * the size/load/close helpers can operate on it, mirroring the old file-static
 * 'filp'. Not re-entrant, exactly like the stock code (fw download is a single
 * serialized bring-up step, never concurrent).
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Open (acquire) the firmware image into kernel space via request_firmware.
 *
 * \param[in] prGlueInfo   Pointer of GLUE Data Structure
 *
 * \retval WLAN_STATUS_SUCCESS   firmware acquired, prGlueInfo->prFw valid
 * \retval WLAN_STATUS_FAILURE   request_firmware failed
 */
/*----------------------------------------------------------------------------*/
static WLAN_STATUS
kalFirmwareOpen(
	IN P_GLUE_INFO_T    prGlueInfo
	)
{
	struct device *prDev;
	INT_32 i4Ret;

	ASSERT(prGlueInfo);

	prGlueInfo->prFw = NULL;

	/* firmware device = the AHB platform device captured at probe time */
	prDev = prGlueInfo->rHifInfo.Dev;
	if (prDev == NULL) {
		DBGLOG(INIT, ERROR,
		       ("kalFirmwareOpen: no firmware device (rHifInfo.Dev NULL)\n"));
		return WLAN_STATUS_FAILURE;
	}

	i4Ret = request_firmware((const struct firmware **)&prGlueInfo->prFw,
				 WIFI_FW_IMAGE_PATH, prDev);
	if (i4Ret != 0 || prGlueInfo->prFw == NULL) {
		DBGLOG(INIT, INFO,
		       ("Open FW image: %s failed (%d)\n",
			WIFI_FW_IMAGE_PATH, i4Ret));
		prGlueInfo->prFw = NULL;
		return WLAN_STATUS_FAILURE;
	}

	DBGLOG(INIT, INFO,
	       ("Open FW image: %s done (%zu bytes)\n",
		WIFI_FW_IMAGE_PATH, prGlueInfo->prFw->size));

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Release the firmware image acquired by kalFirmwareOpen.
 *
 * \param[in] prGlueInfo   Pointer of GLUE Data Structure
 *
 * \retval WLAN_STATUS_SUCCESS
 */
/*----------------------------------------------------------------------------*/
static WLAN_STATUS
kalFirmwareClose(
	IN P_GLUE_INFO_T    prGlueInfo
	)
{
	ASSERT(prGlueInfo);

	if (prGlueInfo->prFw != NULL) {
		release_firmware(prGlueInfo->prFw);
		prGlueInfo->prFw = NULL;
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Query the size (in bytes) of the acquired firmware image.
 *
 * \param[in]  prGlueInfo   Pointer of GLUE Data Structure
 * \param[out] pu4Size      Firmware image size in bytes
 *
 * \retval WLAN_STATUS_SUCCESS
 * \retval WLAN_STATUS_FAILURE   no image acquired
 */
/*----------------------------------------------------------------------------*/
static WLAN_STATUS
kalFirmwareSize(
	IN  P_GLUE_INFO_T   prGlueInfo,
	OUT PUINT_32        pu4Size
	)
{
	ASSERT(prGlueInfo);
	ASSERT(pu4Size);

	if (prGlueInfo->prFw == NULL || prGlueInfo->prFw->data == NULL) {
		*pu4Size = 0;
		return WLAN_STATUS_FAILURE;
	}

	*pu4Size = (UINT_32)prGlueInfo->prFw->size;

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Copy '*pu4Size' bytes of the firmware image (starting at u4Offset) into
 *        the caller-provided buffer. request_firmware already resident in RAM, so
 *        this is a bounded memcpy (replaces the old f_op->read()).
 *
 * \param[in]     prGlueInfo   Pointer of GLUE Data Structure
 * \param[out]    prBuf        Destination buffer
 * \param[in]     u4Offset     Byte offset inside the image
 * \param[in,out] pu4Size      IN: bytes to copy; OUT: bytes actually copied
 *
 * \retval WLAN_STATUS_SUCCESS
 * \retval WLAN_STATUS_FAILURE
 */
/*----------------------------------------------------------------------------*/
static WLAN_STATUS
kalFirmwareLoad(
	IN     P_GLUE_INFO_T    prGlueInfo,
	OUT    PVOID            prBuf,
	IN     UINT_32          u4Offset,
	IN OUT PUINT_32         pu4Size
	)
{
	UINT_32 u4FwSize;
	UINT_32 u4CopyLen;

	ASSERT(prGlueInfo);
	ASSERT(prBuf);
	ASSERT(pu4Size);

	if (prGlueInfo->prFw == NULL || prGlueInfo->prFw->data == NULL)
		return WLAN_STATUS_FAILURE;

	u4FwSize = (UINT_32)prGlueInfo->prFw->size;

	/* offset past EOF -> nothing to copy */
	if (u4Offset >= u4FwSize) {
		*pu4Size = 0;
		return WLAN_STATUS_FAILURE;
	}

	/* clamp the requested length to what is actually available from u4Offset */
	u4CopyLen = *pu4Size;
	if (u4CopyLen > (u4FwSize - u4Offset))
		u4CopyLen = u4FwSize - u4Offset;

	kalMemCopy(prBuf, prGlueInfo->prFw->data + u4Offset, u4CopyLen);

	*pu4Size = u4CopyLen;

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Load firmware image into a vmalloc'd, 4-byte-aligned buffer and hand
 *        the core a stable pointer + length it can section-download from.
 *
 * The core (nic_pwr_mgt.c) keeps the returned handle to pass back to
 * kalFirmwareImageUnmapping(). We return the vmalloc buffer itself as the
 * handle; *ppvMapFileBuf is the same buffer (that is what the core actually
 * reads the sections out of).
 *
 * We copy into a private vmalloc buffer (rather than returning prFw->data
 * directly) because: (a) prFw->data is 'const' and some section-download paths
 * scribble a trailing pad after ALIGN_4; (b) it keeps the buffer lifetime under
 * our control and identical to the stock behaviour the core was written against.
 *
 * \param[in]  prGlueInfo     Pointer of GLUE Data Structure
 * \param[out] ppvMapFileBuf  Pointer of pointer to the mapped firmware image
 * \param[out] pu4FileLength  Firmware image length (== mapped length)
 *
 * \retval  Map file handle (used for unmapping), or NULL on failure.
 */
/*----------------------------------------------------------------------------*/
PVOID
kalFirmwareImageMapping(
	IN  P_GLUE_INFO_T   prGlueInfo,
	OUT PPVOID          ppvMapFileBuf,
	OUT PUINT_32        pu4FileLength
	)
{
	PVOID prFwBuffer = NULL;
	UINT_32 u4FwSize = 0;
	UINT_32 u4LoadSize;

	DEBUGFUNC("kalFirmwareImageMapping");

	ASSERT(prGlueInfo);
	ASSERT(ppvMapFileBuf);
	ASSERT(pu4FileLength);

	*ppvMapFileBuf = NULL;
	*pu4FileLength = 0;

	do {
		/* <1> Acquire firmware via request_firmware */
		if (kalFirmwareOpen(prGlueInfo) != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, TRACE, ("kalFirmwareOpen fail!\n"));
			break;
		}

		/* <2> Query firmware size */
		if (kalFirmwareSize(prGlueInfo, &u4FwSize) != WLAN_STATUS_SUCCESS ||
		    u4FwSize == 0) {
			DBGLOG(INIT, TRACE, ("kalFirmwareSize fail!\n"));
			kalFirmwareClose(prGlueInfo);
			break;
		}

		/* <3> Allocate a 4-byte-aligned coalescing buffer (large trunk) */
		prFwBuffer = vmalloc(ALIGN_4(u4FwSize));
		if (prFwBuffer == NULL) {
			DBGLOG(INIT, ERROR,
			       ("vmalloc(%u) for FW image fail!\n",
				(unsigned int)ALIGN_4(u4FwSize)));
			kalFirmwareClose(prGlueInfo);
			break;
		}

		/* <4> Copy the whole image into the buffer */
		u4LoadSize = u4FwSize;
		if (kalFirmwareLoad(prGlueInfo, prFwBuffer, 0, &u4LoadSize)
		    != WLAN_STATUS_SUCCESS || u4LoadSize != u4FwSize) {
			DBGLOG(INIT, TRACE,
			       ("kalFirmwareLoad fail! (%u/%u)\n",
				(unsigned int)u4LoadSize,
				(unsigned int)u4FwSize));
			vfree(prFwBuffer);
			prFwBuffer = NULL;
			kalFirmwareClose(prGlueInfo);
			break;
		}

		/* the request_firmware handle is no longer needed once copied out */
		kalFirmwareClose(prGlueInfo);

		/* <5> Write back info */
		*pu4FileLength = u4FwSize;
		*ppvMapFileBuf = prFwBuffer;

		return prFwBuffer;

	} while (FALSE);

	return NULL;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Release the buffer returned by kalFirmwareImageMapping.
 *
 * \param[in] prGlueInfo    Pointer of GLUE Data Structure
 * \param[in] prFwHandle    Handle returned by kalFirmwareImageMapping
 * \param[in] pvMapFileBuf  Mapped firmware image (== handle here)
 *
 * \retval none
 */
/*----------------------------------------------------------------------------*/
VOID
kalFirmwareImageUnmapping(
	IN P_GLUE_INFO_T    prGlueInfo,
	IN PVOID            prFwHandle,
	IN PVOID            pvMapFileBuf
	)
{
	DEBUGFUNC("kalFirmwareImageUnmapping");

	ASSERT(prGlueInfo);

	/*
	 * We already released the request_firmware handle inside the mapping
	 * helper; here we only free the vmalloc coalescing buffer. prFwHandle
	 * and pvMapFileBuf are the same pointer (see kalFirmwareImageMapping);
	 * free via whichever is non-NULL, but only once.
	 */
	if (pvMapFileBuf != NULL) {
		vfree(pvMapFileBuf);
	} else if (prFwHandle != NULL) {
		vfree(prFwHandle);
	}

	/* defensive: make sure no stale request_firmware handle survives */
	if (prGlueInfo->prFw != NULL)
		kalFirmwareClose(prGlueInfo);
}

#endif /* CFG_ENABLE_FW_DOWNLOAD */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Firmware load address, read straight out of the registry (rRegInfo).
 *
 * \param[in] prGlueInfo   Pointer of GLUE Data Structure
 * \retval    u4LoadAddress
 */
/*----------------------------------------------------------------------------*/
UINT_32
kalGetFwLoadAddress(
	IN P_GLUE_INFO_T    prGlueInfo
	)
{
	ASSERT(prGlueInfo);

	return prGlueInfo->rRegInfo.u4LoadAddress;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Firmware start (entry) address, read straight out of the registry.
 *
 * \param[in] prGlueInfo   Pointer of GLUE Data Structure
 * \retval    u4StartAddress
 */
/*----------------------------------------------------------------------------*/
UINT_32
kalGetFwStartAddress(
	IN P_GLUE_INFO_T    prGlueInfo
	)
{
	ASSERT(prGlueInfo);

	return prGlueInfo->rRegInfo.u4StartAddress;
}
