#include "NetCDFFile.h"
#include "NetCDFcommon.h"
#include <iostream>
#include <netcdf.h>
#include <string>
#include <exception>


NetCDFFile::NetCDFFile(std::string file_name, std::string mode, std::string format){
    this->name = file_name;
    
	this->file_format = NC_NETCDF4;
	if (!format.empty()) {
		if (format == "classic") {
			this->file_format = 0;
		} else if (format == "classic64") {
			this->file_format = NC_64BIT_OFFSET;
		} else if (format == "netcdf4") {
			this->file_format = NC_NETCDF4;
		} else if (format == "netcdf4classic") {
			this->file_format = NC_NETCDF4 | NC_CLASSIC_MODEL;
		} else {
			throw ;
		}
	}
    open(this->name.c_str(), mode, this->file_format);
}

NetCDFFile::~NetCDFFile() {
    if (!closed) {
		NETCDF_CALL(nc_close(id));
	}
}

void NetCDFFile::open(const char *filename, std::string mode, int &format) {
	if (mode == "r") {
		NETCDF_CALL(nc_open(name.c_str(), NC_NOWRITE, &id));
	} else if (mode == "w") {
        NETCDF_CALL(nc_open(name.c_str(), NC_WRITE, &id));
	} else if (mode == "c") {
        NETCDF_CALL(nc_open(name.c_str(), format | NC_NOCLOBBER, &id));
	} else if (mode == "c!") {
        NETCDF_CALL(nc_open(name.c_str(), format| NC_CLOBBER, &id));
	} else {
        throw ;		
	}
    NETCDF_CALL(nc_inq_format_extended(id, &format, NULL));
    this->closed = false;
}

void NetCDFFile::Close() {
    if (!this->closed) {
		this->closed = true;
		int retval = nc_close(this->id);
		if (retval != NC_NOERR && retval != NC_EBADID) {
			throw ; //Napi::Error::New(info.Env(), nc_strerror(retval)).ThrowAsJavaScriptException();
		}
	}
}

void NetCDFFile::Sync() {}

void NetCDFFile::DataMode() {}

int NetCDFFile::GetId() {
    return id;
}

std::string NetCDFFile::GetName() {
    return name;
}

bool NetCDFFile::IsClosed() {
    return closed;
}

std::string NetCDFFile::GetFormat() {
    std::string format;
    switch (file_format)
    {
    case 0:
        format = "classic";
        break;
    case NC_64BIT_OFFSET:
        format = "classic64";
        break;
    case NC_NETCDF4:
        format = "netcdf4";
        break;
    case NC_NETCDF4 | NC_CLASSIC_MODEL:
        format = "netcdf4classic";
        break;
    default:
        format = "netcdf4";
        break;
    }
    return format;
}

std::string NetCDFFile::Inspect() {
    return string_format(
			"[%s%s file %s]",
			this->closed?"Closed ":"Open",
			GetFormat().c_str(),
			this->name.c_str()
		);
}
