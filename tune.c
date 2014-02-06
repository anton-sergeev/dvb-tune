

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
#define CMD_BUF_SIZE	256
//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
//#define SYS_DVBC_ANNEX_A SYS_DVBC_ANNEX_AC
//#endif

#define ARRAY_SIZE(arr)	(sizeof(arr) / sizeof(arr[0]))

/******************************************************************
* LOCAL TYPEDEFS                                                  *
*******************************************************************/
typedef struct {
	fe_delivery_system_t	system;
	char					*name;
} delivery_system_desc_t;

typedef struct {
	fe_caps_t				capability;
	char					*name;
} fe_caps_desc_t;

typedef struct {
	fe_modulation_t			modulation;
	char					*name;
} fe_mod_desc_t;

/******************************************************************
* STATIC DATA                                                     *
*******************************************************************/
static char *fe_typeNames[] = {
	"DVB-S",
	"DVB-C",
	"DVB-T",
	"ATSC"
};

delivery_system_desc_t delivery_system_desc[] = {
	{SYS_UNDEFINED,		"SYS_UNDEFINED"},
	{SYS_DVBC_ANNEX_A,	"SYS_DVBC_ANNEX_A"},//DVB-C
	{SYS_DVBC_ANNEX_B,	"SYS_DVBC_ANNEX_B"},
	{SYS_DVBT,			"SYS_DVBT"},
	{SYS_DSS,			"SYS_DSS"},
	{SYS_DVBS,			"SYS_DVBS"},
	{SYS_DVBS2,			"SYS_DVBS2"},
	{SYS_DVBH,			"SYS_DVBH"},
	{SYS_ISDBT,			"SYS_ISDBT"},
	{SYS_ISDBS,			"SYS_ISDBS"},
	{SYS_ISDBC,			"SYS_ISDBC"},
	{SYS_ATSC,			"SYS_ATSC"},
	{SYS_ATSCMH,		"SYS_ATSCMH"},
	{SYS_DMBTH,			"SYS_DMBTH"},
	{SYS_CMMB,			"SYS_CMMB"},
	{SYS_DAB,			"SYS_DAB"},
	{SYS_DVBT2,			"SYS_DVBT2"},
	{SYS_TURBO,			"SYS_TURBO"},
	{SYS_DVBC_ANNEX_C,	"SYS_DVBC_ANNEX_C"},
};

fe_caps_desc_t fe_caps_desc[] = {
	{FE_IS_STUPID,					"FE_IS_STUPID"},
	{FE_CAN_INVERSION_AUTO,			"FE_CAN_INVERSION_AUTO"},
	{FE_CAN_FEC_1_2,				"FE_CAN_FEC_1_2"},
	{FE_CAN_FEC_2_3,				"FE_CAN_FEC_2_3"},
	{FE_CAN_FEC_3_4,				"FE_CAN_FEC_3_4"},
	{FE_CAN_FEC_4_5,				"FE_CAN_FEC_4_5"},
	{FE_CAN_FEC_5_6,				"FE_CAN_FEC_5_6"},
	{FE_CAN_FEC_6_7,				"FE_CAN_FEC_6_7"},
	{FE_CAN_FEC_7_8,				"FE_CAN_FEC_7_8"},
	{FE_CAN_FEC_8_9,				"FE_CAN_FEC_8_9"},
	{FE_CAN_FEC_AUTO,				"FE_CAN_FEC_AUTO"},
	{FE_CAN_QPSK,					"FE_CAN_QPSK"},
	{FE_CAN_QAM_16,					"FE_CAN_QAM_16"},
	{FE_CAN_QAM_32,					"FE_CAN_QAM_32"},
	{FE_CAN_QAM_64,					"FE_CAN_QAM_64"},
	{FE_CAN_QAM_128,				"FE_CAN_QAM_128"},
	{FE_CAN_QAM_256,				"FE_CAN_QAM_256"},
	{FE_CAN_QAM_AUTO,				"FE_CAN_QAM_AUTO"},
	{FE_CAN_TRANSMISSION_MODE_AUTO,	"FE_CAN_TRANSMISSION_MODE_AUTO"},
	{FE_CAN_BANDWIDTH_AUTO,			"FE_CAN_BANDWIDTH_AUTO"},
	{FE_CAN_GUARD_INTERVAL_AUTO,	"FE_CAN_GUARD_INTERVAL_AUTO"},
	{FE_CAN_HIERARCHY_AUTO,			"FE_CAN_HIERARCHY_AUTO"},
	{FE_CAN_8VSB,					"FE_CAN_8VSB"},
	{FE_CAN_16VSB,					"FE_CAN_16VSB"},
	{FE_HAS_EXTENDED_CAPS,			"FE_HAS_EXTENDED_CAPS"},
	{FE_CAN_2G_MODULATION,			"FE_CAN_2G_MODULATION"},
	{FE_NEEDS_BENDING,				"FE_NEEDS_BENDING"},
	{FE_CAN_RECOVER,				"FE_CAN_RECOVER"},
	{FE_CAN_MUTE_TS,				"FE_CAN_MUTE_TS"},
};

fe_mod_desc_t fe_mod_desc[] = {
	{QPSK,		"QPSK"},
	{QAM_16,	"QAM_16"},
	{QAM_32,	"QAM_32"},
	{QAM_64,	"QAM_64"},
	{QAM_128,	"QAM_128"},
	{QAM_256,	"QAM_256"},
	{QAM_AUTO,	"QAM_AUTO"},
	{VSB_8,		"VSB_8"},
	{VSB_16,	"VSB_16"},
	{PSK_8,		"PSK_8"},
	{APSK_16,	"APSK_16"},
	{APSK_32,	"APSK_32"},
	{DQPSK,		"DQPSK"},
};

/******************************************************************
* FUNCTION IMPLEMENTATION                     <Module>_<Word>+    *
*******************************************************************/
static fe_delivery_system_t parse_delivery(char *mod_str)
{
	uint32_t	i;
	for(i = 0; i < ARRAY_SIZE(delivery_system_desc); i++) {
		delivery_system_desc_t *cur_sys = delivery_system_desc + i;
		if(strcmp(mod_str, cur_sys->name) == 0) {
			return cur_sys->system;
		}
	}
	return SYS_UNDEFINED;
}

static char *get_delivery_system_name(fe_delivery_system_t delivery_system)
{
	uint32_t	i;
	for(i = 0; i < ARRAY_SIZE(delivery_system_desc); i++) {
		delivery_system_desc_t *cur_sys = delivery_system_desc + i;
		if(cur_sys->system == delivery_system) {
			return cur_sys->name;
		}
	}
	return "unknown";
}

static fe_modulation_t parse_modulation(char *mod_str)
{
	uint32_t	i;
	for(i = 0; i < ARRAY_SIZE(fe_mod_desc); i++) {
		fe_mod_desc_t *cur_mod = fe_mod_desc + i;
		if(strcmp(mod_str, cur_mod->name) == 0) {
			return cur_mod->modulation;
		}
	}
	return QAM_AUTO;
}

static char *get_modulation_name(fe_modulation_t mod)
{
	uint32_t	i;
	for(i = 0; i < ARRAY_SIZE(fe_mod_desc); i++) {
		fe_mod_desc_t *cur_mod = fe_mod_desc + i;
		if(cur_mod->modulation == mod) {
			return cur_mod->name;
		}
	}
	return "unknown";
}

static int dvb_printFrontendInfo(int frontend_fd)
{
	int			err;
	uint32_t	i;
	struct dvb_frontend_info fe_info;

	do {
		err = ioctl(frontend_fd, FE_GET_INFO, &fe_info);
		if (err < 0) {
			printf("%s: ioctl FE_GET_INFO failed", __FUNCTION__);
		}
	} while(err < 0);
	printf( "Tuner info:\n"
			"\tDVB Model=%s\n"
			"\tType=%s\n"
			"\tfrequency_min  =%9u Hz, frequency_max  =%9u Hz, frequency_stepsize=%9u Hz\n"
			"\tsymbol_rate_min=%9u Hz, symbol_rate_max=%9u Hz\n",
			fe_info.name, fe_typeNames[fe_info.type],
			fe_info.frequency_min, fe_info.frequency_max, fe_info.frequency_stepsize,
			fe_info.symbol_rate_min, fe_info.symbol_rate_max);
	printf("\tCapabilities:\n");
	for(i = 0; i < ARRAY_SIZE(fe_caps_desc); i++) {
		fe_caps_desc_t *cur_desc = fe_caps_desc + i;
		if(cur_desc->capability & fe_info.caps) {
			printf("\t\t%s\n", cur_desc->name);
		}
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
					(status & FE_HAS_SIGNAL)?"FE_HAS_SIGNAL ":"",
					(status & FE_HAS_CARRIER)?"FE_HAS_CARRIER ":"",
					(status & FE_HAS_VITERBI)?"FE_HAS_VITERBI ":"",
					(status & FE_HAS_SYNC)?"FE_HAS_SYNC ":"",
					(status & FE_HAS_LOCK)?"FE_HAS_LOCK ":"",
					(status & FE_TIMEDOUT)?"FE_TIMEDOUT ":"",
					(status & FE_REINIT)?"FE_REINIT ":"");

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
		printf("%s: set property failed: %s\n", __FUNCTION__, strerror(errno));
		return -1;
	}

	return 0;
}

int32_t dvb_openFronend(uint32_t adap, uint32_t fe, int32_t *fd)
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
		if((*fd = open(buf, O_RDWR)) >= 0) {
			printf("success\n");
			return 0;
		}
		printf("fail\n");
	}

	return -1;
}

static void usage(char *progname)
{
	uint32_t i;

	printf("Usage: %s [OPTIONS]\n", progname);
	printf("\t-h, --help                    - Print this message\n");
	printf("\t-v, --verbose                 - Be verbose\n");
	printf("\t-d, --device=DEVID            - Choose dvb device /dev/dvb<DEVID>.frontend0\n");
	printf("\t-i, --info                    - Print tuner info\n");
	printf("\t-t, --del-sys=<");
	for(i = 0; i < ARRAY_SIZE(delivery_system_desc); i++) {
		printf("%s%s", i ? "|" : "", delivery_system_desc[i].name);
	}
	printf("> - Select delivery type\n");

	printf("\t-f, --frequency=FREQUENCY     - Set frequency in Hz\n");
	printf("\t-s, --symbol-rate=SYMBOLRATE  - Set symbol rate in symbol per second\n");
	printf("\t-p, --plp-id=PLPID            - Set plp id (for DVB-T2)\n");

	printf("\t-m, --modulation=<");
	for(i = 0; i < ARRAY_SIZE(fe_mod_desc); i++) {
		printf("%s%s", i ? "|" : "", fe_mod_desc[i].name);
	}
	printf("> - Set modulation\n");
	printf("\t-c, --close-fe                - Close frontend at the end (infinity wait is default)\n");
	printf("\t-w, --wait-count=WAIT_COUNT   - Wait at most WAIT_COUNT times for frontend locking\n");
	
	return;
}


int main(int argc, char **argv)
{
	int32_t							fd_frontend;
	int32_t							opt;
	uint32_t						device = 0;
	static int32_t					show_tuner_info = 0;
	static int32_t					verbose = 0;
	fe_delivery_system_t			delivery_system = SYS_DVBC_ANNEX_A;
	uint32_t						frequency = 0;
	uint32_t						symbol_rate = 6900000;//6,9MHz
	fe_modulation_t					modulation = QAM_AUTO;
	uint32_t						plp_id = 0;
	int32_t							option_index = 0;
	int32_t							inversion = INVERSION_AUTO;
	int32_t							dont_close_fe = 1;
	int32_t							wait_count = -1;
	int32_t							has_lock = 0;
	static struct option			long_options[] = {
		{"help",		no_argument,		0, 'h'},
		{"info",		no_argument,		0, 'i'},
		{"verbose",		no_argument,		0, 'v'},
		{"device",		required_argument,	0, 'd'},
		{"del-sys",		required_argument,	0, 't'},
		{"frequency",	required_argument,	0, 'f'},
		{"symbol-rate",	required_argument,	0, 's'},
		{"modulation",	required_argument,	0, 'm'},
		{"plp-id",		required_argument,	0, 'p'},
		{"inversion",	required_argument,	0, 'n'},
		{"close-fe",	no_argument,		0, 'c'},
		{"wait-count",	required_argument,	0, 'w'},
		{0, 0, 0, 0},
	};

	while((opt = getopt_long(argc, argv, "hivd:t:f:s:m:p:n:cw:", long_options, &option_index)) != -1) {
		switch(opt) {
			case 'h':
				usage(argv[0]);
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
			default:
				usage(argv[0]);
				return -3;
				break;
		}
	}

	if(frequency == 0) {
		printf("ERROR: Frequency not setted!\n");
		usage(argv[0]);
		return -5;
	}

	if(dvb_openFronend(device, 0, &fd_frontend) != 0) {
		printf("%s[%d]: Error open device=%d frontend\n", __FILE__, __LINE__, device);
		return -1;
	}

	dvb_setFrontendType(fd_frontend, delivery_system);
	if(show_tuner_info)
		dvb_printFrontendInfo(fd_frontend);
	printf( "Selected delivery sistem: %s\n", get_delivery_system_name(delivery_system));

	struct dtv_property dtv[16];//check if it enough
	struct dtv_properties cmdseq;

	if(delivery_system == SYS_DVBC_ANNEX_A) {//DVB-C
		printf( "Tune frontend on:\n"
				"\tfreq        = %9d Hz\n"
				"\tsymbol_rate = %9d Hz\n"
				"\tmodulation  = %s\n",
				frequency, symbol_rate, get_modulation_name(modulation));

		dtv[0].cmd = DTV_FREQUENCY; 		dtv[0].u.data = frequency;//(12666000-10600000);
		dtv[1].cmd = DTV_MODULATION; 		dtv[1].u.data = modulation;
		dtv[2].cmd = DTV_SYMBOL_RATE; 		dtv[2].u.data = symbol_rate;
		dtv[3].cmd = DTV_INVERSION; 		dtv[3].u.data = inversion;
		dtv[4].cmd = DTV_INNER_FEC; 		dtv[4].u.data = FEC_AUTO;
		dtv[5].cmd = DTV_TUNE;

		cmdseq.num = 6;

	} else if((delivery_system == SYS_DVBT) || (delivery_system == SYS_DVBT2)) { //DVB-T/T2
		int32_t propCount;
		printf( "Tune frontend on:\n"
				"\tfreq        = %9d Hz\n"
				"\tmodulation  = %s\n",
				frequency, get_modulation_name(modulation));

		dtv[0].cmd = DTV_FREQUENCY; 		dtv[0].u.data = frequency;//(12666000-10600000);
		dtv[1].cmd = DTV_INVERSION; 		dtv[1].u.data = inversion;
		dtv[2].cmd = DTV_BANDWIDTH_HZ; 		dtv[2].u.data = BANDWIDTH_8_MHZ;//BANDWIDTH_AUTO
		dtv[3].cmd = DTV_CODE_RATE_HP; 		dtv[3].u.data = FEC_AUTO;//FEC_7_8;
		dtv[4].cmd = DTV_CODE_RATE_LP; 		dtv[4].u.data = FEC_AUTO;//FEC_7_8;
		dtv[5].cmd = DTV_MODULATION; 		dtv[5].u.data = modulation;
		dtv[6].cmd = DTV_TRANSMISSION_MODE;	dtv[6].u.data = TRANSMISSION_MODE_AUTO;//TRANSMISSION_MODE_8K;
		dtv[7].cmd = DTV_GUARD_INTERVAL; 	dtv[7].u.data = GUARD_INTERVAL_AUTO;//GUARD_INTERVAL_1_16
		dtv[8].cmd = DTV_HIERARCHY; 		dtv[8].u.data = HIERARCHY_AUTO;
		propCount = 9;
		if(delivery_system == SYS_DVBT2) {
			dtv[propCount].cmd = DTV_STREAM_ID; 	dtv[9].u.data = plp_id;
			propCount++;
		}
		dtv[propCount].cmd = DTV_TUNE;
		propCount++;

		cmdseq.num = propCount;

	} else if((delivery_system == SYS_ATSC) || (delivery_system == SYS_DVBC_ANNEX_B)) { //ATSC
		printf( "Tune frontend on:\n"
				"\tfreq        = %9d Hz\n"
				"\tmodulation  = %s\n",
				frequency, get_modulation_name(modulation));

		dtv[0].cmd = DTV_FREQUENCY; 		dtv[0].u.data = frequency;//(12666000-10600000);
		dtv[1].cmd = DTV_MODULATION; 		dtv[1].u.data = modulation;//VSB_8
		dtv[2].cmd = DTV_INVERSION; 		dtv[1].u.data = inversion;//INVERSION_ON;
		dtv[3].cmd = DTV_TUNE;

		cmdseq.num = 4;

	} else {
		printf("Not supported delivery system: %s\n", get_delivery_system_name(delivery_system));
		return -2;
	}

	cmdseq.props = dtv;

	if(ioctl(fd_frontend, FE_SET_PROPERTY, &cmdseq) == -1) {
		perror("FRONTEND FE_SET_FRONTEND: ");
		return -3;
	}

	usleep(10000);//10ms

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
