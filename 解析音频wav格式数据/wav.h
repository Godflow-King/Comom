/*
 * wav.h
 *
 *  Created on: 2018年2月1日
 *      Author: touch
 */

#ifndef INCLUDE_WAV_H_
#define INCLUDE_WAV_H_

#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
using namespace std;

#define FOURCC uint32_t
#define MAKE_FOURCC(a,b,c,d) \
( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )

template <char ch0, char ch1, char ch2, char ch3> struct MakeFOURCC{ enum { value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24) }; };

// Format chunk data field
struct Wave_format
{

	uint16_t format_tag;      // WAVE的数据格式，PCM数据该值为1
	uint16_t channels;        // 声道数
	uint32_t sample_per_sec;  // 采样率
	uint32_t bytes_per_sec;   // 码率，channels * sample_per_sec * bits_per_sample / 8
	uint16_t block_align;     // 音频数据块，每次采样处理的数据大小，channels * bits_per_sample / 8
	uint16_t bits_per_sample; // 量化位数，8、16、32等
	uint16_t ex_size;         // 扩展块的大小，附加块的大小
	Wave_format()
	{
		format_tag      = 1; // PCM format data
		ex_size         = 0; // don't use extesion field

		channels        = 0;
		sample_per_sec  = 0;
		bytes_per_sec   = 0;
		block_align     = 0;
		bits_per_sample = 0;
	}
	Wave_format(uint16_t nb_channel, uint32_t sample_rate, uint16_t sample_bits)
		:channels(nb_channel), sample_per_sec(sample_rate), bits_per_sample(sample_bits)
	{
		format_tag    = 0x01;                                           // PCM format data
		bytes_per_sec = channels * sample_per_sec * bits_per_sample / 8; // 码率
		block_align   = channels * bits_per_sample / 8;
		ex_size       = 0;                                               // don't use extension field
	}
};

// The basic chunk of RIFF file format
struct Base_chunk
{
	FOURCC fcc;    // FourCC id
	uint32_t cb_size; // 数据域的大小
	Base_chunk(FOURCC fourcc)
		: fcc(fourcc)
	{
		cb_size = 0;
	}
};

/*

	数据格式为PCM的WAV文件的基本结构
	--------------------------------
	| Base_chunk | RIFF	|
	---------------------
	|	WAVE            |
	---------------------
	| Base_chunk | fmt  |	Header
	---------------------
	| Wave_format|      |
	---------------------
	| Base_chunk | data |
	---------------------------------
	|    PCM data                   |
	---------------------------------
*/

/*

	数据格式为PCM的WAV文件头
	--------------------------------
	| Base_chunk | RIFF	|
	---------------------
	|	WAVE            |
	---------------------
	| Base_chunk | fmt  |	Header
	---------------------
	| Wave_format|      |
	---------------------
	| Base_chunk | data |
	--------------------------------
*/

struct Wave_header
{

	shared_ptr<Base_chunk> riff;
	FOURCC wave_fcc;
	shared_ptr<Base_chunk> fmt;
	shared_ptr<Wave_format>  fmt_data;
	shared_ptr<Base_chunk> data;
	Wave_header(uint16_t nb_channel, uint32_t sample_rate, uint16_t sample_bits)
	{
		riff      = make_shared<Base_chunk>(MakeFOURCC<'R', 'I', 'F', 'F'>::value);
		fmt       = make_shared<Base_chunk>(MakeFOURCC<'f', 'm', 't', ' '>::value);
		fmt->cb_size = 18;

		fmt_data  = make_shared<Wave_format>(nb_channel, sample_rate, sample_bits);
		data      = make_shared<Base_chunk>(MakeFOURCC<'d', 'a', 't', 'a'>::value);

		wave_fcc = MakeFOURCC<'W', 'A', 'V', 'E'>::value;
	}
	Wave_header()
	{
		riff         = nullptr;
		fmt          = nullptr;

		fmt_data     = nullptr;
		data         = nullptr;

		wave_fcc     = 0;
	}
};


class  CWaveFile
{
public:
	CWaveFile()
	{
		header = nullptr;
		data = nullptr;
	}
	virtual ~CWaveFile(){};
	// Write wav file
	bool static write(const string& filename, const Wave_header &header, void *data, uint32_t length);
	bool read(const string &filename);
private:
	// Read wav file header
	bool read_header(const string &filename);

public:
	shared_ptr<Wave_header> header;
	unique_ptr<uint8_t[]> data;
	Wave_format format;
};


#endif /* INCLUDE_WAV_H_ */
