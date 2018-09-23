#include "wav.h"
using namespace std;

bool CWaveFile::write(const string& filename, const Wave_header &header, void *data, uint32_t length)
{
	ofstream ofs(filename, ofstream::binary);
	if (!ofs)
		return false;

	// Calculate size of RIFF chunk data
	header.data->cb_size = ((length + 1) / 2) * 2;
	header.riff->cb_size = 4 + 4 + header.fmt->cb_size + 4 + 4 + header.data->cb_size + 4;

	// Write header

	// Write RIFF
	char *chunk = (char*)header.riff.get();
	ofs.write(chunk, sizeof(Base_chunk));

	// Write WAVE fourcc
	ofs.write((char*)&(header.wave_fcc), 4);

	// Write fmt
	chunk = (char*)header.fmt.get();
	ofs.write(chunk, sizeof(Base_chunk));

	// Write fmt_data
	chunk = (char*)header.fmt_data.get();
	ofs.write(chunk, header.fmt->cb_size);

	// Write data
	chunk = (char*)header.data.get();
	ofs.write(chunk, sizeof(Base_chunk));

	// Write data
	ofs.write((char*)data, length);

	ofs.close();
	return true;
}

bool CWaveFile::read(const string &filename)
{
	if (!read_header(filename))
		return false;

	// PCM 数据相对文件头位置的偏移量， +8（RIFF fourcc +4，size + 4）
	uint32_t offset = header->riff->cb_size - header->data->cb_size + 8;
	data = unique_ptr<uint8_t[]>(new uint8_t[header->data->cb_size]);

	ifstream ifs(filename, ifstream::binary);
	if (!ifs)
		return false;

	ifs.seekg(offset);

	ifs.read((char*)(data.get()), header->data->cb_size);

	return true;
}

	// Read wav file header
bool CWaveFile::read_header(const string &filename)
{
	ifstream ifs(filename, ifstream::binary);
	if (!ifs)
		return false;

	header = make_shared<Wave_header>();

	// Read RIFF chunk
	FOURCC fourcc;
	ifs.read((char*)&fourcc, sizeof(FOURCC));

	if (fourcc != MakeFOURCC<'R', 'I', 'F', 'F'>::value) // 判断是不是RIFF
		return false;
	Base_chunk riff_chunk(fourcc);
	ifs.read((char*)&riff_chunk.cb_size, sizeof(uint32_t));

	header->riff = make_shared<Base_chunk>(riff_chunk);

	// Read WAVE FOURCC
	ifs.read((char*)&fourcc, sizeof(FOURCC));
	if (fourcc != MakeFOURCC<'W', 'A', 'V', 'E'>::value)
		return false;
	header->wave_fcc = fourcc;

	// Read format chunk
	ifs.read((char*)&fourcc, sizeof(FOURCC));
	if (fourcc != MakeFOURCC<'f', 'm', 't', ' '>::value)
		return false;

	Base_chunk fmt_chunk(fourcc);
	ifs.read((char*)&fmt_chunk.cb_size, sizeof(uint32_t));

	header->fmt = make_shared<Base_chunk>(fmt_chunk);

	// Read format data
	ifs.read((char*)&format, fmt_chunk.cb_size);

	// Read data chunk
	ifs.read((char*)&fourcc, sizeof(fourcc));
	if (fourcc != MakeFOURCC<'d', 'a', 't', 'a'>::value)
		return false;

	Base_chunk data_chunk(fourcc);
	ifs.read((char*)&data_chunk.cb_size, sizeof(uint32_t));

	header->data = make_shared<Base_chunk>(data_chunk);

	ifs.close();

	return true;
}

