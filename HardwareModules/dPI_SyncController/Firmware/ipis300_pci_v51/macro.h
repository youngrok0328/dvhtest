#ifndef __MACRO_HEADER_FILE__
#define __MACRO_HEADER_FILE__

//          DEF16	mda_value;
#define		mda_temp_value		mda_value.def_word
#define		pzt_value_h	    	mda_value.ml[1]
#define		pzt_value_l		    mda_value.ml[0]
#define		sdi_value		    mda_value.bit.bit11
		
//          DEF16	int_data;
#define		int_value	    	int_data.def_word
#define		int_value_h		    int_data.ml[1]
#define		int_value_l		    int_data.ml[0]

//          DEF8	mda_temp;
#define		mda_temp_data	    mda_temp.def_byte
#define		mda_temp0	        mda_temp.bit.bit0
#define		mda_temp1	        mda_temp.bit.bit1
#define		mda_temp2           mda_temp.bit.bit2

//          DEF8    led_stor;
#define		led_ch_temp         led_stor.def_byte
#define		led_storage         led_stor.bit.bit4	//led control board에 값을 저장 하기 위한 포트

#endif
