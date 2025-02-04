#include "tl_common.h"
#include "app_cfg.h"

#include "app_i2c.h"
#include "shtv3_sensor.h"

u8 sens_wakeup[] = {0x35,0x17};
u8 sens_sleep[] = {0xB0,0x98};
u8 sens_reset[] = {0x80,0x5D};

u8 measure_cmd[] = {0xfd};

//Since we now got version B1.4 B1.6 and B1.9 of the Thermometer we need to detect the correct sensor it is using
// B1.4 = SHTC3 = 0 = address 0x70/0xE0
// B1.6 and B1.9 = SHV4 = 1 = address 0x44/0x88
_attribute_data_retention_ u8 sensor_version = 2;  // unknown
_attribute_data_retention_ u8 i2c_address_sensor = 0xE0;

void init_sensor(){

	if(test_i2c_device(0x70)){
		sensor_version = 0;
		i2c_address_sensor = 0xE0;
	}else if(test_i2c_device(0x44)){
		sensor_version = 1;
		i2c_address_sensor = 0x88;
	}


	if(sensor_version == 0){
		send_i2c(i2c_address_sensor,sens_wakeup, sizeof(sens_wakeup));
		sleep_us(240);
		send_i2c(i2c_address_sensor,sens_reset, sizeof(sens_reset));
		sleep_us(240);
		send_i2c(i2c_address_sensor,sens_sleep, sizeof(sens_sleep));
	}else if(sensor_version == 1){
		send_i2c(i2c_address_sensor,(u8 *)0x94, 1);
		sleep_us(1000);
	}else if(sensor_version == 2){

	}else{
		//UNKNOWN SENSOR, how did we got here ???
	}
}

void read_sensor(s16 *temp, u16 *humi) {
    init_sensor();

    if(sensor_version == 0){

        send_i2c(i2c_address_sensor,sens_wakeup, sizeof(sens_wakeup));
        sleep_us(240);
        u8 read_buff[5];
        reg_i2c_mode |= FLD_I2C_HOLD_MASTER;// Enable clock stretching for Sensor
        i2c_set_id(i2c_address_sensor);
        i2c_read_series(0x7CA2, 2, (u8*)read_buff,  5);
        reg_i2c_mode &= ~FLD_I2C_HOLD_MASTER;// Disable clock stretching for Sensor
        send_i2c(i2c_address_sensor,sens_sleep, sizeof(sens_sleep));

        *temp = (s16)(((17500 * ((u32)read_buff[0] << 8 | (u32)read_buff[1])) >> 16) - 4500);
        *humi = (u16)(10000 * ((u32)read_buff[3] << 8 | (u32)read_buff[4])) >> 16;

    }else if(sensor_version == 1){
        send_i2c(i2c_address_sensor,measure_cmd, sizeof(measure_cmd));
        sleep_us(1000*10);
        u8 read_buff[5];
        i2c_set_id(i2c_address_sensor);
        i2c_read_series(0, 0, (u8*)read_buff,  5);

        *temp = (s16)(((17500 * ((u32)read_buff[0] << 8 | (u32)read_buff[1])) >> 16) - 4500);
        *humi = (u16)(((12500 * ((u32)read_buff[3] << 8 | (u32)read_buff[4])) >> 16) - 600);

    }else if(sensor_version == 2){

    }else{
        //UNKNOWN SENSOR, how did we got here ???
    }
}
