

/******************************************************************
* INCLUDE FILES                                                   *
*******************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> //atoi
#include <fcntl.h> //open
#include <sys/ioctl.h>
#include <unistd.h> //sleep, close
#include <getopt.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
//linux
//#include <linux/dvb/frontend.h>
//#include <linux/version.h>
#include "frontend.h"


/******************************************************************
* LOCAL MACROS                                                    *
*******************************************************************/
#define CMD_BUF_SIZE  256
//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
//#define SYS_DVBC_ANNEX_A SYS_DVBC_ANNEX_AC
//#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define SET_DTV_PRPERTY_1(prop, id, _cmd, val) \
		if(ARRAY_SIZE(prop) > (id + 1)) { \
			prop[id].cmd = _cmd; \
			prop[id].u.data = val; \
			id++; \
		} else { \
			printf("%s(): can't set %s, too many properties\n", __func__, #_cmd); \
		}
#define SET_DTV_PRPERTY SET_DTV_PRPERTY_1
#define SET_DTV_PRPERTY_0(prop, id, _cmd) SET_DTV_PRPERTY_1(prop, id, _cmd, 0)


#define TABLE_UINT_END_VALUE      0xdeadbeaf
#define TABLE_STR_END_VALUE       NULL
#define TABLE_UINT_STR_END_VALUE  {TABLE_UINT_END_VALUE, TABLE_STR_END_VALUE}
#define TABLE_UINT_STR_VALUE(v)   {v, #v}

#define table_for_each_entry(p_pos, table) \
  for (const table_UintStr_t *p_pos = table; \
     p_pos->value != TABLE_STR_END_VALUE; p_pos++)



#define TUNER_C_BAND_START         3000000 /*kHZ*/
#define TUNER_C_BAND_END           4200000 /*kHZ*/
#define TUNER_KU_LOW_BAND_START   10700000 /*kHZ*/
#define TUNER_KU_LOW_BAND_END     11700000 /*kHZ*/
#define TUNER_KU_HIGH_BAND_START  11700001 /*kHZ*/
#define TUNER_KU_HIGH_BAND_END    12750000 /*kHZ*/

#define get_modulation_name(mod)          table_UintStrLookup(fe_mod_desc, mod, "unknown")
#define get_delSys_name(delSys)           table_UintStrLookup(delivery_system_desc, delSys, "unknown")

#define get_polarization_name(pol)        table_UintStrLookup(fe_pol_desc, pol, "unknown")
#define parse_polarization_name(polName)  table_UintStrLookupR(fe_pol_desc, polName, SEC_VOLTAGE_13)

#define get_feType_name(type)             table_UintStrLookup(fe_typeNames, type, "unknown")

/******************************************************************
* LOCAL TYPEDEFS                                                  *
*******************************************************************/

typedef struct {
	uint32_t key;
	const char *value;
} table_UintStr_t;

/******************************************************************
* STATIC DATA                                                     *
*******************************************************************/
const char *version_str = "1.1 (2024-12-17)";

table_UintStr_t fe_typeNames[] = {
	{FE_QPSK, "DVB-S"},
	{FE_QAM,  "DVB-C"},
	{FE_OFDM, "DVB-T"},
	{FE_ATSC, "ATSC"},
	TABLE_UINT_STR_END_VALUE
};

table_UintStr_t delivery_system_desc[] = {
	TABLE_UINT_STR_VALUE(SYS_UNDEFINED),
	TABLE_UINT_STR_VALUE(SYS_DVBC_ANNEX_A), //DVB-C
	TABLE_UINT_STR_VALUE(SYS_DVBC_ANNEX_B),
	TABLE_UINT_STR_VALUE(SYS_DVBT),
	TABLE_UINT_STR_VALUE(SYS_DSS),
	TABLE_UINT_STR_VALUE(SYS_DVBS),
	TABLE_UINT_STR_VALUE(SYS_DVBS2),
	TABLE_UINT_STR_VALUE(SYS_DVBH),
	TABLE_UINT_STR_VALUE(SYS_ISDBT),
	TABLE_UINT_STR_VALUE(SYS_ISDBS),
	TABLE_UINT_STR_VALUE(SYS_ISDBC),
	TABLE_UINT_STR_VALUE(SYS_ATSC),
	TABLE_UINT_STR_VALUE(SYS_ATSCMH),
	TABLE_UINT_STR_VALUE(SYS_DTMB),
	TABLE_UINT_STR_VALUE(SYS_CMMB),
	TABLE_UINT_STR_VALUE(SYS_DAB),
	TABLE_UINT_STR_VALUE(SYS_DVBT2),
	TABLE_UINT_STR_VALUE(SYS_TURBO),
	TABLE_UINT_STR_VALUE(SYS_DVBC_ANNEX_C),
	TABLE_UINT_STR_VALUE(SYS_DVBC2),
	TABLE_UINT_STR_END_VALUE
};

table_UintStr_t fe_caps_desc[] = {
	TABLE_UINT_STR_VALUE(FE_IS_STUPID),
	TABLE_UINT_STR_VALUE(FE_CAN_INVERSION_AUTO),
	TABLE_UINT_STR_VALUE(FE_CAN_FEC_1_2),
	TABLE_UINT_STR_VALUE(FE_CAN_FEC_2_3),
	TABLE_UINT_STR_VALUE(FE_CAN_FEC_3_4),
	TABLE_UINT_STR_VALUE(FE_CAN_FEC_4_5),
	TABLE_UINT_STR_VALUE(FE_CAN_FEC_5_6),
	TABLE_UINT_STR_VALUE(FE_CAN_FEC_6_7),
	TABLE_UINT_STR_VALUE(FE_CAN_FEC_7_8),
	TABLE_UINT_STR_VALUE(FE_CAN_FEC_8_9),
	TABLE_UINT_STR_VALUE(FE_CAN_FEC_AUTO),
	TABLE_UINT_STR_VALUE(FE_CAN_QPSK),
	TABLE_UINT_STR_VALUE(FE_CAN_QAM_16),
	TABLE_UINT_STR_VALUE(FE_CAN_QAM_32),
	TABLE_UINT_STR_VALUE(FE_CAN_QAM_64),
	TABLE_UINT_STR_VALUE(FE_CAN_QAM_128),
	TABLE_UINT_STR_VALUE(FE_CAN_QAM_256),
	TABLE_UINT_STR_VALUE(FE_CAN_QAM_AUTO),
	TABLE_UINT_STR_VALUE(FE_CAN_TRANSMISSION_MODE_AUTO),
	TABLE_UINT_STR_VALUE(FE_CAN_BANDWIDTH_AUTO),
	TABLE_UINT_STR_VALUE(FE_CAN_GUARD_INTERVAL_AUTO),
	TABLE_UINT_STR_VALUE(FE_CAN_HIERARCHY_AUTO),
	TABLE_UINT_STR_VALUE(FE_CAN_8VSB),
	TABLE_UINT_STR_VALUE(FE_CAN_16VSB),
	TABLE_UINT_STR_VALUE(FE_HAS_EXTENDED_CAPS),
	TABLE_UINT_STR_VALUE(FE_CAN_2G_MODULATION),
	TABLE_UINT_STR_VALUE(FE_NEEDS_BENDING),
	TABLE_UINT_STR_VALUE(FE_CAN_RECOVER),
	TABLE_UINT_STR_VALUE(FE_CAN_MUTE_TS),
	TABLE_UINT_STR_END_VALUE
};

table_UintStr_t fe_mod_desc[] = {
	TABLE_UINT_STR_VALUE(QPSK),
	TABLE_UINT_STR_VALUE(QAM_16),
	TABLE_UINT_STR_VALUE(QAM_32),
	TABLE_UINT_STR_VALUE(QAM_64),
	TABLE_UINT_STR_VALUE(QAM_128),
	TABLE_UINT_STR_VALUE(QAM_256),
	TABLE_UINT_STR_VALUE(QAM_AUTO),
	TABLE_UINT_STR_VALUE(VSB_8),
	TABLE_UINT_STR_VALUE(VSB_16),
	TABLE_UINT_STR_VALUE(PSK_8),
	TABLE_UINT_STR_VALUE(APSK_16),
	TABLE_UINT_STR_VALUE(APSK_32),
	TABLE_UINT_STR_VALUE(DQPSK),
	TABLE_UINT_STR_VALUE(QAM_4_NR),
	TABLE_UINT_STR_VALUE(QAM_1024),
	TABLE_UINT_STR_VALUE(QAM_4096),
	TABLE_UINT_STR_VALUE(APSK_8_L),
	TABLE_UINT_STR_VALUE(APSK_16_L),
	TABLE_UINT_STR_VALUE(APSK_32_L),
	TABLE_UINT_STR_VALUE(APSK_64),
	TABLE_UINT_STR_VALUE(APSK_64_L),
	TABLE_UINT_STR_END_VALUE
};

//http://forum.sat-expert.com/antenny/2558-chto-takoe-h-v-i-l-r-ili-kak-resiver-pereklyuchaet-polyarizaciyu-i-ob-22kgc.html
table_UintStr_t fe_pol_desc[] = {
	TABLE_UINT_STR_VALUE(SEC_VOLTAGE_13),
	TABLE_UINT_STR_VALUE(SEC_VOLTAGE_18),
	TABLE_UINT_STR_VALUE(SEC_VOLTAGE_OFF),

	{SEC_VOLTAGE_18,   "0"},
	{SEC_VOLTAGE_18,   "h"},
	{SEC_VOLTAGE_18,   "hor"},
	{SEC_VOLTAGE_18,   "horizontal"},
	{SEC_VOLTAGE_18,   "left"},

	{SEC_VOLTAGE_13,   "1"},
	{SEC_VOLTAGE_13,   "v"},
	{SEC_VOLTAGE_13,   "ver"},
	{SEC_VOLTAGE_13,   "vertical"},
	{SEC_VOLTAGE_18,   "right"},

	{SEC_VOLTAGE_OFF,  "off"},
	TABLE_UINT_STR_END_VALUE
};

table_UintStr_t fe_tateFlags[] = {
	TABLE_UINT_STR_VALUE(FE_HAS_SIGNAL),
	TABLE_UINT_STR_VALUE(FE_HAS_CARRIER),
	TABLE_UINT_STR_VALUE(FE_HAS_VITERBI),
	TABLE_UINT_STR_VALUE(FE_HAS_SYNC),
	TABLE_UINT_STR_VALUE(FE_HAS_LOCK),
	TABLE_UINT_STR_VALUE(FE_TIMEDOUT),
	TABLE_UINT_STR_VALUE(FE_REINIT),
	TABLE_UINT_STR_END_VALUE
};


/******************************************************************
* FUNCTION IMPLEMENTATION                     <Module>_<Word>+    *
*******************************************************************/
const char *table_UintStrLookup(const table_UintStr_t table[], uint32_t key, const char *defaultValue)
{
/*	if(key < 0) {
		return defaultValue;
	}*/

	table_for_each_entry(p_item, table) {
		if(p_item->key == key) {
			return p_item->value;
		}
	}
	return defaultValue;
}

int32_t table_UintStrLookupR(const table_UintStr_t table[], const char *value, int32_t defaultValue)
{
	if(!value) {
		return defaultValue;
	}
	table_for_each_entry(p_item, table) {
		if(!strcasecmp(p_item->value, value)) {
			return p_item->key;
		}
	}
	return defaultValue;
}


static int cmppointers(const void *p1, const void *p2)
{
	const void *p1p = *((const void **)p1);
	const void *p2p = *((const void **)p2);
	return (p1p == p2p) ? 0 : ((p1p < p2p) ? -1 : 1);
}

static int32_t parse_name(const char *name, const table_UintStr_t dictionary[], int32_t default_id, const char *prefix)
{
	#define NAME_MAX_LEN 256
	int32_t name_id = default_id;
	const char *replaceSymbolsPtr[5]; // Note: last pointer is reserved for name end
	const char replaceSymbols[] = {' ', '-'};
	const char replaceSymbolTo[] = {0x0, '_'}; // 0x0 mean skip
	size_t replaceSymbolCount = 0;
	size_t name_len = 0;
	size_t prefix_len = 0;
	bool addPrefix = false;

	if(name == NULL) {
		printf("Error: No name passed to parse!");
		return name_id;
	} else {
		name_len = strlen(name);
	}

	if(prefix != NULL) {
		prefix_len = strlen(prefix);
		if(strncasecmp(name, prefix, prefix_len) != 0) {
			addPrefix = true;
		}
	}

	if((name_len + prefix_len) >= NAME_MAX_LEN) {
		printf("Error: Name too long to parse, length=%ld, name=\"%s\"!", name_len, name);
		return name_id;
	}

	for(size_t i = 0; i < ARRAY_SIZE(replaceSymbols); i++) {
		const char *symbol = name;
		while(replaceSymbolCount < ARRAY_SIZE(replaceSymbolsPtr)) {
			symbol = strchr(symbol, replaceSymbols[i]);
			if(symbol == NULL) {
				break;
			}
			replaceSymbolsPtr[replaceSymbolCount] = symbol;
			replaceSymbolCount++;
			symbol++;
		}
	}
	if(replaceSymbolCount >= ARRAY_SIZE(replaceSymbolsPtr)) {
		printf("Too much variants in delivery system value, skip some!");
	}
	replaceSymbolsPtr[replaceSymbolCount] = name + name_len;
	qsort(replaceSymbolsPtr, replaceSymbolCount + 1, sizeof(replaceSymbolsPtr[0]), cmppointers);

	for(int32_t mask = 0; mask < (1 << replaceSymbolCount); mask++) {
		char mangled_name[NAME_MAX_LEN];
		char *_name_ptr = mangled_name;

		if(addPrefix) {
			memcpy(_name_ptr, prefix, prefix_len);
			_name_ptr += prefix_len;
		}
		memcpy(_name_ptr, name, replaceSymbolsPtr[0] - name);
		_name_ptr += replaceSymbolsPtr[0] - name;
		for(uint32_t replaceSymbolId = 0; replaceSymbolId < replaceSymbolCount; replaceSymbolId++) {
			char replSym = replaceSymbolTo[(mask >> replaceSymbolId) & 0x01];
			if(replSym != 0x0) {
				*_name_ptr = replSym;
				_name_ptr++;
			}
			size_t pieceSize = replaceSymbolsPtr[replaceSymbolId + 1] - replaceSymbolsPtr[replaceSymbolId] - 1;
			if(pieceSize > 0) {
				memcpy(_name_ptr, replaceSymbolsPtr[replaceSymbolId] + 1, pieceSize);
				_name_ptr += pieceSize;
			}
		}
		*_name_ptr = 0x0;

		name_id = table_UintStrLookupR(dictionary, mangled_name, default_id);
		if(name_id != default_id) {
			// printf("%s()[%d]: Debug: mangled_name=%s, name_id=%d, addPrefix=%d, replaceSymbolCount=%ld\n",
			// 		__func__, __LINE__, mangled_name, name_id, addPrefix, replaceSymbolCount);
			return name_id;
		}
	}

	return name_id;
}

static int32_t parse_delivery(const char *delSysName)
{
	return parse_name(delSysName, delivery_system_desc, SYS_UNDEFINED, "SYS_");
}

static int32_t parse_modulation(const char *modName)
{
	return parse_name(modName, fe_mod_desc, QAM_AUTO, NULL);
}

static int dvb_isDelSysSatellite(fe_delivery_system_t delSys)
{
	return ((delSys == SYS_DVBS) || (delSys == SYS_DVBS2) ||
		(delSys == SYS_TURBO) || (delSys == SYS_ISDBS) || (delSys == SYS_DSS));
}

static int dvb_printFrontendInfo(int frontend_fd)
{
	int                       err;
	struct dvb_frontend_info  fe_info;
	struct dtv_properties     cmds;
	struct dtv_property       dvb_prop[DTV_IOCTL_MAX_MSGS];
	uint32_t                  propCount = 0;
	fe_delivery_system_t      curDelSys = SYS_UNDEFINED;

	SET_DTV_PRPERTY_0(dvb_prop, propCount, DTV_API_VERSION);
	cmds.num = propCount;
	cmds.props = dvb_prop;
	if(ioctl(frontend_fd, FE_GET_PROPERTY, &cmds) == -1) {
		printf("DVB API v3\n");
	} else {
		printf("DVB API v%d.%d\n", dvb_prop[0].u.data >> 8, dvb_prop[0].u.data & 0xff);
		if(dvb_prop[0].u.data >= 0x505) {
			propCount = 0;
			SET_DTV_PRPERTY_0(dvb_prop, propCount, DTV_ENUM_DELSYS);
			SET_DTV_PRPERTY_0(dvb_prop, propCount, DTV_DELIVERY_SYSTEM);
			cmds.num = propCount;
			cmds.props = dvb_prop;
			if(ioctl(frontend_fd, FE_GET_PROPERTY, &cmds) != -1) {
				for(uint32_t i = 0; i < cmds.num; i++) {
					switch(dvb_prop[i].cmd) {
					case DTV_ENUM_DELSYS:
						if(dvb_prop[i].u.buffer.len > 0) {
							printf("Tuner support the following delivery systems: ");
							for(uint32_t j = 0; j < dvb_prop[i].u.buffer.len; j++) {
								fe_delivery_system_t delSys = dvb_prop[i].u.buffer.data[j];
								printf("%s ", get_delSys_name(delSys));
							}
							printf("\n");
						}
						break;
					case DTV_DELIVERY_SYSTEM:
						curDelSys = dvb_prop[i].u.data;
						break;
					default:
						printf("%s(): warn, unexpected command: %d\n", __func__, dvb_prop[i].cmd);
						break;
					}
				}
			} else {
				printf("%s(): querying delivery system failed: %s\n", __func__, strerror(errno));
			}
		}
	}
	do {
		err = ioctl(frontend_fd, FE_GET_INFO, &fe_info);
		if(err < 0) {
			printf("%s(): ioctl FE_GET_INFO failed", __func__);
		}
	} while(err < 0);

	bool is_sat = dvb_isDelSysSatellite(curDelSys);
	printf( "Tuner info:\n"
			"\tDVB Model=%s\n"
			"\tType=%s\n"
			"\tfrequency range: %5.1f..%5.1f %s, stepsize: %5.1f %s\n"
			"\tsymbol rate range: %5.2f..%5.2f Msps\n",
			fe_info.name, get_delSys_name(curDelSys), //get_feType_name(fe_info.type),
			(float)fe_info.frequency_min / 1000000.0,
			(float)fe_info.frequency_max / 1000000.0, is_sat ? "GHz" : "MHz",
			(float)fe_info.frequency_stepsize / 1000.0, is_sat ? "MHz" : "kHz", // TODO: need to check
			(float)fe_info.symbol_rate_min / 1000000.0,
			(float)fe_info.symbol_rate_max / 1000000.0);
	printf("\tCapabilities:\n");

	table_for_each_entry(cur_desc, fe_caps_desc) {
		if(cur_desc->key & fe_info.caps) {
			printf("\t\t%s\n", cur_desc->value);
		}
	}
	return 0;
}

/**  @ingroup dvb
 *   @brief Print into stdout lock info
 *
 *   @param[in]  fd_frontend       Tuner file descriptor.
 *   @param[in]  p_status          Pointer to return status. Do nothing if NULL is pass.
 */
static void dvb_printLockInfo(int32_t fd_frontend, uint32_t *p_status)
{
	uint16_t  snr = 0, str = 0;
	int32_t   ber = 0, uncorrected_blocks = 0;
	uint32_t  status = 0;

	#define ioctl_read_and_check(_request, _var) { \
	  int ret = ioctl(fd_frontend, _request, &_var); \
	  if(ret != 0) { \
	    printf("%s(): get property %s failed: %s\n", __func__, #_request, strerror(errno)); \
	  } \
	}
	ioctl_read_and_check(FE_READ_STATUS,              status);
	ioctl_read_and_check(FE_READ_SNR,                 snr);
	ioctl_read_and_check(FE_READ_BER,                 ber);
	ioctl_read_and_check(FE_READ_SIGNAL_STRENGTH,     str);
	ioctl_read_and_check(FE_READ_UNCORRECTED_BLOCKS,  uncorrected_blocks);

	if(p_status) {
		*p_status = status;
	}

	printf("status=0x%08x: ", status);
	table_for_each_entry(p_pos, fe_tateFlags) {
		if(status & p_pos->key) {
			printf("%s ", p_pos->value);
		}
	}
	puts("");

	printf("v3 stat: str=%6.2f%%, snr=%6.2f%%, ber=%7d, uncorrected_blocks=%d\n",
		(float)str / 655.35, (float)snr / 655.35, ber, uncorrected_blocks);

#if !defined(DISABLE_DVB_V5_STATS)
	{
		// Check if DVB version is minimum 5.10.
		// * https://linuxtv.org/docs/libdvbv5/index.html#dvbv5_stats ;
		// * https://www.linuxtv.org/downloads/v4l-dvb-apis-old/frontend-properties.html#frontend-stat-properties
		struct dtv_properties    cmds;
		struct dtv_property      props[DTV_IOCTL_MAX_MSGS];
		uint32_t propCount = 0;
		
		table_UintStr_t fe_stats[] = {
			{DTV_STAT_SIGNAL_STRENGTH,      "signal strength"},
			{DTV_STAT_CNR,                  "carrier SNR"},
			{DTV_STAT_PRE_ERROR_BIT_COUNT,  "pre bit errors"},
			{DTV_STAT_PRE_TOTAL_BIT_COUNT,  "pre bits total"},
			{DTV_STAT_POST_ERROR_BIT_COUNT, "post bit errors"},
			{DTV_STAT_POST_TOTAL_BIT_COUNT, "post bits total"},
			{DTV_STAT_ERROR_BLOCK_COUNT,    "block errors"},
			{DTV_STAT_TOTAL_BLOCK_COUNT,    "blocks total"},
			TABLE_UINT_STR_END_VALUE
		};
		
		table_for_each_entry(p_pos, fe_stats) { // for(uint32_t i = 0; i < ARRAY_SIZE(stats); i++)
			SET_DTV_PRPERTY_0(props, propCount, p_pos->key);
		}
		cmds.num = propCount;
		cmds.props = props;
		if(ioctl(fd_frontend, FE_GET_PROPERTY, &cmds) == 0) {
			printf("v5 stat: ");
			for(uint32_t i = 0; i < cmds.num; i++) {
				switch(props[i].cmd) {
					case DTV_STAT_SIGNAL_STRENGTH:
					case DTV_STAT_CNR:
					case DTV_STAT_PRE_ERROR_BIT_COUNT:
					case DTV_STAT_PRE_TOTAL_BIT_COUNT:
					case DTV_STAT_POST_ERROR_BIT_COUNT:
					case DTV_STAT_POST_TOTAL_BIT_COUNT:
					case DTV_STAT_ERROR_BLOCK_COUNT:
					case DTV_STAT_TOTAL_BLOCK_COUNT:
						if(props[i].u.st.len > 0) {
							const char *stat_name = table_UintStrLookup(fe_stats, props[i].cmd, "unknown");
							printf("%s: ", stat_name);
							for(uint32_t j = 0; j < props[i].u.st.len; j++) {
								struct dtv_stats _st = props[i].u.st.stat[j];
								printf("%s", (j > 0) ? "|" : "");
								switch(_st.scale) {
								case FE_SCALE_NOT_AVAILABLE:
									printf("NA");
									break;
								case FE_SCALE_DECIBEL:
									printf("%7.3fdB", (float)_st.svalue / 1000.0);
									break;
								case FE_SCALE_RELATIVE:
									printf("%6.2f%%", (float)_st.uvalue / 655.35);
									break;
								case FE_SCALE_COUNTER:
									printf("%llu", _st.uvalue);
									break;
								default:
									printf("unknown scale %d", _st.scale);
									break;
								}
							}
							printf(", ");
						}
						break;
					default:
						printf("%s(): warn, unexpected command: %d\n", __func__, props[i].cmd);
						break;
				}
			}
			puts("");
		} else {
			printf("%s(): failed getting V5 statistics: %s\n", __func__, strerror(errno));
		}
	}
#endif
}

int dvb_setFrontendType(int32_t fd_frontend, fe_delivery_system_t type)
{
	struct dtv_property p = { .cmd = DTV_DELIVERY_SYSTEM, };
	struct dtv_properties cmdseq = {
		.num = 1,
		.props = &p
	};
	p.u.data = type;

	if (ioctl(fd_frontend, FE_SET_PROPERTY, &cmdseq) == -1) {
		printf("%s(): set property failed: %s\n", __func__, strerror(errno));
		return -1;
	}

	return 0;
}

int32_t dvb_openFronend(uint32_t adap, uint32_t fe, int32_t *fd, int32_t read_only)
{
	char *pathTemplate[] = {
		"/dev/dvb%d.frontend%d",
		"/dev/dvb/adapter%d/frontend%d",
	};
	uint32_t i;

	for(i = 0; i < ARRAY_SIZE(pathTemplate); i++) {
		char buf[CMD_BUF_SIZE];
		snprintf(buf, sizeof(buf), pathTemplate[i], adap, fe);
		if((*fd = open(buf, read_only ? O_RDONLY : O_RDWR)) >= 0) {
			printf("Opened %s\n", buf);
			return 0;
		}
		printf("fail\n");
	}

	return -1;
}


int dvb_diseqcSend(int frontend_fd, const uint8_t* tx, size_t tx_len)
{
	struct dvb_diseqc_master_cmd cmd;
	size_t i;

	printf("%s(): sending %zu:\n", __func__, tx_len);
	for(i = 0; i < tx_len; i++) {
		printf(" 0x%02x", tx[i]);
	}
	printf("\n");

	cmd.msg_len = tx_len;
	memcpy(cmd.msg, tx, cmd.msg_len);
	ioctl(frontend_fd, FE_DISEQC_SEND_MASTER_CMD, &cmd);

	return 0;
}

static inline uint8_t diseqc_data_lo(int satellite_position, int is_vertical, uint32_t f_khz)
{
	return (satellite_position & 0x03) << 2 | (is_vertical ? 0 : 1) << 1 | (f_khz > 11700000);
}

typedef enum {
	diseqcSwitchNone = 0,
	diseqcSwitchSimple,
	diseqcSwitchMulti,
	diseqcSwitchTypeCount,
} diseqcSwitchType_t;
uint32_t g_uncommited = 0;

int dvb_diseqcSetup(int frontend_fd, uint32_t frequency, diseqcSwitchType_t type, uint32_t port, uint32_t polarization)
{
	if(g_uncommited) {
		uint8_t ucmd[4] = { 0xe0, 0x10, 0x39, g_uncommited - 1 };
		dvb_diseqcSend(frontend_fd, ucmd, 4);
	}

	int _port = (type == diseqcSwitchMulti) ? port & 1 : port;
	uint8_t data_hi = (type == diseqcSwitchMulti) ? 0x70 : 0xF0;
	uint8_t cmd[4] = { 0xe0, 0x10, 0x38, data_hi | diseqc_data_lo(_port, (polarization == SEC_VOLTAGE_13), frequency) };

	dvb_diseqcSend(frontend_fd, cmd, 4);
	return 0;
}

static void usage(char *progname, int32_t verbose)
{

	printf("Usage: %s [OPTIONS]\n", progname);
	printf("Setup or read status of Linux DVB API v5 tuner frontends.\n\n");
	printf("Options:\n");
	printf("\t-h, --help                    - Print this message\n");
	printf("\t-V, --version                 - Print version\n");
	printf("\t-v, --verbose                 - Be verbose\n");
	printf("\t-d, --device=DEVID            - Choose dvb device /dev/dvb<DEVID>.frontend0\n");
	printf("\t-i, --info                    - Print tuner info\n");
	printf("\t-t, --del-sys=<");

	table_for_each_entry(p_item, delivery_system_desc) {
		printf("%s%s", (p_item == delivery_system_desc) ? "" : "|", p_item->value);
	}
	printf("> - Select delivery type\n");

	printf("\t-f, --frequency=FREQUENCY     - Set frequency in Hz (in kHz for sattelite delivery system)\n");
	printf("\t-s, --symbol-rate=SYMBOLRATE  - Set symbol rate in symbol per second\n");
	printf("\t-p, --plp-id=PLPID            - Set plp id (for DVB-T2)\n");

	printf("\t-m, --modulation=<");
	table_for_each_entry(p_item, fe_mod_desc) {
		printf("%s%s", (p_item == fe_mod_desc) ? "" : "|", p_item->value);
	}
	printf("> - Set modulation\n");
	printf("\t-c, --close-fe                - Close frontend at the end (infinity wait is default)\n");
	printf("\t-w, --wait-count=WAIT_COUNT   - Wait at most WAIT_COUNT times for frontend locking\n");
	printf("\t-z, --polarization=N, --pol=N - 0/h/horizontal/left - 18V, 1/v/vrtical/right - 13V\n");
	printf("\t-q, --dyseqc=PORT             - Use dyseqc SwitchSimple PORT (for satelite delivery system only)\n");
	printf("\t    --custom-LNB-LO=FREQUENCY - Use custom LNB local oscilator frequency in kHz.\n");
	if(verbose > 0) {
		printf("\t                                Note: This option disables checking for main frequency out-of-bounds.\n");
		printf("\t                                By default the following scheme (named \"Universal\" for Ku band) is used:\n");
		printf("\t                                  *  3.40- 4.20 GHz, C band       => LO:  5.15 GHz\n");
		printf("\t                                  * 10.70-11.70 GHz, Ku  low band => LO:  9.75 GHz\n");
		printf("\t                                  * 11.70-12.75 GHz, Ku high band => LO: 10.60 GHz\n");
		printf("\t                                Other known LNB types:\n");
		printf("\t                                  * 11.70-12.20 GHz, Ku band, LO: 10.75  GHz - Standard North America\n");
		printf("\t                                  * 12.20-12.70 GHz, Ku band, LO: 10.25  GHz - North America DBS\n");
		printf("\t                                  *                  Ku band, LO: 10.60  GHz - from tune-s2\n");
		printf("\t                                  *                  Ku band, LO: 10.745 GHz - from tune-s2\n");
		printf("\t                                  *                  Ku band, LO: 10.00  GHz - \"standard\" from szap-s2\n");
		printf("\t                                  *  18.2-19.2  GHz, Ka band, LO: 17.25  GHz - Norsat Ka band\n");
		printf("\t                                  *  20.2-21.2  GHz, Ka band, LO: 19.25  GHz -  low Ka band\n");
		printf("\t                                  *  21.2-22.2  GHz, Ka band, LO: 20.25  GHz - high Ka band\n");
	}
	printf("\t-r, --read-only               - Don't setup tuner, just read state\n");

	return;
}

static void version(void)
{
	printf("%s\n", version_str);
	return;
}


int main(int argc, char **argv)
{
	int32_t                fd_frontend;
	int32_t                opt;
	uint32_t               device = 0;
	int32_t                show_tuner_info = 0;
	int32_t                verbose = 0;
	fe_delivery_system_t   delivery_system = SYS_DVBC_ANNEX_A;
	uint32_t               frequency = 0;
	uint32_t               symbol_rate = 6900000;//6,9MHz
	fe_modulation_t        modulation = QAM_AUTO;
	uint32_t               plp_id = 0;
	int32_t                option_index = 0;
	int32_t                inversion = INVERSION_AUTO;
	int32_t                dont_close_fe = 1;
	int32_t                wait_count = -1;
	int32_t                has_lock = 0;
	int32_t                polarization = SEC_VOLTAGE_13;//0 - horizontal, 1 - vertical
	int32_t                dyseqc_port = -1;
	int32_t                read_only = 0;
	int32_t                custom_LNB_LO = -1;
	static struct option   long_options[] = {
		{"help",          no_argument,        0, 'h'},
		{"version",       no_argument,        0, 'V'},
		{"info",          no_argument,        0, 'i'},
		{"verbose",       no_argument,        0, 'v'},
		{"device",        required_argument,  0, 'd'},
		{"del-sys",       required_argument,  0, 't'},
		{"frequency",     required_argument,  0, 'f'},
		{"symbol-rate",   required_argument,  0, 's'},
		{"modulation",    required_argument,  0, 'm'},
		{"plp-id",        required_argument,  0, 'p'},
		{"inversion",     required_argument,  0, 'n'},
		{"close-fe",      no_argument,        0, 'c'},
		{"wait-count",    required_argument,  0, 'w'},
		{"polarization",  required_argument,  0, 'z'},
		{"pol",           required_argument,  0, 'z'},
		{"dyseqc",        required_argument,  0, 'q'},
		{"read-only",     no_argument,        0, 'r'},
		{"custom-LNB-LO", required_argument,  0, 0x10000000},
		{0, 0, 0, 0},
	};

	while((opt = getopt_long(argc, argv, "hVivd:t:f:s:m:p:n:cw:z:q:r", long_options, &option_index)) != -1) {
		switch(opt) {
			case 'h':
				usage(argv[0], verbose);
				return 0;
				break;
			case 'V':
				version();
				return 0;
				break;
			case 'i':
				show_tuner_info = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'd':
				device = atoi(optarg);
				break;
			case 't':
				delivery_system = parse_delivery(optarg);
				break;
			case 'f':
				frequency = atoi(optarg);
				break;
			case 's':
				symbol_rate = atoi(optarg);
				break;
			case 'm':
				modulation = parse_modulation(optarg);
				break;
			case 'p':
				plp_id = atoi(optarg);
				break;
			case 'n': {
				int32_t inv = atoi(optarg);
				if(inv > 0) {
					inversion = INVERSION_ON;
				} else if(inv == 0) {
					inversion = INVERSION_OFF;
				}
				break;
			}
			case 'c':
				dont_close_fe = 0;
				break;
			case 'w':
				wait_count = atoi(optarg);
				break;
			case 'z':
				polarization = parse_polarization_name(optarg);
				break;
			case 'q':
				dyseqc_port = atoi(optarg);
				break;
			case 'r':
				read_only = 1;
				break;
			case 0x10000000: // custom-LNB-LO
				custom_LNB_LO = atoi(optarg);
				break;
			default:
				usage(argv[0], verbose);
				return -3;
				break;
		}
	}

	if(dvb_openFronend(device, 0, &fd_frontend, read_only) != 0) {
		printf("%s()[%d]: Error open device=%d frontend\n", __func__, __LINE__, device);
		return -1;
	}

	if(read_only == 0) {
		struct dtv_property   dtv[DTV_IOCTL_MAX_MSGS];
		struct dtv_properties cmdseq;
		uint32_t              propCount = 0;

		if(frequency == 0) {
			printf("ERROR: Frequency not setted!\n");
			usage(argv[0], verbose);
			return -5;
		}

		printf( "Selected delivery system: %s\n", get_delSys_name(delivery_system));
		// TODO: check if we on DVB API v5 here.
		dvb_setFrontendType(fd_frontend, delivery_system);
		if(show_tuner_info) {
			dvb_printFrontendInfo(fd_frontend);
		}

		SET_DTV_PRPERTY_0(dtv, propCount, DTV_CLEAR);
		if(delivery_system == SYS_DVBC_ANNEX_A) {//DVB-C
			printf( "Tune frontend on:\n"
					"\tfreq        = %9d Hz\n"
					"\tsymbol_rate = %9d Hz\n"
					"\tmodulation  = %s\n",
					frequency, symbol_rate, get_modulation_name(modulation));

			SET_DTV_PRPERTY(dtv, propCount, DTV_FREQUENCY, frequency);
			SET_DTV_PRPERTY(dtv, propCount, DTV_MODULATION, modulation);
			SET_DTV_PRPERTY(dtv, propCount, DTV_SYMBOL_RATE, symbol_rate);
			SET_DTV_PRPERTY(dtv, propCount, DTV_INVERSION, inversion);
			SET_DTV_PRPERTY(dtv, propCount, DTV_INNER_FEC, FEC_AUTO);

		} else if((delivery_system == SYS_DVBT) || (delivery_system == SYS_DVBT2)) { //DVB-T/T2
			printf( "Tune frontend on:\n"
					"\tfreq        = %9d Hz\n"
					"\tmodulation  = %s\n",
					frequency, get_modulation_name(modulation));

			SET_DTV_PRPERTY(dtv, propCount, DTV_FREQUENCY, frequency);
			SET_DTV_PRPERTY(dtv, propCount, DTV_INVERSION, inversion);
			SET_DTV_PRPERTY(dtv, propCount, DTV_BANDWIDTH_HZ, 8000000);//BANDWIDTH_AUTO
			SET_DTV_PRPERTY(dtv, propCount, DTV_CODE_RATE_HP, FEC_AUTO);//FEC_7_8
			SET_DTV_PRPERTY(dtv, propCount, DTV_CODE_RATE_LP, FEC_AUTO);//FEC_7_8
			SET_DTV_PRPERTY(dtv, propCount, DTV_MODULATION, modulation);
			SET_DTV_PRPERTY(dtv, propCount, DTV_TRANSMISSION_MODE, TRANSMISSION_MODE_AUTO);//TRANSMISSION_MODE_8K
			SET_DTV_PRPERTY(dtv, propCount, DTV_GUARD_INTERVAL, GUARD_INTERVAL_AUTO);//GUARD_INTERVAL_1_16
			SET_DTV_PRPERTY(dtv, propCount, DTV_HIERARCHY, HIERARCHY_AUTO);

			if(delivery_system == SYS_DVBT2) {
				SET_DTV_PRPERTY(dtv, propCount, DTV_STREAM_ID, plp_id);
			}

		} else if((delivery_system == SYS_ATSC) || (delivery_system == SYS_DVBC_ANNEX_B)) { //ATSC
			printf( "Tune frontend on:\n"
					"\tfreq        = %9d Hz\n"
					"\tmodulation  = %s\n",
					frequency, get_modulation_name(modulation));

			SET_DTV_PRPERTY(dtv, propCount, DTV_FREQUENCY, frequency);
			SET_DTV_PRPERTY(dtv, propCount, DTV_INVERSION, inversion);//INVERSION_ON
			SET_DTV_PRPERTY(dtv, propCount, DTV_MODULATION, modulation);//VSB_8

		} else if((delivery_system == SYS_DVBS) || (delivery_system == SYS_DVBS2)) {//DVB-S/S2
			uint32_t frequencyOrig = frequency;
			uint32_t freqLO;
			uint32_t tone = SEC_TONE_OFF;

			//diseqc
			if((dyseqc_port >= 0) && (dyseqc_port < 4)) {
				dvb_diseqcSetup(fd_frontend, frequency, diseqcSwitchSimple, dyseqc_port, polarization);
			}

			if(custom_LNB_LO > 0) {
				freqLO = custom_LNB_LO;
				tone = SEC_TONE_OFF;
			} else if((TUNER_C_BAND_START <= frequency) && (frequency <= TUNER_C_BAND_END)) {
				freqLO = 5150000;
				tone = SEC_TONE_OFF;
			} else if((TUNER_KU_LOW_BAND_START <= frequency) && (frequency <= TUNER_KU_LOW_BAND_END)) {
				freqLO = 9750000;
				tone = SEC_TONE_OFF;
			} else if((TUNER_KU_HIGH_BAND_START <= frequency) && (frequency <= TUNER_KU_HIGH_BAND_END)) {
				freqLO = 10600000;
				tone = SEC_TONE_ON;
			} else {
				printf("%s()[%d]: !!!!!!\n", __func__, __LINE__);
			}
			frequency = abs((int32_t)frequency - (int32_t)freqLO);

			printf(
				"Tune frontend on:\n"
				"\tfreq         = %9d kHz\n"
				"\tfreq LO      = %9d kHz%s\n"
				"\tfreq inter   = %9d kHz\n"
				"\tsymbol_rate  = %9d Hz\n"
				"\tmodulation   = %s\n"
				"\tpolarization = %s\n",
				frequencyOrig, freqLO, (custom_LNB_LO > 0) ? " <- custom" : "",
				frequency, symbol_rate,
				get_modulation_name(modulation), get_polarization_name(polarization)
			);

			SET_DTV_PRPERTY(dtv, propCount, DTV_FREQUENCY, frequency);
			SET_DTV_PRPERTY(dtv, propCount, DTV_INVERSION, inversion);//INVERSION_ON
			SET_DTV_PRPERTY(dtv, propCount, DTV_MODULATION, modulation);
			SET_DTV_PRPERTY(dtv, propCount, DTV_SYMBOL_RATE, symbol_rate);
			SET_DTV_PRPERTY(dtv, propCount, DTV_VOLTAGE, polarization);
			SET_DTV_PRPERTY(dtv, propCount, DTV_TONE, tone);

		} else if(delivery_system == SYS_ISDBT) { // ISDB-T
			printf( "Tune frontend on:\n"
					"\tfreq        = %9d Hz\n",
					frequency);

			SET_DTV_PRPERTY(dtv, propCount, DTV_FREQUENCY, frequency);
			SET_DTV_PRPERTY(dtv, propCount, DTV_INVERSION, inversion);
			SET_DTV_PRPERTY(dtv, propCount, DTV_BANDWIDTH_HZ, 6000000);//BANDWIDTH_AUTO
			SET_DTV_PRPERTY(dtv, propCount, DTV_TRANSMISSION_MODE, TRANSMISSION_MODE_AUTO);//TRANSMISSION_MODE_8K
			SET_DTV_PRPERTY(dtv, propCount, DTV_GUARD_INTERVAL, GUARD_INTERVAL_AUTO);//GUARD_INTERVAL_1_16

			/*
			DTV_ISDBT_LAYER_ENABLED
			DTV_ISDBT_PARTIAL_RECEPTION
			DTV_ISDBT_SOUND_BROADCASTING
			DTV_ISDBT_SB_SUBCHANNEL_ID
			DTV_ISDBT_SB_SEGMENT_IDX
			DTV_ISDBT_SB_SEGMENT_COUNT
			DTV_ISDBT_LAYERA_FEC
			DTV_ISDBT_LAYERA_MODULATION
			DTV_ISDBT_LAYERA_SEGMENT_COUNT
			DTV_ISDBT_LAYERA_TIME_INTERLEAVING
			DTV_ISDBT_LAYERB_FEC
			DTV_ISDBT_LAYERB_MODULATION
			DTV_ISDBT_LAYERB_SEGMENT_COUNT
			DTV_ISDBT_LAYERB_TIME_INTERLEAVING
			DTV_ISDBT_LAYERC_FEC
			DTV_ISDBT_LAYERC_MODULATION
			DTV_ISDBT_LAYERC_SEGMENT_COUNT
			DTV_ISDBT_LAYERC_TIME_INTERLEAVING
			*/

		} else {
			printf("Not supported delivery system: %s\n", get_delSys_name(delivery_system));
			return -2;
		}
		SET_DTV_PRPERTY_0(dtv, propCount, DTV_TUNE);
		cmdseq.num = propCount;
		cmdseq.props = dtv;

		if(ioctl(fd_frontend, FE_SET_PROPERTY, &cmdseq) == -1) {
			perror("FRONTEND FE_SET_FRONTEND: ");
			return -3;
		}

		usleep(10000);//10ms
	} else {
		if(show_tuner_info) {
			dvb_printFrontendInfo(fd_frontend);
		}
	}

	while(1) {
		uint32_t status;
		dvb_printLockInfo(fd_frontend, &status);

		if(status & FE_HAS_LOCK) {
			has_lock = 1;
			printf("%s()[%d]: Locked success!!!\n", __func__, __LINE__);
			break;
		} else {
			sleep(1);
//			usleep(100000);
		}
		if(wait_count == 0) {
			break;
		} else if(wait_count > 0) {
			wait_count--;
		}
	}
	while(dont_close_fe) {
		sleep(1);
		if(verbose) {
			dvb_printLockInfo(fd_frontend, NULL);
		}
	}

	close(fd_frontend);
	return has_lock ? 0 : -4;
}
