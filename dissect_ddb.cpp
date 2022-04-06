// SPDX-License-Identifier: MIT

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

using std::cout;
using std::cin;
using std::string;
using u32 = uint32_t;
using i32 = int32_t;
using i16 = int16_t;
using u8 = uint8_t;
using i8 = int8_t;

#define TOSTR reinterpret_cast<char*>

void xferToFile(std::ifstream &from, std::ofstream &to, i32 length, u8* buf, i32 bufferSize);

int main()
{
	string ddbPath, outPath;
	int opt;
	
	cout << "DDB File: ";
	std::getline(cin, ddbPath);
	cout << "Output path: ";
	std::getline(cin, outPath);
	cout << "Option 1:FRM2 2:SND 3:FRM2+SND >";
	cin >> opt;
	
	std::ifstream ddb;
	std::ofstream frm, snd;
	u32 segLength;
	constexpr int bufferSize = 4096 * 1024;
	u8 ddbIdent[4];
	static u8 buf[bufferSize];
	char readOffset[9];
	
	ddb.open(ddbPath, std::ios_base::binary | std::ios_base::in);
	
	if(!ddb)
		return cout << "Can't open DDB\n", 1;
	
	while(!ddb.eof())
	{
		ddb.read(TOSTR(ddbIdent), 4); // Fuck you C++
		ddb.read(TOSTR(&segLength), 4);
		
		if(!strncmp("FRM2", TOSTR(ddbIdent), 4))
		{
			// FRM2
			if(!(opt & 1))
			{
				ddb.seekg(segLength - 8, std::ios_base::cur);
				continue;
			}
			auto off = (int)ddb.tellg();
			sprintf(readOffset, "%X", off);
			frm.open(outPath + '/' + readOffset + ".frm2",
				std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
			if(!frm)
			{
				cout << "Can't open FRM2 <<" << readOffset << ">> output file.\n" << std::dec;
				ddb.seekg(segLength - 8, std::ios_base::cur);
				continue;
			}
			cout << "Write FRM2 at " << readOffset << ", length " << segLength << ".\n";
			xferToFile(ddb, frm, segLength - 8, buf, bufferSize);
			frm.close();
		}
		else if(!strncmp("SND ", TOSTR(ddbIdent), 4))
		{
			// SND
			if(!(opt & 2))
			{
				ddb.seekg(segLength - 14, std::ios_base::cur);
				continue;
			}
			sprintf(readOffset, "%X", (int)ddb.tellg());
			snd.open(outPath + '/' + readOffset + ".snd",
				std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
			if(!snd)
			{
				cout << "Can't open SND <<" << readOffset << ">> output file.\n" << std::dec;
				ddb.seekg(segLength - 14, std::ios_base::cur);
				continue;
			}
			cout << "Write SND at " << readOffset << ", length " << segLength << ".\n";
			i32 sampleRate;
			i16 channel;
			ddb.read(TOSTR(&sampleRate), 2);
			ddb.read(TOSTR(&channel), 4);
			snd << ddbIdent << segLength << sampleRate << channel;
			xferToFile(ddb, snd, segLength - 14, buf, bufferSize);
			snd.close();
		}
		else
		{
			cout << "Unidentified identifier <<" << std::hex << *(int*)ddbIdent << ">>\n\n\n";
			cout << "At position " << std::hex << ddb.tellg() << std::endl;
			return  1;
		}
	}
	
	return 0;
}

void xferToFile(std::ifstream &from, std::ofstream &to, i32 length, u8 *buf, i32 bufferSize)
{
	while(length > 0 && from)
	{
		from.read(TOSTR(buf), bufferSize > length ? length : bufferSize);
		to.write(TOSTR(buf), bufferSize > length ? length : bufferSize);
		
		length -= bufferSize;
	}
}
