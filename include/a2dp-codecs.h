/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef A2DP_CODECS_H_
#define A2DP_CODECS_H_

#define A2DP_CODEC_SBC			0x00
#define A2DP_CODEC_MPEG12		0x01
#define A2DP_CODEC_AAC			0x02
#define A2DP_CODEC_ATRAC		0x03

#define SBC_SAMPLING_FREQ_16000		(1 << 3)
#define SBC_SAMPLING_FREQ_32000		(1 << 2)
#define SBC_SAMPLING_FREQ_44100		(1 << 1)
#define SBC_SAMPLING_FREQ_48000		1

#define SBC_CHANNEL_MODE_MONO		(1 << 3)
#define SBC_CHANNEL_MODE_DUAL_CHANNEL	(1 << 2)
#define SBC_CHANNEL_MODE_STEREO		(1 << 1)
#define SBC_CHANNEL_MODE_JOINT_STEREO	1

#define SBC_BLOCK_LENGTH_4			(1 << 3)
#define SBC_BLOCK_LENGTH_8			(1 << 2)
#define SBC_BLOCK_LENGTH_12			(1 << 1)
#define SBC_BLOCK_LENGTH_16			1

#define SBC_SUBBANDS_4				(1 << 1)
#define SBC_SUBBANDS_8				1

#define SBC_ALLOCATION_SNR			(1 << 1)
#define SBC_ALLOCATION_LOUDNESS		1

#define SBC_MIN_BITPOOL				2
#define SBC_MAX_BITPOOL				64

#define MPEG_CHANNEL_MODE_MONO		(1 << 3)
#define MPEG_CHANNEL_MODE_DUAL_CHANNEL	(1 << 2)
#define MPEG_CHANNEL_MODE_STEREO	(1 << 1)
#define MPEG_CHANNEL_MODE_JOINT_STEREO	1

#define MPEG_LAYER_MP1				(1 << 2)
#define MPEG_LAYER_MP2				(1 << 1)
#define MPEG_LAYER_MP3				1

#define MPEG_SAMPLING_FREQ_16000	(1 << 5)
#define MPEG_SAMPLING_FREQ_22050	(1 << 4)
#define MPEG_SAMPLING_FREQ_24000	(1 << 3)
#define MPEG_SAMPLING_FREQ_32000	(1 << 2)
#define MPEG_SAMPLING_FREQ_44100	(1 << 1)
#define MPEG_SAMPLING_FREQ_48000	1

#define MPEG_CRC_SUPPORT			1
// MPF1 is always supported RFC 2250
// MPF2 is optional RFC 3119
#define MPEG_MPF2_SUPPORT			1
#define MPEG_VBR_SUPPORT			1

// bitrate2 field
#define MPEG_BITRATE_IDX0_SUPPORT	1
#define MPEG_BITRATE_IDX1_SUPPORT	(1 << 1)
#define MPEG_BITRATE_IDX2_SUPPORT	(1 << 2)
#define MPEG_BITRATE_IDX3_SUPPORT	(1 << 3)
#define MPEG_BITRATE_IDX4_SUPPORT	(1 << 4)
#define MPEG_BITRATE_IDX5_SUPPORT	(1 << 5)
#define MPEG_BITRATE_IDX6_SUPPORT	(1 << 6)
#define MPEG_BITRATE_IDX7_SUPPORT	(1 << 7)
// bitrate1 field
#define MPEG_BITRATE_IDX8_SUPPORT	1
#define MPEG_BITRATE_IDX9_SUPPORT	(1 << 1)
#define MPEG_BITRATE_IDX10_SUPPORT	(1 << 2)
#define MPEG_BITRATE_IDX11_SUPPORT	(1 << 3)
#define MPEG_BITRATE_IDX12_SUPPORT	(1 << 4)
#define MPEG_BITRATE_IDX13_SUPPORT	(1 << 5)
#define MPEG_BITRATE_IDX14_SUPPORT	(1 << 6)

#define MAX_BITPOOL 64
#define MIN_BITPOOL 2

#define AAC_MPEG4_SCALABLE			(1 << 4)
#define AAC_MPEG4_LTP				(1 << 5)
#define AAC_MPEG4_LC				(1 << 6)
#define AAC_MPEG2_LC				(1 << 7)

// sampling_freq1 field
#define AAC_SAMPLING_FREQ_8000      (1 << 7)
#define AAC_SAMPLING_FREQ_11025		(1 << 6)
#define AAC_SAMPLING_FREQ_12000		(1 << 5)
#define AAC_SAMPLING_FREQ_16000		(1 << 4)
#define AAC_SAMPLING_FREQ_22050		(1 << 3)
#define AAC_SAMPLING_FREQ_24000		(1 << 2)
#define AAC_SAMPLING_FREQ_32000		(1 << 1)
#define AAC_SAMPLING_FREQ_44100		1

// sampling_freq2 field
#define AAC_SAMPLING_FREQ_48000		(1 << 3)
#define AAC_SAMPLING_FREQ_64000		(1 << 2)
#define AAC_SAMPLING_FREQ_88200		(1 << 1)
#define AAC_SAMPLING_FREQ_96000		1

#define AAC_CHANNEL_MODE_MONO		(1 << 1)
#define AAC_CHANNEL_MODE_STEREO		1

#define AAC_VBR_SUPPORT				1
// AAC media format is RFC 3016

#define ATRAC_VERSION_1				1
#define ATRAC_VERSION_2				2
#define ATRAC_VERSION_3				3

#define ATRAC_CHANNEL_MODE_MONO			(1 << 2)
#define ATRAC_CHANNEL_MODE_DUAL_CHANNEL	(1 << 1)
#define ATRAC_CHANNEL_MODE_JOINT_STEREO	1

#define ATRAC_SAMPLING_FREQ_44100		(1 << 1)
#define ATRAC_SAMPLING_FREQ_48000		1

#define ATRAC_VBR_SUPPORT				1

// bitrate1 field
#define ATRAC_BITRATE_IDX0_SUPPORT	(1 << 2)
#define ATRAC_BITRATE_IDX1_SUPPORT	(1 << 1)
#define ATRAC_BITRATE_IDX2_SUPPORT	1

// bitrate2 field
#define ATRAC_BITRATE_IDX3_SUPPORT	(1 << 7)
#define ATRAC_BITRATE_IDX4_SUPPORT	(1 << 6)
#define ATRAC_BITRATE_IDX5_SUPPORT	(1 << 5)
#define ATRAC_BITRATE_IDX6_SUPPORT	(1 << 4)
#define ATRAC_BITRATE_IDX7_SUPPORT	(1 << 3)
#define ATRAC_BITRATE_IDX8_SUPPORT	(1 << 2)
#define ATRAC_BITRATE_IDX9_SUPPORT	(1 << 1)
#define ATRAC_BITRATE_IDX10_SUPPORT	1

// bitrate3 field
#define ATRAC_BITRATE_IDX11_SUPPORT	(1 << 7)
#define ATRAC_BITRATE_IDX12_SUPPORT	(1 << 6)
#define ATRAC_BITRATE_IDX13_SUPPORT	(1 << 5)
#define ATRAC_BITRATE_IDX14_SUPPORT	(1 << 4)
#define ATRAC_BITRATE_IDX15_SUPPORT	(1 << 3)
#define ATRAC_BITRATE_IDX16_SUPPORT	(1 << 2)
#define ATRAC_BITRATE_IDX17_SUPPORT	(1 << 1)
#define ATRAC_BITRATE_IDX18_SUPPORT	1

#if __BYTE_ORDER == __LITTLE_ENDIAN

typedef struct {
	uint8_t channel_mode:4;
	uint8_t frequency:4;
	uint8_t allocation_method:2;
	uint8_t subbands:2;
	uint8_t block_length:4;
	uint8_t min_bitpool;
	uint8_t max_bitpool;
} __attribute__ ((packed)) a2dp_sbc_t;

typedef struct {
	uint8_t channel_mode:4;
	uint8_t crc:1;
	uint8_t layer:3;
	uint8_t frequency:6;
	uint8_t mpf2:1;
	uint8_t rfa:1;
	uint8_t bitrate1:7;
	uint8_t vbr:1;
	uint8_t bitrate2;
} __attribute__ ((packed)) a2dp_mpeg_t;

typedef struct {
	uint8_t object_type;
	uint8_t sampling_freq1;

	uint8_t rfa:2;
	uint8_t channels:2;
	uint8_t sampling_freq2:4;

	uint8_t bitrate_hi:7;
	uint8_t vbr:1;

	uint8_t bitrate_mid;
	uint8_t bitrate_lo;
} __attribute__ ((packed)) a2dp_aac_t;

typedef struct {
	uint8_t rfa1:2;
	uint8_t channel_mode:3;
	uint8_t version:3;

	uint8_t bitrate1:3;
	uint8_t vbr:1;
	uint8_t fs:2;
	uint8_t rfa2:2;

	uint8_t bitrate2;
	uint8_t bitrate3;
	uint8_t max_sul;
	uint8_t rfa3;
} __attribute__ ((packed)) a2dp_atrac_t;
#elif __BYTE_ORDER == __BIG_ENDIAN

typedef struct {
	uint8_t frequency:4;
	uint8_t channel_mode:4;
	uint8_t block_length:4;
	uint8_t subbands:2;
	uint8_t allocation_method:2;
	uint8_t min_bitpool;
	uint8_t max_bitpool;
} __attribute__ ((packed)) a2dp_sbc_t;

typedef struct {
	uint8_t layer:3;
	uint8_t crc:1;
	uint8_t channel_mode:4;
	uint8_t rfa:1;
	uint8_t mpf:1;
	uint8_t frequency:6;
	uint8_t vbr:1;
	uint8_t bitrate1:7;
	uint8_t bitrate2;
} __attribute__ ((packed)) a2dp_mpeg_t;

#else
#error "Unknown byte order"
#endif

#endif /* A2DP_CODECS_H_ */
