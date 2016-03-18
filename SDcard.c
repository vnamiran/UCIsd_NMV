/* ============================================================================
 * Copyright (c) 2008-2012 Texas Instruments Incorporated.
 * Except for those rights granted to you in your license from TI, all rights
 * reserved.
* ============================================================================*/


#define CSL_SD_CLOCK_MAX_KHZ      (20000u)

AtaState gstrAtaDrive;
AtaState *gpstrAtaDrive = &gstrAtaDrive;

AtaFile strAtaFile;
AtaFile *pAtaFile = &strAtaFile;

#pragma DATA_ALIGN(AtaWrBuf ,4);
AtaUint16 AtaWrBuf[256];
AtaMMCState gstrAtaMMCState;
AtaMMCState *gpstrAtaMMCState = &gstrAtaMMCState;

char ataFileName[10];

/* CSL MMCSD Data structures */
CSL_MMCControllerObj pMmcsdContObj;
CSL_MmcsdHandle mmcsdHandle;
CSL_MMCCardObj mmcCardObj;
CSL_MMCCardIdObj sdCardIdObj;
CSL_MMCCardCsdObj sdCardCsdObj;

/* CSL DMA data structures */
CSL_DMA_Handle dmaWrHandle;
CSL_DMA_Handle dmaRdHandle;
CSL_DMA_ChannelObj dmaWrChanObj;
CSL_DMA_ChannelObj dmaRdChanObj;
CSL_DMA_Handle dmaHandle;

Uint16 gMmcWriteBuf[CSL_MMCSD_ATA_BUF_SIZE];
Uint16 gMmcReadBuf[CSL_MMCSD_ATA_BUF_SIZE];

#define CSL_MMCSD_ATAFS_DISKTYPE   (0)
CSL_Status configSdCard(CSL_MMCSDOpMode opMode);

AtaError mmcConfigFs(char *fileName);Uint16 computeClkRate(void);

void main(void) {
	return;
}
volatile Int16 PaSs_StAtE = 0x0001; // Init to 1. Reset to 0 at any monitored execution error.
volatile Int16 PaSs = 0x0000; // Init to 0.  Updated later with PaSs_StAtE when and if

void mmcFileTest(void) {
	CSL_Status status;
	AtaError ataStatus;
	Bool testFailed;

	testFailed = FALSE;

	printf("MMCSD-ATAFS TESTS!\n\n");
	printf("\n\n\nMMCSD-ATAFS DMA MODE TEST!\n\n");

	status = configSdCard(CSL_MMCSD_OPMODE_DMA);
	if (status != CSL_SOK) {
		testFailed = TRUE;
		/////  Reseting PaSs_StAtE to 0 if error detected here.
		PaSs_StAtE = 0x0000; // Was intialized to 1 at declaration.
		printf("SD card initialization Failed\n");
		printf("\nMMCSD-ATAFS DMA MODE TEST FAILED!!\n");
	} else {
		printf("SD card initialization Successful\n");
		ataStatus = mmcConfigFs("dmaTest");
		if (ataStatus != ATA_ERROR_NONE) {
			testFailed = TRUE;
			/////  Reseting PaSs_StAtE to 0 if error detected here.
			PaSs_StAtE = 0x0000; // Was intialized to 1 at declaration.
			printf("\nMMCSD-ATAFS DMA MODE TEST FAILED!!\n");
		} else {
			printf("\nMMCSD-ATAFS DMA MODE TEST PASSED!!\n");
		}
	}

	/////  At program exit, copy "PaSs_StAtE" into "PaSs".
	PaSs = PaSs_StAtE; //If flow gets here, override PaSs' initial 0 with

	if (testFailed == TRUE) {
		printf("\n\nMMCSD-ATAFS TESTS FAILED!!\n\n");
		exit(EXIT_FAILURE);
	} else {
		printf("\n\nMMCSD-ATAFS TESTS PASSED!!\n\n");
		exit(EXIT_SUCCESS);
	}
}

AtaError mmcConfigFs(char *fileName) {
	Uint16 index;
	AtaError ata_error;
	unsigned int diskType;

	ata_error = ATA_ERROR_NONE;

	for (index = 0; index < CSL_MMCSD_ATA_BUF_SIZE; index++) {
		gMmcWriteBuf[index] = 0x4142;
		AtaWrBuf[index] = 0x4344;
		gMmcReadBuf[index] = 0x0;
	}

	/* Call init function initialize ATA state structure */
	gpstrAtaDrive->AtaInitAtaMediaState = (AtaError (*)(void *)) MMC_initState;
	gpstrAtaMMCState->hMmcSd = mmcsdHandle;
	gpstrAtaDrive->pAtaMediaState = gpstrAtaMMCState;
	gpstrAtaDrive->AtaInitAtaMediaState(gpstrAtaDrive);

	/* Set the temp write buffer */
	gpstrAtaDrive->_AtaWriteBuffer = AtaWrBuf;

	diskType = CSL_MMCSD_ATAFS_DISKTYPE;
	/* Call ATA_systemInit() to intialize some values whcih are
	 used by chk_mmc() function */
	ata_error = ATA_systemInit(gpstrAtaDrive, diskType);

	chk_mmc(gpstrAtaDrive, &diskType);
	if (diskType != CSL_MMCSD_ATAFS_DISKTYPE) {
		ata_error = ATA_systemInit(gpstrAtaDrive, diskType);
		if (ata_error != ATA_ERROR_NONE) {
			printf("ATA_systemInit Failed\n");
			printf("Format the SD card\n");
			return (ata_error);
		}
	}

	printf("\nATA File System Initialization successful\n");

	/* Find the first file available */
	ata_error = ATA_fileInit(gpstrAtaDrive, pAtaFile);
	if (ata_error) {
		printf("ATA_fileInit error (0x%x)\n", ata_error);
		return (ata_error);
	}

	ata_error = ATA_setLongDirectoryName(pAtaFile, fileName);
	if (ata_error != ATA_ERROR_NONE) {
		printf("ATA_setLongDirectoryName Failed\n");
		return (ata_error);
	}

	ata_error = ATA_createDirectoryLong(pAtaFile, fileName);
	if (ata_error != ATA_ERROR_NONE) {
		printf("ATA_createDirectoryLong Failed\n");
		return (ata_error);
	}

	ata_error = ATA_cdRoot(pAtaFile);
	if (ata_error != ATA_ERROR_NONE) {
		printf("ATA_cdRoot Failed\n");
		return (ata_error);
	}

	do {
		ata_error = ATA_findNext(pAtaFile);
		if (ata_error != ATA_ERROR_NONE) {
			printf("Cannot find created directory\n");
			return (ata_error);
		}

		ata_error = ATA_getLongName(pAtaFile, ataFileName, 0, 10);
		if (ata_error != ATA_ERROR_NONE) {
			printf("ATA_getLongName failed\n");
			return (ata_error);
		}
	} while (strcmp(ataFileName, fileName) != 0);

	ata_error = ATA_cd(pAtaFile);
	if (ata_error != ATA_ERROR_NONE) {
		printf("ATA_cdRoot Failed\n");
		return (ata_error);
	}

	/* Set the file name */
	ATA_setFileName(pAtaFile, fileName, "txt");

	ata_error = ATA_create(pAtaFile);
	if (ata_error != ATA_ERROR_NONE) {
		printf("ATA_create Failed\n");
		return (ata_error);
	} else {
		printf("\nFile Creation on SD card is Successful\n");
	}

	/* Write data to the file */
	ata_error = ATA_write(pAtaFile, gMmcWriteBuf, CSL_MMCSD_ATA_BUF_SIZE);
	if (ata_error != ATA_ERROR_NONE) {
		printf("ATA_write Failed\n");
		return (ata_error);
	} else {
		printf("\nWriting Data to the file on SD card successful\n");
	}

	/* Reset the file pointer to the beginning */
	ATA_seek(pAtaFile, 0);

	/* Read the data from the file in little endian mode */
	ata_error = ATA_readLittleEndian(pAtaFile, gMmcReadBuf,
			CSL_MMCSD_ATA_BUF_SIZE);
	if (ata_error != ATA_ERROR_NONE) {
		printf("ATA_readLittleEndian Failed\n");
		return (ata_error);
	} else {
		printf("\nReading Data from the file on SD card successful\n");
	}

	/* Close the file */
	ata_error = ATA_close(pAtaFile);
	if (ata_error != ATA_ERROR_NONE) {
		printf("ATA_close Failed\n");
		return (ata_error);
	}

	/* Compare the data read and data written */
	for (index = 0; index < CSL_MMCSD_ATA_BUF_SIZE; index++) {
		if (gMmcWriteBuf[index] != gMmcReadBuf[index]) {
			ata_error = 1;
			printf("\nMMCSD Read and Write Buffers do not Match\n");
			break;
		}
	}

	if (ata_error == 0) {
		printf("\nMMCSD Read and Write Buffers Match\n");
	}

	return (ata_error);
}

CSL_Status configSdCard(CSL_MMCSDOpMode opMode) {
	CSL_Status status;
	Uint16 actCard;
	Uint16 clockDiv;
	Uint16 rca;

	/* Get the clock divider value for the current CPU frequency */
	clockDiv = computeClkRate();

	/* Initialize MMCSD CSL module */
	status = MMC_init();

	status = SYS_setEBSR(CSL_EBSR_FIELD_SP0MODE, CSL_EBSR_SP0MODE_0);
	status |= SYS_setEBSR(CSL_EBSR_FIELD_SP1MODE, CSL_EBSR_SP1MODE_0);
	if (CSL_SOK != status) {
		printf("SYS_setEBSR failed\n");
		return (status);
	}

	/* Open MMCSD CSL module */
#ifdef C5515_EZDSP
	mmcsdHandle = MMC_open(&pMmcsdContObj, CSL_MMCSD1_INST,
			opMode, &status);
#else
	mmcsdHandle = MMC_open(&pMmcsdContObj, CSL_MMCSD1_INST, opMode, &status);
#endif
	if (mmcsdHandle == NULL) {
		printf("MMC_open Failed\n");
		return (status);
	}

	/* Configure the DMA in case of operating mode is set to DMA */
	if (opMode == CSL_MMCSD_OPMODE_DMA) {
		/* Initialize Dma */
		status = DMA_init();
		if (status != CSL_SOK) {
			printf("DMA_init Failed!\n");
			return (status);
		}

		/* Open Dma channel for MMCSD write */
		dmaWrHandle = DMA_open(CSL_DMA_CHAN0, &dmaWrChanObj, &status);
		if ((dmaWrHandle == NULL) || (status != CSL_SOK)) {
			printf("DMA_open for MMCSD Write Failed!\n");
			return (status);
		}

		/* Open Dma channel for MMCSD read */
		dmaRdHandle = DMA_open(CSL_DMA_CHAN1, &dmaRdChanObj, &status);
		if ((dmaRdHandle == NULL) || (status != CSL_SOK)) {
			printf("DMA_open for MMCSD Read Failed!\n");
			return (status);
		}

		/* Set the DMA handle for MMC read */
		status = MMC_setDmaHandle(mmcsdHandle, dmaWrHandle, dmaRdHandle);
		if (status != CSL_SOK) {
			printf("API: MMC_setDmaHandle for MMCSD Failed\n");
			return (status);
		}
	}

	/* Reset the SD card */
	status = MMC_sendGoIdle(mmcsdHandle);
	if (status != CSL_SOK) {
		printf("MMC_sendGoIdle Failed\n");
		return (status);
	}

	/* Check for the card */
	status = MMC_selectCard(mmcsdHandle, &mmcCardObj);
	if ((status == CSL_ESYS_BADHANDLE) || (status == CSL_ESYS_INVPARAMS)) {
		printf("MMC_selectCard Failed\n");
		return (status);
	}

	/* Verify whether the SD card is detected or not */
	if (mmcCardObj.cardType == CSL_SD_CARD) {
		printf("SD Card detected\n");

		/* Check if the card is high capacity card */
		if (mmcsdHandle->cardObj->sdHcDetected == TRUE) {
			printf("SD card is High Capacity Card\n");
			printf("Memory Access will use Block Addressing\n");
		} else {
			printf("SD card is Standard Capacity Card\n");
			printf("Memory Access will use Byte Addressing\n");
		}
	} else {
		if (mmcCardObj.cardType == CSL_CARD_NONE) {
			printf("No Card detected\n");
		} else {
			printf("SD Card not detected\n");
		}
		printf("Please Insert SD Card\n");
		return (CSL_ESYS_FAIL);
	}

	/* Set the init clock */
	status = MMC_sendOpCond(mmcsdHandle, 70);
	if (status != CSL_SOK) {
		printf("MMC_sendOpCond Failed\n");
		return (status);
	}

	/* Send the card identification Data */
	status = SD_sendAllCID(mmcsdHandle, &sdCardIdObj);
	if (status != CSL_SOK) {
		printf("SD_sendAllCID Failed\n");
		return (status);
	}

	/* Set the Relative Card Address */
	status = SD_sendRca(mmcsdHandle, &mmcCardObj, &rca);
	if (status != CSL_SOK) {
		printf("SD_sendRca Failed\n");
		return (status);
	}

	/* Read the SD Card Specific Data */
	status = SD_getCardCsd(mmcsdHandle, &sdCardCsdObj);
	if (status != CSL_SOK) {
		printf("SD_getCardCsd Failed\n");
		return (status);
	}

	/* Set the card type in internal data structures */
	status = MMC_setCardType(&mmcCardObj, CSL_SD_CARD);
	if (status != CSL_SOK) {
		printf("MMC_setCardType Failed\n");
		return (status);
	}

	/* Set the card pointer in internal data structures */
	status = MMC_setCardPtr(mmcsdHandle, &mmcCardObj);
	if (status != CSL_SOK) {
		printf("MMC_setCardPtr Failed\n");
		return (status);
	}

	/* Get the number of cards */
	status = MMC_getNumberOfCards(mmcsdHandle, &actCard);
	if (status != CSL_SOK) {
		printf("MMC_getNumberOfCards Failed\n");
		return (status);
	}

	/* Set clock for read-write access */
	status = MMC_sendOpCond(mmcsdHandle, clockDiv);
	if (status != CSL_SOK) {
		printf("MMC_sendOpCond Failed\n");
		return (status);
	}

	/* Set Endian mode for read and write operations */
	status = MMC_setEndianMode(mmcsdHandle, CSL_MMCSD_ENDIAN_LITTLE,
			CSL_MMCSD_ENDIAN_LITTLE);
	if (status != CSL_SOK) {
		printf("MMC_setEndianMode Failed\n");
		return (status);
	}

	/* Set block length for the memory card
	 * For high capacity cards setting the block length will have
	 * no effect
	 */
	status = MMC_setBlockLength(mmcsdHandle, CSL_MMCSD_BLOCK_LENGTH);
	if (status != CSL_SOK) {
		printf("MMC_setBlockLength Failed\n");
		return (status);
	}

	return (status);
}


Uint16 computeClkRate(void) {
	Uint32 sysClock;
	Uint32 remainder;
	Uint32 memMaxClk;
	Uint16 clkRate;

	sysClock = 0;
	remainder = 0;
	memMaxClk = CSL_SD_CLOCK_MAX_KHZ;	//CSL_SD_CLOCK_MAX_KHZ      (20000u)
	clkRate = 0;

	/* Get the clock value at which CPU is running */
	sysClock = getSysClk();

	if (sysClock > memMaxClk) {
		if (memMaxClk != 0) {
			clkRate = sysClock / memMaxClk;
			remainder = sysClock % memMaxClk;


			if (remainder != 0) {
				clkRate++;
			}

			if (clkRate % 2 != 0) {
				clkRate++;
			}

			clkRate = clkRate / 2;
			clkRate = clkRate - 1;
			 */
			if (clkRate > CSL_MMC_MAX_CLOCK_RATE) {
				clkRate = CSL_MMC_MAX_CLOCK_RATE;
			}
		} else {
			clkRate = CSL_MMC_MAX_CLOCK_RATE;
		}
	}

	return (clkRate);
}

