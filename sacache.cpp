/*
	Eddie Kaiger
	ECS 154A HW5
	Set Associative Mapping Cache
 */


#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;



/* CACHE LINE Class */
class CacheLine {
    
private:
    unsigned int tag;			// 8 bits long
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
    unsigned int getTag() const { return tag; }
    unsigned int isDirty() const { return dirty; }
    string getOffset(int offset) const { return data[offset]; }
    
    // Setters
    void setTag(unsigned int t) { tag = t; }
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
        output << "Tag:" << line.getTag() << " Dirty:" << line.isDirty()
        << " Data:" << line.getOffset(0) << line.getOffset(1)
        << line.getOffset(2) << line.getOffset(3) << endl;
        return output;
    }
};

/* SET Class */
class Set {
    
private:
    CacheLine lines[5];	// Cache lines in set
    int counters[5]; 	// Where 0 means line just updated
    
public:
    // Constructor
    Set() {
        for (int i = 0; i < 5; i++) {
            counters[i] = 4;
        }
    }
    
    // Retrieves least recently used line
    CacheLine* leastRecentlyUsedLine() {
        
        // Find index of least recently used line
        int lru_idx = 0;
        for (int i = 1; i < 5; i++) {
            if (counters[i] > counters[lru_idx]) {
                lru_idx = i;
            }
        }
        
        // Adjust counters
        counters[lru_idx] = 0;
        for (int i = 0; i < 5; i++) {
            if (i != lru_idx && counters[i] < 4) {
                counters[i]++;
            }
        }
        
        return &(lines[lru_idx]);
    }
    
    // Checks if set contains a line with the same tag
    int containsTag(unsigned int tag) {
        for (int i = 0; i < 5; i++) {
            if (tag == lines[i].getTag()) {
                return 1;
            }
        }
        return 0;
    }
    
    // Returns pointer to cache line that contains given tag
    CacheLine* lineForTag(unsigned int tag) {
        for (int i = 0; i < 5; i++) {
            if (tag == lines[i].getTag()) {
                return &(lines[i]);
            }
        }
        return 0;
    }

    void resetCounterWithTag(unsigned int tag) {
        // We want to bump up line whenever there's a cache hit
        for (int i = 0; i < 5; i++) {
            if (tag == lines[i].getTag()) {
                int prev_count = counters[i];
                counters[i] = 0;
                for (int j = 0; j < 5; j++) {
                    if (j != i && counters[j] <= prev_count) {
                        counters[j]++;
                    }
                }
                return;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    
    // Main memory = 2^16 = 65,536
    const int MEM_LEN = 65536;
    string memory[MEM_LEN];
    for (int i = 0; i < MEM_LEN; i++) {
        memory[i] = "00";
    }
    
    // Cache = 8 Sets, 5 lines each
    const int NO_OF_SETS = 8;
    Set cache[NO_OF_SETS];
    
    // Recognize input file
   	ifstream inputFile(argv[1]);
    
    // Create output file
    ofstream outputFile;
    outputFile.open("sa-mine-out.txt");
    
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
        
        // Generate tag, set, and offset
        unsigned int tag = (address >> 5);
        unsigned int set = (address >> 2) & 7;
        unsigned int offset = address & 3;
        
        cout << "Tag:" << tag << " Set:" << set << " Offset:" << offset << endl << endl;
        
        // Determine if we need to evict from cache
        
        int hit_miss = 1;
        
        if (!cache[set].containsTag(tag)) {
            
            // It's a miss
            hit_miss = 0;
            
            CacheLine *lru_line = cache[set].leastRecentlyUsedLine();
            
            if (lru_line->isDirty()) {
                
                // We get the starting point of where we want to put this in memory
                int offsetStart = (lru_line->getTag() << 5) | (set << 2);
                
                memory[offsetStart] = lru_line->getOffset(0);
                memory[offsetStart+1] = lru_line->getOffset(1);
                memory[offsetStart+2] = lru_line->getOffset(2);
                memory[offsetStart+3] = lru_line->getOffset(3);			
            }
            
            // Read line from RAM
            int ramAddrStart = address - offset;
            lru_line->setOffset(0, memory[ramAddrStart]);
            lru_line->setOffset(1, memory[ramAddrStart+1]);
            lru_line->setOffset(2, memory[ramAddrStart+2]);
            lru_line->setOffset(3, memory[ramAddrStart+3]);
            
            // Set tag, clear dirty
            lru_line->setTag(tag);
            lru_line->clearDirty();

        } else {
        	// Adjust counter
        	set[cache].resetCounterWithTag(tag);
        }
        
        if (rw == "FF") {	// WRITING 
            
            cache[set].lineForTag(tag)->write(data, offset);
            
        } else {			// READING
            
            cache[set].lineForTag(tag)->print(hit_miss, outputFile, offset);
        }
    }
    
    // Close files
    inputFile.close();
    outputFile.close();
    
    return 0;
}


