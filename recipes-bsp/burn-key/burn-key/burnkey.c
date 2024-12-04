/*
 *  device infor write/read
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/types.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define DEV_MAGIC  "GENIATECH!"
#define MAGIC_SIZE 20
#define SN_LEN 50
#define MAC_LEN 50
#define OFFSET 34
#define PARAMS_PATH "/dev/disk/by-partlabel/params"
//#define PARAMS_PATH "/dev/disk/by-partlabel/params"
#define EEROM_PATH "/sys/devices/platform/fdd40000.i2c/i2c-0/0-0050/eeprom"
#define FLASH_LOCK  "FLASH_LOCK"
#define FLASH_UNLOCK  "FLASH_UNLOCK"
#define FLASH_LOCK_MAGIC   0x20181012
#define FLASH_UNLOCK_MAGIC 0xffffffff

enum {
	false	= 0,
	true	= 1
};

struct dev_info
{
	char magic[MAGIC_SIZE];
	char  board_sn[SN_LEN];
	char  mac_addr[MAC_LEN];
	unsigned int flash_lock;
};

//struct dev_info *devinfo_tmp;
static uint8_t read_flag;
static uint8_t write_flag;
static uint8_t force_flag;
static uint32_t status_flag;

static  char *flash_lock = "FLASH";

#define SN_FLAG		 0x1 << 0
#define MAC_FLAG         0x1 << 1
#define FLASH_FLAG       0x1 << 2

static void print_usage(const char *prog)
{
	printf("Usage: %s [-hRWFsm]\n", prog);
	puts("  -h --help         help \n"
	     "  -R --read         read the device info\n"
	     "  -W --write        write the device info\n"
	     "  -F --force        force write  the device info\n"
	     "  -s --board_sn     board_sn\n"
	     "  -m --mac_addr     eth mac add\n"
	     "  -l --flash_lock   flash lock\n");
	
	exit(1);
}

static const struct option lopts[] = {
	{ "help",         0, 0, 'h' },
	{ "read",         0, 0, 'R' },
        { "write",        0, 0, 'W' },
        { "force",        0, 0, 'F' },
	{ "board_sn",     1, 0, 's' },
	{ "mac_addr",     1, 0, 'm' },
        { "flash_lock",   1, 0, 'l' },
	{ NULL, 0, 0, 0 },
};
const char *optstring = "hRWFs:m:l:";

static void print_write_success(struct dev_info *info)
{   
    if ((status_flag & (SN_FLAG | MAC_FLAG)) == (SN_FLAG | MAC_FLAG)){
        printf("[WRITE_SUCCESS] SN:%s;MAC:%s \n",info->board_sn,info->mac_addr);
    }else{ 
        if (status_flag & SN_FLAG)
            printf("[WRITE_SUCCESS] SN:%s \n",info->board_sn);
        if (status_flag & MAC_FLAG)
            printf("[WRITE_SUCCESS] MAC:%s \n",info->mac_addr );
    }
}

static void print_fail()
{       
        printf("[BURN_FAIL]");
}

static void print_conflict()
{
        printf("[BURN_CONFLICT]");
}

static void print_unlock_success()
{
        printf("[UNLOCK_SUCCESS]");
}

static void parse_opts(int argc, char *argv[],const struct option *longopts,struct dev_info *info)
{
	int c; 

         if(info == NULL){
                printf("the devinfo is NULL failed\n");
                return ;
        }

	while (1) {

		c = getopt_long(argc, argv, optstring, longopts, NULL);

		if (c == -1)
			break;

		switch (c) {
                case 'R':
                        read_flag = true;
                        break;
                case 'W':
                        write_flag = true;
                        break;
                case 'F':
                        force_flag = true;
                        break;
		case 's':
			status_flag |= SN_FLAG;
			strcpy(info->board_sn,optarg);
			break;
		case 'm':
			status_flag |= MAC_FLAG;
			strcpy(info->mac_addr,optarg);
			break;
                case 'l':
                        status_flag |= FLASH_FLAG;
			flash_lock = optarg;
                        break;
		case 'h':
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

static int  write_devinfo_print(struct dev_info *info)
{
	if(info == NULL){
		printf("the devinfo is NULL failed\n");
		print_fail();
		return -1;
	}
	if(strcmp(info->magic,DEV_MAGIC)){
		printf("the magic not right failed\n");
		print_fail();
		return -1;
	}

    print_write_success(info);


	return 0;
}

static int  read_devinfo_print(struct dev_info *info)
{
	if(info == NULL){
		printf("the devinfo is NULL failed\n");
		return -1;
	}
	if(strcmp(info->magic,DEV_MAGIC)){
		printf("the magic not right failed\n");
		return -1;
	}
	if (status_flag & SN_FLAG)
		printf("SN:%s \n",info->board_sn);

	if (status_flag & MAC_FLAG)
		printf("MAC:%s \n",info->mac_addr );

        if (status_flag & FLASH_FLAG)
                printf("FLASH:0x%x \n",info->flash_lock );

	if(!status_flag ){
		printf("SN:%s \n",info->board_sn);
		printf("MAC:%s \n",info->mac_addr );
	}


	return 0;
}

static int open_dev(int mode)
{
    int fd;

    fd = open(EEROM_PATH, mode);
    if(fd >= 0){
        printf("Found eerom\n");
        return fd;
    }
    fd = open(PARAMS_PATH, mode);
    if(fd >= 0){
        return fd;
    }

    return fd;
}

static int get_key_value(struct dev_info *info)
{
	int fd;

	 if(info == NULL){
                printf("the dev info is NULL failed\n");
                return -1;
        }
        memset((void *)info, 0 , sizeof(struct dev_info));
	
    fd = open_dev(O_RDWR);
  	if(fd >= 0){
  		lseek(fd,OFFSET,SEEK_SET);
	  	if (read(fd, info, sizeof(struct dev_info)) < 0) {
			printf("read dev info failed\n");
			close(fd);
			return -1;	
	  	}
 	}else{
		printf("open  dev portfailed\n");
	}

	close(fd);
        return 0;
}

static int write_key_value(struct dev_info *info)
{
        int fd;
	int wsize=0;

         if(info == NULL){
                printf("the dev info is NULL failed\n");
                return -1;
        }

        fd = open_dev(O_RDWR);
        if(fd >= 0){
                lseek(fd,OFFSET,SEEK_SET);
                wsize=write(fd, info, sizeof(struct dev_info));
                if (wsize <= 0){
                        printf(" write devinfo fail\n");
                        close(fd);
                        return -1;;
                }
        }else{
                printf("open fd fail\n");
        }

        close(fd);
        return 0;
}

static int prepare_write_data(struct dev_info *devinfo,struct dev_info *devinfo_tmp)
{

         if((devinfo == NULL)||(devinfo_tmp == NULL)){
                printf("the dev info is NULL failed\n");
                return -1;
        }

	 if (status_flag & SN_FLAG)
		 strcpy(devinfo->board_sn,devinfo_tmp->board_sn);

	 if (status_flag & MAC_FLAG)
		 strcpy(devinfo->mac_addr,devinfo_tmp->mac_addr);

	 if (status_flag & FLASH_FLAG){
		 if (!strcmp(flash_lock,FLASH_LOCK))
			 devinfo->flash_lock = FLASH_LOCK_MAGIC;
		 else if (!strcmp(flash_lock,FLASH_UNLOCK))
			 devinfo->flash_lock = FLASH_UNLOCK_MAGIC;
		 else{
            int temp = FLASH_FLAG;
			status_flag &= (~temp);
			printf("the lock key not right\n");
			goto err;
		}
	}


	 return 0;
err:
	return -1;
}

int main(int argc, char *argv[])
{
	int ret = 0;

	read_flag = false;
	write_flag = false;
	force_flag = false;
	status_flag = 0;

	struct dev_info *devinfo_tmp = (struct dev_info *)malloc(sizeof(struct dev_info)); 
	if(devinfo_tmp == NULL){
		printf("malloc key devinfo_tmp failed\n");
		print_fail();
		return -1;
	}
	memset((void *)devinfo_tmp, 0 , sizeof(struct dev_info));
	parse_opts(argc, argv,lopts,devinfo_tmp);

	struct dev_info *devinfo = (struct dev_info *)malloc(sizeof(struct dev_info));        
	if(devinfo == NULL){
		printf("malloc key devinfo failed\n");
		free(devinfo_tmp);
		print_fail();
		return -1;
	}
	memset((void *)devinfo, 0 , sizeof(struct dev_info));

	ret =get_key_value(devinfo);
	if (ret != 0){
		printf(" read devinfo failed \n");
		print_fail();
		goto out;
	}
	if (write_flag == true){

		ret = prepare_write_data(devinfo,devinfo_tmp);
		if (ret){
			print_fail();
			goto out;
		}

		if (strcmp(devinfo->magic,DEV_MAGIC)){
			strcpy(devinfo->magic,DEV_MAGIC);
			ret = write_key_value(devinfo);
			if (ret){
				print_fail();
				goto out;
			}
		}else{
			if(force_flag == true){
				strcpy(devinfo->magic,DEV_MAGIC);
				ret = write_key_value(devinfo);
				if (ret){
					print_fail();
					goto out;
				}
			}else{
				printf("device devinfo been burned\n");
				print_conflict();
				ret = -2;
				goto out;
			}
		}

		memset((void *)devinfo, 0 , sizeof(struct dev_info));
		if (!get_key_value(devinfo)){
			
			if (status_flag & FLASH_FLAG){
				if(devinfo->flash_lock == FLASH_UNLOCK_MAGIC){
					print_unlock_success();
				}else{
					write_devinfo_print(devinfo);
				}
			}else{
				write_devinfo_print(devinfo);
			}
		}
	}
	/* print the devinfo*/
	if ((read_flag == true)||(argc == 1))
		read_devinfo_print(devinfo);
out:
	free(devinfo_tmp);
	free(devinfo);
	return ret;
}
