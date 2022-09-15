#include <iostream>
#include <netcdf.h>
#include <string>
#include <node_version.h>
#include "netcdf4-async.h"
#include "async.h"
#include "Attribute.h"
#include "Macros.h"


namespace netcdf4async {

struct DeflateInfo {
	bool shuffle;
	bool deflate;
	int level;
};

Napi::FunctionReference Variable::constructor;

Napi::Object Variable::Build(Napi::Env env, int id, int parent_id, std::string name, nc_type type, int ndims) {
	return constructor.New({Napi::Number::New(env, id),
		Napi::Number::New(env, parent_id),
		Napi::String::New(env, name),
		Napi::Number::New(env, type),
		Napi::Number::New(env, ndims)});
}

Variable::Variable(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Variable>(info) {
	if (info.Length() < 5) {
		Napi::TypeError::New(info.Env(), "Wrong number of arguments").ThrowAsJavaScriptException();
		return;
	}
	this->id = info[0].As<Napi::Number>().Int32Value();
	this->parent_id = info[1].As<Napi::Number>().Int32Value();
	this->name = info[2].As<Napi::String>().Utf8Value();
	this->type = info[3].As<Napi::Number>().Int32Value();
	this->ndims = info[4].As<Napi::Number>().Int32Value();
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
                InstanceMethod("getEndianness", &Variable::GetEndianness),
                InstanceMethod("setEndianness", &Variable::SetEndianness),
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
    Napi::Env env = info.Env();
	Variable *variable = this;
	auto worker=new NCAsyncWorker<std::string>(
		env,
		[id = this->id, parent_id = this->parent_id] (const NCAsyncWorker<std::string>* worker) {
			char var_name[NC_MAX_NAME + 1];
			NC_CALL(nc_inq_varname(parent_id, id, var_name));
			return std::string(var_name);
		},
		[variable] (Napi::Env env, std::string result) {
			variable->set_name(result);
			return Napi::String::New(env, result);
		}
	);
	worker->Queue();
    return worker->Deferred().Promise();	
}

Napi::Value Variable::GetNameSync(const Napi::CallbackInfo &info) {
    return Napi::String::New(info.Env(),this->name.c_str());
}

Napi::Value  Variable::SetName(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (info.Length() < 1) {
		deferred.Reject(Napi::String::New(info.Env(),"Wrong number of arguments"));
		return deferred.Promise();
	}
    Napi::Env env = info.Env();
	Variable *variable =  this;
	std::string new_name = info[0].As<Napi::String>().Utf8Value();
	auto worker=new NCAsyncWorker<std::string>(
		env, deferred,
		[id = this->id, parent_id = this->parent_id, new_name] (const NCAsyncWorker<std::string>* worker) {
			NC_CALL(nc_rename_var(parent_id, id,  new_name.c_str()));
			return std::string(new_name);
		},
		[variable] (Napi::Env env, std::string result) {
			variable->set_name(result);
			return Napi::String::New(env, result);
		}
	);
	worker->Queue();
    return worker->Deferred().Promise();
}

Napi::Value Variable::GetDimensions(const Napi::CallbackInfo &info) {
	struct DementionInfo {
		int id;
		int parent_id;
		std::string name;
		size_t length;
	};

	Napi::Env env = info.Env();
    auto worker = new NCAsyncWorker<std::vector<DementionInfo>>(
		env,
		[id = this->id, parent_id = this->parent_id, ndims = this->ndims] (const NCAsyncWorker<std::vector<DementionInfo>>* worker) {
			int *dim_ids = new int[ndims];
			NC_CALL(nc_inq_vardimid(parent_id, id, dim_ids));
			std::vector<DementionInfo> dementions;
			for (int i = 0; i < ndims; i++) {
				DementionInfo dementionInfo;
				dementionInfo.id = dim_ids[i];
				dementionInfo.parent_id = parent_id;
				char varName[NC_MAX_NAME + 1];
				NC_CALL(nc_inq_dim(dementionInfo.parent_id, dementionInfo.id, varName, &dementionInfo.length));
				dementionInfo.name = std::string(varName);
				dementions.push_back(dementionInfo);
			}
			return dementions;
		},
		[] (Napi::Env env, std::vector<DementionInfo> result) {
			Napi::Object dimensions = Napi::Object::New(env);
			for (auto nc_dim= result.begin(); nc_dim != result.end(); nc_dim++){
				auto len = nc_dim->length==NC_UNLIMITED?Napi::String::New(env,"unlimited"):Napi::Number::New(env,nc_dim->length);
				dimensions.Set(Napi::String::New(env,nc_dim->name), len);
			}
         	return dimensions;
		}
	);
	worker->Queue();
    return worker->Deferred().Promise();
}

Napi::Value Variable::GetFill(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    int id = this->id;
    int parent_id = this->parent_id;
    nc_type type = this->type;
    auto worker=new NCAsyncWorker<UnionType>(
		env, 
		[id, parent_id, type] (const NCAsyncWorker<UnionType>* worker) {
            UnionType result;
            switch (type) {
	        case NC_BYTE: 
		        TYPED_VALUE(result,int8_t,1)
		    break;
	        case NC_CHAR: 
		        TYPED_VALUE(result,char,2)
		        result.s[1] = 0; 
	        break;
	        case NC_SHORT: 
		        TYPED_VALUE(result,int16_t,1)
	        break;
	        case NC_INT:
		        TYPED_VALUE(result,int32_t,1)
		    break;
	        case NC_FLOAT: 
		        TYPED_VALUE(result,float,1)
			break;
	        case NC_DOUBLE:
		        TYPED_VALUE(result,double,1)
			break;
	        case NC_UBYTE: 
		        TYPED_VALUE(result,uint8_t,1)
		    break;
	        case NC_USHORT: 
		        TYPED_VALUE(result,uint16_t,1)
	        break;
	        case NC_UINT:
		        TYPED_VALUE(result,uint32_t,1)
		    break;
#if NODE_MAJOR_VERSION > 9
	        case NC_UINT64:
		        TYPED_VALUE(result,uint64_t,1)
		    break;
	        case NC_INT64:
		        TYPED_VALUE(result,int64_t,1)
		    break;
#endif
	        case NC_STRING:
		        result.ps = new char *[1];
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

Napi::Value  Variable::SetFill(const Napi::CallbackInfo &info) {
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


Napi::Value Variable::GetChunked(const Napi::CallbackInfo &info) {
	Napi::Env env =info.Env();
    auto worker=new NCAsyncWorker<Item>(
		env,
		[parent_id = this->parent_id, id = this-> id, ndims = this->ndims] (const NCAsyncWorker<Item>* worker) {
            int v;
			Item item;
			item.value.size =  new size_t[ndims];
			NC_CALL(nc_inq_var_chunking(parent_id, id, &v, item.value.size));
	
			switch (v) {
			case NC_CONTIGUOUS:
				item.name = "contiguous";
				break;
			case NC_CHUNKED:
				item.name = "chunked";
				break;
			default:
				item.name = "unknown";
			break;
			}
			item.len = ndims;
			
		    return item;
		},
		[] (Napi::Env env, Item result) {
			Napi::Value value;
			Item* nc_item = &result;
			ITEM_TO_VAL(int32_t);
			Napi::Object obj = Napi::Object::New(env);
			obj.Set(Napi::String::New(env, nc_item->name), value); 
            return obj;
		}
		
	);
	worker->Queue();
    
    return worker->Deferred().Promise();
}

Napi::Value  Variable::SetChunked(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (info.Length() < 2) {
		deferred.Reject(Napi::String::New(info.Env(),"Wrong number of arguments"));
		return deferred.Promise();
	}
    Napi::Env env = info.Env();
	std::string arg = info[0].As<Napi::String>().Utf8Value();
	int v;
	if (arg == "contiguous") {
		v = NC_CONTIGUOUS;
	} else if (arg == "chunked") {
		v = NC_CHUNKED;
	} else {
		deferred.Reject(Napi::String::New(info.Env(),"Unknown value"));
		return deferred.Promise();
	}
	Napi::Value value = info[1];
	if (!value.IsTypedArray()) {
		deferred.Reject(Napi::String::New(info.Env(),"Expecting an array"));
		return deferred.Promise();
	}
	Napi::Uint32Array array = value.As<Napi::Uint32Array>();
	if (array.ElementLength() != static_cast<size_t>(this->ndims)) {
		deferred.Reject(Napi::String::New(info.Env(),"Wrong array size"));
		return deferred.Promise();
	}
	size_t *sizes = new size_t[this->ndims];
	for (int i = 0; i < this->ndims; i++) {
		sizes[i] = array[i];
	}
	auto worker = new NCAsyncWorker<int>(
		env, deferred, 
		[parent_id = this->parent_id, id = this-> id, ndims = this->ndims, v, sizes] (const NCAsyncWorker<int>* worker) {
			NC_CALL(nc_def_var_chunking(parent_id, id, v, sizes));
			delete[] sizes;
            return 1;
		},
		[] (Napi::Env env, int result) {
			return Napi::String::New(env, "OK");
		}
		
	);
	worker->Queue();

    return deferred.Promise();
}

Napi::Value Variable::GetDeflateInfo(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	auto worker = new NCAsyncWorker<DeflateInfo>(
		env, 
		[parent_id = this->parent_id, id = this-> id] (const NCAsyncWorker<DeflateInfo>* worker) {
			DeflateInfo result;
			int shuffle;
			int deflate;
			int level;
			NC_CALL(nc_inq_var_deflate(parent_id, id, &shuffle, &deflate, &level));
			result.shuffle = shuffle==1;
			result.deflate = deflate==1;
			result.level = level;
            return result;
		},
		[] (Napi::Env env, DeflateInfo result) {
			Napi::Object obj = Napi::Object::New(env);
			obj.Set(Napi::String::New(env, "shuffle"), Napi::Boolean::New(env, result.shuffle));
			obj.Set(Napi::String::New(env, "deflate"), Napi::Boolean::New(env, result.deflate));
			obj.Set(Napi::String::New(env, "level"), Napi::Number::New(env, result.level));
			return obj;
		}
	);
	worker->Queue();
    return worker->Deferred().Promise();
}

Napi::Value  Variable::SetDeflateInfo(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (info.Length() < 3) {
		deferred.Reject(Napi::String::New(info.Env(),"Wrong number of arguments"));
		return deferred.Promise();
	}
	if (!info[0].IsBoolean() || !info[1].IsBoolean()) {
		deferred.Reject(Napi::String::New(info.Env(),"Expecting a boolean"));
		return deferred.Promise();
	}
    DeflateInfo deflateInfo;
	deflateInfo.shuffle = info[0].As<Napi::Boolean>();
	deflateInfo.deflate = info[1].As<Napi::Boolean>();
	deflateInfo.level = info[2].As<Napi::Number>().Uint32Value();
	auto worker = new NCAsyncWorker<DeflateInfo>(
		env, deferred, 
		[parent_id = this->parent_id, id = this-> id, deflateInfo] (const NCAsyncWorker<DeflateInfo>* worker) {
			NC_CALL(nc_def_var_deflate(parent_id, id, deflateInfo.shuffle? 1:0, deflateInfo.deflate? 1:0, deflateInfo.level));
	        return deflateInfo;
		},
		[] (Napi::Env env, DeflateInfo result) {
			Napi::Object obj = Napi::Object::New(env);
			obj.Set(Napi::String::New(env, "shuffle"), Napi::Boolean::New(env, result.shuffle));
			obj.Set(Napi::String::New(env, "deflate"), Napi::Boolean::New(env, result.deflate));
			obj.Set(Napi::String::New(env, "level"), Napi::Number::New(env, result.level));
			return obj;
		}
	);
	worker->Queue();
    return worker->Deferred().Promise();
}

Napi::Value Variable::GetEndianness(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
	auto worker=new NCAsyncWorker<int>(
		env,
		[id=this->id, parent_id=this->parent_id] (const NCAsyncWorker<int>* worker) {
            int v;
			NC_CALL(nc_inq_var_endian(parent_id, id, &v));
		    return v;
		},
		[] (Napi::Env env, int result) {
			const char *res;
            switch (result) {
			case NC_ENDIAN_LITTLE:
				res = "little";
				break;
			case NC_ENDIAN_BIG:
				res = "big";
				break;
			case NC_ENDIAN_NATIVE:
				res = "native";
				break;
			default:
				res = "unknown";
				break;
			}
			return Napi::String::New(env, res);
		}
		
	);
	worker->Queue();
	
    return worker->Deferred().Promise();
}


Napi::Value Variable::SetEndianness(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	Napi::Env env = info.Env();
	if (info.Length() < 1) {
		deferred.Reject(Napi::String::New(info.Env(),"Wrong number of arguments"));
		return deferred.Promise();
	}
    std::string arg = info[0].As<Napi::String>().Utf8Value();
	int v;
	if (arg == "little") {
		v = NC_ENDIAN_LITTLE;
	} else if (arg == "big") {
		v = NC_ENDIAN_BIG;
	} else if (arg == "native") {
		v = NC_ENDIAN_NATIVE;
	} else {
		deferred.Reject(Napi::String::New(info.Env(),"Unknown value"));
		return deferred.Promise();
	}
	auto worker=new NCAsyncWorker<int>(
		env, deferred,
		[id=this->id, parent_id=this->parent_id, v](const NCAsyncWorker<int>* worker) {
            NC_CALL(nc_def_var_endian(parent_id,id,v));
		    return v;
		},
		[] (Napi::Env env, int result) {
			return Napi::Number::New(env, result);
		});
	worker->Queue();
    return worker->Deferred().Promise();
}


Napi::Value Variable::GetChecksumMode(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
	auto worker=new NCAsyncWorker<int>(
		env,
		[id=this->id, parent_id=this->parent_id](const NCAsyncWorker<int>* worker) {
            int v;
			NC_CALL(nc_inq_var_fletcher32(parent_id, id, &v));
			return v;
		},
		[] (Napi::Env env, int result) {
			std::string res;
			switch (result) {
			case NC_NOCHECKSUM:
				res = "none";
				break;
			case NC_FLETCHER32:
				res = "fletcher32";
				break;
			default:
				res = "unknown";
				break;
			}
			return Napi::String::New(env, res);
		});
	worker->Queue();	
    return worker->Deferred().Promise();
}

Napi::Value  Variable::SetChecksumMode(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	Napi::Env env = info.Env();
	if (info.Length() < 1) {
		deferred.Reject(Napi::String::New(info.Env(),"Wrong number of arguments"));
		return deferred.Promise();
	}
    std::string arg = info[0].As<Napi::String>().Utf8Value();
	int v;
	if (arg == "none") {
		v = NC_NOCHECKSUM;
	} else if (arg == "fletcher32") {
		v = NC_FLETCHER32;
	} else {
		deferred.Reject(Napi::String::New(info.Env(), "Unknown value"));
		return deferred.Promise();
	}

    auto worker=new NCAsyncWorker<int>(
		env,
		[id=this->id, parent_id=this->parent_id, v](const NCAsyncWorker<int>* worker) {
            NC_CALL(nc_def_var_fletcher32(parent_id, id, v));
			return 1;
		},
		[] (Napi::Env env, int result) {
			return Napi::Number::New(env, result);
		});
	worker->Queue();	
    return worker->Deferred().Promise();
}


Napi::Value Variable::GetAttributes(const Napi::CallbackInfo &info) {
   bool return_type = true;
	if(info.Length() >0) {
		return_type = info[0].As<Napi::Boolean>();
	}
	Napi::Env env = info.Env();
	int id = this->id;
	int parent_id = this->parent_id;
 	return netcdf4async::get_attributes(env, parent_id, id, return_type).Promise();
}

Napi::Value Variable::AddAttribute(const Napi::CallbackInfo &info) {
   Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
	if (info.Length() != static_cast<size_t>(3)) {
		deferred.Reject(Napi::String::New(info.Env(),"Not all parameters from name,type,value bound"));
		return deferred.Promise();
	}
	std::string type_str=info[1].As<Napi::String>().ToString();
	int type=get_type(type_str);
	std::string name=info[0].As<Napi::String>().ToString();
	Napi::Env env = info.Env();
	int id = this->id;
	int parent_id = this->parent_id;
	return add_attribute(env, deferred, parent_id, id, name, type, info[2]);
}


Napi::Value Variable::SetAttribute(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::RenameAttribute(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    Napi::Env env =  info.Env();
	if (info.Length() <2) {
		deferred.Reject(Napi::String::New(info.Env(),"Wrong number of arguments"));
		return deferred.Promise();
	}
	std::string old_attribute_name = info[0].As<Napi::String>().Utf8Value();
	std::string new_attribute_name = info[0].As<Napi::String>().Utf8Value();
    return rename_attribute(env, deferred, this->parent_id, this->id, old_attribute_name, new_attribute_name).Promise();
}

Napi::Value Variable::DeleteAttribute(const Napi::CallbackInfo &info) {
	Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    Napi::Env env =  info.Env();
	if (info.Length() <1) {
		deferred.Reject(Napi::String::New(info.Env(),"Wrong number of arguments"));
		return deferred.Promise();
	}
	std::string attribute_name = info[0].As<Napi::String>().Utf8Value();
    return delete_attribute(env, deferred, this->parent_id, this->id, attribute_name).Promise();
}

Napi::Value Variable::Write(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::WriteSlice(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}

Napi::Value Variable::WriteStridedSlice(const Napi::CallbackInfo &info) {
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    deferred.Reject(Napi::String::New(info.Env(),"Not implemented yet"));
    return deferred.Promise();
}


Napi::Value Variable::Read(const Napi::CallbackInfo &info) {
	Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred=Napi::Promise::Deferred::New(info.Env());
    if (info.Length() != static_cast<size_t>(this->ndims)) {
		deferred.Reject(Napi::String::New(info.Env(), "Wrong number of arguments"));
    	return deferred.Promise();
	}
	
	size_t *pos = new size_t[this->ndims];
	size_t *size = new size_t[this->ndims];
	for (int i = 0; i < this->ndims; i++) {
		pos[i] = info[i].As<Napi::Number>().Int64Value();
		size[i] = 1;
	}
    auto worker=new NCAsyncWorker<Item>(
		env, deferred, 
		[id=this->id, parent_id=this->parent_id, type = this->type, pos, size](const NCAsyncWorker<Item>* worker) {
			Item result;
			result.type = type;
			result.len = 1;
			typedValue(&result);
            NC_CALL(nc_get_vara(parent_id, id, pos, size, result.value.v));
			delete[] pos;
			delete[] size;
			return result;
		},
		[] (Napi::Env env, Item result) {
			
			return item2value(env, &result);
		});
	worker->Queue();	
	
    return worker->Deferred().Promise();
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

void Variable::set_name(std::string new_name) {
	this->name = new_name;
}


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