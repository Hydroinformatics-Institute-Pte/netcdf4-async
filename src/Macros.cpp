#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"
#include "async.h"
#include "utils.h"
#include <node_version.h>
#include "Macros.h"

namespace netcdf4async {


Napi::Value item2value(Napi::Env env, Item *nc_item) {
	Napi::Value value;
//	printf("Attr %s type %i\n",nc_attribute->name.c_str(),nc_attribute->type);
	switch (nc_item->type) {
		case NC_BYTE:
			ITEM_TO_VAL(int8_t);
		break;
		case NC_SHORT:
			ITEM_TO_VAL(int16_t);
		break;
		case NC_INT:
			ITEM_TO_VAL(int32_t);
		break;
		case NC_FLOAT:
			ITEM_TO_VAL(float);
		break;
		case NC_DOUBLE:
			ITEM_TO_VAL(double);
		break;
		case NC_UBYTE:
			ITEM_TO_VAL(uint8_t);
		break;
		case NC_USHORT:
			ITEM_TO_VAL(uint16_t);
		break;
		case NC_UINT:
			ITEM_TO_VAL(uint32_t);
		break;
		case NC_UINT64:
			ITEM_TO_VAL(uint64_t);
		break;
		case NC_INT64:
			ITEM_TO_VAL(int64_t);
		break;
		case NC_CHAR: 
			value = Napi::String::New(env, nc_item->value.s);
			delete[] nc_item->value.s;
		break;
		case NC_STRING:
			if (nc_item->len == 1) {
				std::string *s = new std::string(nc_item->value.ps[0]);
				value = Napi::String::New(env, s->c_str());
				delete s;
			}
			else {
				Napi::Array result_array = Napi::Array::New(env, nc_item->len);
				for (int i = 0; i < static_cast<int>(nc_item->len); i++){
					std::string *res_str=new std::string(nc_item->value.ps[i]);
					result_array[i] = Napi::String::New(env, res_str->c_str());
					delete res_str;
				}
				value = result_array;
			}
		break;
	}	
	return value;
}

void typedValue(Item* pItem) {
    switch (pItem->type) {
	case NC_BYTE: 
	    TYPED_VALUE(pItem->value,int8_t,pItem->len)
	    break;
	case NC_CHAR: 
	    TYPED_VALUE(pItem->value,char,pItem->len+1)
	    pItem->value.s[pItem->len] = 0; 
	    break;
	case NC_SHORT: 
	    TYPED_VALUE(pItem->value,int16_t,pItem->len)
	    break;
	case NC_INT:
	    TYPED_VALUE(pItem->value,int32_t,pItem->len)
	    break;
	case NC_FLOAT: 
	    TYPED_VALUE(pItem->value,float,pItem->len)
		break;
	case NC_DOUBLE:
	    TYPED_VALUE(pItem->value,double,pItem->len)
		break;
	case NC_UBYTE: 
	    TYPED_VALUE(pItem->value,uint8_t,pItem->len)
	    break;
	case NC_USHORT: 
	    TYPED_VALUE(pItem->value,uint16_t,pItem->len)
	    break;
	case NC_UINT:
	    TYPED_VALUE(pItem->value,uint32_t,pItem->len)
	    break;
#if NODE_MAJOR_VERSION > 9
	case NC_UINT64:
	    TYPED_VALUE(pItem->value,uint64_t,pItem->len)
	    break;
	case NC_INT64:
	    TYPED_VALUE(pItem->value,int64_t,pItem->len)
	    break;
#endif
	case NC_STRING:
	    pItem->value.ps = new char *[pItem->len];
	    break;
	default:
    	throw std::runtime_error("Variable type not supported yet");
	}
}

}