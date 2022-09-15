const { copyFileSync, unlinkSync } = require("fs");
const { tmpdir } = require("os");
const { join } = require("path");
const uniqueFilename = require('unique-filename');
const netcdf4 = require("..");

let files=[];

const newFile=async (name,mode='w',filetype=undefined)=>{
    const tempFileName = uniqueFilename(tmpdir(), `nc`);
    if (mode[0]!=='c') {
        copyFileSync(name, tempFileName);
    }
    const file=await netcdf4.open(tempFileName, mode,filetype);
    files.push(file);
    return file;
  };

  const closeAll=async ()=>{
    for (const f of files) {
      try{
        if (!f.closed) {
          await f.close();
        }
        unlinkSync(f.name);          
      } catch (e) {
        console.log(`Exception ${e.message} due file ${file.name} closing`);
      }
    };
    files=[];
  };

  const arrTypes={
    "byte":[Int8Array,Number],
    "short":[Int16Array,Number],
    "int":[Int32Array,Number],
    "float":[Float32Array,Number],
    "double":[Float64Array,Number],
    "ubyte":[Uint8Array,Number],
    "ushort":[Uint16Array,Number],
    "uint":[Uint32Array,Number],
    "string":[Array.from,String]
  };
  if (process.versions.node.split(".")[0]>=10) {
    arrTypes["uint64"]=[BigUint64Array,BigInt];
    arrTypes["int64"]=[BigInt64Array,BigInt];
  };

  module.exports={
    newFile,closeAll,arrTypes
  };

