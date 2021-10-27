/* sample code for Part 2 */      
#include <stdio.h>
#include <fcntl.h>

#include <sys/types.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

struct partition {
	u8 drive;             // drive number FD=0, HD=0x80, etc.

	u8  head;             // starting head 
	u8  sector;           // starting sector
	u8  cylinder;         // starting cylinder

	u8  sys_type;         // partition type: NTFS, LINUX, etc.

	u8  end_head;         // end head 
	u8  end_sector;       // end sector
	u8  end_cylinder;     // end cylinder

	u32 start_sector;     // starting sector counting from 0 
	u32 nr_sectors;       // number of of sectors in partition
};

char *dev = "vdisk";
int fd;
    
// read a disk sector into char buf[512]
int read_sector(int fd, int sector, char *buf)
{
    lseek(fd, sector*512, SEEK_SET);  // lssek to byte sector*512
    read(fd, buf, 512);               // read 512 bytes into buf[ ]
}

int main()
{
  struct partition *p;
  char buf[512];

  fd = open(dev, O_RDONLY);   // open dev for READ
  read_sector(fd, 0, buf);    // read in MBR at sector 0    

  p = (struct partition *)(&buf[0x1be]); // p->P1
   
  // print P1's start_sector, nr_sectors, sys_type;
  // Write code to print all 4 partitions;
  int i = 0;
  while(i < 4)
  {
    printf("---partition %d---\n", i+1);
    printf("start sector -> %d \nnr_sectors -> %d \nsys_type -> %d\n", p->start_sector, p->nr_sectors, p->sys_type);
    i++;
    p++;
  }
  p--; //make start back on p4
  
  //Assume p4 is EXTEND type
  
  //let extstart be p4 start sector and print to see
  int extStart = p->start_sector;
  int localMBR = extStart;
  printf("local MBR = %d\n", localMBR);
  
  printf("---Looking for Extended Partition---\n");
  
  //show we start at p4
  printf("Extended partition 4: start_sector = 1440\n");
  
  //var for loop
  int currStart, currEnd, numSectors;
  i = 5;
  
  while(1)
  {
    //read in sector 1440 to buf
    read_sector(fd, localMBR, buf);
    
    //p is the first entry of two in buf
    p = (struct partition *)(&buf[0x1be]);
    
    //compute P(i) size
    currStart = localMBR + p->start_sector;
    currEnd = currStart + p->nr_sectors - 1;
    numSectors = p->nr_sectors;
    
    //print two entries
    printf("entry 1 start = %d || entry 1 nr_sectors = %d\n", p->start_sector, p->nr_sectors);
    p++;
    printf("entry 2 start = %d || entry 2 nr_sectors = %d\n", p->start_sector, p->nr_sectors);
    
    
    
    //print curr partition start, end, and numSectors
    printf("P%d : start_sector = %d  end_sector = %d  nr_sectors = %d\n", i, currStart, currEnd, numSectors);
    
    
    //check if we hit the end
    if(p->start_sector == 0)
    {
      break;
    }
    
    //to increment, next localMBR sector += start of entry2 (p)
    localMBR = extStart + p->start_sector;
    printf("Next sector = %d + %d = %d\n", extStart, p->start_sector, localMBR);
    i++;
  }

}
