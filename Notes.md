# File-Systems Notes

Software disk will never have IO errors

Properties to track:

+ Name
+ Location
+ Size

It will be a random access Filesystem.

File Control Block is something to implement

Cannot use:

+ Contiguous allocation
+ Linked allocation


Can Use:

+ FAT: File Allocation Table
  + Group all pointers into a table rather than spreading them all over the disk: Maybe cache the table into memory
  + Dir entry:
    + Name ... Start block

+ Unix inodes
  + Maintains file information in a struct that then references the blocks
  + first 12 -15 blocks are stored
  + Direct and single indirect
    + Indirect point to a raw block subdivided into unsigned longs that point at more blocks
  + set 15 direct blocks and maybe a single indirect

+ Free space management
  + Use bit vectors no need to look at the others
  + & <- bianary and
  + | <- bianary or


+ Do's and Don'ts
  + Disk: Free space management, then
  + dont dynamically allocate anything
