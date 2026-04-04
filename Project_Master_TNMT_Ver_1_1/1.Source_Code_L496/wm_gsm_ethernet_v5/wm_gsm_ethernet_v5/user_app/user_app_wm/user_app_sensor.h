

#ifndef USER_APP_SENSOR_H_
#define USER_APP_SENSOR_H_

#define USING_APP_SENSOR

#define USING_APP_SENSOR_DEBUG

#define TIME_RESEND_WARNING         5

#define SCALE_SENSOR_DEFAULT        0xFE


#include "event_driven.h"
#include "user_util.h"
#include "user_app_rs485.h"

#define RESET_VALUES(sensor, ...) do {                              \
    float *vars[] = { __VA_ARGS__ };                                \
    for (int i = 0; i < (int)(sizeof(vars)/sizeof(vars[0])); i++) { \
        *vars[i] = 0;                                               \
    }                                                               \
} while(0)

#define SENSOR_CHECK(sensor, ...) do {                              \
    if ((sensor).nConnect_u8 == 0) {                                \
        (sensor).State_Connect_u8 = _SENSOR_DISCONNECT;             \
        RESET_VALUES(sensor, __VA_ARGS__);                          \
    } else if ((sensor).nConnect_u8 >= 3) {                         \
        (sensor).State_Connect_u8 = _SENSOR_CONNECT;                \
    }                                                               \
} while(0)

typedef enum
{
    _EVENT_SENSOR_ENTRY,
    _EVENT_SENSOR_LOG_TSVH,
    
    _EVENT_SENSOR_DATA_MEASURE,
    _EVENT_SENSOR_HANDLE_STATE,
    
    _EVENT_SENSOR_END,
}eKindEventSensor;

typedef enum
{
    _INACTIVE_SENSOR,       //Non active sensor
    _ACTIVE_SENSOR,         //Active sensor
}eKindDCU_UserSensor;

typedef enum
{
    _SS_TEMP = 0,          // hoac SS_TEMP
    _SS_PH,                 // SS pH
    _SS_CLO,                // SS Clo
    _SS_EC,                 // SS EC
    _SS_TURB,               // SS TURB
    _SS_COD,                // SS COD
    _SS_TSS,                // SS TSS
    _SS_NH4,                // SS NH4
    _SS_DO,                 // SS DO
    _SS_SALT,               // SS SALT
    _SS_TDS,                // SS TDS
    _SS_NO3,                // SS NO3
    
    _END_SENSOR,
}eKind_Sensor;

typedef enum
{
    _E_DANG_DO = 0,     
    _E_HIEU_CHUAN,
    _E_BAO_LOI_THIET_BI,
}eKind_State_Parameter_TNMT;

typedef uint8_t (*_Cb_Handler_Sensor_Connect) (uint8_t);

typedef struct
{
    uint8_t e_Name;
    uint8_t *State_Active;
    uint8_t *State_Connect_Now;
    uint8_t State_Connect_Befor;
    uint32_t Gettick_Handle;
    _Cb_Handler_Sensor_Connect 	_Cb_Handler_SS_Connect;
}Struct_SensorWarning;

typedef struct
{
    uint8_t e_Name;
    Struct_RS485_Measure   *sMeasureSensor;
    Struct_SS_Value         *sMeasureHanlde;
    uint8_t Scale_1;        //Scale 1
    float   ParaScale;      // Measure>=ParaScale-> Scale_1, Measure<ParaScale-> Scale_2
    uint8_t Scale_2;        //Scale 2
}Struct_MeasureHanle;

typedef struct
{
    uint8_t Year_Before;        //Check Year log du lieu de xoa neu du lieu qua cu
    uint8_t Month_Before;       //Check Month log du lieu de xoa neu du lieu qua cu
    uint8_t Date_Before;        //Check Date log du lieu de xoa neu du lieu qua cu
    uint8_t Hour_Before;        //Check Hour log du lieu de xoa neu du lieu qua cu 
    uint8_t Num_Success;        //Dem so lan du lieu chinh xac de log
    uint8_t Num_Error;          //Dem so lan du lieu error
    float Sum_Value_f;          //Tong gia tri sau Num_Success log (Average = Sum_Value_f/Num_Success) 
}Struct_AverageOneHour;

typedef struct
{
    uint8_t State;              //Trang thai du lieu (dang do/hieu chuan/error)
    float Value_f;              //Gia tri trung binh
}Struct_AverageMeasure;

extern sEvent_struct                sEventAppSensor[];
extern Struct_RS485_Measure            s485Measure[];

extern Struct_AverageOneHour       sAverageOneHour[];
extern Struct_AverageMeasure       sAverageMeasure[];
extern uint8_t                     StateCalibSensor;
/*=============== Function handle ================*/

uint8_t     AppSensor_Task(void);
void        Init_AppSensor(void);

void Log_EventWarnig(uint8_t Obis, uint8_t LengthData, uint8_t *aDataWaring);
void Save_TimeWarningSensor(uint8_t Duration);
void Init_TimeWarningSensor(void);

void Save_OffsetMeasure(uint8_t KindOffset, float Var_Offset_f);
void Init_OffsetMeasure(void);

void Save_UserSensor(uint8_t KindSensor, uint8_t State);
void Init_UserSensor(void);

void Save_StateCalibSensor(uint8_t State);
void Init_StateCalibSensor(void);

uint8_t AppSensor_Packet_Param (char *pdata, uint8_t chann);
void AppSensor_Packet_TNMT(void);

void Save_AverageOneHour(void);
void Init_AverageOneHour(void);
void Average_One_Hour(void);
void Calculator_Average_One_Hour(void);

uint32_t ConvertToHours(uint8_t year, uint8_t month, uint8_t day, uint8_t hour);
#endif
