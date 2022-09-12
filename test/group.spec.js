const chai = require("chai");
const expect = chai.expect;
const chaiAsPromised = require('chai-as-promised');
const chaiAlmost=require("chai-almost");
chai.use(chaiAsPromised);
chai.use(chaiAlmost(0.0001));

const netcdf4 = require("..");
const { join } = require("path");

const fixture = join(__dirname, "test_hgroups.nc");
const fixture1 = join(__dirname, "testrh.nc");

const {newFile,closeAll,arrTypes}=require("./utils");

describe("Group", function () {
  let file;


  beforeEach(async function () {
    file = await newFile(fixture);
  });

  afterEach(async function () {
    await closeAll();
  });

  it("should read name", async  function () {
    const realname=await expect(file.root.getName()).to.be.fulfilled;
    expect(realname,'/');
  });

  it("should read group path", async function () {
    const realname=await expect(file.root.getPath()).to.be.fulfilled;
    expect(realname).to.equal("/");
  });

  it("has custom inspection", function () {
    expect(file.root.inspect()).eq("[Group /]");
  });

  it("should read list of subgroups", async function () {
    // const subgroups = await expect(file.root.getSubgroups()).to.be.fulfilled;
    const subgroups = await file.root.getSubgroups();
    expect(subgroups).to.have.property(
      "mozaic_flight_2012030319051051_descent"
    );
    expect(subgroups).to.have.property(
      "mozaic_flight_2012030319051051_descent"
    );
    expect(subgroups).to.have.property(
      "mozaic_flight_2012030321335035_descent"
    );
    expect(subgroups).to.have.property("mozaic_flight_2012030403540535_ascent");
    expect(subgroups).to.have.property(
      "mozaic_flight_2012030403540535_descent"
    );
    expect(subgroups).to.have.property("mozaic_flight_2012030412545335_ascent");
    expect(subgroups).to.have.property("mozaic_flight_2012030419144751_ascent");
  });


  it("should read name/fullname/subgroup of subgroups", async function () {
    const subgroups = await expect(file.root.getSubgroups()).to.be.fulfilled;
    expect(subgroups.mozaic_flight_2012030419144751_ascent.inspect(),'[Group mozaic_flight_2012030419144751_ascent]');
    await expect(subgroups.mozaic_flight_2012030419144751_ascent.getName()).become('mozaic_flight_2012030419144751_ascent');
    await expect(subgroups.mozaic_flight_2012030419144751_ascent.getPath()).become('/mozaic_flight_2012030419144751_ascent');
  });

  it("should get subgroup", async function () {
    const subgroup = await expect(file.root.getSubgroup("mozaic_flight_2012030419144751_ascent")).to.be.fulfilled;
    expect(subgroup.inspect(),'[Group mozaic_flight_2012030419144751_ascent]');
    await expect(subgroup.getName()).become('mozaic_flight_2012030419144751_ascent');
    await expect(subgroup.getPath()).become('/mozaic_flight_2012030419144751_ascent');
  });

  it("getSubgroup(..) should throw with unexisting subgroup", async function () {
    const subgroup = await expect(file.root.getSubgroup("no-exist")).to.rejectedWith("NetCDF4: Bad or missing group \"no-exist\"");
  });

  it("getSubgroup(..) should throw when parameters missing", async function () {
    const subgroup = await expect(file.root.getSubgroup()).to.rejectedWith("Expected subgroup name");
  });


  it("should rename groups", async function () {
    var subgroups = await expect(file.root.getSubgroups()).to.be.fulfilled;
    var subgroup = subgroups.mozaic_flight_2012030419144751_ascent;
    await expect(subgroup.setName('new_mozaic_flight')).become('new_mozaic_flight');
    await expect(subgroup.getName()).become('new_mozaic_flight');
    await expect(subgroup.getPath()).become('/new_mozaic_flight');
    await file.close();
    file = await netcdf4.open(file.name, "r");
    var subgroups = await expect(file.root.getSubgroups()).to.be.fulfilled;
    var subgroup = subgroups.new_mozaic_flight;
    await expect(subgroup.getName()).become('new_mozaic_flight');
    await expect(subgroup.getPath()).become('/new_mozaic_flight');

  });

  it("should create new group",async function() {
    var newGroup=await expect(file.root.addSubgroup("new_group")).be.fulfilled;
    await expect(newGroup.getName()).become('new_group');
    await expect(newGroup.getPath()).become('/new_group');
    await expect(file.root.getSubgroups()).eventually.to.have.property("new_group");
    var newSubGroup=await expect(newGroup.addSubgroup("new_nested_group")).be.fulfilled;
    await expect(newSubGroup.getName()).become('new_nested_group');
    await expect(newSubGroup.getPath()).become('/new_group/new_nested_group');
    await expect(newGroup.getSubgroups()).eventually.to.have.property("new_nested_group");
    await file.close();
    file = await new netcdf4.open(file.name, "r");
    var subgroup = (await file.root.getSubgroups()).new_group;
    await expect(subgroup.getName()).become('new_group');
    await expect(subgroup.getPath()).become('/new_group');
    await expect(subgroup.getSubgroups()).eventually.to.have.property("new_nested_group");

  })

  it("should read list of dimensions", async function () {
    await expect(file.root.getDimensions()).eventually.to.have.property("recNum");
  });

  it("should add new dimension",async function() {
    await expect(file.root.getDimensions()).eventually.to.not.have.property("new_dim");
    await expect(file.root.addDimension("new_dim",10)).eventually.to.be.deep.eq({"new_dim":10});
    await expect(file.root.getDimensions()).eventually.to.have.property("new_dim").to.be.deep.eq(10);
    await file.close();
    file = await netcdf4.open(file.name, "r");
    await expect(file.root.getDimensions()).eventually.to.have.property("new_dim").to.be.deep.eq(10);
  });

  it("should add new unlimited dimension",async function() {
    await expect(file.root.getDimensions()).eventually.to.not.have.property("new_dim");
    await expect(file.root.getDimensions(true)).eventually.to.not.have.property("new_dim");
    await expect(file.root.addDimension("new_dim",'unlimited')).eventually.to.be.deep.eq({"new_dim":"unlimited"});
    await expect(file.root.getDimensions()).eventually.to.have.property("new_dim").to.be.deep.eq("unlimited");
    await expect(file.root.getDimensions(true)).eventually.to.have.property("new_dim").to.be.deep.eq("unlimited");
    await file.close();
    file = await netcdf4.open(file.name, "r");
    await expect(file.root.getDimensions()).eventually.to.have.property("new_dim").to.be.deep.eq("unlimited");
    await expect(file.root.getDimensions(true)).eventually.to.have.property("new_dim").to.be.deep.eq("unlimited");
  });

  it("should read list of variables", async function () {
    await expect(file.root.getVariables()).eventually.to.have.property("UTC_time");
  });

  it("should create new variable ", async function () {
    await expect(file.root.getVariables()).eventually.to.not.have.property("test_variable");
    var newVar=await expect(file.root.addVariable('test_variable','byte',['recNum'])).be.fulfilled;
    expect(newVar.inspect(),'[Variable test_variable, type byte, 1 dimension(s)]');
    await expect(file.root.getVariables()).eventually.to.have.property("test_variable");
    var vars=await file.root.getVariables();
    expect(vars.test_variable.inspect(),'[Variable test_variable, type byte, 1 dimension(s)]');
    await file.close();
    file = await netcdf4.open(file.name, "r");
    await expect(file.root.getVariables()).eventually.to.have.property("test_variable");
    vars=await file.root.getVariables();
    expect(vars.test_variable.inspect(),'[Variable test_variable, type byte, 1 dimension(s)]');
  });

  it("should read list of attributes", async function () {
    const attributes =
      await expect(
        file.root.getSubgroup("mozaic_flight_2012030419144751_ascent").then(group=>group.getAttributes()))
        .to.be.fulfilled;
    console.log(attributes);
    expect(attributes).to.have.property("airport_dep");
    expect(attributes).to.have.property("flight");
    expect(attributes).to.have.property("level");
    expect(attributes).to.have.property("airport_arr");
    expect(attributes).to.have.property("mission");
    expect(attributes).to.have.property("time_dep");
    expect(attributes).to.have.property("aircraft");
    expect(attributes).to.have.property("link");
    expect(attributes).to.have.property("phase");
    expect(attributes).to.have.property("time_arr");
  });

  
  const testAddAttr=(fileType,type,value,values)=>{

    console.log(arrTypes[type]);
    [
      [arrTypes[type][1](value)," "],
      [values=arrTypes[type][0].prototype?new arrTypes[type][0](values):arrTypes[type][0](values)," array "]
    ].forEach(([value,scalar])=>{
      it(`[${fileType}] should add attribute${scalar}type ${type}`,async function() {
        console.log(`[${fileType}] should add attribute${scalar}type ${type}`);
        let file=await newFile(fileType==='hdf5'?fixture:fixture1,fileType==='hdf5'?"w":"c");
        if (fileType==='netcdf3') {
          await file.dataMode();
        }
        await expect(file.root.getAttributes()).eventually.to.not.have.property("root_attr_prop");
        const attr=await expect(file.root.addAttribute("root_attr_prop",type,value)).to.be.fulfilled;
        console.log(attr);
        expect(attr).deep.almost.equal({"root_attr_prop":{"type":type,"value":value}});
        expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop");
        await file.close();
        file = await netcdf4.open(file.name, "r");
        await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop");
        await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop").to.deep.property("value").to.deep.almost.equal(value);
        await expect(file.root.getAttributes(true)).eventually.to.have.property("root_attr_prop").to.deep.almost.equal({"type":type,"value":value});
      });
    });

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
  if (process.versions.node.split(".")[0]>=10) {
    testSuiteOld.push(['uint64',1024,[20,512555,333,77788889].map(v=>BigInt(v)),BigInt(100)]);
    testSuiteOld.push(['int64',100000,[0,200,3000,555666].map(v=>BigInt(v)),BigInt(100)]);
  };
  testSuiteOld
  .forEach(v=>testAddAttr('hdf5',v[0],v[1],v[2]));
  testSuiteOld
  .filter(v=>['ubyte','ushort','uint','int64','uint64','string'].indexOf(v[0])===-1)
  .forEach(v=>testAddAttr('netcdf3',v[0],v[0],v[1],v[2]));



  // it("should add attribute",async function() {
  //   await expect(file.root.getAttributes()).eventually.to.not.have.property("root_attr_prop");
  //   const attr = await expect(file.root.addAttribute("root_attr_prop","string","root attr property")).to.be.fulfilled;
  //   console.log(attr);
  //   await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop");
  //   await file.close();
  //   file = await netcdf4.open(tempFileName, "r");
  //   await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop");
  //   await expect(file.root.getAttributes()).eventually.to.have.property("root_attr_prop").to.deep.property("value").to.deep.equal("root attr property");
  //   await expect(file.root.getAttributes(true)).eventually.to.have.property("root_attr_prop").to.deep.equal({"type":"string","value":"root attr property"});
  // })  


});
