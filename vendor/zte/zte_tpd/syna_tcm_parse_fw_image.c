#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/crc32.h>
#include <linux/errno.h>

struct syna_tcm_partition {
    unsigned char enabled;          // offset 0
    unsigned char padding[7];       // offset 1-7
    unsigned char *data_ptr;        // offset 8
    unsigned int size;              // offset 16
    unsigned int addr;              // offset 20
    unsigned char id;               // offset 24
    unsigned char padding2[3];      // offset 25-27
    unsigned int checksum;          // offset 28
};

struct partition_info {
    unsigned int image_size;        // offset 0
    unsigned char padding[4];       // offset 4
    unsigned char *image_data;      // offset 8
    struct syna_tcm_partition partitions[24]; // offset 16 (0x10)
};

static const size_t partition_name_lens[23] = {
    9, 15, 11, 8, 10, 12, 14, 13, 6, 6, 17, 4, 7, 4, 11, 4, 5, 17, 17, 17, 15, 21, 16
};

static const char *get_partition_name(int idx) {
    switch (idx) {
        case 0: return "APP_CODE";
        case 1: return "APP_CODE_COPRO";
        case 2: return "APP_CONFIG";
        case 3: return "DISPLAY";
        case 4: return "BOOT_CODE";
        case 5: return "BOOT_CONFIG";
        case 6: return "APP_PROD_TEST";
        case 7: return "F35_APP_CODE";
        case 8: return "FORCE";
        case 9: return "GAMMA";
        case 10: return "TEMPERATURE_GAMMA";
        case 11: return "LCM";
        case 12: return "LOOKUP";
        case 13: return "OEM";
        case 14: return "OPEN_SHORT";
        case 15: return "OTP";
        case 16: return "PPDT";
        case 17: return "ROMBOOT_APP_CODE";
        case 18: return "TOOL_BOOT_CONFIG";
        case 19: return "JSON_CONFIG_AREA";
        case 20: return "CUSTOM_CS_AREA";
        case 21: return "CUSTOM_LOCKDOWN_AREA";
        case 22: return "CUSTOM_MTP_AREA";
        default: return " ";
    }
}

static const char *get_partition_name_alt(int idx) {
    switch (idx) {
        case 0: return "APP_CODE";
        case 1: return "APP_CODE_COPRO";
        case 2: return "APP_CONFIG";
        case 3: return "DISPLAY";
        case 4: return "BOOT_CODE";
        case 5: return "BOOT_CONFIG";
        case 6: return "APP_PROD_TEST";
        case 7: return "F35_APP_CODE";
        case 8: return "FORCE";
        case 9: return "GAMMA";
        case 10: return "TEMPERATURE_GAMMA";
        case 11: return "LCM";
        case 12: return "LOOKUP";
        case 13: return "OEM";
        case 14: return "OPEN_SHORT";
        case 15: return "OTP";
        case 16: return "PPDT";
        case 17: return "ROMBOOT_APP_CODE";
        case 18: return "TOOL_BOOT_CONFIG";
        case 19: return " ";
        case 20: return "CUSTOM_CS_AREA";
        case 21: return "CUSTOM_LOCKDOWN_AREA";
        case 22: return "CUSTOM_MTP_AREA";
        default: return " ";
    }
}

__int64 __fastcall syna_tcm_parse_fw_image(__int64 a1, _QWORD a2, _QWORD *a3)
{
    unsigned char *image_data = (unsigned char *)a1;
    unsigned int image_size = (unsigned int)a2;
    struct partition_info *parsed_info = (struct partition_info *)a3;
    unsigned int num_partitions;
    unsigned int i;
    int j;
    unsigned int offset;
    unsigned char *partition_ptr;
    unsigned int magic;
    unsigned int size;
    unsigned int addr;
    unsigned int file_checksum;
    unsigned int calculated_crc;
    unsigned char *data_ptr;
    unsigned int partition_id;
    unsigned int size_offset, data_offset;
    const char *name;
    size_t len;

    if (!image_data) {
        printk(KERN_ERR "%s: Invalid given data\n", "syna_tcm_parse_fw_image");
        return -241; // -EINVAL
    }
    if (!parsed_info) {
        printk(KERN_ERR "%s: Invalid image blob to store the parsed data\n", "syna_tcm_parse_fw_image");
        return -241; // -EINVAL
    }
    if (!image_size) {
        printk(KERN_ERR "%s: Invalid image data\n", "syna_tcm_parse_fw_image");
        return -241; // -EINVAL
    }

    // Initialize parsed_info
    parsed_info->image_size = image_size;
    parsed_info->image_data = image_data;
    memset((char *)parsed_info + 4, 0, 0x30c);

    // Validate magic
    magic = *(unsigned int *)image_data;
    if (magic != 0x4818472B) {
        printk(KERN_ERR "%s: Invalid image file magic value\n", "syna_tcm_parse_fw_image");
        return -241; // -EINVAL
    }

    num_partitions = *(unsigned int *)(image_data + 4);
    if (num_partitions == 0) {
        return 0;
    }

    for (i = 0; i < num_partitions; i++) {
        offset = *(unsigned int *)(image_data + 8 + i * 4);
        partition_ptr = image_data + offset;
        magic = *(unsigned int *)partition_ptr;

        if (magic == 0x7c05e516) {
            // Standard partition
            partition_id = 0;
            for (j = 22; j >= -1; j--) {
                if (j >= 0) {
                    name = get_partition_name(j);
                    len = strnlen(name, partition_name_lens[j]);
                } else {
                    name = " ";
                    len = 1;
                }

                if (strncmp((char *)(partition_ptr + 4), name, len) == 0) {
                    if (j >= 0) {
                        partition_id = j + 1;
                    } else {
                        // j == -1
                        partition_id = 0;
                    }
                    break;
                }
            }

            if (j == -2) {
                // Not found
                printk(KERN_WARNING "%s: Un-defined area string, %s\n", "syna_tcm_get_partition_id", partition_ptr + 4);
                continue;
            }

            if (partition_id >= 24) {
                continue;
            }

            size_offset = 28;
            data_offset = 36;
        } else if (magic == 0xc1fbc1d8) {
            // Alternative magic
            partition_id = 20; // JSON_CONFIG_AREA (id 20)
            size_offset = 4;
            data_offset = 8;
        } else {
            continue;
        }

        size = *(unsigned int *)(partition_ptr + size_offset);
        data_ptr = partition_ptr + data_offset;

        if (partition_id == 20) {
            parsed_info->partitions[20].enabled = 1;
            parsed_info->partitions[20].data_ptr = data_ptr;
            parsed_info->partitions[20].size = size;
            parsed_info->partitions[20].addr = 0;
            parsed_info->partitions[20].id = 20;

            printk(KERN_INFO "%s: AREA_TOOL_JSON area - size:%d\n", "syna_tcm_save_flash_partition_data", size);
        } else {
            file_checksum = *(unsigned int *)(partition_ptr + 32);
            calculated_crc = ~crc32_le(~0U, data_ptr, size);

            if (file_checksum != calculated_crc) {
                const char *err_name = (partition_id > 0 && partition_id <= 23) ? get_partition_name_alt(partition_id - 1) : " ";
                printk(KERN_ERR "%s: partition %s checksum error, image file: 0x%x (0x%x)\n",
                       "syna_tcm_save_flash_partition_data", err_name, file_checksum, calculated_crc);
                continue;
            }

            addr = *(unsigned int *)(partition_ptr + 24) * 2;

            parsed_info->partitions[partition_id].enabled = 1;
            parsed_info->partitions[partition_id].data_ptr = data_ptr;
            parsed_info->partitions[partition_id].size = size;
            parsed_info->partitions[partition_id].addr = addr;
            parsed_info->partitions[partition_id].id = partition_id;
            parsed_info->partitions[partition_id].checksum = file_checksum;

            const char *ok_name = (partition_id > 0 && partition_id <= 23) ? get_partition_name_alt(partition_id - 1) : " ";
            printk(KERN_INFO "%s: %s area - address:0x%08x (%d), size:%d\n",
                   "syna_tcm_save_flash_partition_data", ok_name, addr, addr, size);
        }

        printk(KERN_INFO "%s: debug area:%d, address:%px size:%d\n",
               "syna_tcm_parse_fw_image", partition_id, parsed_info->partitions[partition_id].data_ptr, parsed_info->partitions[partition_id].size);
    }

    return 0;
}
