#include <iostream>
#include <netcdf.h>
#include <string>
#include "netcdf4-async.h"
#include "async.h"
#include "Attribute.h"
#include "Macros.h"

namespace netcdf4async {

/*
const unsigned char Variable::type_sizes[] = {
	0, // NC_NAT // unknown type
	1, // NC_BYTE
	1, // NC_CHAR
	2, // NC_SHORT
	4, // NC_INT / NC_LONG
	4, // NC_FLOAT
	8, // NC_DOUBLE
	1, // NC_UBYTE
	2, // NC_USHORT
	4  // NC_UINT
};
*/

union UnionType{
	int8_t* i8;
	int16_t* i16;
	int32_t* i32;
	float* f;
	double* d;
	uint8_t* u8;
	uint16_t* u16;
	uint32_t* u32;
	uint64_t* u64;
	int64_t* i64;
	char* s;
	char** ps;
	const char* text;
	void* v;
};

Napi::FunctionReference Variable::constructor;

Napi::Object Variable::Build(Napi::Env env, int id, int parent_id) {
	return constructor.New({Napi::Number::New(env, id), Napi::Number::New(env, parent_id)});
}

Variable::Variable(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Variable>(info) {
	// if (info.Length() < 2) {
	// 	Napi::TypeError::New(info.Env(), "Wrong number of arguments").ThrowAsJavaScriptException();
	// 	return;
	// }

	// char varName[NC_MAX_NAME + 1];
	// id = info[0].As<Napi::Number>().Int32Value();
	// parent_id = info[1].As<Napi::Number>().Int32Value();
	// NC_CALL_VOID(nc_inq_var(parent_id, id, varName, &type, &ndims, NULL, NULL));
	// name = std::string(varName);
}

void Variable::Init(Napi::Env env) {
	Napi::HandleScope scope(env);
    Napi::Function func =
		DefineClass(env, "Variable",
			{
                InstanceMethod("read", &Variable::Read),
		        InstanceMethod("readSlice", &Variable::ReadSlice),
		        InstanceMethod("readStridedSlice", &Variable::ReadStridedSlice),
		        InstanceMethod("write", &Variable::Write),
		        InstanceMethod("writeSlice", &Variable::WriteSlice),
		        InstanceMethod("writeStridedSlice", &Variable::WriteStridedSlice),
		        InstanceMethod("addAttribute", &Variable::AddAttribute),
		        InstanceMethod("inspect", &Variable::Inspect),

                InstanceMethod("getName", &Variable::GetName),
                InstanceMethod("setName", &Variable::SetName),
                InstanceMethod("getDimensions", &Variable::GetDimensions),
                InstanceMethod("getFill", &Variable::GetFill),
                InstanceMethod("setFill", &Variable::SetFill),
                InstanceMethod("getChunked", &Variable::GetChunked),
                InstanceMethod("setChunked", &Variable::SetChunked),
                InstanceMethod("getDeflateInfo", &Variable::GetDeflateInfo),
                InstanceMethod("setDeflateInfo", &Variable::SetDeflateInfo),
                InstanceMethod("getEndiannes", &Variable::GetEndiannes),
                InstanceMethod("setEndiannes", &Variable::SetEndiannes),
                InstanceMethod("getChecksumMode", &Variable::GetChecksumMode),
                InstanceMethod("setChecksumMode", &Variable::SetChecksumMode),
                InstanceMethod("getAttributes", &Variable::GetAttributes),
                InstanceMethod("setAttribute", &Variable::SetAttribute),
                InstanceMethod("addAttribute", &Variable::AddAttribute),
                InstanceMethod("renameAttribute", &Variable::RenameAttribute),
                InstanceMethod("deleteAttribute", &Variable::DeleteAttribute),

		        InstanceAccessor<&Variable::GetTypeSync>("type"),
		        InstanceAccessor<&Variable::GetNameSync>("name"),
		        
			}
		);
    constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();

}

Napi::Value Variable::GetTypeSync(const Napi::CallbackInfo &info) {
	const char *res=NC_TYPES(this->type);
	return Napi::String::New(info.Env(),res);

}

Napi::Value Variable::GetName(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();	
}


Napi::Value Variable::WriteSlice(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::Read(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::ReadSlice(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::ReadStridedSlice(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::AddAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetId(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), this->id);
}

Napi::Value Variable::GetDimensions(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetAttributes(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::SetAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::RenameAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::DeleteAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetType(const Napi::CallbackInfo &info) {
	const char *res=NC_TYPES(this->type);
	return Napi::String::New(info.Env(),res);

}

Napi::Value Variable::GetName(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value  Variable::SetName(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetEndiannes(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::SetEndiannes(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetChecksumMode(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value  Variable::SetChecksumMode(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetChunkMode(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value  Variable::SetChunkMode(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetChunkSizes(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value  Variable::SetChunkSizes(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetFillMode(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value  Variable::SetFillMode(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::GetFillValue(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    int id = this->id;
    int parent_id = this->parent_id;
    nc_type type = this->type;
    auto worker=new NCAsyncWorker<UnionType>(
		env, 
		[id, parent_id, type] (const NCAsyncWorker<UnionType>* worker) {
            UnionType result;
            switch (type) {
	        case NC_BYTE: {
		        result.i8 = new int8_t[1];
		    } break;
	        case NC_CHAR: {
                result.s = new char[2];
		        result.s[1] = 0; 
	        } break;
	        case NC_SHORT: { 
		        result.i16 = new int16_t[1];
	        } break;
	        case NC_INT: {
		        result.i32 = new int32_t[1];
		    } break;
	        case NC_FLOAT: {
		        result.f = new float[1];
	        } break;
	        case NC_DOUBLE: {
		        result.d = new double[1];
	        } break;
	        case NC_UBYTE: {
		        result.u8 = new uint8_t[1];
		    } break;
	        case NC_USHORT: {
		        result.u16 = new uint16_t[1];
	        } break;
	        case NC_UINT: {
		        result.u32 = new uint32_t[1];
	        } break;
#if NODE_MAJOR_VERSION > 9
	        case NC_UINT64:{
		        result.u64 = new uint64_t[1];
	        }
	        break;
	        case NC_INT64:{
		        result.i64 = new int64_t[1];
        	}
	        break;
#endif
	        case NC_STRING:{
		        result.ps = new char *[1];
		    }
	        break;
	        default:
            	throw std::runtime_error("Variable type not supported yet");
	        }
	        NC_CALL(nc_inq_var_fill(parent_id, id, NULL, result.v));
		    return result;
		},
		[type] (Napi::Env env, UnionType result) {
            return Napi::String::New(env,"OK");
		}
		
	);
	worker->Queue();
    return worker->Deferred().Promise();
}

Napi::Value  Variable::SetFillValue(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    Napi::Env env = info.Env();
	Napi::Value value = info[0];
    int id = this->id;
    int parent_id = this->parent_id;
    Item nc_item;

    switch (this->type) {
	case NC_BYTE: {
		VAL_TO_ITEM(int8_t);
	}break;
	case NC_CHAR: {
		std::string v = value.As<Napi::String>().ToString();
		nc_item.value.s = new char[v.length()+1];
		nc_item.value.s[v.length()] = 0;
		strcpy(nc_item.value.s, v.c_str());
		nc_item.len = v.length();
	} break;
	case NC_SHORT: {
		VAL_TO_ITEM(int16_t);
	} break;
	case NC_INT: {
		VAL_TO_ITEM(int32_t);
	} break;
	case NC_FLOAT: {
		VAL_TO_ITEM(float);
	} break;
	case NC_DOUBLE: { 
		VAL_TO_ITEM(double);
	} break;
	case NC_UBYTE: {
		VAL_TO_ITEM(uint8_t);
	} break;
	case NC_USHORT: {
		VAL_TO_ITEM(uint16_t);
	} break; 
	case NC_UINT: { 
		VAL_TO_ITEM(uint32_t);
	} break; 
#if NODE_MAJOR_VERSION >= 10
	case NC_UINT64: {
		VAL_TO_ITEM(uint64_t);
	} 
	break;
	case NC_INT64: {
		VAL_TO_ITEM(int64_t);
	} 
	break;
#endif
	case NC_STRING:{
		std::vector<std::unique_ptr<const std::string > >* string = new std::vector<std::unique_ptr<const std::string > >() ;
				std::vector<const char*>* cstrings = new std::vector<const char*>();
				if(value.IsArray()){
					auto arr = value.As<Napi::Array>();
					nc_item.len = static_cast<int>(arr.Length());
					for (int i =0; i<static_cast<int>(arr.Length()); i++){
						Napi::Value napiV=arr[i];
						string->push_back(std::make_unique<std::string>(std::string(napiV.ToString().Utf8Value())));
						cstrings->push_back(string->at(i)->c_str());
					}
				} else {
					string->push_back(std::make_unique<std::string>(std::string(value.As<Napi::String>().ToString().Utf8Value())));
					cstrings->push_back(string->at(0)->c_str());
					nc_item.len = 1;
				}
				nc_item.value.v = cstrings->data();
	}
	break;
	default:
        deferred.Reject(Napi::String::New(info.Env(),"Variable type not supported yet"));
		return deferred.Promise();
	}

    auto worker=new NCAsyncWorker<int>(
		env, deferred,
		[id, parent_id, nc_item] (const NCAsyncWorker<int>* worker) {
            int mode;
	        NC_CALL(nc_inq_var_fill(parent_id, id, &mode, NULL));
	        NC_CALL(nc_def_var_fill(parent_id, id, mode, nc_item.value.v));
	        
		    return 1;
		},
		[] (Napi::Env env, int result) {
            return Napi::String::New(env,"OK");
		}
		
	);
	worker->Queue();
    
    return deferred.Promise();
}

// Napi::Value Variable::GetCompressionShuffle(const Napi::CallbackInfo &info) {
//     Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
//     deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
//     return deferred.Promise();
// }

// void Variable::SetCompressionShuffle(const Napi::CallbackInfo &info) {
//     Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
//     deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
//     return deferred.Promise();
// }

// Napi::Value Variable::GetCompressionDeflate(const Napi::CallbackInfo &info) {
//     Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
//     deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
//     return deferred.Promise();
// }

// void Variable::SetCompressionDeflate(const Napi::CallbackInfo &info) {
//     Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
//     deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
//     return deferred.Promise();
// }

// Napi::Value Variable::GetCompressionLevel(const Napi::CallbackInfo &info) {
//     Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
//     deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
//     return deferred.Promise();
// }

// void Variable::SetCompressionLevel(const Napi::CallbackInfo &info) {
//     Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
//     deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
//     return deferred.Promise();
// }

Napi::Value Variable::Inspect(const Napi::CallbackInfo &info) {
	return Napi::String::New(info.Env(), 
		string_format(
			"[Variable %s, type %s, %i dimension(s)]",
			this->name.c_str(),
			NC_TYPES(this->type),
			this->ndims)
	);
}


}// namespace netcdf4async