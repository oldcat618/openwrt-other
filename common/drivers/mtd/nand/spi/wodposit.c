// SPDX-License-Identifier: GPL-2.0
/*
 * WODPOSIT SPI NAND driver
 *
 * Based on raw ID: a5 a0 81 a5 a0 (0xA5 manuf, 0xA0 device)
 * WPS3NS01W: 128MiB, 2KiB pages, 64 OOB, on-die ECC (strength 4-bit/512B)
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_WODPOSIT		0xA5

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int wodposit_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	return -ERANGE;
}

static int wodposit_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 2;
	region->length = mtd->oobsize - 2;

	return 0;
}

static const struct mtd_ooblayout_ops wodposit_ooblayout = {
	.ecc = wodposit_ooblayout_ecc,
	.free = wodposit_ooblayout_free,
};

static int wodposit_ecc_get_status(struct spinand_device *spinand, u8 status)
{
	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;
	case STATUS_ECC_HAS_BITFLIPS:
		/* On-die ECC corrects up to 4 bits per 512 bytes */
		return 4;
	default:
		break;
	}
	return -EBADMSG;
}

static const struct spinand_info wodposit_spinand_table[] = {
	SPINAND_INFO("WPS3NS01W",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0xA0),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 20, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&wodposit_ooblayout,
				     wodposit_ecc_get_status)),
};

static const struct spinand_manufacturer_ops wodposit_spinand_manuf_ops = {
};

const struct spinand_manufacturer wodposit_spinand_manufacturer = {
	.id = SPINAND_MFR_WODPOSIT,
	.name = "wodposit",
	.chips = wodposit_spinand_table,
	.nchips = ARRAY_SIZE(wodposit_spinand_table),
	.ops = &wodposit_spinand_manuf_ops,
};
