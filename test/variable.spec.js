const chai = require("chai");
const expect = chai.expect;
const chaiAsPromised = require('chai-as-promised');
const chaiAlmost=require("chai-almost");
chai.use(chaiAsPromised);
chai.use(chaiAlmost(0.0001));


const netcdf4 = require("..");
const { join } = require("path");

const fixture = join(__dirname, "testrh.nc");
const fixture1 = join(__dirname, "test_hgroups.nc");
const {newFile,closeAll,arrTypes}=require("./utils");


describe.only("Variable", function () {
  let fileold,filenew;

  beforeEach(async function () {
    fileold = await newFile(fixture);
    filenew = await newFile(fixture1);
  });

  afterEach(async function () {
    await closeAll();
  });

  it("should read variable params (hdf5)", async function() {
    const variable=await expect(filenew.root.getVariables()).eventually.to.have.property("UTC_time");
    expect(variable.inspect()).to.be.equal('[Variable UTC_time, type string, 1 dimension(s)]')
    expect(variable.name).to.be.equal('UTC_time')
    await expect(variable.getName()).eventually.to.be.equal('UTC_time')
    expect(variable.type).to.be.equal('string')
    await expect(variable.getEndianness()).eventually.to.be.equal('native')
    const attributes=await expect(variable.getAttributes()).to.be.fulfilled;
    expect(attributes).to.have.property('name')
    expect(attributes).to.have.property('unit')
    console.log("getDimension")
    await expect(variable.getDimensions()).eventually.to.be.equal({"recNum":74});
  });

  it("should read variable params (netCDF3)", async function () {
    const variable=await expect(fileold.root.getVariables()).eventually.to.have.property("var1");
    expect(variable.inspect(),'[Variable var1, type float, 1 dimenison(s)]')
    expect(variable.name).to.be.equal('var1')
    await expect(variable.getName()).eventually.to.be.equal('var1')
    expect(variable.type).to.be.equal('float')

    await expect(variable.getAttributes()).eventually.to.be.empty
    console.log("getDimension")
    await expect(variable.getDimensions()).eventually.to.be.equal({"dim1":10000});
  });

  it("should read an existing (netCDF3)",async function () {
    const variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;
    const ex=variable.read(0);
    await expect(ex).eventually.to.be.equal(420);
  });

  it("should read an existing (hdf5)", async function () {
    const variable = await expect(filenew.root.getVariable("UTC_time")).to.be.fulfilled;
    await expect(variable.read(0)).eventually.to.be.equal('2012-03-04 03:54:19');
    await expect(variable.read(1)).eventually.to.be.equal('2012-03-04 03:54:42');
  });

  it("should write an existing (hdf5)", async function () {
    let variable = await expect(filenew.root.getVariable("UTC_time")).to.be.fulfilled;
    await expect(variable.read(0)).eventually.to.be.equal('2012-03-04 03:54:19');
    await expect(variable.write(0,'2012-03-04 03:54:29')).to.be.fulfilled;
    await expect(variable.read(0)).eventually.to.be.equal('2012-03-04 03:54:29');
    await filenew.close();
      
    filenew =await netcdf4.open(filenew.name, "r");
    variable = await expect(filenew.root.getVariable("UTC_time")).to.be.fulfilled;
    await expect(variable.read(0)).eventually.to.be.equal('2012-03-04 03:54:29');
  });

  it("should write an existing (netcdf3)", async function () {
    let variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;
    await expect(variable.write(0,42)).to.be.fulfilled;
    await expect(variable.read(0)).eventually.to.be.equal(42);
    await fileold.close();
      
    fileold =await netcdf4.open(fileold.name, "r");
    await expect(variable.read(0)).eventually.to.be.equal('2012-03-04 03:54:29');
    await fileold.close();
    fileold =await netcdf4.open(fileold.name, "r");
    variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;    
    await expect(variable.read(0)).eventually.to.be.equal(42);
  });


  it("should rename an existing (netcdf3)",async function(){
    let variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;
    await expect(variable.setName("var2")).to.be.fulfilled;
    await expect(fileold.root.getVariables()).eventually.to.have.property("var2");
    await expect(variable.getName()).eventually.to.be.equal("var2");
    expect(variable.name).to.be.equal("var2");
    await fileold.close();
    fileold =await netcdf4.open(fileold.name, "r");
    variable = await expect(fileold.root.getVariable("var2")).to.be.fulfilled;    
    await expect(variable.getName()).eventually.to.be.equal("var2");
    expect(variable.name).to.be.equal("var2");
  });

  it("should rename an existing (hdf5)",async function(){
    let variable = await expect(filenew.root.getVariable("UTC_time")).to.be.fulfilled;
    await expect(variable.setName("UTC_timestamp")).to.be.fulfilled;
    await expect(filenew.root.getVariables()).eventually.to.have.property("UTC_timestamp");
    await expect(variable.getName()).eventually.to.be.equal("UTC_timestamp");
    expect(variable.name).to.be.equal("UTC_timestamp");
    await filenew.close();
    filenew =await netcdf4.open(filenew.name, "r");
    variable = await expect(filenew.root.getVariable("UTC_timestamp")).to.be.fulfilled;    
    await expect(variable.getName()).eventually.to.be.equal("UTC_timestamp");
    expect(variable.name).to.be.equal("UTC_timestamp");
  });


  it("should read a slice of existing",async function () {
    let variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;
    var res = await expect(variable.readSlice(0, 4)).to.be.fulfilled;
    var results = Array.from(res);
    expect(results).to.deep.equal([420, 197, 391.5, 399]);
//    console.log(res);
  });

  it("should read a strided slice",async function () {
    let variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;
    var res = await expect(variable.readSlice(0, 2,2)).to.be.fulfilled;
    var results = Array.from(res);
    expect(results).to.deep.equal([420, 391.5]);
  });

  it("should write a slice of existing",async function () {
    let variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;
    const varr=new Float32Array([10,10.5,20,20.5])
    await expect(variable.writeSlice(0, 4,varr)).to.be.fulfilled;
    var res = await expect(variable.readSlice(0, 4)).to.be.fulfilled;
    var results = Array.from(res);
    expect(results).to.deep.equal([10,10.5,20,20.5]);
    await fileold.close();
    fileold =await netcdf4.open(fileold.name, "r");
    variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;
    res = await expect(variable.readSlice(0, 4)).to.be.fulfilled;
    results = Array.from(res);
    expect(results).to.deep.equal([10,10.5,20,20.5]);
  });

  it("should write a strided slice", async function () {
    const varr=new Float32Array([30,20.5])
    let variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;
    await expect(variable.writeStridedSlice(0, 2, 2,varr)).to.be.fulfilled;
    var res = await expect(variable.readStridedSlice(0, 2, 2)).to.be.fulfilled;
    var results = Array.from(res);
    expect(results).to.deep.equal([30, 20.5]);
    res = await expect(variable.readSlice(0, 4)).to.be.fulfilled;
    results = Array.from(res);
    expect(results).to.deep.equal([30,197,20.5,399]);
    await fileold.close();
    fileold =await netcdf4.open(fileold.name, "r");
    variable = await expect(fileold.root.getVariable("var1")).to.be.fulfilled;
    res = await expect(variable.readSlice(0, 4)).to.be.fulfilled;
    results = Array.from(res);
    expect(results).to.deep.equal([30, 20.5]);
    res = await expect(variable.readSlice(0, 4)).to.be.fulfilled;
    results = Array.from(res);
    expect(results).to.deep.equal([30,197,20.5,399]);
  });

  /*
  it("should add new Variable whith set all parametrs", function(){
    //    console.log(filenew.root.dimensions);
        expect(filenew.root.variables).to.not.have.property("test_variable");
        var newVar=filenew.root.addVariable('test_variable','byte',[filenew.root.dimensions.recNum.id]);
        expect(newVar.inspect(),'[Variable test_variable, type byte, 1 dimension(s)]');
        expect(filenew.root.variables).to.have.property("test_variable");
        expect(filenew.root.variables.test_variable.inspect(),'[Variable test_variable, type byte, 1 dimension(s)]');
        newVar.endianness = "little";
        newVar.checksummode = "fletcher32";
        newVar.chunkmode = "chunked";
        newVar.chunksizes = new Uint32Array([8]);
        newVar.fillmode = true;
        newVar.fillvalue = 8;
        newVar.compressionshuffle = true;
        newVar.compressiondeflate = true;
        newVar.compressionlevel = 8;
        newVar.addAttribute("len", "int", 42);
        expect(newVar.attributes).to.have.property("len");
        filenew.close();
      
        filenew = new netcdf4.File(tempFileNewName, "r");
        expect(filenew.root.variables).to.have.property("test_variable");
        expect(filenew.root.variables.test_variable.inspect(),'[Variable test_variable, type byte, 1 dimension(s)]');
        newVar = filenew.root.variables.test_variable;
        expect(newVar.endianness).to.equal("little");
        expect(newVar.checksummode).to.equal("fletcher32");
        expect(newVar.chunkmode).to.equal("chunked");
        expect(newVar.chunksizes).to.deep.equal([8]);
        expect(newVar.fillmode).to.equal(true);
        expect(newVar.compressionshuffle).to.equal(true);
        expect(newVar.compressiondeflate).to.equal(true);
        expect(newVar).to.have.property("compressionlevel");
        expect(newVar.attributes).to.have.property("len");
      });
    

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

  const testFunc=(file,type,value,values,defaultValue)=>{
    const methods=arrTypes[type];

    it(`should create/read/write ${type} variable (${file})`,
      function(){
        let [fd,path]=file==='netcdf3'?[fileold,tempFileOldName]:[filenew,tempFileNewName];
        if (file==='netcdf3') {
            try{
              fd.close();
            } catch (ignore) {
              console.log(`Got ${ignore.message} during file closing, ingored`)
            }
            fd=new netcdf4.File(path,'c!','classic')
          fd.root.addDimension("dim1",75)
        }

        const dim=file==='netcdf3'?"dim1":"recNum";

        const issue1=(type==='string' && (netcdf4.version.minor<6 || (netcdf4.version.minor===6 && netcdf4.version.patch===0)));
        const issue2=(file=='netcdf3' && netcdf4.version.minor<7);

        expect(methods).to.be.not.empty;
        expect(fd.root.variables).to.not.have.property("test_variable");
        newVar=fd.root.addVariable('test_variable',type,[dim]);
        expect(newVar.name).to.be.equal('test_variable')
        expect(newVar.type).to.be.equal(type)
        if (issue1 || issue2) {
          // In netcdf4 library before 5.6.1 set default fill value for string leading to segfault
          // Same 
          fd.dataMode();
        }
        else {
          newVar.fillvalue=methods[1](defaultValue);
          expect(newVar.fillvalue).to.be.almost.eql(methods[1](defaultValue));  
          fd.dataMode();
          expect(newVar.read(0)).to.be.equal(newVar.fillvalue);
        }
        newVar.write(0,methods[1](value));
        expect(newVar.read(0)).to.be.almost.eql(methods[1](value));
        try{
          fd.close();
        } catch (ignore) {
          console.log(`Got ${ignore.message} during file closing, ingored`)
        }
        fd=new netcdf4.File(path,'r');
        expect(fd.root.variables).to.have.property("test_variable");
        expect(fd.root.variables.test_variable.read(0)).to.almost.eql(methods[1](value));
        if (!(issue1 || issue2)) {
          expect(fd.root.variables.test_variable.fillvalue).to.be.almost.eql(methods[1](defaultValue));
        }
      }
    )
    it(`should read/write slice ${type} variable (${file}) `,
      function() {
        let [fd,path]=file==='netcdf3'?[fileold,tempFileOldName]:[filenew,tempFileNewName];
        if (file==='netcdf3') {
          try{
            fd.close();
          } catch (ignore) {
            console.log(`Got ${ignore.message} during file closing, ingored`)
          }
          fd=new netcdf4.File(path,'c!','classic')
          fd.root.addDimension("dim1",75)
        }
        const dim=file==='netcdf3'?"dim1":"recNum"
        expect(methods).to.be.not.empty;
        expect(fd.root.variables).to.not.have.property("test_variable");
        newVar=fd.root.addVariable('test_variable',type,[dim]);
        fd.dataMode();
        newVar.writeSlice(0, 4,methods[0].prototype?new methods[0](values):methods[0](values))
        const result=fd.root.variables.test_variable.readSlice(0,4);
        expect(Array.from(fd.root.variables.test_variable.readSlice(0,4))).to.deep.almost.equal(values);
        try{
          fd.close();
        } catch (ignore) {
          console.log(`Got ${ignore.message} during file closing, ingored`)
        }
        fd=new netcdf4.File(path,'r');
        expect(fd.root.variables).to.have.property("test_variable");
        expect(Array.from(fd.root.variables.test_variable.readSlice(0,4))).to.deep.almost.equal(values);
      }
    )
    it(`should read/write strided slice ${type} variable (${file}) `,
      function() {
        let [fd,path]=file==='netcdf3'?[fileold,tempFileOldName]:[filenew,tempFileNewName];
        if (file==='netcdf3') {
          try{
            fd.close();
          } catch (ignore) {
            console.log(`Got ${ignore.message} during file closing, ingored`)
          }
          fd=new netcdf4.File(path,'c!','classic')
          fd.root.addDimension("dim1",75)
        }
        const dim=file==='netcdf3'?"dim1":"recNum"
        expect(methods).to.be.not.empty;
        expect(fd.root.variables).to.not.have.property("test_variable");
        newVar=fd.root.addVariable('test_variable',type,[dim]);
        fd.dataMode();
        newVar.writeStridedSlice(0, 2,2,methods[0].prototype?new methods[0]([values[0],values[2]]):methods[0]([values[0],values[2]]))
        expect(Array.from(fd.root.variables.test_variable.readStridedSlice(0,2,2))).to.deep.almost.equal([values[0],values[2]]);
        try{
          fd.close();
        } catch (ignore) {
          console.log(`Got ${ignore.message} during file closing, ingored`)
        }
        fd=new netcdf4.File(path,'r');
        expect(fd.root.variables).to.have.property("test_variable");
        expect(Array.from(fd.root.variables.test_variable.readStridedSlice(0,2,2))).to.deep.almost.equal([values[0],values[2]]);
      }
    )    
  }

  const testSuiteOld=[
    ['byte',10,[10,20,30,40],127],
    ['short',1024,[20,512,333,1024],32767],
    ['int',100000,[0,-200,3000,555666],32767],
    ['float',153.2,[-12,33,55.5,106.2],-555.555],
    ['double',153.2,[-12,33,55.5,106.2],-555.555],
    ['ubyte',10,[10,20,30,40],127],
    ['ushort',1024,[20,512,333,1024],127],
    ['uint',100000,[0,200,3000,555666],127],
    ['string',"Test value",["111","222","333","444"],"fill"]
  ];
  console.log(process.versions.node.split(".")[0])
  if (process.versions.node.split(".")[0]>=10) {
    testSuiteOld.push(['uint64',1024,[20,512555,333,77788889].map(v=>BigInt(v)),BigInt(100)]);
    testSuiteOld.push(['int64',100000,[0,200,3000,555666].map(v=>BigInt(v)),BigInt(100)]);
  };
  testSuiteOld.forEach(v=>testFunc('hdf5',v[0],v[1],v[2],v[3]));
  testSuiteOld.filter(v=>['ubyte','ushort','uint','string','int64','uint64'].indexOf(v[0])===-1).forEach(v=>testFunc('netcdf3',v[0],v[1],v[2],v[3]));
*/

});
