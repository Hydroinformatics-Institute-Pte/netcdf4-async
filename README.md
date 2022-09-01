# netcdf4-async

NodeJS library provided async access to the 
[Network Common Data Form (NetCDF)](https://www.unidata.ucar.edu/software/netcdf/) files.

Build upon version >=4 of libnetcdf and nodejs >=10.x

## Installation

### Prerequestment

You will need `libnetcdf` >= 4.x installed.

#### **On Linux/Unix/OSX**

* Make sure your system fulfills all the prerequisites of [node-gyp](https://github.com/nodejs/node-gyp#on-unix)

* Install NetCDF4 using your package manager:

    * Ubuntu/Debian:

        ```bash
        sudo apt-get install libnetcdf-dev
        ```

    * Alpine based:

        ```bash
        sudo apk add "netcdf-dev"
        ```

* Buld from source code. Either download source code from [unidata](https://www.unidata.ucar.edu/downloads/netcdf/index.jsp) site or from [github repository](https://github.com/Unidata/netcdf-c), then follow the build instructions


#### **On Windows**

* Make sure your system fulfills all the prerequisites of [node-gyp](https://github.com/nodejs/node-gyp#on-windows)

* Install NetCDF4 from [unidata](https://www.unidata.ucar.edu/downloads/netcdf/index.jsp) site

* Set environment variable `NETCDF_DIR` to your netcdf installation, e.g
    ```bat
    C:> SET NETCDF_DIR=C:\Program files\netCDF 4.9.0
    ```

#### **On MacOS**

* Make sure your system fulfills all the prerequisites of [node-gyp](https://github.com/nodejs/node-gyp#on-macOS)

* Install NetCDF via homebrew:
    ```bash
    brew install netcdf
    ```

### **_Installation_**

Install `netcdf4-async` using npm:
```bash
npm install netcdf4-async
```


## Usage

* **_TODO_**

## Knowing flaws

* Reading `variable.fillvalue` for string type variables causes segfault with netcdf4 version prior to 4.6.1 due to knowing issue [nc_inq_var_fill() doesn't work for NC_STRING if a fill value is set - segfault results](https://github.com/Unidata/netcdf-c/issues/732). So, ubuntu<=18.04 is affected. 


