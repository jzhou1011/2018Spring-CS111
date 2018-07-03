#!/usr/local/cs/bin/python3

import sys,csv

class Superblock:
	def __init__(self, num_block=0, num_inode=0, block_size=0, inode_size=0,first_nr_inode=0):
		self.num_block = num_block
		self.num_inode = num_inode
		self.block_size = block_size
		self.inode_size = inode_size
		self.first_nr_inode = first_nr_inode

class Group:
	def __init__(self, group_number=0, num_blocks=0, num_inodes=0, num_free_blocks=0,num_free_inodes=0,first_block_of_inodes=0):
		self.group_number = group_number
		self.num_blocks = num_blocks
		self.num_inodes = num_inodes
		self.num_free_blocks = num_free_blocks
		self.num_free_inodes = num_free_inodes
		self.first_block_of_inodes = first_block_of_inodes

class Inode:
	def __init__(self, inode_no=0, link_count=0, mode = 0, blocks=[], single=0, double=0,triple=0):
		self.no = inode_no
		self.links = 0
		self.link_count = link_count
		self.mode = mode
		self.blocks = blocks
		self.single = single
		self.double = double
		self.triple = triple

class Dirent:
	def __init__(self, parent_no, ref_no, dir_name):
		self.parent_no = parent_no
		self.ref_no = ref_no
		self.name = dir_name

class Indirect:
	def __init__(self, level, block_offset, ref_blockno):
		self.level = level
		self.block_offset = block_offset
		self.ref_blockno = ref_blockno


super_block = Superblock()
group = Group()
dirent_list = set()
ino_inode = dict()
dir_par = {2: 2} #key:dir ino number, value: parent ino number, root dir is 2
freeino_list = set()
indirect_list = dict()
freeblock_list = set()
ref_block_dict = dict() #key: block number, value: referenced by which inode, offset
return_code = 0
reference_block = dict()


#check inode linkcount with links
	#link count already in csv, iterate through dirent and count links
	#INODE 17 HAS 0 LINKS BUT LINKCOUNT IS 1
#check if referring to invalid or unallocated inode
	#INVALID I-node is one whose number <1 or >last I-node in the system
	#not in inode list
	#DIRECTORY INODE 2 NAME 'nullEntry' UNALLOCATED INODE 17
	#DIRECTORY INODE 2 NAME 'bogusEntry' INVALID INODE 26
#check . and .. dirent: . should be parent, .. should be grandparent




def handle_dir():
	for dirent in dirent_list:
		ref_inode = dirent.ref_no
		parent_inode = dirent.parent_no
		cur_name = dirent.name
		if ref_inode > super_block.num_inode or (ref_inode < super_block.first_nr_inode and ref_inode!=2):
			print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(parent_inode,cur_name,ref_inode))
			return_code = 2
			continue
		if ref_inode not in ino_inode or ino_inode[ref_inode].mode == 0:
			print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(parent_inode,cur_name,ref_inode))
			return_code = 2
			continue
		ino_inode[ref_inode].links += 1
		if cur_name == '\'.\'':
			if ref_inode != parent_inode:
				return_code = 2
				print("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(parent_inode,ref_inode,parent_inode))
		elif cur_name == '\'..\'':
			grandparent_inode = dir_par[parent_inode]
			if ref_inode != grandparent_inode:
				return_code = 2
				print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(parent_inode,ref_inode,grandparent_inode))
	for inode in ino_inode.values():
		if inode.links != inode.link_count:
			return_code = 2
			print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(inode.no, inode.links, inode.link_count))



def block_consistency():
	first_nonreserved_block = int(group.first_block_of_inodes + super_block.inode_size * group.num_inodes/super_block.block_size)
	# print(first_nonreserved_block)
	# print(group.num_blocks)
	for block_no in reference_block:
		temp = reference_block[block_no]
		if block_no > group.num_blocks-1 or block_no < 0:
			for i in temp:
				if i[1] == 0:
					print("INVALID BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
				elif i[1] == 1:
					print("INVALID INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
				elif i[1] == 2:
					print("INVALID DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
				elif i[1] == 3:
					print("INVALID TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
				return_code = 2
		elif block_no < first_nonreserved_block and block_no > 0:
			for i in temp:
				if i[1] == 0:
					print("RESERVED BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
				elif i[1] == 1:
					print("RESERVED INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
				elif i[1] == 2:
					print("RESERVED DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
				elif i[1] == 3:
					print("RESERVED TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
	for block_no in range(first_nonreserved_block, group.num_blocks):
		if block_no in reference_block:
			temp = reference_block[block_no]
			if block_no in freeblock_list:
				print("ALLOCATED BLOCK {} ON FREELIST".format(block_no))
			elif len(temp) > 1:
				for i in temp:
					if i[1] == 0:
						print("DUPLICATE BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
					elif i[1] == 1:
						print("DUPLICATE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
					elif i[1] == 2:
						print("DUPLICATE DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))
					elif i[1] == 3:
						print("DUPLICATE TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block_no, i[0],i[2]))				
		if block_no not in freeblock_list and block_no not in reference_block:
			print("UNREFERENCED BLOCK {}".format(block_no))

# Inode allocation audits:
# Key: node number
# Value: on freelist or not (bool): IFREE;
# 	allocated or not (bool): INODE summary, if allocated, it appears, 
#					if not allocated, it does not appear in csv file;

def inode_allocation():
	if 2 not in range(super_block.first_nr_inode, super_block.num_inode+1):
		if 2 in freeino_list and 2 in ino_inode:
			print("ALLOCATED INODE 2 ON FREELIST")
		if 2 not in freeino_list and 2 not in ino_inode:
			print("UNALLOCATED INODE 2 NOT ON FREELIST")		
	for inode in range(super_block.first_nr_inode, super_block.num_inode+1):
		if inode in freeino_list and inode in ino_inode:
			print("ALLOCATED INODE {} ON FREELIST".format(inode))
		if inode not in freeino_list and inode not in ino_inode:
			print("UNALLOCATED INODE {} NOT ON FREELIST".format(inode))




if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Bad arguments. Correct usage: ./lab3a FILE_SYSTEM_IMAGE.",file=sys.stderr)
		sys.exit(1)
	try:
		fh = open(sys.argv[1],'r')
	except:
		print("Error: Failure opening file {}.".format(sys.argv[1]), file=sys.stderr)
		sys.exit(1)

	read_csv = csv.reader(fh)
	for row in read_csv:
		if row[0] == "INODE":
			if len(row) == 27:
				cur_inode = Inode(int(row[1]), int(row[6]), int(row[3]), list(map(int, row[12:24])), int(row[24]), int(row[25]), int(row[26]))
			else:
				#list(map(int, row[12:]))
				cur_inode = Inode(int(row[1]), int(row[6]), int(row[3]))
			ino_inode[int(row[1])] = cur_inode
			#add here to read reference block
		elif row[0] == "INDIRECT":
			cur_indirect = Indirect(int(row[2]),int(row[3]),int(row[5]))
			if int(row[1]) not in indirect_list:
				indirect_list[int(row[1])] = []
			indirect_list[int(row[1])].append(cur_indirect)
		elif row[0] == "BFREE":
			freeblock_list.add(int(row[1]))
		elif row[0] == "DIRENT":
			cur_dirent = Dirent(int(row[1]), int(row[3]), row[6])
			dirent_list.add(cur_dirent)
			if row[6] != '\'.\'' and row[6] != '\'..\'':
				dir_par[int(row[3])] = int(row[1])
		elif row[0] == "SUPERBLOCK":
			super_block.num_block = int(row[1])
			super_block.num_inode = int(row[2])
			super_block.block_size = int(row[3])
			super_block.inode_size = int(row[4])
			super_block.first_nr_inode = int(row[7])
		elif row[0] == "IFREE":
			freeino_list.add(int(row[1]))
		elif row[0] == "GROUP":
			group.group_number = int(row[1])
			group.num_blocks = int(row[2])
			group.num_inodes = int(row[3])
			group.num_free_blocks = int(row[4])
			group.num_free_inodes = int(row[5])
			group.first_block_of_inodes = int(row[8])


# check indirect block for reference blocks
	for inode in indirect_list:
		indirect_object = indirect_list[inode]
		for i in indirect_object:
			block = i.ref_blockno
			if block not in reference_block:
				reference_block[block] = []
			reference_block[block].append([inode, i.level, i.block_offset])
# check inode summary for reference blocks
	for inode in ino_inode:
		inode_object = ino_inode[inode]
		direct = inode_object.blocks
		for block in direct:
			if block != 0:
				if block not in reference_block:
					reference_block[block] = [] #-1 for offset unknown
				reference_block[block].append([inode, 0, 0])
		if inode_object.single != 0:
			if inode_object.single not in reference_block:
				reference_block[inode_object.single] = []
			reference_block[inode_object.single].append([inode, 1, 12])
		if inode_object.double != 0:
			if inode_object.double not in reference_block:
				reference_block[inode_object.double] = []
			reference_block[inode_object.double].append([inode, 2, 268])
		if inode_object.triple != 0:
			if inode_object.triple not in reference_block:
				reference_block[inode_object.triple] = []
			reference_block[inode_object.triple].append([inode, 3, 65804])		

	block_consistency()
	inode_allocation()
	handle_dir()
	sys.exit(return_code)
