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

### Import 

```javascript
const netcdf4 = require("netcdf4-async");
```

### Supported netcdf types

  | type | two-char synonym | one-char synonym | Note |
  | --- | --- | --- | --- |
  | byte | i1 | b B | |
  | char |  |  | |
  | short | i2 | h s | |
  | int | i4 | i l | |
  | ubyte | u1 | | |
  | ushort | u2 | | |
  | uint | u4 | | | |
  | float | f4 | f | |
  | double | f8 | d | |
  | uint64 | u8 | | NodeJS v>=10 |
  | int64 | i8 | | NodeJS v>=10 |
  | string | S1 | | |

**NB!** Not all types support in all file types

### Module properties

* `version` : Contains netcdf4 library version. Properties are:
  * `major` : Major version (i.e. 4)
  * `minor` : Minor version (i.e. 8)
  * `patch` : Patch version (i.e. 1)
  * `version` : Version string (i.e. "4.8.1")

Example:  

```json
    {
    "major" : 4,
    "minor" : 8,
    "patch" : 1,
    "version" : "4.8.1"
    }
```

### Methods

* `open(path,mode[,format])`: Return a promise resolved to `File` if file successfully opened, rejected otherwise
    * Parameters
        * `path`: path to file
        * `mode`: file open mode

        | mode | Description |
        | --- | --- |
        | r | read only |
        | w | read/write |
        | c | create if file not exsits, fail otherwise |
        | c! | create new or overwrite existing |

        * `format`: File type. Means `classic` or `netcdf4` if omitted 
    * Examples
        * Promises
        ```javascript
        netcdf4.open('test.nc','r')
            .then(file=>do_process(file))
            .catch(e=>console.log(`Can't open file - ${e.message}`));
        ```
        * Async/await
        ```javascript
        try{
            const file=await netcdf4.open('test.nc','r');
            await do_process(file);
        } catch (e) {
            console.log(`Can't open file - ${e.message}`)
        }
        ```

## Classes

### **File**

Represent netcdf file instance

* Properties
    * `name`: file path
    * `format`: file format
    * `open`: Set to `true`, if file open
    * `root`: Instance main `Group` object. Definied only when file opened
* Methods
    * `sync()`: Return a promise resolved if file successfully synced
    * `close()`: Return a promise resolved if file successfully closed, rejected otherwise
        * Examples
            * Promises
            ```javascript
            file.close()
                .then(()=>console.log('File closed'))
                .catch(e=>console.log(`Error closing file - ${e.message}`));
            ```
            * Async/await
            ```javascript
            try{
                await file.close();
            } catch (e) {
                console.log(`Error closing file - ${e.message}`)
            }
            ```
    * `dataMode()`: Return a promise resolved if file successfully perform `nc_enddef(..)`
    
### **Group**

NetCDF group implementation.

From original documentation:

>NetCDF-4 added support for hierarchical groups within netCDF datasets.
>
>Group operations are only permitted on netCDF-4 files. Groups are not compatible with the netCDF classic data model except the root group.
>
>Variable are only visible in the group in which they are defined. The same applies to attributes.
>
>Dimensions are visible in their groups, and all child groups.

* Methods
    * `getName()` : Resolve promise to group name
    * `setName(name)`: Rename group
    * `getPath()` : Resolve promise to full name (path in file)
    * `getVariables()` : Resolve to associative array of variables in group
    * `addVariable(name,type,dimensions)`: Added variable to group. Resolves to instance of `Variable`.
        * Parameters:
            * `name`: Variable name
            * `type`: Variable type
            * `dimensions`: Array of dimenison names
    * `getDimensions([unlimited])` : Resolve to associative array of dimensions or unlimited dimensions in group 
        * Parameters
            * `unlimited`: Boolean. If set to true then retrun only unlimited dimensions
        * Resolved as list of objects
        ```json
        {
            "name of dimension":length or 'unlimited',
            "name of dimension":length or 'unlimited',
            . . .
            "name of dimension":length or 'unlimited'
        }
        ```
    * `addDimension(name,length)`: Added new dimension in a group. 
    * `renameDimension(oldName,newName)`: Rename dimension
    * `getAttributes([asDefined])` : Resolve to associative array of attributes of group
        * Parameters
            * `asDefined`: Boolean. If set to true then instead value
            will return type and value 
        * Resolved as list of objects
            * `asDefined`===false
            ```json
            {
                "attribute_1":value_1,
                "attribute_1":value_1,
                . . .
                "attribute_n":value_n
            }
            ```
            * `asDefined`===false or not set
            ```json
            {
                "attribute_1":{
                    "type":"type of attribute",
                    "value":value_1
                },
                "attribute_2":{
                    "type":"type of attribute",
                    "value":value_2
                },
                . . .
                "attribute_n":{
                    "type":"type of attribute",
                    "value":value_n
                },
            }
            ```
    * `setAttribute(name,value,type?)`: Set value of attribute
    * `renameAttribute(oldName,newName)`: Rename attribute
    * `deleteAttribute(name)`: Delete attribute
    * `getSubrgroups(..)` : Resolve to associative array of subgroups of group
    * `getSubgroups(name)` : Resolve to subgroup
    * `addSubgroup(name)` : Resolve to new created subgroup

### **Variable**

NetCDF variable

From original documentation:

>Variables hold multi-dimensional arrays of data.
>
> A netCDF variable has a name, a type, and a shape, which are specified when it is defined. A variable may also have values, which are established later in data mode.
>
>Attributes may be associated with a variable to specify such properties as units.


* Properties
    * `type`: Variable type
    * `name`: Variable name. 

* Methods

    * `getName()` : Resolve promise to variable name
    * `setName(name)`: Rename variable
    * `getDimensions()` : Resolve to associative array of variable dimensions
        * Resolved as list of objects
        ```json
        {
            "name of dimension":length or 'unlimited',
            "name of dimension":length or 'unlimited',
            . . .
            "name of dimension":length or 'unlimited'
        }
        ```
    * `getFill()`: Resolve to defaut fill value or undefined, if variable in no fill mode
    ```json
    {
        "mode":"chunk_mode",
        "value":"fill value"
    }
    ```
    * `setFill(mode,value?)`: Set default fill value or switch variable to no fill mode if value is undefined/not provided
    * `getChunked()`: Resolve to current chunk information
    ```json
    {
        "mode":"chunk_mode",
        "sizes":[dim1_size,dim2_size,...,dimn_size]
    }
    ```
    * `setChunked(mode,size?)`: Update information. `size` if provided must have same length as dimensions;
    * `getDeflateInfo()`: Resolve to current shuffle/deflate info
    ```json
    {
        "shuffle":boolean,
        "deflate":boolean,
        "level":0-9
    }
    ```
    * `setDeflateInfo(shuffle,deflate,deflateLevel)`: set shuffle/deflation value
    * `getEndiannes()`: Resolve to one of three value `little`,`big`,`native`
    * `setEndianees(value)`: Set endiannes
    * `getChecksumMode()`: Resolve to `fletcher` or `none`
    * `setChecksumMode(mode)`: Set checksum mode

    * `getAttributes([asDefined])` : Resolve to associative array of attributes of group
        * Parameters
            * `asDefined`: Boolean. If set to true then instead value
            will return type and value 
        * Resolved as list of objects
            * `asDefined`===false
            ```json
            {
                "attribute_1":value_1,
                "attribute_1":value_1,
                . . .
                "attribute_n":value_n
            }
            ```
            * `asDefined`===false or not set
            ```json
            {
                "attribute_1":{
                    "type":"type of attribute",
                    "value":value_1
                },
                "attribute_2":{
                    "type":"type of attribute",
                    "value":value_2
                },
                . . .
                "attribute_n":{
                    "type":"type of attribute",
                    "value":value_n
                },
            }
            ```
    * `setAttribute(name,value)`: Set value of attribute
    * `renameAttribute(oldName,newName)`: Rename attribute
    * `deleteAttribute(name)`: Delete attribute

    
* Data access methods    
    * `read(pos....)` : Reads and returns a single value at positions
    given as for `write`.
    * `readSlice(pos, size....)` : Reads and returns an array of values (cf.
    ["Specify a Hyperslab"](https://www.unidata.ucar.edu/software/netcdf/docs/programming_notes.html#specify_hyperslab))
    at positions and sizes given for each dimension, `readSlice(pos1,
    size1, pos2, size2, ...)` e.g. `readSlice(2, 3, 4, 2)` gives an
    array of the values at position 2 for 3 steps along the first
    dimension and position 4 for 2 steps along the second one.
    * `readStridedSlice(pos, size, stride....)` : Similar to `readSlice()`, but it
    adds a *stride* (interval between indices) parameter to each dimension. If stride is 4,
    the function will take 1 value, discard 3, take 1 again, etc.
    So for instance `readStridedSlice(2, 3, 2, 4, 2, 1)` gives an
    array of the values at position 2 for 3 steps with stride 2 (i.e.
    every other value) along the first dimension and position 4 for 2 steps
    with stride 1 (i.e. with no dropping) along the second dimension.
    * `write(pos..., value)` : Write `value` at positions given,
    e.g. `write(2, 3, "a")` writes `"a"` at position 2 along the first
    dimension and position 3 along the second one.
    * `writeSlice(pos, size..., valuearray)` : Write values in `valuearray`
    (must be a typed array) at positions and sizes given for each
    dimension, e.g. `writeSlice(2, 3, 4, 2, new
    Int32Array([0, 1, 2, 3, 4, 5]))` writes the array at position 2 for
    3 steps along the first dimension and position 4 for 2 step along
    the second one (cf.
    ["Specify a Hyperslab"](https://www.unidata.ucar.edu/software/netcdf/docs/programming_notes.html#specify_hyperslab)).
    * `writeStridedSlice(pos, size, stride..., valuearray)` : Similar to
    `writeSlice()`, but it adds a *stride* parameter to each dimension.
    So for instance `writeStridedSlice(2, 3, 2, 4, 2, 1), new
    Int32Array([0, 1, 2, 3, 4, 5])` writes the array
    at position 2 for 3 steps with stride 2 (i.e.
    every other value) along the first dimension and position 4 for 2 steps
    with stride 1 (i.e. with no dropping) along the second dimension.
    




## Knowing flaws

* Reading `variable.fillvalue` for string type variables causes segfault with netcdf4 version prior to 4.6.1 due to knowing issue [nc_inq_var_fill() doesn't work for NC_STRING if a fill value is set - segfault results](https://github.com/Unidata/netcdf-c/issues/732). So, ubuntu<=18.04 is affected. 


