
void display(char *);
void create(char *);
void remfd(char *);
int removeFile(int);
int removeDirectory(int);

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