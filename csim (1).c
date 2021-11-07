#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
//declare global variable
    int s;  
    int e;
    int b;
	int h;
	int v;
    char* trace;
	int B;// 2^b
	int E;//number of lines per set
	int S;//number of sets 2^s
    extern char *optarg; 
    extern int optind;

typedef struct{
//track the solution
	int num_hits;
	int num_misses;
	int num_evicts;
} cacheProperty;

typedef struct{
    long valid;
    unsigned long tag;
} line;

typedef struct{
    line** array;
} cache;

//initiates a cache
cache* make_cache(int S, int E){
    cache* new = malloc(sizeof(cache));
    new->array = (line**) malloc(S * (sizeof(line*)));
    if(new->array == NULL){
        fprintf(stderr, "cache empty");
        exit(1);
    }
    for(int i = 0; i < S; i++){
        new->array[i] = (line*) malloc(E * sizeof(line));
        for(int j = 0; j < E; j++){
            new->array[i][j].valid = 0;
            new->array[i][j].tag = 0;

        }
    
    }
    return new;

}
//calculates B
int calculate_B(int b) {
    return pow(2,b);

}
//calculates S
int calculate_S(int s) {
    return pow(2,s);

}
//amount of little ts
int tag_size(int s, int b){
    return 64-(s+b);
}

unsigned long mask(unsigned long num, int start, int end){
    unsigned long x = 0;
    for(unsigned int i = start - 1;i < end; i++){
        x|= 1 << i;
    }
    return (x & num) >> start;
}
//calulates the set index in cache
unsigned long set_index(unsigned long address){
    //return mask(address,(s + b),64);
    //return (address >> (64-s)) & (0x7FFFFFFF >> (64-s)); // check this
    unsigned long mask1 = (64-s-b);
    unsigned long mask2 = (64-s);
    unsigned long si = (address << mask1 >> mask2);
    return si;
}
//calculates the value of the tag
unsigned long tag_value(unsigned long address){
    //return mask(address,b,(b+s));
    //return (address >> b) & ~(0x1 << tag_size(s,b));
    unsigned long sum = (s + b);
    unsigned long tag  = address >> sum;
    return tag;
}

//if E = 5 this initiates an array 01234
int* LRU_array(){
    int* array = (int*) malloc(sizeof(int)*E);
    if(!array)
        return NULL;
    
    for(int j = 0; j < E;j++){
        array[j]=j;
    }
    return array;
}

//returns the index of the line to evict
//LRU(5,2)= (0,1,2,3,4)->(0,1,3,4,2) LRU-----MRU
//problem? my j for loop always sets it to 0,1,2,3,4 -- E whenever LRU is called
//01342 useing line index 3
//01324 LRU---MRU

//012345

//135024
//130245

//im looking for line 5
//give the index in the array

int line_i_arr(int* lru_arr,int line_num){
    for(int i = 0; i < E; i++){
        if(lru_arr[i]==line_num){
            return i;
        }
    }
    return -1;
}

void LRU_array_update(int* array, int line_num){
    int index_line = line_i_arr(array,line_num);
    int store = array[index_line];
    for(int i = index_line; i < (E-1);i++){
    array[i] = array[i+1];
    }
    array[E-1] = store;
    //return array[0];
}
//check if the set is full
int check_full(cache* c, int set_num){   
        for(int j; j<E; j++){
            if(c->array[set_num][j].valid == 0){
                return 0;
            }
        }
    return 1;
}

int next_empty_line(cache* c,int set_index){
    for(int i = 0; i < E; i++){
        if(c->array[set_index][i].valid == 0){
            return i;
        }
    }
    return -1;
}

cacheProperty* simulate(cache* c, cacheProperty* cp, unsigned long address,int** lru_array){
    //check if there is a hit
    int set_i = set_index(address);
    //printf("%d\n",set_i);
    int tag_v = tag_value(address);
    //int* lru_array = LRU_array(); // only want to call it once for every set
    int bool_hit = 0;

    //loops through all the lines to check if it is a hit or not
    for(int i = 0; i < E; i++ ){
    if((c->array[set_i][i].tag == tag_v) && (c->array[set_i][i].valid == 1)){
        cp->num_hits += 1;
        printf("hit\n");
        bool_hit = 1;
        LRU_array_update(lru_array[set_i],i); //updates the line
        break;
        }
    }
    //if there is no hit and there is an empty line ie a line with valid == 0
    if (bool_hit == 0 && check_full(c,set_i)==0){
        cp->num_misses += 1;
        printf("miss\n");
        int add_tag = next_empty_line(c,set_i);
        c->array[set_i][add_tag].tag = tag_v;
        c->array[set_i][add_tag].valid = 1;
        LRU_array_update(lru_array[set_i],add_tag); //update the LRU array

    }
    //no empty spaces so you have to evict
    else if(bool_hit == 0 && check_full(c,set_i)==1){
        cp->num_misses += 1;
        printf("miss\n");
        cp->num_evicts += 1;
        printf("evict\n");
        int evict_line_num = lru_array[set_i][0];
        c->array[set_i][evict_line_num].tag = tag_v;
        c->array[set_i][evict_line_num].valid = 1;
        LRU_array_update(lru_array[set_i],evict_line_num);
        }
    
        return cp;

}  
int main(int argc, char** argv)
{
    //initiate the char to store the trace
    //char* trace;
    int input;
    //parse input
    printf("a\n");
    while((input=getopt(argc,argv,"hvs:E:b:t:")) != -1){
        switch(input){
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace = optarg;
            break;
        case 'v':
            v = 1;
            break;
        case 'h':
            h = 1;
            break;
      
        }
    }
    //initialise cache
    S = calculate_S(s);
    B = calculate_B(b);
    cache* cache = make_cache(S,E);
    cacheProperty cp;
    cp.num_hits = 0;
	cp.num_misses = 0;
	cp.num_evicts = 0;

    //printf("s:%d E:%d b:%d",s,E,b);
    int** twod_lru_arr = (int**) malloc(sizeof(int*) * S);
    for(int i = 0; i < S; i++){
        twod_lru_arr[i] = LRU_array();
    }

    char fl;
    unsigned long addr;
    int size;
    FILE* file;
    char buff[500];
    file = fopen(trace,"r");
    if(file == NULL){
        printf("something weird is going on");
    }
    //printf("m\n");
    while(fgets(buff,500,file)!= NULL){
        //printf("%s",fgets(buff,500,file));
        if(buff[0]=='I')
            ;
        else{
            //simulate cache
            //printf("before scanf\n");
            sscanf(buff," %c %lx,%d",&fl,&addr,&size);
                simulate(cache,&cp,addr,twod_lru_arr);
            if(fl == 'M'){
                simulate(cache,&cp,addr,twod_lru_arr);
            }
        }
    }
   printf("end");
    fclose(file);
    printSummary(cp.num_hits,cp.num_misses,cp.num_evicts);
    return 0;
}