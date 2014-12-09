/* 
	Eddie Kaiger
	ECS 154A HW5
	Direct Mapping Cache
*/


#include <iostream>
#include <fstream>

using namespace std;


/* CACHE LINE Class */
class CacheLine {

private:
	char tag;			// 8 bits long
	unsigned int dirty; // 0 or 1 (boolean)
	string data;		// 4 bytes (8 hex values)	

public:
	// Constructor
	CacheLine() {
		tag = -1;
		dirty = 0;
		data = "00000000";
	}

	// Getters
	char getTag() const { return tag; };
	unsigned int isDirty() const { return dirty; };
	string getData() const { return data; };

	// Setters
	void setTag(char t) { tag = t; };
	void setDirty(unsigned int d) { dirty = d; };
	void setData(string str) { data = str; };

	// Write
	void write(char tag, string data, unsigned int offset) {		
		dirty = 1;			// Set dirty bit
		this->tag = tag;	// Set tag

		// Replace the data corresponding to the offset
		this->data.replace(offset * 2, 2, data);
	}

	// Print
	void print(char newTag ostream& output, unsigned int offset) {
		int hit_miss = tag == newTag ? 1 : 0;
		string dataString = data.substr(offset * 2, 2);
		output << dataString << " " << hit_miss << " " << dirty << endl;
	}

	// Overrode << operator for debugging
	friend ostream& operator << (ostream& output, const CacheLine& line) {
		output << "Tag:" << (int)line.getTag() << " Dirty:" << line.getDirty() 
				<< " Data:" << line.getData() << endl;
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
		unsigned int address = stoul(addr, nullptr, 16);
		char tag = stoul(addr.substr(0,2), nullptr, 16);
		unsigned char line_offset = stoul(addr.substr(2,2), nullptr, 16);
		unsigned int offset = line_offset & 3;
		unsigned int line = ((int)line_offset >> 2);

		// Determine if we need to evict from cache
		if (tag != cache[line].getTag()) {
			// If it's dirty, we need to write back to memory first!
			if (cache[line].isDirty()) {

			}
		}



		// Determine if read or write

		if (rw == "FF") { 	// WRITING
			
			// Write to main memory
			memory[address] = data;

			// Write to cache			
			cache[line].write(tag, data, offset);
			
		} else {			// READING

			cache[line].print(tag, outputFile, offset);

			// If we get a miss, bring in line from memory
			if (cache[line].getTag() != tag) {
				
			}

		}

		cout << cache[line] << endl;

	}

	// Close files
	inputFile.close();
	outputFile.close();

	return 0;
}


