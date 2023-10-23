

/******************************************************************
* INCLUDE FILES                                                   *
*******************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> //atoi
#include <fcntl.h> //open
#include <sys/ioctl.h>
#include <unistd.h> //sleep, close
#include <getopt.h>
#include <string.h>
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

#define SET_DTV_PRPERTY(prop, id, _cmd, val) \
		{ \
			prop[id].cmd = _cmd; \
			prop[id].u.data = val; \
			id++; \
		}

#define TABLE_INT_END_VALUE     0xdeadbeaf
#define TABLE_STR_END_VALUE     NULL
#define TABLE_INT_STR_END_VALUE {TABLE_INT_END_VALUE, TABLE_STR_END_VALUE}

#define TUNER_C_BAND_START         3000000 /*kHZ*/
#define TUNER_C_BAND_END           4200000 /*kHZ*/
#define TUNER_KU_LOW_BAND_START   10700000 /*kHZ*/
#define TUNER_KU_LOW_BAND_END     11700000 /*kHZ*/
#define TUNER_KU_HIGH_BAND_START  11700001 /*kHZ*/
#define TUNER_KU_HIGH_BAND_END    12750000 /*kHZ*/

#define get_modulation_name(mod)          table_UintStrLookup(fe_mod_desc, mod, "unknown")
#define parse_modulation(modName)         table_UintStrLookupR(fe_mod_desc, modName, QAM_AUTO)

#define get_delSys_name(delSys)           table_UintStrLookup(delivery_system_desc, delSys, "unknown")
#define parse_delivery(delSysName)        table_UintStrLookupR(delivery_system_desc, delSysName, SYS_UNDEFINED)

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
const char *version_str = "1.0 (2014-11-11)";

table_UintStr_t fe_typeNames[] = {
	{FE_QPSK, "DVB-S"},
	{FE_QAM,  "DVB-C"},
	{FE_OFDM, "DVB-T"},
	{FE_ATSC, "ATSC"},
	TABLE_INT_STR_END_VALUE
};

table_UintStr_t delivery_system_desc[] = {
	{SYS_UNDEFINED,    "SYS_UNDEFINED"},
	{SYS_DVBC_ANNEX_A, "SYS_DVBC_ANNEX_A"},//DVB-C
	{SYS_DVBC_ANNEX_B, "SYS_DVBC_ANNEX_B"},
	{SYS_DVBT,         "SYS_DVBT"},
	{SYS_DSS,          "SYS_DSS"},
	{SYS_DVBS,         "SYS_DVBS"},
	{SYS_DVBS2,        "SYS_DVBS2"},
	{SYS_DVBH,         "SYS_DVBH"},
	{SYS_ISDBT,        "SYS_ISDBT"},
	{SYS_ISDBS,        "SYS_ISDBS"},
	{SYS_ISDBC,        "SYS_ISDBC"},
	{SYS_ATSC,         "SYS_ATSC"},
	{SYS_ATSCMH,       "SYS_ATSCMH"},
	{SYS_DMBTH,        "SYS_DMBTH"},
	{SYS_CMMB,         "SYS_CMMB"},
	{SYS_DAB,          "SYS_DAB"},
	{SYS_DVBT2,        "SYS_DVBT2"},
	{SYS_TURBO,        "SYS_TURBO"},
	{SYS_DVBC_ANNEX_C, "SYS_DVBC_ANNEX_C"},
	TABLE_INT_STR_END_VALUE
};

table_UintStr_t fe_caps_desc[] = {
	{FE_IS_STUPID,                   "FE_IS_STUPID"},
	{FE_CAN_INVERSION_AUTO,          "FE_CAN_INVERSION_AUTO"},
	{FE_CAN_FEC_1_2,                 "FE_CAN_FEC_1_2"},
	{FE_CAN_FEC_2_3,                 "FE_CAN_FEC_2_3"},
	{FE_CAN_FEC_3_4,                 "FE_CAN_FEC_3_4"},
	{FE_CAN_FEC_4_5,                 "FE_CAN_FEC_4_5"},
	{FE_CAN_FEC_5_6,                 "FE_CAN_FEC_5_6"},
	{FE_CAN_FEC_6_7,                 "FE_CAN_FEC_6_7"},
	{FE_CAN_FEC_7_8,                 "FE_CAN_FEC_7_8"},
	{FE_CAN_FEC_8_9,                 "FE_CAN_FEC_8_9"},
	{FE_CAN_FEC_AUTO,                "FE_CAN_FEC_AUTO"},
	{FE_CAN_QPSK,                    "FE_CAN_QPSK"},
	{FE_CAN_QAM_16,                  "FE_CAN_QAM_16"},
	{FE_CAN_QAM_32,                  "FE_CAN_QAM_32"},
	{FE_CAN_QAM_64,                  "FE_CAN_QAM_64"},
	{FE_CAN_QAM_128,                 "FE_CAN_QAM_128"},
	{FE_CAN_QAM_256,                 "FE_CAN_QAM_256"},
	{FE_CAN_QAM_AUTO,                "FE_CAN_QAM_AUTO"},
	{FE_CAN_TRANSMISSION_MODE_AUTO,  "FE_CAN_TRANSMISSION_MODE_AUTO"},
	{FE_CAN_BANDWIDTH_AUTO,          "FE_CAN_BANDWIDTH_AUTO"},
	{FE_CAN_GUARD_INTERVAL_AUTO,     "FE_CAN_GUARD_INTERVAL_AUTO"},
	{FE_CAN_HIERARCHY_AUTO,          "FE_CAN_HIERARCHY_AUTO"},
	{FE_CAN_8VSB,                    "FE_CAN_8VSB"},
	{FE_CAN_16VSB,                   "FE_CAN_16VSB"},
	{FE_HAS_EXTENDED_CAPS,           "FE_HAS_EXTENDED_CAPS"},
	{FE_CAN_2G_MODULATION,           "FE_CAN_2G_MODULATION"},
	{FE_NEEDS_BENDING,               "FE_NEEDS_BENDING"},
	{FE_CAN_RECOVER,                 "FE_CAN_RECOVER"},
	{FE_CAN_MUTE_TS,                 "FE_CAN_MUTE_TS"},
	TABLE_INT_STR_END_VALUE
};

table_UintStr_t fe_mod_desc[] = {
	{QPSK,     "QPSK"},
	{QAM_16,   "QAM_16"},
	{QAM_32,   "QAM_32"},
	{QAM_64,   "QAM_64"},
	{QAM_128,  "QAM_128"},
	{QAM_256,  "QAM_256"},
	{QAM_AUTO, "QAM_AUTO"},
	{VSB_8,    "VSB_8"},
	{VSB_16,   "VSB_16"},
	{PSK_8,    "PSK_8"},
	{APSK_16,  "APSK_16"},
	{APSK_32,  "APSK_32"},
	{DQPSK,    "DQPSK"},
	{QAM_4_NR, "QAM_4_NR"},
	TABLE_INT_STR_END_VALUE
};

//http://forum.sat-expert.com/antenny/2558-chto-takoe-h-v-i-l-r-ili-kak-resiver-pereklyuchaet-polyarizaciyu-i-ob-22kgc.html
table_UintStr_t fe_pol_desc[] = {
	{SEC_VOLTAGE_13,   "SEC_VOLTAGE_13"},
	{SEC_VOLTAGE_18,   "SEC_VOLTAGE_18"},
	{SEC_VOLTAGE_OFF,  "SEC_VOLTAGE_OFF"},

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
	TABLE_INT_STR_END_VALUE
};

/******************************************************************
* FUNCTION IMPLEMENTATION                     <Module>_<Word>+    *
*******************************************************************/
const char *table_UintStrLookup(const table_UintStr_t table[], uint32_t key, char *defaultValue)
{
	int32_t i;
/*	if(key < 0) {
		return defaultValue;
	}*/
	for(i = 0; table[i].value != TABLE_STR_END_VALUE; i++) {
		if(table[i].key == key) {
			return table[i].value;
		}
	}
	return defaultValue;
}

int32_t table_UintStrLookupR(const table_UintStr_t table[], char *value, int32_t defaultValue)
{
	int32_t i;
	if(!value) {
		return defaultValue;
	}
	for(i = 0; table[i].value != TABLE_STR_END_VALUE; i++) {
		if(!strcasecmp(table[i].value, value)) {
			return table[i].key;
		}
	}
	return defaultValue;
}

static int dvb_printFrontendInfo(int frontend_fd)
{
	int                      err;
	table_UintStr_t          *cur_desc = fe_caps_desc;
	struct dvb_frontend_info fe_info;
	struct dtv_properties    dtv_prop;
	struct dtv_property      dvb_prop[DTV_MAX_COMMAND];

	dvb_prop[0].cmd = DTV_API_VERSION;

	dtv_prop.num = 1;
	dtv_prop.props = dvb_prop;
	if(ioctl(frontend_fd, FE_GET_PROPERTY, &dtv_prop) == -1) {
		printf("DVB API v3\n");
	} else {
		printf("DVB API v%d.%d\n", dvb_prop[0].u.data >> 8, dvb_prop[0].u.data & 0xff);
		if(dvb_prop[0].u.data >= 0x505) {
			dvb_prop[0].cmd = DTV_ENUM_DELSYS;
			dtv_prop.num = 1;
			dtv_prop.props = dvb_prop;
			if(ioctl(frontend_fd, FE_GET_PROPERTY, &dtv_prop) != -1) {
				if(dvb_prop[0].u.buffer.len > 0) {
					uint32_t i;
					printf("Supported delivery systems: ");
					for(i = 0; i < dvb_prop[0].u.buffer.len; i++) {
						fe_delivery_system_t delSys = dvb_prop[0].u.buffer.data[i];
						printf("%s ", get_delSys_name(delSys));
					}
					printf("\n");
				}
			}
		}
	}
	do {
		err = ioctl(frontend_fd, FE_GET_INFO, &fe_info);
		if (err < 0) {
			printf("%s: ioctl FE_GET_INFO failed", __func__);
		}
	} while(err < 0);
	printf( "Tuner info:\n"
			"\tDVB Model=%s\n"
			"\tType=%s\n"
			"\tfrequency_min  =%9u Hz, frequency_max  =%9u Hz, frequency_stepsize=%9u Hz\n"
			"\tsymbol_rate_min=%9u Hz, symbol_rate_max=%9u Hz\n",
			fe_info.name, get_feType_name(fe_info.type),
			fe_info.frequency_min, fe_info.frequency_max, fe_info.frequency_stepsize,
			fe_info.symbol_rate_min, fe_info.symbol_rate_max);
	printf("\tCapabilities:\n");

	while(cur_desc->value != TABLE_STR_END_VALUE) {
		if(cur_desc->key & fe_info.caps) {
			printf("\t\t%s\n", cur_desc->value);
		}
		cur_desc++;
	}
	return 0;
}

/**  @ingroup dvb
 *   @brief Print into stdout lock info
 *
 *   @param[in]  fd_frontend       Tuner file descriptor.
 *   @param[in]  p_status          Pointer to readed status. Force trying to read status if NULL pass.
 */
static void dvb_printLockInfo(int32_t fd_frontend, uint32_t *p_status)
{
	int32_t ber;
	u_int16_t snr, str;
	int32_t uncorrected_blocks;
	uint32_t status;

	if(p_status) {
		status = *p_status;
	} else {
		ioctl(fd_frontend, FE_READ_STATUS, &status);
	}
	ioctl(fd_frontend, FE_READ_SNR, &snr);
	ioctl(fd_frontend, FE_READ_SIGNAL_STRENGTH, &str);
	ioctl(fd_frontend, FE_READ_BER, &ber);
	ioctl(fd_frontend, FE_READ_UNCORRECTED_BLOCKS, &uncorrected_blocks);

	printf("status=0x%02x: %s%s%s%s%s%s%s\n", status,
					(status & FE_HAS_SIGNAL)  ? "FE_HAS_SIGNAL "  : "",
					(status & FE_HAS_CARRIER) ? "FE_HAS_CARRIER " : "",
					(status & FE_HAS_VITERBI) ? "FE_HAS_VITERBI " : "",
					(status & FE_HAS_SYNC)    ? "FE_HAS_SYNC "    : "",
					(status & FE_HAS_LOCK)    ? "FE_HAS_LOCK "    : "",
					(status & FE_TIMEDOUT)    ? "FE_TIMEDOUT "    : "",
					(status & FE_REINIT)      ? "FE_REINIT "      : "");

	printf("snr=%3d%%\tstr=%3d%%\tber=%7d\tuncorrected_blocks=%d\n", (int)snr*100/65535, (int)str*100/65535, ber, uncorrected_blocks);

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
		printf("%s: set property failed: %s\n", __func__, strerror(errno));
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
		printf("Try to open %s ... ", buf);
		if((*fd = open(buf, read_only ? O_RDONLY : O_RDWR)) >= 0) {
			printf("success\n");
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

	printf("%s: sending %zu:\n", __func__, tx_len);
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

static void usage(char *progname)
{
	table_UintStr_t *table_IntStr_ptr;

	printf("Usage: %s [OPTIONS]\n", progname);
	printf("\t-h, --help                    - Print this message\n");
	printf("\t-V, --version                 - Print version\n");
	printf("\t-v, --verbose                 - Be verbose\n");
	printf("\t-d, --device=DEVID            - Choose dvb device /dev/dvb<DEVID>.frontend0\n");
	printf("\t-i, --info                    - Print tuner info\n");
	printf("\t-t, --del-sys=<");

	table_IntStr_ptr = delivery_system_desc;
	while(table_IntStr_ptr->value != TABLE_STR_END_VALUE) {
		printf("%s%s", (table_IntStr_ptr == delivery_system_desc) ? "" : "|", table_IntStr_ptr->value);
		table_IntStr_ptr++;
	}
	printf("> - Select delivery type\n");

	printf("\t-f, --frequency=FREQUENCY     - Set frequency in Hz (in KHz for sattelite delivery system)\n");
	printf("\t-s, --symbol-rate=SYMBOLRATE  - Set symbol rate in symbol per second\n");
	printf("\t-p, --plp-id=PLPID            - Set plp id (for DVB-T2)\n");

	printf("\t-m, --modulation=<");
	table_IntStr_ptr = fe_mod_desc;
	while(table_IntStr_ptr->value != TABLE_STR_END_VALUE) {
		printf("%s%s", (table_IntStr_ptr == fe_mod_desc) ? "" : "|", table_IntStr_ptr->value);
		table_IntStr_ptr++;
	}
	printf("> - Set modulation\n");
	printf("\t-c, --close-fe                - Close frontend at the end (infinity wait is default)\n");
	printf("\t-w, --wait-count=WAIT_COUNT   - Wait at most WAIT_COUNT times for frontend locking\n");
	printf("\t-z, --polarization=N, --pol=N - 0/h/horizontal/left - 18V, 1/v/vrtical/right - 13V\n");
	printf("\t-q, --dyseqc=PORT             - Use dyseqc SwitchSimple PORT (for satelite delivery system only)\n");
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
	struct dtv_property    dtv[16];//check if it enough
	struct dtv_properties  cmdseq;
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
	int32_t                propCount = 0;
	int32_t                dyseqc_port = -1;
	int32_t                read_only = 0;
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
		{0, 0, 0, 0},
	};

	while((opt = getopt_long(argc, argv, "hVivd:t:f:s:m:p:n:cw:z:q:r", long_options, &option_index)) != -1) {
		switch(opt) {
			case 'h':
				usage(argv[0]);
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
			default:
				usage(argv[0]);
				return -3;
				break;
		}
	}

	if(dvb_openFronend(device, 0, &fd_frontend, read_only) != 0) {
		printf("%s[%d]: Error open device=%d frontend\n", __FILE__, __LINE__, device);
		return -1;
	}

	if(read_only == 0) {
		if(frequency == 0) {
			printf("ERROR: Frequency not setted!\n");
			usage(argv[0]);
			return -5;
		}

		dvb_setFrontendType(fd_frontend, delivery_system);
		if(show_tuner_info) {
			dvb_printFrontendInfo(fd_frontend);
		}
		printf( "Selected delivery sistem: %s\n", get_delSys_name(delivery_system));

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

		} else if((delivery_system == SYS_DVBS) || (delivery_system == SYS_DVBS2)) {//DVB-C
			uint32_t freqLO;
			uint32_t tone = SEC_TONE_OFF;

			//diseqc
			if((dyseqc_port >= 0) && (dyseqc_port < 4)) {
				dvb_diseqcSetup(fd_frontend, frequency, diseqcSwitchSimple, dyseqc_port, polarization);
			}

			if((TUNER_C_BAND_START <= frequency) && (frequency <= TUNER_C_BAND_END)) {
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
			//north america: freqLO = 11250000
			frequency -= freqLO;

			printf( "Tune frontend on:\n"
					"\tfreq         = %9d KHz\n"
					"\tfreqLO       = %9d KHz\n"
					"\tsymbol_rate  = %9d Hz\n"
					"\tmodulation   = %s\n"
					"\tpolarization = %s\n",
					frequency, freqLO, symbol_rate, get_modulation_name(modulation), get_polarization_name(polarization));

			SET_DTV_PRPERTY(dtv, propCount, DTV_FREQUENCY, frequency);
			SET_DTV_PRPERTY(dtv, propCount, DTV_INVERSION, inversion);//INVERSION_ON
			SET_DTV_PRPERTY(dtv, propCount, DTV_MODULATION, modulation);
			SET_DTV_PRPERTY(dtv, propCount, DTV_SYMBOL_RATE, symbol_rate);
			SET_DTV_PRPERTY(dtv, propCount, DTV_VOLTAGE, polarization);
			SET_DTV_PRPERTY(dtv, propCount, DTV_TONE, tone);

		} else {
			printf("Not supported delivery system: %s\n", get_delSys_name(delivery_system));
			return -2;
		}
		dtv[propCount].cmd = DTV_TUNE;
		propCount++;
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
		ioctl(fd_frontend, FE_READ_STATUS, &status);
		dvb_printLockInfo(fd_frontend, &status);

		if(status & FE_HAS_LOCK) {
			has_lock = 1;
			printf("%s[%d]: Locked success!!!\n", __FILE__, __LINE__);
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
			uint32_t status;
			ioctl(fd_frontend, FE_READ_STATUS, &status);
			dvb_printLockInfo(fd_frontend, &status);
		}
	}

	close(fd_frontend);
	return has_lock ? 0 : -4;
}
