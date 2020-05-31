//Author:Zhicun Chen
//Student ID:518030910173

#include "cachelab.h"
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct{
	int valid;
	int tag;
	int LRUnumber;
}Line;

typedef struct{
	Line* lines;
}Set;

typedef struct{
	int set_count;
	int line_count;
	Set* sets;
}Cache;

int misses=0,hits=0,evicts=0;
int parseCommand(int argc,char** argv,int* s,int *E,int* b,char* fileName,int* isVerbose);
void printHelpMenu();
int init_Cache(int s,int E,int b,Cache *cache);
void destroy_Cache(Cache *cache);
int updateLRU(Cache* cache,int setIndex,int lineIndex);
int findVictim(Cache* cache,int setIndex);
int isMiss(Cache* cache,int setIndex,int tag);
int updateCache(Cache* cache,int setIndex,int tag);
void executeLorS(Cache* cache,int setIndex,int tag,int isVerbose);
void executeM(Cache* cache,int setIndex,int tag,int isVerbose);
int getSet(int addr,int s,int b);
int getTag(int addr,int s,int b);


int main(int argc,char** argv)
{
	int s=0,E=0,b=0,isVerbose=0,addr,size;
	char fileName[50],opt[20];
	Cache cache;

	parseCommand(argc,argv,&s,&E,&b,fileName,&isVerbose);
	init_Cache(s,E,b,&cache);
	FILE* tracefile=fopen(fileName,"r");

	//do some format scan to get the useful information from trace.
	//variable isVerbose determines thether show details during simulation.
	while(fscanf(tracefile,"%s %x,%d",opt,&addr,&size)!=EOF){
		if(strcmp(opt,"I")==0)
			continue;
		int setIndex=getSet(addr,s,b);
		int tag=getTag(addr,s,b);
		if(isVerbose)
			printf("%s %x,%d",opt,addr,size);
		if((strcmp(opt,"L")==0)||(strcmp(opt,"S")==0))
			executeLorS(&cache,setIndex,tag,isVerbose);
		if(strcmp(opt,"M")==0)
			executeM(&cache,setIndex,tag,isVerbose);
		if(isVerbose)
			printf("\n");
	}

	printSummary(hits, misses, evicts);
	destroy_Cache(&cache);
    return 0;
}


int parseCommand(int argc,char** argv,int* s,int *E,int* b,char* fileName,int* isVerbose){
	int option;
	//use getopt() to simplify extract flags from inputting command.
	while((option=getopt(argc,argv,"hvs:E:b:t:"))!=-1)
	{
		switch(option)
		{
			case 'v':
				*isVerbose=1;
				break;
			case 's':
				*s=atoi(optarg);
				break;
			case 'E':
				*E=atoi(optarg);
				break;
			case 'b':
				*b=atoi(optarg);
				break;
			case 't':
				strcpy(fileName,optarg);
				break;
			case 'h':
			default:
				printHelpMenu();
				exit(0);
		}
	}
	return 1;
}

void printHelpMenu(){
	//just print the help menu.
	printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("\t-h         Print this help message.\n");
	printf("\t-v         Optional verbose flag.\n");
	printf("\t-s <num>   Number of set index bits.\n");
	printf("\t-E <num>   Number of lines per set.\n");
	printf("\t-b <num>   Number of block offset bits.\n");
	printf("\t-t <file>  Trace file.\n");
	printf("\nExamples:\n");
	printf("\tlinux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("\tlinux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace");
}

int init_Cache(int s,int E,int b,Cache *cache){
	//some simple error detection.
	if(s<0){
		printf("Cache set bits must be positive!\n");
		exit(0);
	}
	if(E<0){
		printf("Cache line numbers must be positive!\n");
		exit(0);
	}
	if(b<0){
		printf("Cache block offset bits must be positive!\n");
		exit(0);
	}
	//set the cache size.
	cache->set_count=(2<<s);
	cache->line_count=E;
	cache->sets=(Set*)malloc(sizeof(Set)*cache->set_count);
	//initialize a cache with gicen size.
	int i,j;
	for(i=0;i<cache->set_count;i++){
		cache->sets[i].lines=(Line*)malloc(sizeof(Line)*cache->line_count);
		for(j=0;j<cache->line_count;j++){
			cache->sets[i].lines[j].valid=0;
			cache->sets[i].lines[j].tag=0;
			cache->sets[i].lines[j].LRUnumber=-1;
		}
	}

	return 1;
}

void destroy_Cache(Cache* cache){
	// recall all space after execution.
	int i;
	for(i=0;i<cache->set_count;i++){
		free(cache->sets[i].lines);
	}
	free(cache->sets);
}

int updateLRU(Cache* cache,int setIndex,int lineIndex){
	//update LRU of all lines in one given set.
	int j;
	// decrease all lines' LRU.
	for(j=0;j<cache->line_count;j++)
		cache->sets[setIndex].lines[j].LRUnumber--;
	// set the chosen line's LRU to 0.
	cache->sets[setIndex].lines[lineIndex].LRUnumber=0;
	return 1;
}

int findVictim(Cache* cache,int setIndex){
	// when the set is full, this function is used to find a victim.
	int i,minIndex=0;
	int minLRU=1;
	// go through whole set to find the minimum LRU.
	for(i=0;i<cache->line_count;i++){
		if(cache->sets[setIndex].lines[i].LRUnumber<minLRU){
			minLRU=cache->sets[setIndex].lines[i].LRUnumber;
			minIndex=i;
		}
	}
	return minIndex;
}

int isMiss(Cache* cache,int setIndex,int tag){
	int i,isMiss=1;
	// find whether the tag is in the current cache.
	for(i=0;i<cache->line_count;i++){
		if(cache->sets[setIndex].lines[i].valid==1&&cache->sets[setIndex].lines[i].tag==tag){
			isMiss=0;
			// when find the tag, need to update the LRU of this set. 
			updateLRU(cache,setIndex,i);
			break;
		}
	}
	return isMiss;
}

int updateCache(Cache* cache,int setIndex,int tag){
	int i,isFull=1;
	//find whether the set is full.
	for(i=0;i<cache->line_count;i++){
		if(cache->sets[setIndex].lines[i].valid==0){
			isFull=0;
			break;
		}
	}
	//if it is full, need to find a victim.
	if(isFull){
		int victimIndex=findVictim(cache,setIndex);
		cache->sets[setIndex].lines[victimIndex].valid=1;
		cache->sets[setIndex].lines[victimIndex].tag=tag;
		updateLRU(cache,setIndex,victimIndex);
	}
	//if not, just insert it at the end.
	else{
		cache->sets[setIndex].lines[i].valid=1;
		cache->sets[setIndex].lines[i].tag=tag;
		updateLRU(cache,setIndex,i);
	}
	return isFull;
}

void executeLorS(Cache* cache,int setIndex,int tag,int isVerbose){
	//how to simulate L or S instructions.
	//isVerbose determines whether print details.
	if(isMiss(cache,setIndex,tag)==1){
		// deal with miss happens.
		misses++;
		if(isVerbose)
			printf(" miss");
		if(updateCache(cache,setIndex,tag)){
			evicts++;
			if(isVerbose)
				printf(" eviction");
		}
	}
	else{
		//deal with hit.
		hits++;
		if(isVerbose)
			printf(" hit");
	}
}

void executeM(Cache* cache,int setIndex,int tag,int isVerbose){
	// M is just one L and one S.
	executeLorS(cache,setIndex,tag,isVerbose);
	executeLorS(cache,setIndex,tag,isVerbose);
}

int getSet(int addr,int s,int b){
	return (addr>>b)&((1<<s)-1);
}

int getTag(int addr,int s,int b){
	return addr>>(s+b);
}