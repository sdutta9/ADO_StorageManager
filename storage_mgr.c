#include<stdio.h>
#include<stdlib.h>
#include "storage_mgr.h"
#include <math.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

FILE *fp;
extern void initStorageManager (void){

	fp=NULL;

}
extern RC createPageFile (char *fileName){
	fp = fopen(fileName, "w+");

	if(fp == NULL)
		return RC_FILE_NOT_FOUND;
	else{
	//	fseek(fp, PAGE_SIZE, SEEK_SET); -> this does not wwork
	// instead create empty page in memory and write it to file

		SM_PageHandle EmptyPage = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));  //create Empty Page
		if(fwrite(EmptyPage,sizeof(char),PAGE_SIZE,fp) < PAGE_SIZE){  //write Empty page to file
		printf("fwrite wrong \n");
		}else{
			printf("fwrite correct \n");
		}
		//fseek(fp, PAGE_SIZE, SEEK_SET);
		fclose(fp);  //always close file
		free(EmptyPage);
		return RC_OK;
	}
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle){
	fp = fopen(fileName, "r+");  //open file in read mode

	if(fp == NULL)
		return RC_FILE_NOT_FOUND;
	else{ 
		//update file handle
		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;	
	
	/*	fseek(fp, 0L, SEEK_END);
		fPos = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		fHandle.totalNumPages = int(fPos)/ PAGE_SIZE;
	*/
			//use file stats to get file size and derive number of pages
			struct stat fileStat;
			if(fstat(fileno(fp),&fileStat) < 0)    
		return RC_ERROR;
			fHandle->totalNumPages = fileStat.st_size/ PAGE_SIZE;
		
			fclose(fp);
			return RC_OK;
		
	}
}
extern RC closePageFile (SM_FileHandle *fHandle){
	
	if(fp!=NULL)
		fp=NULL;	
	return RC_OK; 
}


extern RC destroyPageFile (char *fileName){
	fp = fopen(fileName, "r"); //open file with the paramter file name
	
	if(fp == NULL)
		return RC_FILE_NOT_FOUND; 
		
	remove(fileName); //begone File
	return RC_OK;
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	fp = fopen(fHandle->fileName, "r"); //open file in r mode
	if(fp == NULL){
		return RC_FILE_NOT_FOUND; // error check
	}
	if (pageNum > fHandle->totalNumPages || pageNum < 0) {
        fclose(fp);
        return RC_READ_NON_EXISTING_PAGE; 
    	}
	fseek(fp, (pageNum * PAGE_SIZE), SEEK_SET);
	if(fread(memPage, 1, PAGE_SIZE, fp) < PAGE_SIZE)
		return RC_ERROR;
    	fHandle->curPagePos = ftell(fp); //update fhandle
    	fclose(fp); //always close fp
	
    	return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle){
	return fHandle->curPagePos;
}

///////////////////////////////////////

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	fp = fopen(fHandle->fileName, "r+");	//Open file in read mode	
	if(fp == NULL)
		return RC_FILE_NOT_FOUND;	//Throw error if filepointer is null
	int i;
	for(i=0;i<PAGE_SIZE; i++){
		char c = fgetc(fp); // Reading one char from the file.		
		if(feof(fp)){	//if filepointer pointing to end of file then break.
			break;
		}
		else
			memPage[i] = c;
	}
	fHandle->curPagePos = ftell(fp);// set current file position to curPagePos
	fclose(fp);	//closing filepointer
	return RC_OK;
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	if(fHandle->curPagePos <= PAGE_SIZE){	//condition to check if current position of file is on first page.
		printf("File pointer on the first block. no previous block to read.");
		return RC_READ_NON_EXISTING_PAGE;	
	}	
	else{
		int curpagenum = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.
		int startpos= (PAGE_SIZE*(curpagenum - 2)); // storing the previous page start position
		fp=fopen(fHandle->fileName,"r+"); //Open file in read mode
		if(fp == NULL)
			return RC_FILE_NOT_FOUND;  //Throw error if filepointer is null	
		fseek(fp,startpos,SEEK_SET);
		int i;
		for(i=0;i<PAGE_SIZE;i++){ //reading previous block character by character and storing in memPage
			memPage[i] = fgetc(fp);
		}
		fHandle->curPagePos = ftell(fp);// set current file position to curPagePos
		fclose(fp); //closing filepointer
		return RC_OK;				
	}
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	int curpagenum = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.
	int startpos= (PAGE_SIZE*(curpagenum - 1)); // storing the current page start position
	
	fp=fopen(fHandle->fileName,"r+"); //Open file in read mode
	if(fp == NULL)
		return RC_FILE_NOT_FOUND; //Throw error if filepointer is null		
	fseek(fp,startpos,SEEK_SET);
	int i;
	for(i=0;i<PAGE_SIZE;i++){ //reading current block character by character and storing in memPage
		char c = fgetc(fp);		
		if(feof(fp))
			break;
		memPage[i] = c;
	}
	fHandle->curPagePos = ftell(fp);// set current file position to curPagePos	
	fclose(fp); //closing filepointer
	return RC_OK;	
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	int curpagenum = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.	
	if(fHandle->totalNumPages == curpagenum){ //condition to check if current position of file is on last page.
		printf("File pointer on the last block. No next block to read.");
		return RC_READ_NON_EXISTING_PAGE;	
	}	
	else{
		int startpos= (PAGE_SIZE*curpagenum); // storing the next page start position
		
		fp=fopen(fHandle->fileName,"r+"); //Open file in read mode
		if(fp == NULL)
			return RC_FILE_NOT_FOUND; //Throw error if filepointer is null
		fseek(fp,startpos,SEEK_SET);
		int i;
		for(i=0;i<PAGE_SIZE;i++){ //reading next block character by character and storing in memPage
			char c = fgetc(fp);
			if(feof(fp))
				break;
			memPage[i] = c;
		}
		fHandle->curPagePos = ftell(fp);// set current file position to curPagePos
		fclose(fp); //closing filepointer
		return RC_OK;							
	}	
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	fp = fopen(fHandle->fileName, "r+"); //Open file in read mode	
	if(fp == NULL)
		return RC_FILE_NOT_FOUND; //Throw error if filepointer is null
	
	int startpos = (fHandle->totalNumPages - 1) * PAGE_SIZE; // storing the last page start position	
	fseek(fp,startpos,SEEK_SET);
	int i;
	for(i=0;i<PAGE_SIZE; i++){ //reading last block character by character and storing in memPage
		char c = fgetc(fp);
		if(feof(fp))
			break;
		memPage[i] = c;
	}
	fHandle->curPagePos = ftell(fp);// set current file position to curPagePos
	fclose(fp); //closing filepointer		
	return RC_OK;
}

/////////////////////////////////////////

//Write block to Absolute Position (pageNum)  
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	fp = fopen(fHandle->fileName, "r+"); // open file in read+write mode.
	if(fp == NULL)
		return RC_FILE_NOT_FOUND; // Throw an error file not found
	int spec_pos = (pageNum)*PAGE_SIZE; //storing starting position of pageNum
	
	if(pageNum!=0){ // Write content to a non first page.
		fHandle->curPagePos = spec_pos;
		writeCurrentBlock(fHandle,memPage);		
	}
	else{	//write content to the first page
		fseek(fp,spec_pos,SEEK_SET);	
		int i;
		for(i=0;i<PAGE_SIZE;i++) 
		{
			if(feof(fp)) // check file is ending in between writing
			{
				 appendEmptyBlock(fHandle); // append empty block at the end of file
			}
			fputc(memPage[i],fp);// write content to file
		}
		fHandle->curPagePos = ftell(fp);// set current file position to curPagePos 
		fclose(fp); //closing filepointer	
	}	
	return RC_OK;
}

// write block to current Page 
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	fp = fopen(fHandle->fileName, "r+");// open file in read+write mode.
	if(fp == NULL)
		return RC_FILE_NOT_FOUND; // Throw an error file not found.
	long int curPosition = fHandle->curPagePos; //Storing current file position.
	
	appendEmptyBlock(fHandle); //Appending an empty block to make space for the new content.

	fseek(fp,curPosition,SEEK_SET); //Seek to the current position.
	
	int ctr=0;
	while(fgetc(fp)!= EOF) //Calculating the total number of character after the point where we need to insert new data.
		ctr++;
	fseek(fp,curPosition,SEEK_SET);
	char *string1 = malloc(PAGE_SIZE+1);
	fread(string1,1,ctr-PAGE_SIZE,fp); //Storing in string1 the content after the current position.
	
	fseek(fp,curPosition,SEEK_SET);

	fwrite(memPage,1,strlen(memPage),fp);//Writing the memPage to our file.
	fwrite(string1,1,strlen(string1),fp);//Writing the string1 to our file.
	
	fHandle->curPagePos = ftell(fp); // set current file position to curPagePos
	free(string1);	//free string memory
	fclose(fp); //closing filepointer
	return RC_OK;
}

////////////////////////////////////////////////////////

extern RC appendEmptyBlock (SM_FileHandle *fHandle){
	SM_PageHandle EmptyPage = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char)); //creating empty page of PAGE_SIZE bytes 
	fseek(fp, 0, SEEK_END);
	fwrite(EmptyPage,sizeof(char),PAGE_SIZE,fp); //Writing Empty page to the file.
	free(EmptyPage); //free memory from EmptyPage.
	fHandle->totalNumPages++; //Increasing total number of pages.
	return RC_OK;
}


extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
	fp=fopen(fHandle->fileName,"a");
	if(fp==NULL)
		return RC_FILE_NOT_FOUND;
	while(numberOfPages > fHandle->totalNumPages) //If numberOfPages is greater than totalNumPages then add emptly pages.
		appendEmptyBlock(fHandle);

	fclose(fp);
	return RC_OK;
}
