
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "ext2_fs.h"




//exit code
int SYSCALL_FAILURE_EXIT = 2;
int BAD_ARGUMENT = 1;
int SUCCESS_EXIT = 0;



typedef struct ext2_super_block super_block;
typedef struct ext2_group_desc group_desc;

char const* filename = NULL;

int exit_with_error_message(const char* message){
	fprintf(stderr, "%s: %s\ncaused by option : %s\r\n", filename, message, strerror(errno));
	exit(SYSCALL_FAILURE_EXIT);
}

__u32 block_size;
int fd;

void handle_dir(int inode_num, __u32 cur_block_num, __u32 *logical_offset_p){
	__u32 block_offset = 0;
	int entry_ind;
	for(entry_ind=0; block_offset < block_size; entry_ind++){
		struct ext2_dir_entry dir;
		pread(fd, &dir, sizeof(struct ext2_dir_entry),
			cur_block_num * block_size + block_offset);
		char *name = malloc(sizeof(char)*(dir.name_len+1));
		memcpy(name, &dir.name, dir.name_len);
		name[dir.name_len] = 0;
		if (dir.inode == 0){
			block_offset += dir.rec_len;
			continue;
		}
		printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n",inode_num,*logical_offset_p, dir.inode,
			dir.rec_len,dir.name_len,name);
		free(name);
		*logical_offset_p += dir.rec_len;
		block_offset += dir.rec_len;
	}
}


int main(int argc, char **argv)
{

	if (argc != 2){
		fprintf(stderr, "Bad arguments. Correct usage: ./lab3a FILE_SYSTEM_IMAGE.\n");
		exit(BAD_ARGUMENT);
	}
	filename = argv[1];
	fd = open(filename, O_RDONLY);
	if(fd < 0)
		exit_with_error_message("error with open.");
	char *buf = malloc(1024);
	ssize_t r = pread(fd, buf, 1024, 1024);

	if(r < 0)
		exit_with_error_message("error with pread.");

	super_block *sp = malloc(sizeof(super_block));
	group_desc *gdp = malloc(sizeof(group_desc));


	ssize_t ret;

	//superblock summary, SUPERBLOCK,64,24,1024,128,8192,24,11
	//# blocks, # inodes, block size, inode size, blocks per group, 
	//inodes per group, first non-researved inode

	ret = pread(fd, sp, EXT2_MIN_BLOCK_SIZE, EXT2_MIN_BLOCK_SIZE);
	if (ret < 0){
		exit_with_error_message("pread() superblock");
	}

	block_size = EXT2_MIN_BLOCK_SIZE << (sp->s_log_block_size);
	printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",sp->s_blocks_count,sp->s_inodes_count,
		block_size,sp->s_inode_size,sp->s_blocks_per_group,
		sp->s_inodes_per_group,sp->s_first_ino);


	//group summary: GROUP,0,64,24,17,6,3,4,5
	//group number, # blocks in group, #i-nodes in group, # free blocks, # free i-nodes,
	//block # of free block bitmap, block # of free i-node bitmap, 
	//block # of first block of i-nodes

	ret = pread(fd, gdp, sizeof(group_desc), EXT2_MIN_BLOCK_SIZE*2);
	if (ret < 0){
		exit_with_error_message("pread() group descriptor table");
	}

	printf("GROUP,0,%d,%d,%d,%d,%d,%d,%d\n",sp->s_blocks_count,sp->s_inodes_count, 
		gdp->bg_free_blocks_count,gdp->bg_free_inodes_count,gdp->bg_block_bitmap,
		gdp->bg_inode_bitmap, gdp->bg_inode_table);



	unsigned int j, k;

	// free block entries
	for(j = 0; j < block_size; j++){
		uint8_t b_bitmap;
		ret = pread(fd, &b_bitmap, sizeof(uint8_t), gdp->bg_block_bitmap * block_size + j);
		if(ret < 0)
			exit_with_error_message("error with pread for free block entries");
		// visit every bit by masking
		for(k = 0; k < 8*(sizeof(uint8_t)); k++){
			if((b_bitmap & (1 << k)) == 0){
				fprintf(stdout, "BFREE,%d\n", j*8 + k+1);
			}
		}
	}


	// free i-node entries
	for(j = 0; j < block_size; j++){
		uint8_t i_bitmap;
		ret = pread(fd, &i_bitmap, sizeof(uint8_t), gdp->bg_inode_bitmap * block_size + j);
		if(ret < 0)
			exit_with_error_message("error with pread for i-node entries");
		// visit every bit by masking
		for(k = 0; k < 8*(sizeof(uint8_t)); k++){
			// For each free I-node, produce a new-line terminated line, 
			//with two comma-separated fields (with no white space).
			if((i_bitmap & (1 << k)) == 0){
				//Inode number: starts from 1, not 0 (first non-reserved inode)
				fprintf(stdout, "IFREE,%d\n", j*8 + k+1);
			}
		}
	}


	// i-node summary

	// witin the total number of inodes in the file system, iterate through
	for(j = 1; j < sp->s_inodes_count; j++){
		struct ext2_inode inode;
		
		ret = pread(fd, &inode, sizeof(struct ext2_inode), 
			1024 + (gdp->bg_inode_table-1) * block_size + (j-1) * sizeof(struct ext2_inode));
		if(ret < 0)
			exit_with_error_message("error with pread for i-node summary");
		//printf("j: %d, with i_mode: %d, and i_links_count: %d\n",
		// j, inode.i_mode, inode.i_links_count);
		if(inode.i_mode != 0 && inode.i_links_count != 0){

			char file_type;
			// 'f' for file, 
			// 'd' for directory, 
			// 's' for symbolic link, 
			// '?" for anything else)
			if(S_ISREG(inode.i_mode) != 0)
				file_type = 'f';
			else if(S_ISDIR(inode.i_mode) != 0)
				file_type = 'd';
			else if(S_ISLNK(inode.i_mode) != 0)
				file_type = 's';
			else
				file_type = '?';
			//printf("file_type: %c\n", file_type);

			__u16 mode = inode.i_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
			//printf("mode: %o\n", mode);

			char creation_time[18];
			time_t ctime = inode.i_ctime;
			struct tm *ct = gmtime(&ctime);
			strftime(creation_time, 18, "%m/%d/%y %H:%M:%S", ct);
			//printf("creation_time: %s\n", creation_time);

			char modification_time[18];
			time_t mtime = inode.i_mtime;
			struct tm *mt = gmtime(&mtime);
			strftime(modification_time, 18, "%m/%d/%y %H:%M:%S", mt);
			//printf("modification_time: %s\n", modification_time);

			char access_time[18];
			time_t atime = inode.i_atime;
			struct tm *at = gmtime(&atime);
			strftime(access_time, 18, "%m/%d/%y %H:%M:%S", at);
			//printf("access_time: %s\n", access_time);


			if(file_type == 's' && inode.i_blocks == 0){
				fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d,%d\n", 
				j, file_type, mode, inode.i_uid, inode.i_gid, 
				inode.i_links_count, creation_time, modification_time, access_time, inode.i_size, inode.i_blocks, inode.i_block[0]);
			}
			else{
				fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d", 
					j, file_type, mode, inode.i_uid, inode.i_gid, 
					inode.i_links_count, creation_time, modification_time, access_time, inode.i_size, inode.i_blocks );
				
				// the last 3 numbers
				// This number is in units of 512 byte blocks, 

				for(k = 0; k < EXT2_N_BLOCKS; k++)
					fprintf(stdout, ",%d", inode.i_block[k]);
				fprintf(stdout, "\n");
			}

			//indirect: INDIRECT,17,1,12,34,35
			//inode no of owning file,level,logical offset,block num, block referenced

			if (file_type == 'f'){
				int i,ii,iii;
				int max_ind = inode.i_blocks/(2<<sp->s_log_block_size);
				int block_ind = 0;
				__u32 *block_num_array = malloc(sizeof(__u32)*max_ind);
				for(i=0; i<12;i++){
					__u32 cur_block_num = inode.i_block[i];
					if (cur_block_num != 0){
						block_num_array[block_ind] = cur_block_num;
						block_ind++;
					}
				}
				if (block_ind < max_ind){
					__u32 thirteen_block = inode.i_block[12];
					__u32 *temp_buf = malloc(sizeof(__u32)*256);
					ret = pread(fd,temp_buf,sizeof(__u32)*256,inode.i_block[12]*block_size);
					if(ret < 0)
						exit_with_error_message("error with pread");
					for(i = 0; i < 256;i++){
						__u32 cur_block_num = temp_buf[i];
						if (cur_block_num != 0){
							block_num_array[block_ind] = cur_block_num;
							block_ind++;
							printf("INDIRECT,%d,%d,%d,%d,%d\n",j,1,i+12,thirteen_block,cur_block_num);
						}
					}
					free(temp_buf);
				}

				if (block_ind < max_ind){
					__u32 fourteen_block = inode.i_block[13];
					__u32 *ind_blocks_arr = malloc(sizeof(__u32)*256);
					ret = pread(fd,ind_blocks_arr,sizeof(__u32)*256,fourteen_block*block_size);
					if(ret < 0)
						exit_with_error_message("error with pread");
					for(i = 0; i < 256; i++){
						__u32 cur_block_num = ind_blocks_arr[i];
						if(cur_block_num != 0){
							block_num_array[block_ind] = cur_block_num;
							block_ind++;
							printf("INDIRECT,%d,%d,%d,%d,%d\n",j,2,i+268,fourteen_block,cur_block_num);
							__u32 prev_block_num = cur_block_num;
							__u32 *temp_buf = malloc(sizeof(__u32)*256);
							ret = pread(fd,temp_buf,sizeof(__u32)*256,prev_block_num*block_size);
							if(ret < 0)
								exit_with_error_message("error with pread");
							for(ii = 0; ii < 256; ii++){
								cur_block_num = temp_buf[ii];
								if (cur_block_num != 0){
									block_num_array[block_ind] = cur_block_num;
									block_ind++;
									printf("INDIRECT,%d,%d,%d,%d,%d\n",j,1,268+i+ii,prev_block_num,cur_block_num);
								}
							}
							free(temp_buf);
						}
					}
					free(ind_blocks_arr);
				}

				if (block_ind < max_ind){
					__u32 fifteen_block = inode.i_block[14];
					__u32 *doub_ind_blocks_arr = malloc(sizeof(__u32)*256);
					ret = pread(fd,doub_ind_blocks_arr,sizeof(__u32)*256,fifteen_block*block_size);
					if(ret < 0)
						exit_with_error_message("error with pread");
					for(i = 0; i < 256; i++){
						__u32 cur_block_num = doub_ind_blocks_arr[i];
						if (cur_block_num != 0){
							block_num_array[block_ind] = cur_block_num;
							block_ind++;
							printf("INDIRECT,%d,%d,%d,%d,%d\n",j,3,65804+i,fifteen_block,cur_block_num);
							__u32 doub_prev_block_num = cur_block_num;
							__u32 *ind_blocks_arr = malloc(sizeof(__u32)*256);
							ret = pread(fd,ind_blocks_arr,sizeof(__u32)*256,doub_prev_block_num*block_size);
							if(ret < 0)
								exit_with_error_message("error with pread");
							for(ii = 0; ii < 256; ii++){
								cur_block_num = ind_blocks_arr[ii];
								if (cur_block_num != 0){
									block_num_array[block_ind] = cur_block_num;
									block_ind++;
									printf("INDIRECT,%d,%d,%d,%d,%d\n",j,2,65804+i+ii,doub_prev_block_num,cur_block_num);
									__u32 prev_block_num = cur_block_num;
									__u32 *temp_buf = malloc(sizeof(__u32)*256);
									ret = pread(fd,temp_buf,sizeof(__u32)*256,prev_block_num*block_size);
									if(ret < 0)
										exit_with_error_message("error with pread");
									for(iii = 0; iii < 256; iii++){
										cur_block_num = temp_buf[iii];
										if (cur_block_num != 0){
											block_num_array[block_ind] = cur_block_num;
											block_ind++;
											printf("INDIRECT,%d,%d,%d,%d,%d\n",j,1,65804+i+ii+iii,prev_block_num,cur_block_num);
										}
									}
									free(temp_buf);
								}
							}
							free(ind_blocks_arr);
						}
					}
					free(doub_ind_blocks_arr);
				}	
				free(block_num_array);
			}

			//directory entries: DIRENT,2,0,2,12,1,'.'
			//parent inode#, logical byte offset, inode# of ref, 
			//entry length, name length,'name'
			//
			//indirect: INDIRECT,17,1,12,34,35
			//inode no of owning file,level,logical offset,block num, block referenced
			if (file_type == 'd'){
				__u32 logical_offset = 0;
				int i,ii,iii;
				int max_ind = inode.i_blocks/(2<<sp->s_log_block_size);
				int block_ind = 0;
				__u32 *block_num_array = malloc(sizeof(__u32)*max_ind);
				for(i=0; i<12;i++){
					__u32 cur_block_num = inode.i_block[i];
					if (cur_block_num != 0){
						block_num_array[block_ind] = cur_block_num;
						handle_dir(j,block_num_array[block_ind],&logical_offset);
						block_ind++;
					}
				}
				if (block_ind < max_ind){
					__u32 thirteen_block = inode.i_block[12];
					__u32 *temp_buf = malloc(sizeof(__u32)*256);
					ret = pread(fd,temp_buf,sizeof(__u32)*256,inode.i_block[12]*block_size);
					if(ret < 0)
						exit_with_error_message("error with pread");
					for(i = 0; i < 256;i++){
						__u32 cur_block_num = temp_buf[i];
						if (cur_block_num != 0){
							block_num_array[block_ind] = cur_block_num;
							handle_dir(j,block_num_array[block_ind],&logical_offset);
							block_ind++;
							printf("INDIRECT,%d,%d,%d,%d,%d\n",j,1,i+12,thirteen_block,cur_block_num);
						}
					}
					free(temp_buf);
				}

				if (block_ind < max_ind){
					__u32 fourteen_block = inode.i_block[13];
					__u32 *ind_blocks_arr = malloc(sizeof(__u32)*256);
					ret = pread(fd,ind_blocks_arr,sizeof(__u32)*256,fourteen_block*block_size);
					if(ret < 0)
						exit_with_error_message("error with pread");
					for(i = 0; i < 256; i++){
						__u32 cur_block_num = ind_blocks_arr[i];
						if(cur_block_num != 0){
							block_num_array[block_ind] = cur_block_num;
							block_ind++;
							printf("INDIRECT,%d,%d,%d,%d,%d\n",j,2,i+268,fourteen_block,cur_block_num);
							__u32 prev_block_num = cur_block_num;
							__u32 *temp_buf = malloc(sizeof(__u32)*256);
							ret = pread(fd,temp_buf,sizeof(__u32)*256,prev_block_num*block_size);
							if(ret < 0)
								exit_with_error_message("error with pread");
							for(ii = 0; ii < 256; ii++){
								cur_block_num = temp_buf[ii];
								if (cur_block_num != 0){
									block_num_array[block_ind] = cur_block_num;
									handle_dir(j,block_num_array[block_ind],&logical_offset);
									block_ind++;
									printf("INDIRECT,%d,%d,%d,%d,%d\n",j,1,268+i+ii,prev_block_num,cur_block_num);
								}
							}
							free(temp_buf);
						}
					}
					free(ind_blocks_arr);
				}

				if (block_ind < max_ind){
					__u32 fifteen_block = inode.i_block[14];
					__u32 *doub_ind_blocks_arr = malloc(sizeof(__u32)*256);
					ret = pread(fd,doub_ind_blocks_arr,sizeof(__u32)*256,fifteen_block*block_size);
					if(ret < 0)
						exit_with_error_message("error with pread");
					for(i = 0; i < 256; i++){
						__u32 cur_block_num = doub_ind_blocks_arr[i];
						if (cur_block_num != 0){
							block_num_array[block_ind] = cur_block_num;
							block_ind++;
							printf("INDIRECT,%d,%d,%d,%d,%d\n",j,3,65804+i,fifteen_block,cur_block_num);
							__u32 doub_prev_block_num = cur_block_num;
							__u32 *ind_blocks_arr = malloc(sizeof(__u32)*256);
							ret = pread(fd,ind_blocks_arr,sizeof(__u32)*256,doub_prev_block_num*block_size);
							for(ii = 0; ii < 256; ii++){
								cur_block_num = ind_blocks_arr[ii];
								if (cur_block_num != 0){
									block_num_array[block_ind] = cur_block_num;
									block_ind++;
									printf("INDIRECT,%d,%d,%d,%d,%d\n",j,2,65804+i+ii,doub_prev_block_num,cur_block_num);
									__u32 prev_block_num = cur_block_num;
									__u32 *temp_buf = malloc(sizeof(__u32)*256);
									ret = pread(fd,temp_buf,sizeof(__u32)*256,prev_block_num*block_size);
									if(ret < 0)
										exit_with_error_message("error with pread");
									for(iii = 0; iii < 256; iii++){
										cur_block_num = temp_buf[iii];
										if (cur_block_num != 0){
											block_num_array[block_ind] = cur_block_num;
											handle_dir(j,block_num_array[block_ind],&logical_offset);
											block_ind++;
											printf("INDIRECT,%d,%d,%d,%d,%d\n",j,1,65804+i+ii+iii,prev_block_num,cur_block_num);
										}
									}
									free(temp_buf);
								}
							}
							free(ind_blocks_arr);
						}
					}
					free(doub_ind_blocks_arr);
				}	
				// for(i=0; i< max_ind; i++){
				// 	handle_dir(j,block_num_array[i],&logical_offset);
				// }
				free(block_num_array);
			}
		}
	}


	// before exit, delete the malloc buf
	free(sp);
	free(gdp);
	free(buf);

	exit(SUCCESS_EXIT);
}
