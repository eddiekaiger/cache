/* 
	Eddie Kaiger
	ECS 154A HW5
	Direct Mapping Cache
*/


#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;


/* CACHE LINE Class */
class CacheLine {

private:
	char tag;			// 8 bits long
	unsigned int dirty;	// 0 or 1 (boolean)
	string data[4];		// 4 bytes (8 hex values)	

public:
	// Constructor
	CacheLine() {
		tag = -1;
		dirty = 0;
		for (int i = 0; i < 4; i++) {
			data[i] = "00";
		}
	}

	// Getters
	char getTag() const { return tag; }
	unsigned int isDirty() const { return dirty; }
	string getOffset(int offset) const { return data[offset]; }

	// Setters
	void setTag(char t) { tag = t; }
	void setDirty() { dirty = 1; }
	void clearDirty() { dirty = 0; }
	void setOffset(int offset, string str) { data[offset] = str; }

	// Write
	void write(string data, unsigned int offset) {		
		setDirty();
		// Replace the data corresponding to the offset
		this->data[offset] = data;
	}

	// Print
	void print(int hit_miss, ostream& output, unsigned int offset) {
		output << getOffset(offset) << " " << hit_miss << " " << dirty << endl;
	}

	// Overrode << operator for debugging
	friend ostream& operator << (ostream& output, const CacheLine& line) {
		output << "Tag:" << (int)(unsigned char)line.getTag() << " Dirty:" << line.isDirty() 
				<< " Data:" << line.getOffset(0) << line.getOffset(1) 
				<< line.getOffset(2) << line.getOffset(3) << endl;
		return output;
	}
};

int main(int argc, char *argv[]) {

	// Main memory = 2^16 = 65,536
	const int MEM_LEN = 65536;
	string memory[MEM_LEN];
	for (int i = 0; i < MEM_LEN; i++) {
		memory[i] = "00";
	}

	// Cache = 64 CacheLines
	const int CACHE_LEN = 64;
	CacheLine cache[CACHE_LEN];

	// Recognize input file
	ifstream inputFile(argv[1]);

	// Create output file
	ofstream outputFile;
	outputFile.open("dm-mine-out.txt");

	// Read input data
	string str;
	while (getline (inputFile, str)) {

		// CacheLine Format = ADDR RW DT

		cout << str << endl;

		string addr = str.substr(0,4);
		string rw 	= str.substr(5,2);
		string data = str.substr(8,2);

		// Decode address string

		stringstream ss;
		ss << hex << addr;
		unsigned int address;
		ss >> address;

		// Generate tag, line & offset
		char tag = (address >> 8);
		unsigned int line = (address & 0xFF) >> 2;
		unsigned int offset = address & 3;

		// Determine if we need to evict from cache

		int hit_miss = 1;

		if (tag != cache[line].getTag()) {

			// It's a miss
			hit_miss = 0;

			// If previous line is dirty, we need to write back to memory first!
			if (cache[line].isDirty()) {
				
				// we need old tag's address
				int offsetStart = (((int)(unsigned char)(cache[line].getTag())) << 8) | (line << 2);

				cout << "Offset Start: " << offsetStart << endl;

				memory[offsetStart] = cache[line].getOffset(0);
				memory[offsetStart+1] = cache[line].getOffset(1);
				memory[offsetStart+2] = cache[line].getOffset(2);
				memory[offsetStart+3] = cache[line].getOffset(3);
			}

			// Read line from RAM
			int ramAddrStart = address - offset;
			cache[line].setOffset(0, memory[ramAddrStart]);
			cache[line].setOffset(1, memory[ramAddrStart+1]);
			cache[line].setOffset(2, memory[ramAddrStart+2]);
			cache[line].setOffset(3, memory[ramAddrStart+3]);

			// Set tag, clear dirty
			cache[line].setTag(tag);
			cache[line].clearDirty();

		}

		// Determine if read or write

		if (rw == "FF") { 	// WRITING

			// Write to cache			
			cache[line].write(data, offset);
			
		} else {			// READING

			cache[line].print(hit_miss, outputFile, offset);

		}
	}

	// Close files
	inputFile.close();
	outputFile.close();

	return 0;
}


