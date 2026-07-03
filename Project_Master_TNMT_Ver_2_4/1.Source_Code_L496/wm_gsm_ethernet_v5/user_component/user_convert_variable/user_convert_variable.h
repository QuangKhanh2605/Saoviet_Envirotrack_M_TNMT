
#ifndef USER_CONVERT_VARIABLE_H__
#define USER_CONVERT_VARIABLE_H__    

#include "user_util.h"

typedef enum
{
    _E_BE,              //Big Endian        ABCD
    _E_LE,              //Little Endian     DCBA
    _E_BS,              //Byte Swap         BADC
    _E_WS,              //Word Swap         CDAB
}eKindEndianFormat;

typedef enum
{
    _ETYPE_F,           //Kieu du lieu float
    _ETYPE_U32,         //Kieu du lieu uint32_t
    _ETYPE_I32,         //Kieu du lieu int32_t
    _ETYPE_U16,         //Kieu du lieu uint16_t
    _ETYPE_I16,         //Kieu du lieu int16_t
    _ETYPE_U8,          //Kieu du lieu uint8_t
    _ETYPE_I8,          //Kieu du lieu int8_t
}eKindTypeValue;

/*======================Function====================*/

uint32_t    Calculator_Scale(uint8_t Scale);
uint32_t    Calculator_Value_Scale(uint32_t Value, uint8_t Scale, uint8_t Scale_Default);

uint8_t     Convert_Int_To_String_Scale(uint8_t cData[], int var, uint8_t Scale);
uint8_t     Convert_Int_To_String(uint8_t cData[], int var);
void        Convert_Var_Packet_Integer (uint8_t *pTarget, uint16_t *LenTarget, uint32_t Data);


uint8_t     Convert_Point_Int_To_String(uint8_t cData[], uint16_t *Pos, int var);
uint8_t     Convert_Point_Int_To_String_Scale(uint8_t cData[], uint16_t *Pos, int var, uint8_t Scale);
void        Insert_String_To_String(uint8_t destination[], uint16_t *PosDes, uint8_t source[], uint16_t PosSour, uint32_t length);

uint32_t    Convert_String_To_Dec(uint8_t *pData, uint8_t lenData);

void        Convert_uint32Hex_To_Float(uint32_t uint32Hex, float *Var);

int32_t    Hanlde_Float_To_Int32_Scale_Round(float varFloat, uint8_t scale);
int32_t    Handle_HexFloat_To_Int32_Round(uint32_t hexFloat, uint8_t scale);
float      Handle_int32_To_Float_Scale(int32_t var, uint8_t scale);
uint32_t   Handle_Float_To_hexUint32(float num);

void        Convert_Uint64_To_StringDec (sData *Payload, uint64_t Data, uint8_t Scale);
void        Scale_String_Dec (sData *StrDec, uint8_t Scale);

uint32_t   Endian_Format(uint32_t Hex_Data, uint8_t length, uint8_t Type);
float      Decode_Data_Type_u32_to_f(uint32_t Hex_Data, uint8_t Type);
uint32_t   Decode_Data_Type_f_to_u32(float Data_f, uint8_t Type);
#endif

