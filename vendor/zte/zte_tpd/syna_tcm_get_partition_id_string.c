const char *__fastcall syna_tcm_get_partition_id_string(int a1)
{
    switch (a1) {
        case 1: return "Bootloader";
        case 2: return "Application";
        case 3: return "MTP";
        case 4: return "Config";
        case 5: return "Display Config";
        case 6: return "Flash Config";
        case 7: return "Utility";
        case 8: return "Guest";
        case 9: return "Index";
        case 10: return "Properties";
        case 11: return "Testing";
        case 12: return "Custom";
        case 13: return "Unknown1";
        case 14: return "Unknown2";
        case 15: return "Unknown3";
        case 16: return "Unknown4";
        case 17: return "Unknown5";
        case 18: return "Unknown6";
        case 19: return "Unknown7";
        case 20: return "Unknown8";
        case 21: return "Unknown9";
        case 22: return "Unknown10";
        case 23: return "Unknown11";
        default: return " ";
    }
}
