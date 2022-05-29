#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <vector>
std::string addrString;
std::ifstream addressFile;

struct CacheFactory
{
	/// <summary>
	/// Valid bit is whether the is data in the cache location or not
	/// lru is the "least Recently Used" (in this case also FIFo (first In First Out)
	/// Tag is the tag bits
	/// The 4 bytes are the offset 2 bits
	/// </summary>
	bool valid;
	bool lru;
	unsigned int tag;
	char bytes[4];

};

struct CacheFactory directM[16384];							//size determined by the size of the set bits (2^14)
struct CacheFactory assCache[16384];						//size determined by the size of the set bits (2^14)
struct CacheFactory twoWay0[16384];							//size determined by the size of the set bits (2^14)
struct CacheFactory twoWay1[16384];							//this is the the second way or way 1 in two way set associative 

unsigned int CLsDM = (sizeof(directM) / sizeof(*directM));	//Determines how many cache lines are needed for Direct Mapped
unsigned int CLsA = (sizeof(assCache) / sizeof(*assCache));	//Determines how many cache lines are needed for Associative Cache
unsigned int CLsTW = (sizeof(twoWay0) / sizeof(*twoWay0));	//Determines how many cache lines are needed for Two Way Associative Cache

//all lines of the cache locations made invalid to start off with
void startupDM();
void startupAC();
void startupTW();

void directMappedCache();			//Direct Mapped cache code
void associativeCache();			//associative cache
void twoWaySetAssociativeCache();	//Two Way set associative

void displayDirect(int);
void displayAssCache(int);
void displayTwoWay();

void cacheDisplay();


unsigned int MISS = 0, HIT = 0;	//Counts the misses and the hits, unsigned becuse no need for negative numbers
unsigned int cpuAddr = 0;		//Integer that represents the address from the text file
bool brk;	//if a break occurs
std::vector<int> associativeVec;

int addtag = 0, addset = 0, addCI = 0, addbyte = 0;	//used later to store the tag and set of the text files address output
int k = 0, l = 0;
int choice = 0; //for the switch statement

int main()
{
	cacheDisplay();
	while (1)
	{
		std::cout << "Please Choose one of the following options\n1.\tDirect Mapped\n2.\tAssociative Cache\n3.\tTwo Way Set Associative\n";
		std::cin >> choice;
		//system("CLS");
		switch (choice)
		{
		case 1:
			startupDM();				//performs the startup function for the direct mapped
			directMappedCache();		//performs the direct mapped simulation
			displayDirect(CLsDM);		//display for the direct mapped cache
			break;
		case 2:
			startupAC();				//performs the startup function for the fully associative mapped
			associativeCache();			//performs the fully associative simulation
			displayAssCache(CLsA);		//display for the fully associative cache
			break;
		case 3:
			startupTW();				//performs the startup function for the two way associative
			twoWaySetAssociativeCache();//performs the two way set associative simulation
			displayTwoWay();			//display for the two way set associative
			break;
		default:
			//in the instance of an error this will output
			std::cout << "Error, please choose an appropriate choice\n";
			break;
		}
	}


	//startup(CLs);	//start up has input parameter of cache lines
	//directMappedCache();	//Direct mapped cache code
	//AssociativeCache();
	//twoWaySetAssociativeCache();
}

void startupDM()
{
	//all lines of the cache locations made invalid to start off with
	for (int i = 0; i < CLsDM; i++)
	{
		directM[i].valid = false;	//all locations in the cache are made invalid initialy
		directM[i].lru = false;
	}
	std::cout << "All Cache Locations made invallid" << std::endl;	//informs the user fo the status of the cache
}

void startupAC()
{
	//all lines of the cache locations made invalid to start off with
	for (int i = 0; i < CLsA; i++)
	{
		assCache[i].valid = false;	//all locations in the cache are made invalid initialy
		assCache[i].lru = false;
	}
	std::cout << "All Cache Locations made invallid" << std::endl;	//informs the user fo the status of the cache
}

void startupTW()
{
	//all lines of the cache locations made invalid to start off with
	for (int i = 0; i < CLsTW; i++)
	{
		twoWay0[i].valid = false;	//all locations ini the cache are made invalid initialy
		twoWay0[i].lru = false;
		twoWay1[i].valid = false;	//all locations ini the cache are made invalid initialy
		twoWay1[i].lru = false;
	}
	std::cout << "All Cache Locations made invallid" << std::endl;	//informs the user fo the status of the cache
}

void directMappedCache()
{
	addressFile.open("addresses.txt");	//address file is opened
										//left outide of the for loop as this is how the
										//next line will be read instead of just the first
	for (int i = 0; i < 8; i++)
	{
		addressFile >> std::hex >> cpuAddr;	//next line of the text file enetered into "cpuAddr"

		/// <summary>
		/// address = 32 bits, tag = most significant 16 bits, set = next 14 bits, block offset = least significant 2 bits
		/// -------TAG-------
		/// address is bit shifted to the right 16 times so that only those top 16 bits are present
		/// it is then AND with ffff to identify the similarities
		/// EG...
		/// 00ff0004 -> shift right 16 -> 00ff -> AND with ffff -> 00ff is the output
		/// -----cache index CI-----
		/// The least significant bits bar the last 2 are what we want to extract
		/// last 2 bits to be gotted rid of, therefore and with fffC
		/// EG...
		/// 00ff0004 -> AND fffc (1111 1111 1111 1100) -> 0004 output
		/// shift to the right twice ang get an output of 0001
		/// </summary>
		addtag = (cpuAddr >> 16) & 0xffff;
		addCI = (cpuAddr & 0xfffC) >> 2;
		addbyte = cpuAddr & 0x0003;

		//if the tag bits match and the line is set to invalid
		if (directM[addCI].tag == addtag && directM[addCI].valid == true)
		{
			std::cout << "\n\tHIT\n" << std::endl;	//indicates to the user that there has been a hit
			HIT++;	//hit counter incremented by 1
		}
		//		 Set has no information in it
		else if (directM[addCI].valid == false)
		{

			directM[addCI].tag = addtag;	//the tag is updated
			//informs the user the location that has been replaced along with its tag
			std::cout << "Location " << addCI << " now has the tag of " << directM[addCI].tag << std::endl << std::endl;
			directM[addCI].valid = true;	//valid set as true indicating that there is data in this line of cache
			MISS++;	//miss counter incremented by 1
		}
		//Cache index has informationin it
		else
		{
			//Text to the user to inform about more of whats going on inside the cache
			std::cout << "\nLocation " << addCI << " was " << directM[addCI].tag << " but now ";
			directM[addCI].tag = addtag;	//the tag is updated
			//informs the user the location that has been replaced along with its tag
			std::cout << "Location " << addCI << " now has the tag of " << directM[addCI].tag << std::endl << std::endl;
			directM[addCI].valid = true;	//valid set as true indicating that there is data in this line of cache
			MISS++;	//miss counter incremented by 1
		}
		//output to help the user read what is going on
		std::cout << "----------------------------------------" << std::endl;
		std::cout << "Tag:\t" << std::hex << addtag << "\tSet:\t" << std::hex << addCI;	//tells user the the tag and set
		std::cout << "\tByte:\t" << std::hex << addbyte << std::endl;	//tells the user the offset
		std::cout << "\tHIT:\t" << HIT << "\tMISS:\t" << MISS << std::endl;		//Informs the user on the hit and miss status of the cache
		std::cout << "----------------------------------------" << std::endl;
	}
	addressFile.close();
}

void associativeCache()
{
	k = 0;
	addressFile.open("fullyasscache.txt");	//address file is opened
										//left outide of the for loop as this is how the
										//next line will be read instead of just the first

	for (int i = 0; i < 8; i++)
	{
		addressFile >> std::hex >> cpuAddr;	//next line of the text file enetered into "cpuAddr"
		/// <summary>
		/// address = 32 bits, tag = most significant 16 + 14 bits = 30 bits, block offset = least significant 2 bits
		/// -------TAG-------
		///The tag is comprised of the Tag and Set combined together from direct mapped
		/// </summary>
		addtag = (cpuAddr & 0xfffffffC) >> 2;
		addbyte = cpuAddr & 0x0003;
		//k is an integer that updates when there is a miss, this is different to i becuse i updates
		//whether there is a hit or a miss
		//l is the modulus of k from the cache size, meaning l will never reach a number over that of the cache size
		//l will also loop back to 0 once k reaches the cache size repaeting the cycle
		l = k % CLsA;

		if (associativeVec.size() < CLsA)
		{
			associativeVec.push_back(l);	//if the size of the vectors holding the
											//positions of the cache lines if less than 
											//the size of the cache, l is inserted into this vector
		}
		//this bool is used to signiy that there has been a hit
		brk = false;

		for (int j = 0; j < CLsA; j++)
		{
			if (assCache[j].tag == addtag && assCache[j].valid == true)
			{
				std::cout << "\n\tHIT\n" << std::endl;	//indicates to the user that there has been a hit
				HIT++;	//hit counter incremented by 1
				brk = true;	//if there has been a break, the brk has been set to true
				break;
			}
		}
		if (brk == false && associativeVec.size() < CLsA)
		{
			assCache[l].tag = addtag;	//the tag is updated
			//informs the user the location that has been replaced along with its tag
			std::cout << "Location " << l << " now has the tag of " << assCache[l].tag << std::endl << std::endl;
			assCache[l].valid = true;	//valid set as true indicating that there is data in this line of cache
			MISS++;	//miss counter incremented by 1
			k++;
		}
		else if (brk == false && associativeVec.size() >= CLsA)
		{
			std::cout << *associativeVec.begin() << std::endl;
			assCache[*associativeVec.begin()].tag = addtag;
			//informs the user the location that has been replaced along with its tag
			std::cout << "Location " << *associativeVec.begin() << " now has the tag of " << assCache[*associativeVec.begin()].tag << std::endl << std::endl;
			associativeVec.erase(associativeVec.begin());
			MISS++;	//miss counter incremented by 1
			k++;
		}
		std::cout << "----------------------------------------" << std::endl;
		std::cout << "Tag:\t" << std::hex << addtag;
		std::cout << "\tByte:\t" << std::hex << addbyte << std::endl;
		std::cout << "\tHIT:\t" << HIT << "\tMISS:\t" << MISS << std::endl;
		std::cout << "----------------------------------------" << std::endl;
	}
	addressFile.close();
}

void twoWaySetAssociativeCache()
{

	addressFile.open("twoWaySet.txt");

	for (int i = 0; i < 8; i++)
	{
		addressFile >> std::hex >> cpuAddr;	//next line of the text file enetered into "cpuAddr"
		addtag = (cpuAddr >> 16) & 0xffff;	//Tag number is the most significant 16 bits of the address
		addset = cpuAddr & 0xfff8 >> 2;	//Set is the next 14 bits after the tag
		addbyte = cpuAddr & 0x0003;	//byte is the least significant 2 bits
		//if either of the sets have the tag in question present, then ther is a hit
		if ((twoWay0[addset].tag == addtag && twoWay0[addset].valid == true) || (twoWay1[addset].tag == addtag && twoWay1[addset].valid == true))
		{
			std::cout << "\n\tHIT\n" << std::endl;	//indicates to the user that there has been a hit
			HIT++;	//hit counter incremented by 1
		}
		//if there is no data in way 0 set in quesstion, there is a miss
		else if (twoWay0[addset].valid == false)
		{
			twoWay0[addset].tag = addtag;	//the tag is updated
			//informs the user the location that has been replaced along with its tag
			std::cout << "Location " << addset << " now has the tag of " << addset << std::endl << std::endl;
			twoWay0[addset].valid = true;	//valid set as true indicating that there is data in this line of cache
			twoWay0[addset].lru = true;		//lru for way 0 is always set true before way 1
			MISS++;	//increment the miss counter
		}
		//if there is no data in way 1 set in quesstion, there is a miss
		else if (twoWay1[addset].valid == false)
		{
			twoWay1[addset].tag = addtag;	//the tag is updated
			//informs the user the location that has been replaced along with its tag
			std::cout << "Location " << addset << " now has the tag of " << addset << std::endl << std::endl;
			twoWay1[addset].valid = true;	//valid set as true indicating that there is data in this line of cache
			twoWay1[addset].lru = false;	//the lru for way 0 is presumed to be true so this must be false
			MISS++;	//increment the miss counter
		}
		//if both ways have data in the set being looked at and there is no hit, the next code applies
		else if (twoWay0[addset].valid == true && twoWay1[addset].valid == true)
		{
			//if the lru for way 0 is true
			if (twoWay0[addset].lru == true)
			{
				twoWay0[addset].tag = addtag;
				//informs the user the location that has been replaced along with its tag
				std::cout << "Location " << addset << " now has the tag of " << addset << std::endl << std::endl;
				twoWay0[addset].valid = true;	//valid set as true indicating that there is data in this line of cache
				twoWay1[addset].lru = true;		//the lru for the next way must be set true as this way has just been used
				twoWay0[addset].lru = false;	//this way is has just been used, removing the lru for this way
				MISS++;	//increment the miss counter
			}
			else
			{
				twoWay1[addset].tag = addtag;
				//informs the user the location that has been replaced along with its tag
				std::cout << "Location " << addset << " now has the tag of " << addset << std::endl << std::endl;
				twoWay1[addset].valid = true;	//valid set as true indicating that there is data in this line of cache
				twoWay0[addset].lru = true;		//the lru for the next way must be set true as this way has just been used
				twoWay1[addset].lru = false;	//this way is has just been used, removing the lru for this way
				MISS++;	//increment the miss counter
			}
		}
		std::cout << "----------------------------------------" << std::endl;
		std::cout << "Tag:\t" << std::hex << addtag << "\tSet:\t" << std::hex << addset;	//tells user the the tag
		std::cout << "\tByte:\t" << std::hex << addbyte << std::endl;	//tells the user the offset
		std::cout << "\tHIT:\t" << HIT << "\tMISS:\t" << MISS << std::endl;	//Informs the user on the hit and miss status of the cache
		std::cout << "----------------------------------------" << std::endl;
	}
	addressFile.close();
}

void displayDirect(int a)
{
	std::cout << "\tDIRECT MAPPED DISPLAY\n";
	for (int i = 0; i < a; i++)
	{	//displays all lines that are found to have data in them
		if (directM[i].valid == true)
		{
			std::cout << "----------------------------------------" << std::endl;
			std::cout << "Tag:\t" << std::hex << directM[i].tag << "\tSet:\t" << std::hex << i << std::endl;
			std::cout << "----------------------------------------" << std::endl;
		}
	}
	std::cout << "\tHIT:\t" << HIT << "\tMISS:\t" << MISS << std::endl;		//Informs the user on the hit and miss status of the cache
	HIT = 0;
	MISS = 0;
}

void displayAssCache(int a)
{
	std::cout << "\tASSOCIATIVE CACHE DISPLAY\n";
	for (int i = 0; i < a; i++)
	{	//displays all lines that are found to have data in them
		if (assCache[i].valid == true)
		{
			std::cout << "----------------------------------------" << std::endl;
			std::cout << "Tag:\t" << std::hex << assCache[i].tag << "\tSet:\t" << std::hex << i << std::endl;
			std::cout << "----------------------------------------" << std::endl;
		}
	}
	std::cout << "\tHIT:\t" << HIT << "\tMISS:\t" << MISS << std::endl;		//Informs the user on the hit and miss status of the cache
	HIT = 0;
	MISS = 0;
}

void displayTwoWay()
{
	std::cout << "\tTWO WAY SET ASSOCIATIVE DISPLAY\n";
	for (int i = 0; i < CLsTW; i++)
	{
		//displays all lines that are found to have data in them
		//if both way 0 and way 1 have data in the, a display is shown
		if (twoWay0[i].valid == true && twoWay1[i].valid == true)
		{
			std::cout << "----------------------------------------" << std::endl;
			std::cout << "\tWay 0\t\tWay1" << std::endl;
			std::cout << "Set:\t" << i << "Tag:\t" << std::hex << twoWay0[i].tag << "\tTag:\t" << std::hex << twoWay1[i].tag << std::endl;
			std::cout << "----------------------------------------" << std::endl;
		}
		//if only way 1 has data, way 1 is displayed only
		else if (twoWay0[i].valid == false && twoWay1[i].valid == true)
		{
			std::cout << "----------------------------------------" << std::endl;
			std::cout << "\tWay1" << std::endl;
			std::cout << "Set:\t" << i << "\tTag:\t" << std::hex << twoWay1[i].tag << std::endl;
			std::cout << "----------------------------------------" << std::endl;
		}
		//if only way 0 has data, way 0 is displayed only
		else if (twoWay0[i].valid == true && twoWay1[i].valid == false)
		{
			std::cout << "----------------------------------------" << std::endl;
			std::cout << "\tWay 0\t" << std::endl;
			std::cout << "Set:\t" << i << "\tTag:\t" << std::hex << twoWay0[i].tag << std::endl;
			std::cout << "----------------------------------------" << std::endl;
		}
	}
	std::cout << "\tHIT:\t" << HIT << "\tMISS:\t" << MISS << std::endl;		//Informs the user on the hit and miss status of the cache
	HIT = 0;
	MISS = 0;
}

void cacheDisplay()
{
	//Simple Display for the major details of the cache memories being moddeled
	std::cout << "Direct Mapped Cache" << std::endl;
	std::cout << "Addressable Lines:\t" << CLsDM << std::endl << std::endl;

	std::cout << "Fully Associative Cache" << std::endl;
	std::cout << "Addressable Lines:\t" << CLsA << std::endl << std::endl;

	std::cout << "2-Way set Associative" << std::endl;
	std::cout << "Addressable Lines:\t" << CLsTW * 2 << std::endl;
	std::cout << "No. Sets:\t" << CLsTW << std::endl << std::endl;
}
