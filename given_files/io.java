
class io {
    final static int CACHE_READ = 0;
    final static int CACHE_WRITE = 1;
    public static void main(String[] args) throws java.io.IOException {
	int i;
        int read_write;
        int address;
        int data; 

        int memory[] = new int[8192000];
 
	String parseString;
	String[] strings;
	byte c;
        // initialize memory
        for(i=0; i< 100; i++) {
	    memory[i] = i;
        }
 
	// Read until a newline
	while ((c = (byte)System.in.read()) != -1) {
	    parseString = "";

	    while(c != '\n') {
		parseString += (char)c;
		c = (byte)System.in.read();
	    }

	    strings = parseString.split("\\s");

	    if(strings.length < 2) {
		break;
	    }
	    // Read the first character of the line.
	    // It determines whether to read or write to the cache.
	    read_write = Integer.parseInt(strings[0]);

	    // check again if we have reached the end
	    // as this flag is set only after a 'cin'
	    //	    if(feof(stdin)) return 1;


	    // Read the address (as a hex number)
	    address = hex_to_int(strings[1]);

	    //if it is a cache write the we have to read the data
	    if(read_write == CACHE_WRITE) {
		
		data = hex_to_int(strings[2]);
		memory[address] = data;
		
		//output the new contents
	    } 
	    System.out.println("memory[" + address + "] = " + memory[address]);
	    
        }

    }

    public static String int_to_hex(int input_integer) {
	String hex_string;
	hex_string=Integer.toHexString(input_integer);
	while(hex_string.length()<8)
	    {
		hex_string = "0" + hex_string;
	    }
	
	return hex_string;
    }
 
    public static void printCacheAndMemory() {

	
	// Use this code to format and print your output
	/*
	System.out.println("STATISTICS");
	System.out.println("Misses:");
	System.out.println("Total: " + total_misses + " DataReads: " + readmisses + " DataWrites: " + writemisses);
	System.out.println("Miss rate:");
	System.out.println("Total: " + totalmr + " DataReads: " + readmr + " DataWrites: " + writemr);
	System.out.println("Number of Dirty Blocks Evicted from the Cache: " + numevicts);
	System.out.println("CACHE CONTENTS");
	System.out.println("Set   V   Tag      D     Word0     Word1     Word2     Word3     Word4     Word5     Word6     Word7");
	System.out.println(set + "  "+ v + "  " + int_to_hex(tag) + "  " + d + "  " + int_to_hex(word0) + "  " + int_to_hex(word1) + "  "  + int_to_hex(word2)
			   + "  " + int_to_hex(word3) + "  " + int_to_hex(word4) + "  " +  int_to_hex(word5) + "  " + int_to_hex(word6) + "  " + int_to_hex(word7));
	System.out.println();
	System.out.println("MAIN MEMORY:");
	System.out.println("Address   Words");
	System.out.println(int_to_hex(address) + "  " + int_to_hex(memword0) + "  " + int_to_hex(memword1) + "  "  + int_to_hex(memword2) + "  " + int_to_hex(memword3) + "  " + int_to_hex(memword4) + "  " +  int_to_hex(memword5) + "  " + int_to_hex(memword6) + "  " + int_to_hex(memword7));
	*/
    }

    public static int hex_to_int(String s) {
        String digits = "0123456789ABCDEF";
        s = s.toUpperCase();
        int val = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            int d = digits.indexOf(c);
            val = 16*val + d;
        }
        return val;
    }


}




