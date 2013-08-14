

/******************************************************************
* INCLUDE FILES                                                   *
*******************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> //atoi
#include <fcntl.h> //open
#include <sys/ioctl.h>
#include <unistd.h> //sleep, close
#include <string.h>
#include <errno.h>
//linux
#include <linux/dvb/frontend.h>
#include <linux/version.h>

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
static void usage(char *progname)
{
	uint32_t	i;

	printf("Usage: %s [OPTIONS]\n", progname);
	printf("\t-d <device>        - choose dvb device /dev/dvb<device>.frontend0\n");
	printf("\t-i                 - print tuner info\n");
	printf("\t-t <");
	for(i = 0; i < ARRAY_SIZE(delivery_system_desc); i++) {
		printf("%s%s", i ? "|" : "", delivery_system_desc[i].name);
	}
	printf("> - delivery type\n");

	printf("\t-f <frequency>     - set frequency in Hz\n");
	printf("\t-s <symbol rate>   - set symbol rate in Hz\n");
	printf("\t-p <plp id>        - set plp id (for DVB-T2)\n");

	printf("\t-m <");
	for(i = 0; i < ARRAY_SIZE(fe_mod_desc); i++) {
		printf("%s%s", i ? "|" : "", fe_mod_desc[i].name);
	}
	printf("> - chose modulation\n");
	return;
}

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
#ifdef DTV_DELIVERY_SYSTEM
	struct dtv_property p = { .cmd = DTV_DELIVERY_SYSTEM, };
	struct dtv_properties cmdseq = {
		.num = 1,
		.props = &p
	};

/*	switch (type) {
		case DVBT:
			p.u.data = SYS_DVBT; break;
		case DVBC:
			p.u.data = SYS_DVBC_ANNEX_A; break;
		case DVBS:
			p.u.data = SYS_DVBS; break;
		case FE_ATSC:
			p.u.data = SYS_ATSC; break;
		default:
			printf("%s: unknown frontend type %d\n", __FUNCTION__, type);
			return -1;
	}*/
	p.u.data = type;

	if (ioctl(fd_frontend, FE_SET_PROPERTY, &cmdseq) == -1) {
		printf("%s: set property failed: %s\n", __FUNCTION__, strerror(errno));
		return -1;
	}

	return 0;
#else
	return -1;
#endif
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


int main(int argc, char **argv)
{
	int32_t							fd_frontend;
	int32_t							opt;
	uint32_t						device = 0;
	uint32_t						show_tuner_info = 0;
	fe_delivery_system_t			delivery_system = SYS_DVBC_ANNEX_A;
	uint32_t						frequency = 690000000;//690MHz
	uint32_t						symbol_rate = 6900000;//6,9MHz
	fe_modulation_t					modulation = QAM_AUTO;
	uint32_t						plp_id = 0;
	struct dvb_frontend_parameters	fe_params;

/*	if(argc > 1) {
		device = atoi(argv[1]);
	}*/
	while((opt = getopt(argc, argv, "d:it:f:s:m:p:")) != -1) {
		switch(opt) {
			case 'd':
				device = atoi(optarg);
				break;
			case 'i':
				show_tuner_info = 1;
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
			default:
				usage(argv[0]);
				return -3;
				break;
		}
	}

	if(dvb_openFronend(device, 0, &fd_frontend) != 0) {
		printf("%s[%d]: Error open device=%d frontend\n", __FILE__, __LINE__, device);
		return -1;
	}

	dvb_setFrontendType(fd_frontend, delivery_system);
	if(show_tuner_info)
		dvb_printFrontendInfo(fd_frontend);
	printf( "Selected delivery sistem: %s\n", get_delivery_system_name(delivery_system));

	if(delivery_system == SYS_DVBC_ANNEX_A) {//DVB-C
		printf( "Tune frontend on:\n"
				"\tfreq        = %9d Hz\n"
				"\tsymbol_rate = %9d Hz\n"
				"\tmodulation  = %s\n",
				frequency, symbol_rate, get_modulation_name(modulation));

		fe_params.frequency = frequency;//(12666000-10600000);
		fe_params.inversion = INVERSION_AUTO;
		fe_params.u.qam.fec_inner = FEC_AUTO;
		fe_params.u.qam.symbol_rate = symbol_rate;
		fe_params.u.qam.modulation = modulation;
	//	fe_params.u.qam.modulation = QAM_AUTO;
	} else if((delivery_system == SYS_DVBT) || (delivery_system == SYS_DVBT2)) { //DVB-T/T2
		printf( "Tune frontend on:\n"
				"\tfreq        = %9d Hz\n"
				"\tmodulation  = %s\n",
				frequency, get_modulation_name(modulation));

		fe_params.frequency = frequency;//(12666000-10600000);
		fe_params.inversion = INVERSION_AUTO;
		fe_params.u.ofdm.bandwidth = BANDWIDTH_8_MHZ;//BANDWIDTH_AUTO
		fe_params.u.ofdm.code_rate_HP = FEC_AUTO;//FEC_7_8;
		fe_params.u.ofdm.code_rate_LP = FEC_AUTO;//FEC_7_8;
		fe_params.u.ofdm.constellation = modulation;
		fe_params.u.ofdm.transmission_mode = TRANSMISSION_MODE_AUTO;//TRANSMISSION_MODE_8K
		fe_params.u.ofdm.guard_interval = GUARD_INTERVAL_AUTO;//GUARD_INTERVAL_1_16
		fe_params.u.ofdm.hierarchy_information = HIERARCHY_AUTO;
#ifdef DTV_STREAM_ID
		if(delivery_system == SYS_DVBT2) {
			struct dtv_property dtv = { .cmd = DTV_STREAM_ID, .u.data = plp_id, };
			struct dtv_properties cmdseq = { .num = 1, .props = &dtv, };
			if (ioctl(fd_frontend, FE_SET_PROPERTY, &cmdseq) == -1) {
				printf("FAILED to set plp %u\n", plp_id);
				return -4;
			}
			printf("\tT2 plp = %u\n", plp_id);
		}
#endif
	} else if((delivery_system == SYS_ATSC) || (delivery_system == SYS_DVBC_ANNEX_B)) { //ATSC
		printf( "Tune frontend on:\n"
				"\tfreq        = %9d Hz\n"
				"\tmodulation  = %s\n",
				frequency, get_modulation_name(modulation));

		fe_params.frequency = frequency;//(12666000-10600000);
		fe_params.inversion = INVERSION_AUTO;//INVERSION_ON;
		fe_params.u.vsb.modulation = modulation;//VSB_8
	} else {
		printf("Not supported delivery system: %s\n", get_delivery_system_name(delivery_system));
		return -2;
	}

	if (ioctl(fd_frontend, FE_SET_FRONTEND, &fe_params) < 0) {
		perror("FRONTEND FE_SET_FRONTEND: ");
		return -3;
	}
	usleep(10000);//10ms

	while(1) {
		uint32_t status;
		ioctl(fd_frontend, FE_READ_STATUS, &status);
		dvb_printLockInfo(fd_frontend, &status);

		if(status & FE_HAS_LOCK) {
			printf("%s[%d]: Locked success!!!\n", __FILE__, __LINE__);
			break;
		} else {
			sleep(1);
//			usleep(100000);
		}
	}
	while(1) {
		sleep(1);
	}

	close(fd_frontend);
	return 0;
}
