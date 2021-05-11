#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifndef SWITCH_CASE_INIT
#define SWITCH_CASE_INIT
#define SWITCH(X)	for (char* __switch_p__ = X, __switch_next__=1 ; __switch_p__ ; __switch_p__=0, __switch_next__=1) { {
#define CASE(X)			} if (!__switch_next__ || !(__switch_next__ = strcmp(__switch_p__, X))) {
#define DEFAULT		  	} {
#define END			}}
#endif

#define BLOCK_SUPER		0
#define BLOCK_BLOCK_BITMAP	1
#define BLOCK_INODE_BITMAP	2
#define BLOCK_INODE_TABLE	3
#define BLOCK_MAX		99
#define INODE_MAX		127

// structure of an inode entry
typedef struct {
	char TT[2];		// entry type; "DI" means directory and "FI" means file
	char XX[2],YY[2],ZZ[2];	// the blocks for this entry; 00 means not used
} _inode_entry;

// structure of a directory entry
typedef struct {
	char F;			// '1' means used; '0' means unused
	char fname[252];	// name of this entry; remember to include null character into it
	char MMM[3];		// inode entry index which holds more info about this entry
} _directory_entry;

// SFS metadata; read during mounting
int BLB;					// total number of blocks
int INB;					// total number of entries in inode table
char _block_bitmap[1024];			// the block bitmap array
char _inode_bitmap[1024];			// the inode bitmap array
_inode_entry _inode_table[128];			// the inode table containing 128 inode entries

// useful info
int free_disk_blocks;				// number of available disk blocks
int free_inode_entries;				// number of available entries in inode table
int CD_INODE_ENTRY = 0;				// index of inode entry of the current directory in the inode table
char current_working_directory[252]="/";	// name of current directory (useful in the prompt)


FILE *df = NULL;				// THE DISK FILE

// function declarations
// HELPERS
int stoi(char *, int);
void itos(char *, int, int);
void printPrompt();

// DISK ACCESS
void mountSFS();
int readSFS(int, char *);
int writeSFS(int, char *);

// BITMAP ACCESS
int getBlock();
void returnBlock(int);
int getInode();
void returnInode(int);

// COMMANDS
void ls();
void rd();
void cd(char *);
void md(char *);
void stats();

void display(char *);
void create(char *);
void remfd(char *);
int removeFile(int);
int removeDirectory(int);

/*############################################################################*/
/****************************************************************************/
/* returns the integer value of string s; -1 on error
/*
/****************************************************************************/
int main() {
    mountSFS();
    char command[1024], file_name[1024];
    
    while(1) {
    	printPrompt();
    	scanf("%s", command);
    	SWITCH (command)
    		CASE ("exit")
    			return 0;
    			break;
    		CASE ("ls")
    			ls();
    			break;
    		CASE ("cd")
    			scanf("%s", file_name);
    			cd(file_name);
    			break;
    		CASE ("md")
    			scanf("%s", file_name);
    			md(file_name);
    			break;
    		CASE ("rd")
    			rd();
    			break;
    		CASE ("stats")
    			stats();
    			break;
    		CASE ("display")
    			scanf("%s", file_name);
    			display(file_name);
    			break;
    		CASE ("create")
    			scanf("%s", file_name);
	            create(file_name);
	            break;
	        CASE ("rm")
	        	scanf("%s", file_name);
	            remfd(file_name);
	            break;
    		DEFAULT
    			printf("\'%s\' Command not found\n", command);
    		END
    }
    return 0;
}

int stoi(char *s, int n) {
	int i;
	int ret = 0;
	
	for (i=0; i<n; i++) {
		if (s[i] < 48 || s[i] > 57) return -1; // non-digit
		ret+=pow(10,n-i-1)*(s[i]-48);
	}

	return ret;
}

/****************************************************************************/
/* returns the string representation of num in s
/* n is the width of the number; 0 padded if required
/*
/****************************************************************************/

void itos(char *s, int num, int n) {
	char st[1024];
	sprintf(st,"%0*d",n,num);
	strncpy(s,st,n);	
}

/****************************************************************************/
/* prints a prompt with current working directory
/*
/****************************************************************************/

void printPrompt() {
	printf("SFS::%s# ",current_working_directory);
}

/*############################################################################*/
/****************************************************************************/
/* reads SFS metadata into memory structures
/* 
/****************************************************************************/

void mountSFS() {
	int i;
	char buffer[1024];
	
	df = fopen("sfs.disk","r+b");
	if (df == NULL) {
		printf("Disk file sfs.disk not found.\n");
		exit(1);
	}
	// read superblock
	fread(buffer, 1, 1024, df);
	BLB = stoi(buffer,3); 
	INB = stoi(buffer+3,3);
	
	// read block bitmap
	fread(_block_bitmap, 1, 1024, df);
	// initialize number of free disk blocks
	free_disk_blocks = BLB; for (i=0; i<BLB; i++) free_disk_blocks-=(_block_bitmap[i]-48);
	
	// read inode bitmap
	fread(_inode_bitmap, 1, 1024, df);
	// initialize number of unused inode entries
	free_inode_entries = INB; for (i=0; i<INB; i++) free_inode_entries-=(_inode_bitmap[i]-48);
	
	// read the inode table
	fread(_inode_table, 1, 1024, df);
}

/****************************************************************************/
/* reads a block of data from disk file into buffer
/* returns 0 if invalid block number
/*
/****************************************************************************/

int readSFS(int block_number, char buffer[1024]) {
	
	if (block_number < 0 || block_number > BLOCK_MAX) return 0;
	
	if (df == NULL) mountSFS();	// trying to read without mounting...!!!
	
	fseek(df,block_number*1024,SEEK_SET);	// set file pointer at right position
	fread(buffer, 1, 1024, df);		// read a block, i.e. 1024 bytes into buffer
	
	return 1;
}

/****************************************************************************/
/* writes a block of data from buffer to disk file
/* if buffer is null pointer, then writes all zeros
/* returns 0 if invalid block number
/*
/****************************************************************************/
	
int writeSFS(int block_number, char buffer[1024]) {
	char empty_buffer[1024];
	
	if (block_number < 0 || block_number > BLOCK_MAX) return 0;
	
	if (df == NULL) mountSFS();	// trying to write without mounting...!!!
	
	fseek(df,block_number*1024,SEEK_SET);	// set file pointer at right position
	
	if (buffer==NULL) {				// if buffer is null
		memset(empty_buffer,'0',1024);		
		fwrite(empty_buffer, 1, 1024, df); 	// write all zeros
	}
	else 
		fwrite(buffer, 1, 1024, df);
		
	fflush(df);	// making sure disk file is always updated
	
	return 1;
}


/*############################################################################*/
/****************************************************************************/
/* finds the first available block using the block bitmap
/* updates the bitmap
/* writes the block bitmap to disk file
/* returns -1 on error; otherwise the block number
/*
/****************************************************************************/

int getBlock() {
	int i;
	
	if (free_disk_blocks == 0) return -1;
	
	for (i=0; i<BLB; i++) if (_block_bitmap[i]=='0') break;	// 0 means available
	
	_block_bitmap[i]='1';
	free_disk_blocks--;
	
	writeSFS(BLOCK_BLOCK_BITMAP,_block_bitmap);
	
	return i;
}

/****************************************************************************/
/* updates block bitmap when a block is no longer used
/* blocks 0 through 3 are treated special; so they are always in use
/*
/****************************************************************************/

void returnBlock(int index) {
	if (index > 3  && index <= BLOCK_MAX) {
		_block_bitmap[index]='0';
		free_disk_blocks++;
		
		writeSFS(BLOCK_BLOCK_BITMAP,_block_bitmap);
	}
}

/****************************************************************************/
/* finds the first unused position in inode table using the inode bitmap
/* updates the bitmap
/* writes the inode bitmap to disk file
/* returns -1 if table is full; otherwise the position
/*
/****************************************************************************/

int getInode() {
	int i;
	
	if (free_inode_entries == 0) return -1;
	
	for (i=0; i<INB; i++) if (_inode_bitmap[i]=='0') break;	// 0 means available
	
	_inode_bitmap[i]='1';
	free_inode_entries--;
	
	writeSFS(BLOCK_INODE_BITMAP,_inode_bitmap);
		
	return i;
}

/****************************************************************************/
/* updates inode bitmap when an inode entry is no longer used
/*
/****************************************************************************/

void returnInode(int index) {
	if (index > 0  && index <= INODE_MAX) {
		_inode_bitmap[index]='0';
		free_inode_entries++;			
		
		writeSFS(BLOCK_INODE_BITMAP,_inode_bitmap);
	}
}
	
	
/*############################################################################*/
/****************************************************************************/
/* makes root directory the current directory 
/*
/****************************************************************************/

void rd() {
	CD_INODE_ENTRY = 0; // first inode entry is for root directory
	current_working_directory[0]='/'; 
	current_working_directory[1]=0;
}

/****************************************************************************/
/* lists all files and directories in the current directory 
/*
/****************************************************************************/

void ls() {
	char itype;
	int blocks[3];
	_directory_entry _directory_entries[4];		
	
	int total_files=0, total_dirs=0;
	
	int i,j;
	int e_inode;

	// read inode entry for current directory
	// in SFS, an inode can point to three blocks at the most		
	itype = _inode_table[CD_INODE_ENTRY].TT[0];
	blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);

	// its a directory; so the following should never happen		
	if (itype=='F') {
		printf("Fatal Error! Aborting.\n");
		exit(1);
	}
	
	// lets traverse the directory entries in all three blocks
	for (i=0; i<3; i++) {
		if (blocks[i]==0) continue;	// 0 means pointing at nothing
		
		readSFS(blocks[i],(char *)_directory_entries);	// lets read a directory entry; notice the cast
		
		// so, we got four possible directory entries now		
		for (j=0; j<4; j++) {
			if (_directory_entries[j].F=='0') continue;	// means unused entry

			e_inode = stoi(_directory_entries[j].MMM,3);	// this is the inode that has more info about this entry
			
			if (_inode_table[e_inode].TT[0]=='F')  { // entry is for a file
				printf("%.252s\t",_directory_entries[j].fname);
				total_files++;
			}
			else if (_inode_table[e_inode].TT[0]=='D')  { // entry is for a directory; print it in BRED
				printf("\e[1;31m%.252s\e[;;m\t",_directory_entries[j].fname);	
				total_dirs++;
			}
		}
	}	
	
	printf("\n%d file%c and %d director%s.\n",total_files,(total_files<=1?0:'s'),total_dirs,(total_dirs<=1?"y":"ies"));
}

/****************************************************************************/
/* moves into the directory <dname> within the current directory if 
/* it exists
/*
/****************************************************************************/

void cd(char *dname) {
	char itype;
	int blocks[3];
	_directory_entry _directory_entries[4];		
	
	int i,j;
	int e_inode;
	
	char found=0;
	
	// read inode entry for current directory
	// in SFS, an inode can point to three blocks at the most	
	itype = _inode_table[CD_INODE_ENTRY].TT[0];
	blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);
	
	// its a directory; so the following should never happen	
	if (itype=='F') {
		printf("Fatal Error! Aborting.\n");
		exit(1);
	}
	
	// now lets try to see if a directory by the name already exists	
	for (i=0; i<3; i++) {
		if (blocks[i]==0) continue;	// 0 means pointing at nothing
		
		readSFS(blocks[i],(char *)_directory_entries);	// lets read a directory entry; notice the cast
		
		// so, we got four possible directory entries now
		for (j=0; j<4; j++) {
			if (_directory_entries[j].F=='0') continue;	// means unused entry
			
			e_inode = stoi(_directory_entries[j].MMM,3);	// this is the inode that has more info about this entry
			
			if (_inode_table[e_inode].TT[0]=='D') { // entry is for a directory; can't cd into a file, right?
				if (strncmp(dname,_directory_entries[j].fname, 252) == 0) {	// and it is the one we are looking for
					found = 1;	// VOILA
					break;
				}
			}
		}
		if (found) break; // no need to search more	
	}	
	
	if (found) {	
		CD_INODE_ENTRY = e_inode;	// just keep track of which inode entry in the table corresponds to this directory
		strncpy(current_working_directory,dname,252);	// can use it in the prompt
	}
	else {
		printf("%.252s: No such directory.\n",dname);
	}
}

/****************************************************************************/
/* creates a new directory called <dname> in the current directory if the 
/* name is not already taken and there is still space available
/*
/****************************************************************************/

void md(char *dname) {
	char itype;
	int blocks[3];
	_directory_entry _directory_entries[4];		
	
	int i,j;
	
	int empty_dblock=-1,empty_dentry=-1;
	int empty_ientry;
	
	// non-empty name
	if (strlen(dname)==0) {
		printf("Usage: md <directory name>\n");
		return;
	}
	
	// do we have free inodes
	if (free_inode_entries == 0) {
		printf("Error: Inode table is full.\n");
		return;
	}
	
	// read inode entry for current directory
	// in SFS, an inode can point to three blocks at the most
	itype = _inode_table[CD_INODE_ENTRY].TT[0];
	blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX,2);
	blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY,2);
	blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ,2);
	
	// its a directory; so the following should never happen
	if (itype=='F') {
		printf("Fatal Error! Aborting.\n");
		exit(1);
	}
	
	// now lets try to see if the name already exists
	for (i=0; i<3; i++) {
		if (blocks[i]==0) { 	// 0 means pointing at nothing
			if (empty_dblock==-1) empty_dblock=i; // we can later add a block if needed
			continue;
		}	
		
		readSFS(blocks[i],(char *)_directory_entries); // lets read a directory entry; notice the cast
		
		// so, we got four possible directory entries now
		for (j=0; j<4; j++) {
			if (_directory_entries[j].F=='0') { // means unused entry
				if (empty_dentry==-1) { empty_dentry=j; empty_dblock=i; } // AAHA! lets keep a note of it, just in case we have to create the new directory
				continue;
			}
			
			if (strncmp(dname,_directory_entries[j].fname, 252) == 0) { // compare with user given name
					printf("%.252s: Already exists.\n",dname); 
					return;
			}
		}
		
	}
	// so directory name is new	
	
	// if we did not find an empty directory entry and all three blocks are in use; then no new directory can be made
	if (empty_dentry==-1 && empty_dblock==-1) {
		printf("Error: Maximum directory entries reached.\n");
		return;
	}
	else { // otherwise
		if (empty_dentry == -1) { // Great! didn't find an empty entry but not all three blocks have been used
			empty_dentry=0;
			
			if ((blocks[empty_dblock] = getBlock())==-1) {  // first get a new block using the block bitmap
				printf("Error: Disk is full.\n");
				return;
			}						
			
			writeSFS(blocks[empty_dblock],NULL);	// write all zeros to the block (there may be junk from the past!)
			
			switch(empty_dblock) {	// update the inode entry of current dir to reflect that we are using a new block
				case 0: itos(_inode_table[CD_INODE_ENTRY].XX,blocks[empty_dblock],2); break;
				case 1: itos(_inode_table[CD_INODE_ENTRY].YY,blocks[empty_dblock],2); break;
				case 2: itos(_inode_table[CD_INODE_ENTRY].ZZ,blocks[empty_dblock],2); break;
			}
		}
		

		// NOTE: all error checkings have already been done at this point!!
		// time to put everything together
				
		empty_ientry = getInode();	// get an empty place in the inode table which will store info about blocks for this new directory
		
		readSFS(blocks[empty_dblock],(char *)_directory_entries);	// read block of current directory where info on this new directory will be written
		_directory_entries[empty_dentry].F = '1';			// remember we found which directory entry is unused; well, set it to used now
		strncpy(_directory_entries[empty_dentry].fname,dname,252);	// put the name in there
		itos(_directory_entries[empty_dentry].MMM,empty_ientry,3);	// and the index of the inode that will hold info inside this directory
		writeSFS(blocks[empty_dblock],(char *)_directory_entries);	// now write this block back to the disk
		
		strncpy(_inode_table[empty_ientry].TT,"DI",2);		// create the inode entry...first, its a directory, so DI
		strncpy(_inode_table[empty_ientry].XX,"00",2);		// directory is just created; so no blocks assigned to it yet 
		strncpy(_inode_table[empty_ientry].YY,"00",2);		
		strncpy(_inode_table[empty_ientry].ZZ,"00",2);
		
		writeSFS(BLOCK_INODE_TABLE, (char *)_inode_table);	// phew!! write the inode table back to the disk
	}				
}

/****************************************************************************/
/* prints number of free blocks in the disk and free inode entries in the inode table
/*
/****************************************************************************/

void stats() {
	int blocks_free=BLB, inodes_free=INB;
	int i;
	
	for (i=0; i<BLB; i++) blocks_free-=(_block_bitmap[i]-48);
	for (i=0; i<INB; i++) inodes_free-=(_inode_bitmap[i]-48);

	printf("%d block%c free.\n",blocks_free,(blocks_free<=1?0:'s'));
	printf("%d inode entr%s free.\n",inodes_free,(inodes_free<=1?"y":"ies"));
}
		
void display(char *fname){
    char itype;
    char buffer[1024];
    
    itype = _inode_table[CD_INODE_ENTRY].TT[0];
    if(itype == 'F'){
        printf("Display Error: CD_INODE is a file.\n");
        exit(1);
    }
    
    int blocks[3], target_inode = -1;
    blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX, 2);
    blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY, 2);
    blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ, 2);

    for(int i=0; i<3; i++){
        _directory_entry _directory_entries[4];
        readSFS(blocks[i], (char *)_directory_entries);        
        for(int j=0; j<4; j++){   
            if(_directory_entries[j].F == '1'){
                if(strcmp(_directory_entries[j].fname, fname) == 0){
                    itype = _inode_table[target_inode].TT[0];
                    if(itype == 'D')
                        continue;
                    target_inode = stoi(_directory_entries[j].MMM, 3);
                    break;
                }
            }

        }
    }

    if(target_inode == -1){
        printf("File not found in current directory\n");
        return;
    }
    else{
        blocks[0] = stoi(_inode_table[target_inode].XX, 2);
        blocks[1] = stoi(_inode_table[target_inode].YY, 2);
        blocks[2] = stoi(_inode_table[target_inode].ZZ, 2);

        for(int i=0; i<3; i++){
            if(blocks[i] == 0)
                break;
            readSFS(blocks[i], buffer);
            printf("%s", buffer);
        }
        printf("\n");
    }
}

void create(char *fname){
    char itype;
    int blocks[3], i, j, k, empty_block_indx = -1, dir_entry_indx = -1, create_new_block = 0;
    int new_inode;

    char buffer[1024];
    
    itype = _inode_table[CD_INODE_ENTRY].TT[0];
    if(itype == 'F'){
        printf("Create Error: CD_INODE is a file.\n");
        exit(1);
    }

    blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX, 2);
    blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY, 2);
    blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ, 2);

    for(i=0; i<3; i++){
        if(blocks[i] == 0){
            empty_block_indx = i;
            dir_entry_indx = 0;
            create_new_block = 1;
            continue;
        }
        _directory_entry _directory_entries[4];
        readSFS(blocks[i], (char *)_directory_entries);        

        for(j=0; j<4; j++){   
            if(_directory_entries[j].F == '1'){
                if(strcmp(_directory_entries[j].fname, fname) == 0){
                    printf("%s: Already exists.\n", fname);
                    return;
                }
            }
            else{
                empty_block_indx = i;
                dir_entry_indx = j;
                create_new_block = 0;
            }
        }
    }

    if(empty_block_indx == -1 || dir_entry_indx == -1){
        printf("File system is full: No empty space in this directory!\n");
        return;
    }
    else if(create_new_block){
        int new_block = getBlock();
        if(new_block == -1){
            printf("File system is full: No data blocks!\n");
            return;
        }
        
        writeSFS(new_block, NULL);
        new_inode = getInode();

        if(new_inode == -1){
            returnBlock(new_block);
            printf("File system is full: No inodes available!\n");
            return;
        }
        
        switch(empty_block_indx){
            case 0: itos(_inode_table[CD_INODE_ENTRY].XX, new_block, 2); break;
            case 1: itos(_inode_table[CD_INODE_ENTRY].YY, new_block, 2); break;
            case 2: itos(_inode_table[CD_INODE_ENTRY].ZZ, new_block, 2); break;
        }
        blocks[empty_block_indx] = new_block;
    }
    else{
        new_inode = getInode();
        if(new_inode == -1){
            printf("File system is full: No inodes available!\n");
            return;
        }
    }

    _directory_entry new_dir_block[4];
    readSFS(blocks[empty_block_indx], (char *)new_dir_block);

    new_dir_block[dir_entry_indx].F = '1';
    strncpy(new_dir_block[dir_entry_indx].fname, fname, 252);
    itos(new_dir_block[dir_entry_indx].MMM, new_inode, 3);
    
    writeSFS(blocks[empty_block_indx], (char *)new_dir_block);
    
    strncpy(_inode_table[new_inode].TT, "FI", 2);
    strncpy(_inode_table[new_inode].XX, "00", 2);
    strncpy(_inode_table[new_inode].YY, "00", 2);
    strncpy(_inode_table[new_inode].ZZ, "00", 2);

    writeSFS(BLOCK_INODE_TABLE, (char *)_inode_table);

    printf("%s has been created, enter text!\n", fname);
    
    i = 0;
    while(i < 3){
        int new_block = getBlock();
        if(new_block == -1){
            printf("File system full: No data blocks!\n");
            printf("Data will be truncated!\n");
            return;
        }

        switch(i){
            case 0: itos(_inode_table[new_inode].XX, new_block, 2); break;
            case 1: itos(_inode_table[new_inode].YY, new_block, 2); break;
            case 2: itos(_inode_table[new_inode].ZZ, new_block, 2); break;
        }
        
        j = 0;
        while(j < 1024){
            scanf("%c", &buffer[j]);
            if(buffer[j] == 27){
                buffer[j] = '\0';
                writeSFS(new_block, buffer);
                fflush(stdin);
                return;
            }
            j++;
        }

        writeSFS(new_block, buffer);

        i++;
        if(i == 3){
            printf("Maximum file size reached!\n");
            printf("Data will be truncated!\n");
            fflush(stdin);
            return;
        }
    }
}

int removeFile(int inode){
    char itype = _inode_table[inode].TT[0];
    int blocks[3], i;
    if(itype == 'D'){
        printf("Remove file error: inode is a directory!\n");
        exit(1);
    }

    blocks[0] = stoi(_inode_table[inode].XX, 2);
    blocks[1] = stoi(_inode_table[inode].YY, 2);
    blocks[2] = stoi(_inode_table[inode].ZZ, 2);

    for(i=0; i<3; i++){
        if(blocks[i] == 0)
            continue;
        returnBlock(blocks[i]);
    }

    returnInode(inode);
    writeSFS(BLOCK_INODE_TABLE, (char *)_inode_table);
    return 1;
}

int removeDirectory(int inode){
    char itype = _inode_table[inode].TT[0];
    int blocks[3], i, j;
    if(itype == 'F'){
        printf("Remove directory error: inode is a directory!\n");
        exit(1);
    }

    blocks[0] = stoi(_inode_table[inode].XX, 2);
    blocks[1] = stoi(_inode_table[inode].YY, 2);
    blocks[2] = stoi(_inode_table[inode].ZZ, 2);

    for(i=0; i<3; i++){
        if(blocks[i] == 0)
            continue;

        _directory_entry directory_entries[4];
        readSFS(blocks[i], (char *)directory_entries);

        for(j=0; j<4; j++){
            if(directory_entries[j].F == '0')
                continue;
            
            int del_inode = stoi(directory_entries[j].MMM, 3);
            itype = _inode_table[del_inode].TT[0];
            if(itype == 'F'){
                if(!removeFile(del_inode)){
                    printf("Remove directory error: removeFile call failed!\n");
                    exit(1);
                }
            }
            else{
                if(!removeDirectory(del_inode)){
                    printf("Remove directory error: recursive removeDirectory call failed!\n");
                    exit(1);
                }
            }
            directory_entries[j].F = '0';
        }
        writeSFS(blocks[i], (char *)directory_entries);
        returnBlock(blocks[i]);
    }
    
    returnInode(inode);
    writeSFS(BLOCK_INODE_TABLE, (char *)_inode_table);
    return 1;
}

void remfd(char *fdname){
    char itype = _inode_table[CD_INODE_ENTRY].TT[0];
    int blocks[3], i, j, valid_name = 0, item_count = 0;
    
    if(itype == 'F'){
        printf("Remove error: CD_INODE is a file!\n");
        exit(1);
    }
    
    blocks[0] = stoi(_inode_table[CD_INODE_ENTRY].XX, 2);
    blocks[1] = stoi(_inode_table[CD_INODE_ENTRY].YY, 2);
    blocks[2] = stoi(_inode_table[CD_INODE_ENTRY].ZZ, 2);

    for(i=0; i<3; i++){
        if(blocks[i] == 0)
            continue;

        _directory_entry directory_entries[4];
        readSFS(blocks[i], (char *)directory_entries);
        
        item_count = 0;

        for(j=0; j<4; j++){
            if(directory_entries[j].F == '0')
                continue;
            item_count++;
            if(strcmp(fdname, directory_entries[j].fname) == 0){
                valid_name = 1;
                int del_inode = stoi(directory_entries[j].MMM, 3);
                itype = _inode_table[del_inode].TT[0];
                if(itype == 'F')
                    removeFile(del_inode);
                else
                    removeDirectory(del_inode);
                directory_entries[j].F = '0';
                item_count--;
            }
        }
        writeSFS(blocks[i], (char *)directory_entries);
        if(item_count == 0){
            returnBlock(blocks[i]);
            switch(i){
                case 0: itos(_inode_table[CD_INODE_ENTRY].XX, 0, 2); break;
                case 1: itos(_inode_table[CD_INODE_ENTRY].YY, 0, 2); break;
                case 2: itos(_inode_table[CD_INODE_ENTRY].ZZ, 0, 2); break;
            }
            writeSFS(BLOCK_INODE_TABLE, (char *)_inode_table);
        }
    }
    if(valid_name == 0)
        printf("%s not found in current directory!\n", fdname);
}