#ifndef _STK8BAXX_H
#define _STK8BAXX_H

#define STK_ACC_DRIVER_VERSION	"3.7.1_rk_0425_0428"

/*------------------Miscellaneous settings-------------------------*/
#define STK8BAXX_I2C_NAME	"stk8baxx"

#define STK8BAXX_PRECISION			12
#define STK8BAXX_RANGE				16384 * 4
#define STK8BAXX_BOUNDARY			(0x1 << (STK8BAXX_PRECISION - 1))
#define STK8BAXX_GRAVITY_STEP		(STK8BAXX_RANGE / STK8BAXX_BOUNDARY)

/*------------------stk8ba58 registers-------------------------*/
#define STK8BAXX_CHIPID				0x00

#define STK8BAXX_XOUT1				0x02
#define STK8BAXX_XOUT2				0x03
#define STK8BAXX_YOUT1				0x04
#define STK8BAXX_YOUT2				0x05
#define STK8BAXX_ZOUT1				0x06
#define STK8BAXX_ZOUT2				0x07

#define STK8BAXX_INTSTS1			0x09
#define STK8BAXX_INTSTS2			0x0A

#define STK8BAXX_EVENTINFO1			0x0B
#define STK8BAXX_RANGESEL			0x0F
#define STK8BAXX_BWSEL				0x10
#define STK8BAXX_POWMODE			0x11
#define STK8BAXX_DATASETUP			0x13
#define STK8BAXX_SWRST				0x14
#define STK8BAXX_INTEN1				0x16
#define STK8BAXX_INTEN2				0x17
#define STK8BAXX_INTMAP1			0x19
#define STK8BAXX_INTMAP2			0x1A
#define STK8BAXX_INTCFG1			0x20
#define STK8BAXX_INTCFG2			0x21
#define STK8BAXX_SLOPEDLY			0x27
#define STK8BAXX_SLOPETHD			0x28
#define STK8BAXX_SIGMOT1			0x29
#define STK8BAXX_SIGMOT2			0x2A
#define STK8BAXX_SIGMOT3			0x2B
#define STK8BAXX_INTFCFG			0x34
#define STK8BAXX_OFSTCOMP1			0x36
#define STK8BAXX_OFSTX				0x38
#define STK8BAXX_OFSTY				0x39
#define STK8BAXX_OFSTZ				0x3A

/*------------------STK8BAXX_CHIPID-------------------------*/
#define STK8BA50_ID					0x09
#define STK8323_ID					0x23 /* including STK8321 */
#define STK8329_ID					0x25
#define STK8327_ID					0x26
#define STK8BA50R_ID				0x86
#define STK8BA53_ID					0x87 /* including STK8BA58 */

/*------------------STK8BAXX_RANGESEL-------------------------*/
#define STK8BAXX_RNG_2G				0x3
#define STK8BAXX_RNG_4G				0x5
#define STK8BAXX_RNG_8G				0x8

/*------------------STK8BAXX_BWSEL-------------------------*/
#define STK8BAXX_BWSEL_7_81			0x08
#define STK8BAXX_BWSEL_15_63		0x09
#define STK8BAXX_BWSEL_31_25		0x0A    /* ODR = BW x 2 = 62.5Hz */
#define STK8BAXX_BWSEL_62_5			0x0B
#define STK8BAXX_BWSEL_125			0x0C
#define STK8BAXX_BWSEL_250			0x0D
#define STK8BAXX_BWSEL_500			0x0E
#define STK8BAXX_BWSEL_1000			0x0F
#define STK8BAXX_BWSEL_INIT_ODR		0x0A    /* ODR = BW x 2 = 62.5Hz */

/*------------------STK8BAXX_POWMODE-------------------------*/
#define STK8BAXX_MD_SUSPEND			0x80
#define STK8BAXX_MD_NORMAL			0x00

/*------------------STK8BAXX_SWRST-------------------------*/
#define STK8BAXX_SWRST_VAL			0xB6

#endif
